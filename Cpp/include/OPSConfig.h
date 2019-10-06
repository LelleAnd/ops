/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019 Lennart Andersson.
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

#ifndef ops_OPSConfig_h
#define ops_OPSConfig_h

#include <vector>

#include "OPSTypeDefs.h"
#include "Topic.h"
#include "Domain.h"

namespace ops
{
	class OPS_EXPORT OPSConfig : public OPSObject
	{
	public:
		static OPSConfig* getConfig();
		static OPSConfig* getConfig(FileName_T configFile);
		static OPSConfig* getConfig(std::istream& inStream);

		// Should be used with care, since the above methods may return a singleton
		virtual ~OPSConfig();

		virtual Domain* getDomain(ObjectName_T domainID) const
		{
			for(unsigned int i = 0; i < domains.size(); i++)
			{
				if(domains[i]->getDomainID() == domainID)
				{
					return domains[i];
				}
			}
			return nullptr;
		}

		void serialize(ArchiverInOut* archiver) override
		{
			OPSObject::serialize(archiver);
		
			archiver->inout<Domain>("domains", domains);
		}

		std::vector<Domain*> getDomains() const
		{
			return domains;
		}

        std::vector<Domain*>& getRefToDomains()
        {
            return domains;
        }

	private:
		std::vector<Domain* > domains;
	};
}
#endif
