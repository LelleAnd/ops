/**
*
* Copyright (C) 2026 Lennart Andersson.
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

#pragma once

#include <map>

#include "OPSTypeDefs.h"

namespace ops
{

	class TopicsCounter
	{
	public:
		using Count_t = int32_t;
		using Container_t = std::map<ObjectName_T, Count_t>;

		explicit TopicsCounter(bool doEraseOnZero = false) : eraseOnZero(doEraseOnZero) {}

		Count_t update(const ObjectName_T& name, bool add)
		{
			Count_t count = 0;
			const auto it = topics.find(name);
			if (it != topics.end()) {
				count = topics[name];
			}
			if (add) {
				++count;
			}
			else {
				--count;
			}
			if (eraseOnZero && (count == 0)) {
				topics.erase(it);
			}
			else {
				topics[name] = count;
			}
			return count;
		}

		const Container_t& peek() { return topics; }

	private:
		Container_t topics;
		bool eraseOnZero;
	};

}
