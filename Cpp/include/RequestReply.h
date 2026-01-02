/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2020-2025 Lennart Andersson.
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

#ifndef ops_RequestReplyH
#define ops_RequestReplyH

#include <memory>
#include <sstream>
#include <chrono>

#include "OPSTypeDefs.h"
#include "Topic.h"
#include "Subscriber.h"
#include "Publisher.h"
#include "KeyFilterQoSPolicy.h"
#include "TimeHelper.h"

namespace ops
{

	template<class ReqType, class RepType>
    class RequestReply
    {
    public:
		RequestReply(Topic reqTopic, Topic repTopic, ObjectKey_T key_) : keyFilter(key_), key(key_)
		{
			pub = std::make_unique<Publisher>(reqTopic);
			sub = std::make_unique<Subscriber>(repTopic);
			sub->addFilterQoSPolicy(&keyFilter);
			sub->start();
		}

		[[deprecated("Deprecated. Replaced by request() taking chrono duration")]]
		RepType* request(ReqType* req, int timeout)
		{
			return request(req, std::chrono::milliseconds(timeout));
		}
	    RepType* request(ReqType* req, const std::chrono::milliseconds& timeout)
		{
			reqInt++;
			req->setKey(key);
			std::stringstream ss;
			ss << key << reqInt;
			req->requestId = ss.str();
			
			ops_clock::time_point requestLimit = ops_clock::now() + timeout;
			sub->getDataReference();
			pub->writeOPSObject(req);
			while (ops_clock::now() < requestLimit) {
				std::chrono::milliseconds left = std::chrono::duration_cast<std::chrono::milliseconds>(requestLimit - ops_clock::now());
				if (sub->waitForNewData(left)) {
					if (dynamic_cast<RepType*>(sub->getMessage()->getData())->requestId == req->requestId) {
						return dynamic_cast<RepType*>(sub->getMessage()->getData()->clone());
					}
				}
			}
			return nullptr;
		}
	private:
        std::unique_ptr<Subscriber> sub;
        std::unique_ptr<Publisher> pub;
		KeyFilterQoSPolicy keyFilter;
		ObjectKey_T key;
		int reqInt{ 0 };
    };
}
#endif
