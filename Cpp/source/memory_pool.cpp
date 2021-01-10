/**
*
* Copyright (C) 2017-2019 Lennart Andersson.
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

#include <iostream>
#include "memory_pool.h"

namespace ops {
	namespace memory_pools {

		memory_pool_manager& memory_pool_manager::Instance()
		{
			static memory_pool_manager mpm;
			return mpm;
		}

		memory_pool_manager::memory_pool_manager() noexcept
		{
			_root.next = &_root;
			_root.prev = &_root;
		}

		void memory_pool_manager::setLogger(memory_pool_logger* const client) noexcept
		{
			_client = client;
		}

		void memory_pool_manager::Add(node<memory_pool_abs>& nd)
		{
			const SafeLock lck(_mtx);

			// last node = _root.prev
			nd.next = _root.prev->next;
			_root.prev->next = &nd;
			nd.prev = _root.prev;
			_root.prev = &nd;

			_numPools++;
		}

		void memory_pool_manager::Remove(node<memory_pool_abs>& nd)
		{
			const SafeLock lck(_mtx);

			nd.prev->next = nd.next;
			nd.next->prev = nd.prev;
			nd.prev = nullptr;
			nd.next = nullptr;

			_numPools--;
		}

		// Uses the link chain to print statistics from each memory_pool
		void memory_pool_manager::PrintStat(std::ostream& os, const bool skip_header)
		{
			if (!skip_header) {
				os <<
					std::endl << "Memory Pool Statistics" <<
					std::endl << "======================" << std::endl;
			}

			const SafeLock lck(_mtx);

			node<memory_pool_abs>* Ptr = _root.next;
			while (Ptr != &_root) {
				Ptr->owner->PrintStat(os);
				Ptr = Ptr->next;
			}
		}

		void memory_pool_manager::Log(const char* const message, std::exception& e)
		{
			if (_client != nullptr) { _client->Log(message, e); }
		}

		memory_pool_abs::memory_pool_abs() : _node(this)
		{
			memory_pool_manager::Instance().Add(_node);
		}

		memory_pool_abs::~memory_pool_abs()
		{
			memory_pool_manager::Instance().Remove(_node);
		}

		void memory_pool_abs::Log(const char* const message, std::exception& e)
		{
			memory_pool_manager::Instance().Log(message, e);
		}

	}
}
