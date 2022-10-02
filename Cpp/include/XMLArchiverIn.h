/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2020-2022 Lennart Andersson.
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

#ifndef XMLArchiverInH
#define XMLArchiverInH

#include <iostream>
#include <sstream>
#include <string>
#include <stack>
#include <exception>

#include "OPSTypeDefs.h"
#include "ArchiverInOut.h"
#include "SerializableInheritingTypeFactory.h"
#include "tinyxml2.h"

namespace ops
{
    namespace exceptions
    {
        class XMLArchiverException : public std::exception
        {
        private:
			ExceptionMessage_T message;
        public:
            XMLArchiverException(ExceptionMessage_T m)
            {
				message = "XMLArchiverException: ";
				message += m;
            }
			const char* what() const noexcept override { return message.c_str(); }
		};
    }
    using namespace exceptions;

#ifdef USE_FIXED_LENGTH_STRINGS
#define NAME(x) x
#else
#define NAME(x) x.c_str()
#endif

    class XMLArchiverIn : public ArchiverInOut
    {
	private:
		tinyxml2::XMLDocument doc;
		std::string topNode;
		SerializableInheritingTypeFactory * factory;
		tinyxml2::XMLElement* currentNode{ nullptr };
        std::string xmlString;

		std::stack<tinyxml2::XMLElement*> _stack;

		void PushNode(tinyxml2::XMLElement* node)
		{
			_stack.push(node);
		}

		void PopNode(tinyxml2::XMLElement*& node)
		{
			if (_stack.size() == 0) { throw ops::ArchiverException("XMLArchiverIn: Mismatched Push/Pop"); }
			node = _stack.top();
			_stack.pop();
		}

		tinyxml2::XMLElement* getChildElement(tinyxml2::XMLElement* el, const char* name, size_t idx = 0)
		{
			size_t count = 0;
			tinyxml2::XMLElement* el2 = el->FirstChildElement(name);
			while ((el2 != nullptr) && (count < idx)) {
				count++;
				el2 = el2->NextSiblingElement(name);
			}
			return el2;
		}

	public:
		XMLArchiverIn(std::istream& is, std::string topNode_, SerializableInheritingTypeFactory* factory_): 
			topNode(topNode_), factory(factory_)
        {
			std::string tmp;
            while (!is.eof()) {
				is >> tmp;
				xmlString += tmp + " ";
            }
			reset();
        }

        ~XMLArchiverIn()
        {
        }

		// Reset to the start state of XMLArchiverIn() for cases where an exception has been raised
		// and the user want to read again
		void reset()
		{
			tinyxml2::XMLError err = doc.Parse(xmlString.c_str());
			if (err != tinyxml2::XML_SUCCESS) {
				///TODO
				std::string error(doc.ErrorStr());
				throw ops::ArchiverException(error.c_str());
			}
			currentNode = doc.FirstChildElement(topNode.c_str());
		}

		// Returns true if it's an output archiver
		virtual bool isOut() noexcept override { return false; }

		virtual void inout(InoutName_T name, bool& value) override;
		virtual void inout(InoutName_T name, char& value) override;
		virtual void inout(InoutName_T name, int& value) override;
		virtual void inout(InoutName_T name, int16_t& value) override;
		virtual void inout(InoutName_T name, int64_t& value) override;
		virtual void inout(InoutName_T name, float& value) override;
		virtual void inout(InoutName_T name, double& value) override;
		virtual void inout(InoutName_T name, std::string& value) override;

		virtual void inoutfixstring(InoutName_T name, char* value, int& size, int max_size, int idx) override
		{
			tinyxml2::XMLElement* el = getChildElement(currentNode, NAME(name), idx);
			if (el != nullptr) {
				size = 0;
				tinyxml2::XMLNode* node = el->FirstChild();
				if (node != nullptr) {
					tinyxml2::XMLText* t = node->ToText();
					if (t != nullptr) {
						std::string s(t->Value());
						int len = (int)s.size();
						if (len > max_size) throw ops::ArchiverException("Illegal size of fix string received. name: ", name);
						if (len > 0) memcpy(value, s.c_str(), len);
						size = len;
					}
				}
				value[size] = '\0';
			}
		}
		
