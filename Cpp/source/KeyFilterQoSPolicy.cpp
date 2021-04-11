/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2021 Lennart Andersson.
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

#include "OPSTypeDefs.h"
#include "KeyFilterQoSPolicy.h"
#include "OPSObject.h"
namespace ops
{
	KeyFilterQoSPolicy::KeyFilterQoSPolicy() noexcept
	{
	}
	KeyFilterQoSPolicy::KeyFilterQoSPolicy(ObjectKey_T const keyString)
	{
		m_keyStrings.push_back(keyString);
	}
	KeyFilterQoSPolicy::KeyFilterQoSPolicy(std::vector<ObjectKey_T> const keyStrings)
	{
		m_keyStrings = keyStrings;
	}
	void KeyFilterQoSPolicy::setKeys(std::vector<ObjectKey_T> const keyStrings)
	{
		const SafeLock lock(*this);

		m_keyStrings = keyStrings;
	}
	void KeyFilterQoSPolicy::setKey(ObjectKey_T const key)
	{
		const SafeLock lock(*this);

		m_keyStrings.clear();
		m_keyStrings.push_back(key);
	}
	std::vector<ObjectKey_T> KeyFilterQoSPolicy::getKeys()
	{
		const SafeLock lock(*this);

		return m_keyStrings;
	}
    
	KeyFilterQoSPolicy::~KeyFilterQoSPolicy()
    {
    }
    
    bool KeyFilterQoSPolicy::applyFilter(const OPSObject* const o)
	{
		const SafeLock lock(*this);

		// An empty key filter is the same as no filter
		if (m_keyStrings.size() == 0) { return true; }

		for (const auto& x : m_keyStrings) {
			if(o->getKey() == x) {
				return true;	// match, so keep this object
			}
		}
		return false;	// no match, skip this object
    }
}
