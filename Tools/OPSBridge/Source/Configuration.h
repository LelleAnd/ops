/**
*
* Copyright (C) 2010-2012 Saab Dynamics AB
*   author Lennart Andersson <nnnn@saabgroup.com>
*
* Copyright (C) 2018-2022 Lennart Andersson.
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

#include "tinyxml2.h"

namespace opsbridge {

	class Configuration
	{
	private:
		tinyxml2::XMLDocument doc;
        tinyxml2::XMLNode *rootNode{nullptr};
        tinyxml2::XMLNode *currentNode{nullptr};
        tinyxml2::XMLError parseResult;
		std::istream& is;
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
            parseResult = doc.Parse(xmlString.c_str());
            if (parseResult != tinyxml2::XML_SUCCESS) {
                /// TODO
            }
			if (topNode_ != "") {
				rootNode = currentNode = doc.FirstChildElement(topNode_.c_str());
			} else {
				rootNode = currentNode = &doc;
			}
		}

		std::string getParseResult()
		{
			if (parseResult == tinyxml2::XML_SUCCESS) { return ""; }
			return doc.ErrorStr();
		}

		tinyxml2::XMLElement* getChildElement(tinyxml2::XMLNode* el, const char* name, size_t idx = 0)
		{
			size_t count = 0;
			tinyxml2::XMLElement* el2 = el->FirstChildElement(name);
			while ((el2 != nullptr) && (count < idx)) {
				count++;
				el2 = el2->NextSiblingElement(name);
			}
			return el2;
		}

		bool enter(std::string name, int i = 0) 
		{
			tinyxml2::XMLElement* newNode = getChildElement(currentNode, name.c_str(), (size_t)i);
			if (newNode == nullptr) { return false; }
			currentNode = newNode;
			return true;
		}

		void exit()
		{
			currentNode = currentNode->Parent()->ToElement();
		}

		void root()
		{
			currentNode = rootNode;
		}

		size_t countChildElements(tinyxml2::XMLNode* el, const char* name)
		{
			size_t count = 0;
			tinyxml2::XMLElement* el2 = el->FirstChildElement(name);
			while (el2 != nullptr) {
				count++;
				el2 = el2->NextSiblingElement(name);
			}
			return count;
		}

		int numEntries(std::string name = "")
		{
			if (name == "") { return (int)countChildElements(currentNode, nullptr); }
			return (int)countChildElements(currentNode, name.c_str());
		}

		~Configuration(){};

		std::string getString(std::string name, int i = 0)
		{
			tinyxml2::XMLElement* el = getChildElement(currentNode, name.c_str(), (size_t)i);
			if (el != nullptr) {
				tinyxml2::XMLNode* node = el->FirstChild();
				if (node != nullptr) {
					tinyxml2::XMLText* t = node->ToText();
					if (t != nullptr) {
						return t->Value();
					}
				}
			}
			return "";
		}

		std::string getAttribute(std::string name, std::string defaultValue = "")
		{
			tinyxml2::XMLElement* el = currentNode->ToElement();
			if (el != nullptr) {
				const tinyxml2::XMLAttribute* at = el->FindAttribute(name.c_str());
				if (at != nullptr) {
					return at->Value();
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
