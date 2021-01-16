/**
*
* Copyright (C) 2018-2021 Lennart Andersson.
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

#include <map>

#include "Lockable.h"
#include "DataNotifier.h"
#include "Participant.h"
#include "Subscriber.h"
#include "Publisher.h"
#include "BasicError.h"
#include "KeyFilterQoSPolicy.h"

#include "DebugHandler.h"

#ifdef OPS_ENABLE_DEBUG_HANDLER

namespace ops {

	// Key used as filter when listening on the debug topic for DebugRequestResponseData messages
	static ObjectKey_T gKey;

	void DebugHandler::SetKey(const ObjectKey_T& key) noexcept
	{
		gKey = key;
	}

	class DebugHandler::InternalDebugListener : public DataListener
	{
	public:
		explicit InternalDebugListener(Participant& part) : _part(part) {}
		virtual ~InternalDebugListener() { remove(); }

        InternalDebugListener() = delete;
        InternalDebugListener(const InternalDebugListener& other) = delete;
        InternalDebugListener(InternalDebugListener&& other) = delete;
        InternalDebugListener& operator= (const InternalDebugListener& other) = delete;
        InternalDebugListener& operator= (InternalDebugListener&& other) = delete;

		// Used by application to set a handler for "Generic Command" (50)
		void SetAppCallback(DebugNotifyInterface* client)
		{
			const SafeLock lck(_mapLock);
			_appCallback = client;
		}

		// Register/Unregister with the debug handler
		void RegisterPub(DebugNotifyInterface* const client, const ObjectName_T& topicName)
		{
            const SafeLock lck(_mapLock);
            if (_pubMap.find(topicName) == _pubMap.end()) {
				_pubMap[topicName] = client;
			}
		}

		void UnregisterPub(const DebugNotifyInterface* const client, const ObjectName_T& topicName)
		{
            const SafeLock lck(_mapLock);
            const std::map<ObjectName_T, DebugNotifyInterface*>::iterator it = _pubMap.find(topicName);
            if (it == _pubMap.end()) { return; }
            if (it->second != client) { return; }
			_pubMap.erase(it);
		}

		void RegisterSub(DebugNotifyInterface* const client, const ObjectName_T& topicName)
		{
            const SafeLock lck(_mapLock);
            if (_subMap.find(topicName) == _subMap.end()) {
				_subMap[topicName] = client;
			}
		}

		void UnregisterSub(const DebugNotifyInterface* const client, const ObjectName_T& topicName)
		{
            const SafeLock lck(_mapLock);
            const std::map<ObjectName_T, DebugNotifyInterface*>::iterator it = _subMap.find(topicName);
            if (it == _subMap.end()) { return; }
            if (it->second != client) { return; }
			_subMap.erase(it);
		}

		void Start()
		{
			// We require the key to be set to enable the topics
            if (gKey == "") { return; }

			// Check if already started
            if (_sub != nullptr) { return; }

			setup();
		}

		void Stop()
		{
			remove();
		}

#if defined(_MSC_VER) && (_MSC_VER == 1900)
// VS2015 gives a warning on onNewData() "... previous versions of the compiler did not override when ...", which later versions don't do.
// We want it to override so skip warning
#pragma warning( disable : 4373 )
#endif

		virtual void onNewData(DataNotifier* const notifier) override
		{
			Subscriber* const sub = dynamic_cast<Subscriber*> (notifier);
			if (sub != nullptr) {
				opsidls::DebugRequestResponseData* const req = dynamic_cast<opsidls::DebugRequestResponseData*>(sub->getMessage()->getData());
				if (req != nullptr) {
                    if (req->Command == 0) { return; }  // We don't care about responses

					{
                        const SafeLock lck(_mapLock);

						switch (req->Entity) {
						case 0: // Debug
							onRequest(*req, _response);
							break;

						case 1: // Participant
                            if (req->getKey() == "*") { return; }
							// For now no response
							return;
                            break;

						case 2: // Publisher
                            if (req->getKey() == "*") { return; }
							// Don't respond on unknown topics
                            if (_pubMap.find(req->Name) == _pubMap.end()) { return; }
							_pubMap[req->Name]->onRequest(*req, _response);
							break;

						case 3: // Subscriber
                            if (req->getKey() == "*") { return; }
							// Don't respond on unknown topics
                            if (_subMap.find(req->Name) == _subMap.end()) { return; }
							_subMap[req->Name]->onRequest(*req, _response);
							break;
						default: 
							// Unknown entity, don't send a response
							return;
                            break;
						}
					}
					_response.Entity = req->Entity;
					_response.Name = req->Name;
					_response.Command = 0;	// Always a response

					if (_pub != nullptr) { _pub->writeOPSObject(&_response); }

					_response.Param3.clear();

				} else {
					BasicError err("DebugHandler", "onNewData", "Data could not be cast as expected.");
					_part.reportError(&err);
				}
			} else {
				BasicError err("DebugHandler", "onNewData", "Subscriber could not be cast as expected.");
				_part.reportError(&err);
			}
		}

	private:
		void setup()
		{
#ifdef OPS_C14_DETECTED
			_sub = std::make_unique<Subscriber>(_part.createDebugTopic());
#else
			_sub = std::unique_ptr<Subscriber>(new Subscriber(_part.createDebugTopic()));
#endif
			_sub->addDataListener(this);
			std::vector<ObjectKey_T> keyStrings;
			keyStrings.push_back(gKey);
			keyStrings.push_back("*");
			_sub->addFilterQoSPolicy(new KeyFilterQoSPolicy(keyStrings));
			_sub->start();
#ifdef OPS_C14_DETECTED
			_pub = std::make_unique<Publisher>(_part.createDebugTopic());
#else
			_pub = std::unique_ptr<Publisher>(new Publisher(_part.createDebugTopic()));
#endif
			_pub->setKey(gKey);
			_pub->start();
		}

		void remove() noexcept
		{
            _sub.reset();
            _pub.reset();
		}

		// Called with _mapLock held
		void onRequest(opsidls::DebugRequestResponseData& req, opsidls::DebugRequestResponseData& resp)
		{
			resp.Result1 = 0;
			switch (req.Command) {
			case 2: // List
				resp.Param1 = 0;
				if (req.Param1 == 1) {
					resp.Param3.push_back(gKey.c_str());
					resp.Result1 = 1;
				}
				if (req.Param1 == 2) {
					for (auto it = _pubMap.begin(); it != _pubMap.end(); ++it) {
						resp.Param3.push_back(it->first.c_str());
					}
					resp.Result1 = 2;
				}
				if (req.Param1 == 3) {
					for (auto it = _subMap.begin(); it != _subMap.end(); ++it) {
						resp.Param3.push_back(it->first.c_str());
					}
					resp.Result1 = 3;
				}
				break;

			case 50: // Generic command
				if (_appCallback != nullptr) {
					_appCallback->onRequest(req, resp);
					resp.Result1 = 50;
				}
				break;
			default:; 
				break;
			}
		}

		Participant& _part;
		std::unique_ptr<Subscriber> _sub;
		std::unique_ptr<Publisher> _pub;

		opsidls::DebugRequestResponseData _response;

		Lockable _mapLock;
		std::map<ObjectName_T, DebugNotifyInterface*> _pubMap;
		std::map<ObjectName_T, DebugNotifyInterface*> _subMap;
		DebugNotifyInterface* _appCallback{ nullptr };
	};

	DebugHandler::DebugHandler(Participant& part) :
#ifdef OPS_C14_DETECTED
        _pimpl(std::make_unique<InternalDebugListener>(part))
#else
        _pimpl(new InternalDebugListener(part))
#endif
	{
	}
	
	DebugHandler::~DebugHandler()
	{
	}

	void DebugHandler::Start()
	{
		_pimpl->Start();
	}

	void DebugHandler::Stop()
	{
		_pimpl->Stop();
	}

	// Used by application to set a handler for "Generic Command" (50)
	void DebugHandler::SetAppCallback(DebugNotifyInterface* const client)
	{
		_pimpl->SetAppCallback(client);
	}

	// Register/Unregister with the debug handler
	void DebugHandler::RegisterPub(DebugNotifyInterface* const client, const ObjectName_T& topicName)
	{
		_pimpl->RegisterPub(client, topicName);
	}
	
	void DebugHandler::UnregisterPub(const DebugNotifyInterface* const client, const ObjectName_T& topicName)
	{
		_pimpl->UnregisterPub(client, topicName);
	}

	void DebugHandler::RegisterSub(DebugNotifyInterface* const client, const ObjectName_T& topicName)
	{
		_pimpl->RegisterSub(client, topicName);
	}
	
	void DebugHandler::UnregisterSub(const DebugNotifyInterface* const client, const ObjectName_T& topicName)
	{
		_pimpl->UnregisterSub(client, topicName);
	}

}
#endif
