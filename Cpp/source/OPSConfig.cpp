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

#include <fstream>
#ifdef _WIN32
	#include <direct.h>
#endif

#include "OPSTypeDefs.h"
#include "OPSConfig.h"
#include "OPSConfigRepository.h"
#include "XMLArchiverIn.h"
#include "OPSObjectFactory.h"
#include "Participant.h"

namespace ops
{

#ifndef REPLACE_OPS_CONFIG
    std::shared_ptr<OPSConfig> OPSConfig::getConfig(FileName_T configFile)
	{
		std::ifstream inStream(configFile.c_str());
		if (inStream.is_open()) {
			return getConfig(inStream);
		} else {
#ifdef _WIN32
			char* buffer = _getcwd(nullptr, 0);
			if (buffer != nullptr) {
				ErrorMessage_T msg("Failed to open ");
				msg += configFile;
				msg += ". Working Dir: ";
				msg += buffer;
				Participant::getStaticErrorService()->report("OPSConfig", "getConfig", msg);
				free(buffer);
			}
#endif
		}
		return nullptr;
	}
#endif

    std::shared_ptr<OPSConfig> OPSConfig::getConfig()
	{
		static const std::shared_ptr<OPSConfig> theConfiguration = OPSConfigRepository::Instance()->getConfig();
		return theConfiguration;
	}

    std::shared_ptr<OPSConfig> OPSConfig::getConfig(std::istream& inStream)
	{
		XMLArchiverIn archiver(inStream, "root", OPSObjectFactory::getInstance());
		OPSConfig* const theConfig = nullptr;
		return std::shared_ptr<OPSConfig>((OPSConfig*)archiver.inout("ops_config", theConfig));
	}

	OPSConfig::~OPSConfig()
	{
		for (std::vector<Domain* >::iterator it = domains.begin(); it != domains.end(); ++it) {
			delete (*it);
		}
		domains.clear();
	}

}
