/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2021-2025 Lennart Andersson.
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

#ifndef ops_NotifierH
#define ops_NotifierH
#include <vector>
#include "Listener.h"
#include "Lockable.h"

namespace ops
{
    /// <summary>
    /// Class which in the conjunction with Listener forms an implementation of the
    /// observer GoF-pattern. Classes extending this class extends an interface to which
    /// Listeners can register their interest to be notified when new events are available.
    /// </summary>
    /// <typeparam name="ArgType"></typeparam>
    template<typename ArgType>
    class Notifier
    {
    private:
        ///Vector that holds pointers to the Listeners
        std::vector<Listener<ArgType>*> listeners;

        //
        Lockable mutex;

        /// 
        const bool lateArrivals;
        ArgType value{ };
        bool valueValid{ false };

    protected:
        ///Called by subclasses that wishes to notify its listeners of the arrival of new events.
        virtual void notifyNewEvent(ArgType arg)
        {
            if (lateArrivals) {
                value = arg;
                valueValid = true;
            }
            // Methods addListener(), removeListener() and calling of registered callback need to be protected.
            // This also ensures that when a client returns from removeListener(), he can't be called
            // anymore and there can't be an ongoing call in his callback.
            SafeLock lock(mutex);
            for (auto& x : listeners) {
                x->onNewEvent(this, arg);
            }
        }

    public:
        explicit Notifier(bool lateArrivals_ = false) : lateArrivals(lateArrivals_) {}

        virtual void addListener(Listener<ArgType>* listener)
        {
            SafeLock lock(mutex);
            listeners.push_back(listener);
            if (lateArrivals && valueValid) {
                listener->onNewEvent(this, value);
            }
        }

        virtual void removeListener(Listener<ArgType>* listener)
        {
            SafeLock lock(mutex);
            for (auto it = listeners.begin(); it != listeners.end(); ) {
                if (*it == listener) {
                    it = listeners.erase(it);
                } else {
                    ++it;
                }
            }
        }

        int getNrOfListeners()
        {
            SafeLock lock(mutex);
            return (int)listeners.size();
        }

        virtual ~Notifier()
        {
            SafeLock lock(mutex);
            listeners.clear();
        }
    };

    /// <summary>
    /// Class which in the conjunction with SingleListener forms an implementation of
    /// a callback mechanism. Classes extending this class extends an interface to which
    /// the SingleListener can register its interest to be notified with new data.
    /// </summary>
    /// <typeparam name="ArgType"></typeparam>
    template <typename ArgType>
    class SingleNotifier
    {
      private:
        SingleListener<ArgType>* listener{ nullptr };

      protected:
        /// Called by subclass to notify its listener of the arrival of new data.
        void notifyNewEvent(ArgType arg)
        {
            if (listener != nullptr) { listener->onNewEvent(this, arg); }
        }

      public:
        virtual ~SingleNotifier() = default;

        void connect(SingleListener<ArgType>* listener_)
        {
            this->listener = listener_;
        }
    };

} // namespace ops
#endif
