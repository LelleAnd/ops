/*

*/

#pragma once

#include <iostream>
#include <sstream>

#include "xml/xmlParser.h"

	class Configuration
	{
	private:
		opsXML::XMLNode rootNode, currentNode;
		opsXML::XMLResults parseResult;
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
			if (topNode_ != "") {
				rootNode = currentNode = opsXML::XMLNode::parseString(xmlString.c_str(), topNode_.c_str(), &parseResult);
			} else {
				rootNode = currentNode = opsXML::XMLNode::parseString(xmlString.c_str(), 0, &parseResult);
			}
		}

		Configuration(std::string filename, std::string topNode_ = "") 
		{
			if (topNode_ != "") {
				rootNode = currentNode = opsXML::XMLNode::parseFile(filename.c_str(), topNode_.c_str(), &parseResult);
			} else {
				rootNode = currentNode = opsXML::XMLNode::parseFile(filename.c_str(), 0, &parseResult);
			}
		}

		bool SaveToFile(std::string filename)
		{
			return currentNode.writeToFile(filename.c_str()) == opsXML::eXMLErrorNone;
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
				if(currentNode.getChildNode(name.c_str(), i).getText() != NULL) {
					std::string s(currentNode.getChildNode(name.c_str(), i).getText());
					return s;
				}
			}			
			return "";
		}

		std::string getNodeName(int i = 0)
		{
			return currentNode.getChildNode(i).getName();
		}

		bool updateString(std::string name, std::string value, int i = 0)
		{
			if(!currentNode.getChildNode(name.c_str(), i).isEmpty()) {
				if(currentNode.getChildNode(name.c_str(), i).getText() != NULL) {
					currentNode.getChildNode(name.c_str(), i).updateText(value.c_str());
					return true;
				}
			}			
			return false;
		}

		int numAttributes()
		{
			return currentNode.nAttribute();
		}

		std::string getAttributeName(int i = 0)
		{
			return currentNode.getAttributeName(i);
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