		virtual void inout(InoutName_T name, char* buffer, int bufferSize) override
        {
			tinyxml2::XMLElement* el = currentNode->FirstChildElement(NAME(name));
			if (el != nullptr) {
				tinyxml2::XMLNode* node = el->FirstChild();
				if (node != nullptr) {
					tinyxml2::XMLText* t = node->ToText();
					if (t != nullptr) {
						std::string s(t->Value());
						std::istringstream is(s);

						// each char stored as dec number + " " 
						for (int i = 0; i < bufferSize; i++) {
							int tmp;
							if (is.eof()) throw ops::ArchiverException("Illegal size of buffer received. name: ", name);
							is >> tmp;
							if (is.fail()) throw ops::ArchiverException("Illegal size of buffer received. name: ", name);
							buffer[i] = (char)(tmp);
						}
					}
				}
			}
		}

        virtual Serializable* inout(InoutName_T, Serializable* value, int element) override
        {
			if (value != nullptr) { delete value; }
            value = nullptr;

			PushNode(currentNode);

			currentNode = getChildElement(currentNode, "element", element);
			if (currentNode != nullptr) {
				const char* types = currentNode->Attribute("type");
				if (types != nullptr) {
					value = factory->create(types);
					if (value != nullptr) {
						value->serialize(this);
					}
				}
			}

			PopNode(currentNode);

			return value;
        }

        virtual void inout(InoutName_T name, Serializable& value) override
        {
			tinyxml2::XMLElement* el = currentNode->FirstChildElement(NAME(name));
			if (el != nullptr) {
				PushNode(currentNode);
				currentNode = el;
				
				//	const TypeId_T types(currentNode.getAttribute("type"));
				//	///TODO check that type is correct
				const char* typp = currentNode->Attribute("type");
				if (typp != nullptr) {
					value.serialize(this);
				}

				PopNode(currentNode);
			}
		}

        virtual Serializable* inout(InoutName_T name, Serializable* value) override
        {
			if (value != nullptr) { delete value; }
            value = nullptr;

			tinyxml2::XMLElement* el = currentNode->FirstChildElement(NAME(name));
			if (el != nullptr) {
				PushNode(currentNode);
				currentNode = el;

				const char* types = currentNode->Attribute("type");
				if (types != nullptr) {
					value = factory->create(types);
					if (value != nullptr) {
						value->serialize(this);
					}
				}
				
				PopNode(currentNode);
			}
            return value;
        }

		virtual void inout(InoutName_T name, std::vector<bool>& value) override;
		virtual void inout(InoutName_T name, std::vector<char>& value) override;
		virtual void inout(InoutName_T name, std::vector<int>& value) override;
		virtual void inout(InoutName_T name, std::vector<int16_t>& value) override;
		virtual void inout(InoutName_T name, std::vector<int64_t>& value) override;
		virtual void inout(InoutName_T name, std::vector<float>& value) override;
		virtual void inout(InoutName_T name, std::vector<double>& value) override;
		virtual void inout(InoutName_T name, std::vector<std::string>& value) override;

		void inoutfixarr(InoutName_T name, bool* value, int numElements, int) override;
		void inoutfixarr(InoutName_T name, char* value, int numElements, int) override;
		void inoutfixarr(InoutName_T name, int* value, int numElements, int) override;
		void inoutfixarr(InoutName_T name, int16_t* value, int numElements, int) override;
		void inoutfixarr(InoutName_T name, int64_t* value, int numElements, int) override;
		void inoutfixarr(InoutName_T name, float* value, int numElements, int) override;
		void inoutfixarr(InoutName_T name, double* value, int numElements, int) override;
		void inoutfixarr(InoutName_T name, std::string* value, int numElements) override;

		int beginList(InoutName_T name, int) override;
		void endList(InoutName_T) override;

    };
}
#endif
