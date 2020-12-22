/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2020 Lennart Andersson.
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
#include "xml/xmlParser.h"

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
		std::istream& is;
		std::string topNode;
		SerializableInheritingTypeFactory * factory;
        opsXML::XMLNode currentNode;
        std::string xmlString;
        std::string parseString;
        std::stringstream ss;

		std::stack<opsXML::XMLNode> _stack;

		void PushNode(opsXML::XMLNode& node) 
		{
			_stack.push(node);
		}

		void PopNode(opsXML::XMLNode& node)
		{
			if (_stack.size() == 0) throw ops::ArchiverException("XMLArchiverIn: Mismatched Push/Pop");
			node = _stack.top();
			_stack.pop();
		}

		bool toBool(InoutName_T name, std::string s)
		{
			if (s.compare("true") == 0) return true;
			if (s.compare("false") == 0) return false;
			if (s.compare("TRUE") == 0) return true;
			if (s.compare("FALSE") == 0) return false;
			if (s.compare("True") == 0) return true;
			if (s.compare("False") == 0) return false;
			throw ops::ArchiverException("XMLArchiverIn: Illegal value for bool with name: ", name);
		}

    public:

		XMLArchiverIn(std::istream& is_, std::string topNode_, SerializableInheritingTypeFactory* factory_): 
			is(is_), topNode(topNode_), factory(factory_)
        {
            std::string tmp;
            is >> tmp;
            while (!is.eof())
            {
                xmlString += tmp + " ";
                is >> tmp;
            }
            currentNode = opsXML::XMLNode::parseString(xmlString.c_str(), topNode_.c_str());
        }

        ~XMLArchiverIn()
        {
        }

		// Reset to the start state of XMLArchiverIn() for cases where an exception has been raised
		// and the user want to read again
		void reset()
		{
			currentNode = opsXML::XMLNode::parseString(xmlString.c_str(), topNode.c_str());
		}

		// Returns true if it's an output archiver
		virtual bool isOut() noexcept override { return false; }

		virtual void inout(InoutName_T name, bool& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
                std::string s(currentNode.getChildNode(NAME(name)).getText());
				value = toBool(name, s);
            }
        }

        virtual void inout(InoutName_T name, char& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
				std::string s(currentNode.getChildNode(NAME(name)).getText());
                std::stringstream ss(s);

                int inVal;
                ss >> inVal;
                value = (char)inVal;
            }
        }

        virtual void inout(InoutName_T name, int& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
                std::string s(currentNode.getChildNode(NAME(name)).getText());
                std::stringstream ss(s);

                int inVal;
                ss >> inVal;
                value = inVal;
            }
        }

        virtual void inout(InoutName_T name, int16_t& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
                std::string s(currentNode.getChildNode(NAME(name)).getText());
				std::stringstream ss(s);

                int inVal;
                ss >> inVal;
                value = (int16_t)inVal;
            }
        }

        virtual void inout(InoutName_T name, int64_t& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
				std::string s(currentNode.getChildNode(NAME(name)).getText());
				std::stringstream ss(s);

                int64_t inVal;
                ss >> inVal;
                value = inVal;
            }
        }

        virtual void inout(InoutName_T name, float& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
				std::string s(currentNode.getChildNode(NAME(name)).getText());
				std::stringstream ss(s);

                float inVal;
                ss >> inVal;
                value = inVal;
            }
        }

        virtual void inout(InoutName_T name, double& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
				std::string s(currentNode.getChildNode(NAME(name)).getText());
				std::stringstream ss(s);

                double inVal;
                ss >> inVal;
                value = inVal;
            }
        }

        virtual void inout(InoutName_T name, std::string& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
                if (currentNode.getChildNode(NAME(name)).getText() != nullptr)
                {
					std::string s(currentNode.getChildNode(NAME(name)).getText());
                    value = s;
                }
                else
                {
                    value = "";
                }
            }
        }

		virtual void inoutfixstring(InoutName_T name, char* value, int& size, int max_size, int idx) override
		{
			if (!currentNode.getChildNode(NAME(name), idx).isEmpty())
			{
				if (currentNode.getChildNode(NAME(name), idx).getText() != nullptr)
				{
					std::string s(currentNode.getChildNode(NAME(name), idx).getText());
					int len = (int)s.size();
					if (len > max_size) throw ops::ArchiverException("Illegal size of fix string received. name: ", name);
					if (len > 0) memcpy(value, s.c_str(), len);
					size = len;
				}
				else
				{
					size = 0;
				}
				value[size] = '\0';
			}
		}
		
		virtual void inout(InoutName_T name, char* buffer, int bufferSize) override
        {
			if (!currentNode.getChildNode(NAME(name)).isEmpty())
			{
				if (currentNode.getChildNode(NAME(name)).getText() != nullptr)
				{
					std::string s(currentNode.getChildNode(NAME(name)).getText());
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

        virtual Serializable* inout(InoutName_T name, Serializable* value, int element) override
        {
			UNUSED(name);

            if (value) delete value;
            value = nullptr;

            PushNode(currentNode);
            currentNode = currentNode.getChildNode("element", element);
			const TypeId_T types(currentNode.getAttribute("type"));
            Serializable* newSer = factory->create(types);
            if (newSer != nullptr)
            {
                newSer->serialize(this);
            }

			PopNode(currentNode);

            return newSer;
        }

        virtual void inout(InoutName_T name, Serializable& value) override
        {
			if (!currentNode.getChildNode(NAME(name)).isEmpty())
			{
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));
				const TypeId_T types(currentNode.getAttribute("type"));
				///TODO check that type is correct

				value.serialize(this);

				PopNode(currentNode);
			}
		}

        virtual Serializable* inout(InoutName_T name, Serializable* value) override
        {
            if (value) delete value;
            value = nullptr;
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));
				const TypeId_T types(currentNode.getAttribute("type"));
                Serializable* newSer = factory->create(types);
                if (newSer != nullptr)
                {
                    newSer->serialize(this);
                }

				PopNode(currentNode);

                return newSer;
            }
            return value;
        }

        virtual void inout(InoutName_T name, std::vector<bool>& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

                const int size = currentNode.nChildNode("element");
                value.reserve(size);
                value.resize(size, false);
                for (int i = 0; i < size; i++)
                {
					std::string s(currentNode.getChildNode("element", i).getText());
					value[i] = toBool(name, s);
                }

				PopNode(currentNode);
			}
        }

        virtual void inout(InoutName_T name, std::vector<char>& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

                const int size = currentNode.nChildNode("element");
                value.reserve(size);
                value.resize(size, 0);
                for (int i = 0; i < size; i++)
                {
					std::string s(currentNode.getChildNode("element", i).getText());
					std::stringstream ss(s);

                    int inVal;
                    ss >> inVal;
                    value[i] = (char)inVal;
                }

				PopNode(currentNode);
			}
        }

        virtual void inout(InoutName_T name, std::vector<int>& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

                const int size = currentNode.nChildNode("element");
                value.reserve(size);
                value.resize(size, 0);
                for (int i = 0; i < size; i++)
                {
					std::string s(currentNode.getChildNode("element", i).getText());
					std::stringstream ss(s);

                    int inVal;
                    ss >> inVal;
                    value[i] = inVal;
                }

				PopNode(currentNode);
			}
        }

        virtual void inout(InoutName_T name, std::vector<int16_t>& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

                const int size = currentNode.nChildNode("element");
                value.reserve(size);
                value.resize(size, 0);
                for (int i = 0; i < size; i++)
                {
					std::string s(currentNode.getChildNode("element", i).getText());
					std::stringstream ss(s);

                    int inVal;
                    ss >> inVal;
                    value[i] = (int16_t)inVal;
                }

				PopNode(currentNode);
			}
        }

        virtual void inout(InoutName_T name, std::vector<int64_t>& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

                const int size = currentNode.nChildNode("element");
                value.reserve(size);
                value.resize(size, 0);
                for (int i = 0; i < size; i++)
                {
					std::string s(currentNode.getChildNode("element", i).getText());
					std::stringstream ss(s);

                    int64_t inVal;
                    ss >> inVal;
                    value[i] = inVal;
                }

				PopNode(currentNode);
			}
        }

        virtual void inout(InoutName_T name, std::vector<float>& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

                const int size = currentNode.nChildNode("element");
                value.reserve(size);
                value.resize(size, 0.0);
                for (int i = 0; i < size; i++)
                {
					std::string s(currentNode.getChildNode("element", i).getText());
					std::stringstream ss(s);

                    float inVal;
                    ss >> inVal;
                    value[i] = inVal;
                }

				PopNode(currentNode);
			}
        }

        virtual void inout(InoutName_T name, std::vector<double>& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

                const int size = currentNode.nChildNode("element");
                value.reserve(size);
                value.resize(size, 0.0);
                for (int i = 0; i < size; i++)
                {
					std::string s(currentNode.getChildNode("element", i).getText());
					std::stringstream ss(s);

                    double inVal;
                    ss >> inVal;
                    value[i] = inVal;
                }

				PopNode(currentNode);
			}
        }

        virtual void inout(InoutName_T name, std::vector<std::string>& value) override
        {
            if (!currentNode.getChildNode(NAME(name)).isEmpty())
            {
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

                const int size = currentNode.nChildNode("element");
                value.reserve(size);
                value.resize(size, "");
                for (int i = 0; i < size; i++)
                {
					std::string s(currentNode.getChildNode("element", i).getText());
                    value[i] = s;
                }

				PopNode(currentNode);
			}
        }

		void inoutfixarr(InoutName_T name, bool* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize);

			if (!currentNode.getChildNode(NAME(name)).isEmpty())
			{
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

				const int size = currentNode.nChildNode("element");
				if (size != numElements) throw ops::ArchiverException("Illegal size of fix bool array received. name: ", name);

				for (int i = 0; i < size; i++)
				{
					std::string s(currentNode.getChildNode("element", i).getText());
					value[i] = toBool(name, s);
				}

				PopNode(currentNode);
			}
		}

		void inoutfixarr(InoutName_T name, char* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize);

			if (!currentNode.getChildNode(NAME(name)).isEmpty())
			{
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

				const int size = currentNode.nChildNode("element");
				if (size != numElements) throw ops::ArchiverException("Illegal size of fix char array received. name: ", name);

				for (int i = 0; i < size; i++)
				{
					std::string s(currentNode.getChildNode("element", i).getText());
					std::stringstream ss(s);

					int inVal;
					ss >> inVal;
					value[i] = (char)inVal;
				}

				PopNode(currentNode);
			}
		}

		void inoutfixarr(InoutName_T name, int* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize);

			if (!currentNode.getChildNode(NAME(name)).isEmpty())
			{
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

				const int size = currentNode.nChildNode("element");
				if (size != numElements) throw ops::ArchiverException("Illegal size of fix int array received. name: ", name);

				for (int i = 0; i < size; i++)
				{
					std::string s(currentNode.getChildNode("element", i).getText());
					std::stringstream ss(s);

					int inVal;
					ss >> inVal;
					value[i] = inVal;
				}

				PopNode(currentNode);
			}
		}

		void inoutfixarr(InoutName_T name, int16_t* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize);

			if (!currentNode.getChildNode(NAME(name)).isEmpty())
			{
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

				const int size = currentNode.nChildNode("element");
				if (size != numElements) throw ops::ArchiverException("Illegal size of fix int16 array received. name: ", name);

				for (int i = 0; i < size; i++)
				{
					std::string s(currentNode.getChildNode("element", i).getText());
					std::stringstream ss(s);

					int inVal;
					ss >> inVal;
					value[i] = (int16_t)inVal;
				}

				PopNode(currentNode);
			}
		}

		void inoutfixarr(InoutName_T name, int64_t* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize);

			if (!currentNode.getChildNode(NAME(name)).isEmpty())
			{
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

				const int size = currentNode.nChildNode("element");
				if (size != numElements) throw ops::ArchiverException("Illegal size of fix int64 array received. name: ", name);

				for (int i = 0; i < size; i++)
				{
					std::string s(currentNode.getChildNode("element", i).getText());
					std::stringstream ss(s);

					int64_t inVal;
					ss >> inVal;
					value[i] = inVal;
				}

				PopNode(currentNode);
			}
		}

		void inoutfixarr(InoutName_T name, float* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize);

			if (!currentNode.getChildNode(NAME(name)).isEmpty())
			{
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

				const int size = currentNode.nChildNode("element");
				if (size != numElements) throw ops::ArchiverException("Illegal size of fix float array received. name: ", name);

				for (int i = 0; i < size; i++)
				{
					std::string s(currentNode.getChildNode("element", i).getText());
					std::stringstream ss(s);

					float inVal;
					ss >> inVal;
					value[i] = inVal;
				}

				PopNode(currentNode);
			}
		}

		void inoutfixarr(InoutName_T name, double* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize);

			if (!currentNode.getChildNode(NAME(name)).isEmpty())
			{
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

				const int size = currentNode.nChildNode("element");
				if (size != numElements) throw ops::ArchiverException("Illegal size of fix double array received. name: ", name);

				for (int i = 0; i < size; i++)
				{
					std::string s(currentNode.getChildNode("element", i).getText());
					std::stringstream ss(s);

					double inVal;
					ss >> inVal;
					value[i] = inVal;
				}

				PopNode(currentNode);
			}
		}

		void inoutfixarr(InoutName_T name, std::string* value, int numElements) override
        {
			if (!currentNode.getChildNode(NAME(name)).isEmpty())
			{
				PushNode(currentNode);
				currentNode = currentNode.getChildNode(NAME(name));

				const int size = currentNode.nChildNode("element");
				if (size != numElements) throw ops::ArchiverException("Illegal size of fix string array received. name: ", name);

				for (int i = 0; i < size; i++)
				{
					std::string s(currentNode.getChildNode("element", i).getText());
					value[i] = s;
				}

				PopNode(currentNode);
			}
		}

        int beginList(InoutName_T name, int size) override
        {
            UNUSED(size);
			PushNode(currentNode);
			currentNode = currentNode.getChildNode(NAME(name));
			
			if (!currentNode.isEmpty())
            {
                return currentNode.nChildNode("element");
            }
            return 0;
        }

        void endList(InoutName_T name) override
        {
            UNUSED(name);
			PopNode(currentNode);
		}

    };
}
#endif
