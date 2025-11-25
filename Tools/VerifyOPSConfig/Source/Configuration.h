/*

*/

#pragma once

#include <iostream>
#include <sstream>

#include "tinyxml2.h"

	class Configuration
	{
	private:
		tinyxml2::XMLDocument doc;
        tinyxml2::XMLNode *rootNode{nullptr};
        tinyxml2::XMLNode *currentNode{nullptr};
        tinyxml2::XMLError parseResult;
		std::string xmlString;
		std::string parseString;
	public:
		Configuration(std::istream& is_, std::string topNode_ = "")
		{
			std::istream& is(is_);
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

		Configuration(std::string filename, std::string topNode_ = "") 
		{
            parseResult = doc.LoadFile(filename.c_str());
            if (parseResult != tinyxml2::XML_SUCCESS) {
                /// TODO
            }
			if (topNode_ != "") {
				rootNode = currentNode = doc.FirstChildElement(topNode_.c_str());
			} else {
				rootNode = currentNode = &doc;
			}
		}

		bool SaveToFile(std::string /*filename*/)
		{
			return false; ///TODO currentNode.writeToFile(filename.c_str()) == opsXML::eXMLErrorNone;
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

		std::string getNodeName(int i = 0)
		{
			tinyxml2::XMLElement* el = getChildElement(currentNode, nullptr, (size_t)i);
			if (el != nullptr) {
				return el->Name();
			}
			return "";
		}

		bool updateString(std::string name, std::string value, int i = 0)
		{
			tinyxml2::XMLElement* el = getChildElement(currentNode, name.c_str(), (size_t)i);
			if (el != nullptr) {
				tinyxml2::XMLNode* node = el->FirstChild();
				if (node != nullptr) {
					tinyxml2::XMLText* t = node->ToText();
					if (t != nullptr) {
						node->SetValue(value.c_str());
						return true;
					}
				}
			}
			return false;
		}

		size_t countAttributes(tinyxml2::XMLNode *node)
		{
			size_t count = 0;
			tinyxml2::XMLElement* el = node->ToElement();
			if (el != nullptr) {
				const tinyxml2::XMLAttribute* at2 = el->FirstAttribute();
				while (at2 != nullptr) {
					count++;
					at2 = at2->Next();
				}
			}
			return count;
		}

		int numAttributes()
		{
			return (int)countAttributes(currentNode);
		}

		const tinyxml2::XMLAttribute *getAttribute(tinyxml2::XMLNode *node, size_t idx = 0)
		{
			const tinyxml2::XMLAttribute* at2 = nullptr;
			tinyxml2::XMLElement* el = node->ToElement();
			if (el != nullptr) {
				size_t count = 0;
				at2 = el->FirstAttribute();
				while ((at2 != nullptr) && (count < idx)) {
					count++;
					at2 = at2->Next();
				}
			}
			return at2;
		}

		std::string getAttributeName(int i = 0)
		{
			const tinyxml2::XMLAttribute* at = getAttribute(currentNode, i);
			if (at != nullptr) {
				return at->Name();
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

		uint32_t parseInt(std::string s, uint32_t defaultValue)
		{
			uint32_t value = 0;
			try {
				std::stringstream ss(s);
				ss >> value;
				return value;
			}
			catch (...) {
				return defaultValue;
			}
		}

		uint64_t parseInt64(std::string s, uint64_t defaultValue)
		{
			uint64_t value = 0;
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
