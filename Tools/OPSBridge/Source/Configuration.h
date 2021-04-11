/**
*
* Copyright (C) 2010-2012 Saab Dynamics AB
*   author Lennart Andersson <nnnn@saabgroup.com>
*
* Copyright (C) 2018-2019 Lennart Andersson.
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

#include <iostream>
#include <sstream>

#include "xml/xmlParser.h"

namespace opsbridge {

	class Configuration
	{
	private:
		std::istream& is;
		opsXML::XMLNode rootNode, currentNode;
		opsXML::XMLResults parseResult;
		std::string xmlString;
		std::string parseString;
	public:
		Configuration(std::istream& is_, std::string topNode_) : is(is_)
		{
			std::string tmp;
			is >> tmp;
			while(!is.eof()) {
				xmlString += tmp + " ";
				is >> tmp;			
			}
			rootNode = currentNode = opsXML::XMLNode::parseString(xmlString.c_str(), topNode_.c_str(), &parseResult);
		}

		std::string getParseResult()
		{
			if (parseResult.error == opsXML::eXMLErrorNone) return "";
			char tmp[256];
#ifdef _WIN32
			sprintf_s(tmp, sizeof(tmp), "Error parsing XML string. Position: %d", parseResult.nColumn);
#else
			sprintf(tmp, "Error parsing XML string. Position: %d", parseResult.nColumn);
#endif
			return tmp;
		}

		bool enter(std::string name, int i = 0) 
		{
			opsXML::XMLNode newNode = currentNode.getChildNode(name.c_str(), i);
			if (newNode.isEmpty()) return false;
			currentNode = newNode;
			return true;
		}

		void exit()
		{
			currentNode = currentNode.getParentNode();
		}

		void root()
		{
			currentNode = rootNode;
		}

		int numEntries(std::string name = "")
		{
			if (name == "") return currentNode.nChildNode();
			return currentNode.nChildNode(name.c_str());
		}

		~Configuration(){};

		std::string getString(std::string name, int i = 0)
		{
			if(!currentNode.getChildNode(name.c_str(), i).isEmpty()) {
				if(currentNode.getChildNode(name.c_str(), i).getText() != nullptr) {
					std::string s(currentNode.getChildNode(name.c_str(), i).getText());
					return s;
				}
			}			
			return "";
		}

		std::string getAttribute(std::string name, std::string defaultValue = "")
		{
			for (int i = 0; i < currentNode.nAttribute(); i++) {
				if (currentNode.getAttributeName(i) == name) {
					return currentNode.getAttributeValue(i);
				}
			}
			return defaultValue;
		}

		int32_t parseInt(std::string s, int32_t defaultValue)
		{
			int32_t value = defaultValue;
			try {
				std::stringstream ss(s);
				ss >> value;
				return value;
			}
			catch (...) {
				return defaultValue;
			}
		}

		int64_t parseInt64(std::string s, int64_t defaultValue)
		{
			int64_t value = defaultValue;
			try {
				std::stringstream ss(s);
				ss >> value;
				return value;
			}
			catch (...) {
				return defaultValue;
			}
		}
	};

}
