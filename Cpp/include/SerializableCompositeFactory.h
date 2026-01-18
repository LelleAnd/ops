/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
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

#ifndef ops_SerializableCompositeFactoryH
#define ops_SerializableCompositeFactoryH

#include <vector>

#include "SerializableFactory.h"

namespace ops
{

class SerializableCompositeFactory : public SerializableFactory
{

	std::vector<SerializableFactory*> childFactories;

public:
	// Removes the given object from the factory and ownership are returned to the caller.
	bool remove(SerializableFactory* o)
	{
		std::vector<SerializableFactory*>::iterator it = childFactories.begin();
		for(unsigned int i = 0; i < childFactories.size(); i++ )
		{
			if(childFactories[i] == o)
			{
				it += i;
				childFactories.erase(it);
				return true;
			}
		}
		return false;
	}

	// Adds the given object and takes ownership over it.
	void add(SerializableFactory* o)
	{
		if (o) childFactories.push_back(o);
	}

	virtual Serializable* create(CreateType_T type) override
	{
        Serializable* obj = nullptr;

		for(unsigned int i = 0; i < childFactories.size(); i++ )
		{
			obj = childFactories[i]->create(type);
			if(obj != nullptr)
			{
				return obj;
			}
		}
		return obj;
	}

	virtual ~SerializableCompositeFactory()
	{
		// Delete all objects that we still owns
		for(unsigned int i = 0; i < childFactories.size(); i++ )
		{
			delete childFactories[i];
		}
	}
};

}

#endif
