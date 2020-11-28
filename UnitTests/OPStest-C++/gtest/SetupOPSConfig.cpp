/**
*
* Copyright (C) 2018-2020 Lennart Andersson.
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

#include <sstream>
#include "gtest/gtest.h"

#include "Participant.h"
#include "OPSConfigRepository.h"

#include "SetupOPSConfig.h"

using namespace ops;

SetupOPSConfig::SetupOPSConfig()
{
	InternalSetup();
}

void SetupOPSConfig::InternalSetup()
{
	std::string content(
		"<?xml version=\"1.0\" encoding=\"UTF - 8\"?>"
		"<!--"
		"	Description:"
		"-->"
		"<root>"
		"	<ops_config type = \"DefaultOPSConfigImpl\">"
		"		<domains>"
		"			<element type = \"Domain\">"
		"				<domainID>GTestDomain</domainID>"
		"				<domainAddress>234.5.6.8</domainAddress>"
		"				<localInterface>127.0.0.1</localInterface>"
        "               <resendNum>5</resendNum>"
        "               <registerTimeMs>100</registerTimeMs>"
		"				<topics>"
		"					<element type = \"Topic\">"
		"						<name>TestTopic</name>"
		"						<dataType>GTest.TestData</dataType>"
		"						<transport>tcp</transport>"
		"						<address>127.0.0.1</address>"
		"						<port>6689</port>"
		"					</element>"
        "					<element type = \"Topic\">"
        "						<name>TestTopic2</name>"
        "						<dataType>GTest.TestData</dataType>"
        "						<transport>tcp</transport>"
        "						<address>127.0.0.1</address>"
        "						<port>6690</port>"
        "					</element>"
        "				</topics>"
		"			</element>"
		"		</domains>"
		"	</ops_config>"
		"</root>"
	);
	std::istringstream iss(content);
	std::shared_ptr<OPSConfig> const config = OPSConfig::getConfig(iss);
	ASSERT_NE(config, nullptr);
	ASSERT_TRUE(OPSConfigRepository::Instance()->Add(config));
	ASSERT_FALSE(OPSConfigRepository::Instance()->Add(config));	// Duplicate test
}

SetupOPSConfig::~SetupOPSConfig() 
{
	OPSConfigRepository::Instance()->TotalClear();
}
