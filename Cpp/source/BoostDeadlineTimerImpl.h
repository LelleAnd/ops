/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2021-2023 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef ops_BoostDeadlineTimerImpl_h
#define ops_BoostDeadlineTimerImpl_h

#include "chrono"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "Participant.h"
#include "BoostIOServiceImpl.h"

namespace ops
{

	class BoostDeadlineTimerImpl : public DeadlineTimer, Listener<int>
	{
    private:

        // Actual implementation is deferred to an inner class to be able to use a shared_ptr
        // without exposing the shared_ptr to the outside (to be backward compatible). 
        // The shared_ptr is a convenient way to handle the asynch callbacks that may come 
        // after our destructor has been called. When we start an asynch operation in the 
        // inner class, we give boost a shared_ptr to the instance so that the last one
        // of our destructor and the asynch callback will perform the actual delete of the
        // inner class instance.
        class impl;
        boost::shared_ptr<impl> pimpl_;

        // Listens on events from the inner class and forwards them to our user(s).
		void onNewEvent(Notifier<int>* sender, int message) override;

    public:
        explicit BoostDeadlineTimerImpl(boost::asio::io_service* boostIOService);
        virtual ~BoostDeadlineTimerImpl();

        void start(const std::chrono::milliseconds& timeout) override;
        virtual void start(int64_t timeoutMs) override;

		void cancel() override;
    };

    // -------------------------------------------

    class BoostDeadlineTimerImpl::impl : public Notifier<int>, public boost::enable_shared_from_this< BoostDeadlineTimerImpl::impl >
	{
		boost::asio::steady_timer deadlineTimer;
	public:
        explicit impl(boost::asio::io_service* boostIOService) : deadlineTimer(*boostIOService)
		{
		}

        virtual void start(const std::chrono::milliseconds& timeout)
        {
            deadlineTimer.cancel();
            deadlineTimer.expires_from_now(timeout);
            // Here we pass in a shared_ptr to our instance
            using namespace boost::placeholders;
            deadlineTimer.async_wait(boost::bind(&impl::asynchHandleDeadlineTimeout, shared_from_this(), boost::asio::placeholders::error));
        }

		virtual void cancel()
		{
			deadlineTimer.cancel();
		}

		void asynchHandleDeadlineTimeout(const boost::system::error_code& e)
		{
			if (e != boost::asio::error::operation_aborted)	{
				// Timer was not cancelled, take necessary action.
				notifyNewEvent(0);
            }
		}

		virtual ~impl()
		{
			impl::cancel();
		}
	};

    // -------------------------------------------

    BoostDeadlineTimerImpl::BoostDeadlineTimerImpl(boost::asio::io_service* boostIOService): 
        pimpl_(new impl(boostIOService))
    {
        pimpl_->addListener(this);
    }

    BoostDeadlineTimerImpl::~BoostDeadlineTimerImpl()
    {
        BoostDeadlineTimerImpl::cancel();
        // The removeListener() and the calling of our callback "onNewEvent()" are protected so when
        // we return from removeListener() we know that there can't be anyone in the callback.
        pimpl_->removeListener(this);
    }

    void BoostDeadlineTimerImpl::onNewEvent(Notifier<int>* , int message)
    {
        notifyNewEvent(message);    // Just forward the event
    }

    void BoostDeadlineTimerImpl::start(const std::chrono::milliseconds& timeout)
    {
        pimpl_->start(timeout);
    }

    void BoostDeadlineTimerImpl::start(int64_t timeout)
    {
        pimpl_->start(std::chrono::milliseconds(timeout));
    }

    void BoostDeadlineTimerImpl::cancel()
    {
        pimpl_->cancel();
    }

}

#endif
