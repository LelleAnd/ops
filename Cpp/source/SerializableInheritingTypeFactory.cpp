/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2019-2020 Lennart Andersson.
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
#include "SerializableInheritingTypeFactory.h"

namespace ops
{
	/**
	 * Tries to construct the most specialized object in the given typeString list
	 */
	Serializable* SerializableInheritingTypeFactory::create(const TypeId_T& typeString)
	{
		TypeId_T::size_type i = typeString.find_first_not_of(' ');  // Skip any leeding spaces
		TypeId_T::size_type j = typeString.find(' ', i);

		while (j != TypeId_T::npos) {
			Serializable* const serializable = SerializableCompositeFactory::create(typeString.substr(i, j - i));
			if (serializable != nullptr) {
				return serializable;
			}
			i = ++j;
			j = typeString.find(' ', j);
		}
		return SerializableCompositeFactory::create(typeString.substr(i, typeString.length()));
	}

}
