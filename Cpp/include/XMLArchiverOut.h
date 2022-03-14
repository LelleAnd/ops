/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2021 Lennart Andersson.
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

#ifndef XMLArchiverOutH
#define XMLArchiverOutH

#include "ArchiverInOut.h"
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include "OPSObject.h"


namespace ops
{
    class XMLArchiverOut : public ArchiverInOut
    {
        std::ostream& os;
        int currentTabDepth;
        static constexpr int tabSize = 3;
        std::string topNode;
    private:

        std::string tab()
        {
            std::string ret("");
            for (int i = 0; i < currentTabDepth; i++)
                ret += ("   ");
            return ret;
        }
    public:

        XMLArchiverOut(std::ostream& os_, std::string topNode_) : os(os_), currentTabDepth(0), topNode(topNode_)
        {
            os << tab() << "<" << topNode << ">" << std::endl;
            currentTabDepth++;
        }

		// Returns true if it's an output archiver
		virtual bool isOut() noexcept override { return true; }

		void close()
        {
            currentTabDepth--;
            os << tab() << "</" << topNode << ">" << std::endl;
        }

        virtual void inout(InoutName_T name, bool& value) override
        {
            if (value)
            {
                os << tab() << "<" << name << ">" << "true" << "</" << name << ">\n";
            }
            else
            {
                os << tab() << "<" << name << ">" << "false" << "</" << name << ">\n";
            }
        }

        virtual void inout(InoutName_T name, char& value) override
        {
            os << tab() << "<" << name << ">" << (int) value << "</" << name << ">\n";
        }

        virtual void inout(InoutName_T name, int& value) override
        {
            os << tab() << "<" << name << ">" << value << "</" << name << ">\n";
        }

        virtual void inout(InoutName_T name, int16_t& value) override
        {
            os << tab() << "<" << name << ">" << value << "</" << name << ">\n";
        }

        virtual void inout(InoutName_T name, int64_t& value) override
        {
            os << tab() << "<" << name << ">" << value << "</" << name << ">\n";
        }

        virtual void inout(InoutName_T name, float& value) override
        {
            os << tab() << "<" << name << ">" << std::setprecision(9) << value << "</" << name << ">\n";
        }

        virtual void inout(InoutName_T name, double& value) override
        {
            os << tab() << "<" << name << ">" << std::setprecision(15) << value << "</" << name << ">\n";
        }

        virtual void inout(InoutName_T name, std::string& value) override
        {
			///TODO convert string to allowed xml chars
            os << tab() << "<" << name << ">" << value << "</" << name << ">\n";
        }

		virtual void inoutfixstring(InoutName_T name, char* value, int& size, int max_size, int idx) override
		{
			UNUSED(max_size);
			UNUSED(idx);
			
			///TODO convert string to allowed xml chars
			std::string tmp((const char*)value, size);
			os << tab() << "<" << name << ">" << tmp << "</" << name << ">\n";
		}

		virtual void inout(InoutName_T name, char* buffer, int bufferSize) override
        {
			os << tab() << "<" << name << ">";
			for (int i = 0; i < bufferSize; i++) os << ((unsigned int)buffer[i]) << " ";
			os << "</" << name << ">\n";
		}

        virtual Serializable* inout(InoutName_T name, Serializable* value, int element) override
        {
            UNUSED(name);
            UNUSED(element);
            OPSObject* opsO = dynamic_cast<OPSObject*> (value);

            if (opsO != nullptr)
            {
                os << tab() << "<" << "element" << " type = \"" << opsO->getTypeString() << "\" >" << "\n";
                currentTabDepth++;
                value->serialize(this);
                currentTabDepth--;
                os << tab() << "</" << "element" << ">" << "\n";
            }
            return value;
        }

        virtual Serializable* inout(InoutName_T name, Serializable* value) override
        {
            OPSObject* opsO = dynamic_cast<OPSObject*> (value);

            if (opsO != nullptr)
            {
                os << tab() << "<" << name << " type = \"" << opsO->getTypeString() << "\" >" << "\n";
                currentTabDepth++;
                value->serialize(this);
                currentTabDepth--;
                os << tab() << "</" << name << ">" << "\n";
            }
            return value;
        }

        virtual void inout(InoutName_T name, Serializable& value) override
        {
			const OPSObject& opsO = dynamic_cast<OPSObject&> (value);

			os << tab() << "<" << name << " type = \"" << opsO.getTypeString() << "\" >" << "\n";
			currentTabDepth++;
			value.serialize(this);
			currentTabDepth--;
			os << tab() << "</" << name << ">" << "\n";
		}

        virtual void inout(InoutName_T name, std::vector<bool>& value) override
        {
            os << tab() << "<" << name << ">" << std::endl;
            for (unsigned int i = 0; i < value.size(); i++)
            {
                currentTabDepth++;
                bool e = value[i];
                inout("element", e);
                currentTabDepth--;
            }
            os << tab() << "</" << name << ">" << std::endl;
        }

        virtual void inout(InoutName_T name, std::vector<char>& value) override
        {
            os << tab() << "<" << name << ">" << std::endl;
            for (unsigned int i = 0; i < value.size(); i++)
            {
                currentTabDepth++;
                inout("element", value[i]);
                currentTabDepth--;
            }
            os << tab() << "</" << name << ">" << std::endl;
        }

        virtual void inout(InoutName_T name, std::vector<int>& value) override
        {
            os << tab() << "<" << name << ">" << std::endl;
            for (unsigned int i = 0; i < value.size(); i++)
            {
                currentTabDepth++;
                inout("element", value[i]);
                currentTabDepth--;
            }
            os << tab() << "</" << name << ">" << std::endl;
        }

        virtual void inout(InoutName_T name, std::vector<int16_t>& value) override
        {
            os << tab() << "<" << name << ">" << std::endl;
            for (unsigned int i = 0; i < value.size(); i++)
            {
                currentTabDepth++;
                inout("element", value[i]);
                currentTabDepth--;
            }
            os << tab() << "</" << name << ">" << std::endl;
        }

        virtual void inout(InoutName_T name, std::vector<int64_t>& value) override
        {
            os << tab() << "<" << name << ">" << std::endl;
            for (unsigned int i = 0; i < value.size(); i++)
            {
                currentTabDepth++;
                inout("element", value[i]);
                currentTabDepth--;
            }
            os << tab() << "</" << name << ">" << std::endl;
        }

        virtual void inout(InoutName_T name, std::vector<float>& value) override
        {
            os << tab() << "<" << name << ">" << std::endl;
            for (unsigned int i = 0; i < value.size(); i++)
            {
                currentTabDepth++;
                inout("element", value[i]);
                currentTabDepth--;
            }
            os << tab() << "</" << name << ">" << std::endl;
        }

        virtual void inout(InoutName_T name, std::vector<double>& value) override
        {
            os << tab() << "<" << name << ">" << std::endl;
            for (unsigned int i = 0; i < value.size(); i++)
            {
                currentTabDepth++;
                inout("element", value[i]);
                currentTabDepth--;
            }
            os << tab() << "</" << name << ">" << std::endl;
        }

        virtual void inout(InoutName_T name, std::vector<std::string>& value) override
        {
            os << tab() << "<" << name << ">" << std::endl;
            for (unsigned int i = 0; i < value.size(); i++)
            {
                currentTabDepth++;
                inout("element", value[i]);
                currentTabDepth--;
            }
            os << tab() << "</" << name << ">" << std::endl;
        }

		void inoutfixarr(InoutName_T name, bool* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize);

			os << tab() << "<" << name << ">" << std::endl;
			for (int i = 0; i < numElements; i++)
			{
				currentTabDepth++;
				bool e = value[i];
				inout("element", e);
				currentTabDepth--;
			}
			os << tab() << "</" << name << ">" << std::endl;
		}

		void inoutfixarr(InoutName_T name, char* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize);

			os << tab() << "<" << name << ">" << std::endl;
			for (int i = 0; i < numElements; i++)
			{
				currentTabDepth++;
				char e = value[i];
				inout("element", e);
				currentTabDepth--;
			}
			os << tab() << "</" << name << ">" << std::endl;
		}

		void inoutfixarr(InoutName_T name, int* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize);

			os << tab() << "<" << name << ">" << std::endl;
			for (int i = 0; i < numElements; i++)
			{
				currentTabDepth++;
				int e = value[i];
				inout("element", e);
				currentTabDepth--;
			}
			os << tab() << "</" << name << ">" << std::endl;
		}

		void inoutfixarr(InoutName_T name, int16_t* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize);

			os << tab() << "<" << name << ">" << std::endl;
			for (int i = 0; i < numElements; i++)
			{
				currentTabDepth++;
				int16_t e = value[i];
				inout("element", e);
				currentTabDepth--;
			}
			os << tab() << "</" << name << ">" << std::endl;
		}

		void inoutfixarr(InoutName_T name, int64_t* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize);

			os << tab() << "<" << name << ">" << std::endl;
			for (int i = 0; i < numElements; i++)
			{
				currentTabDepth++;
				int64_t e = value[i];
				inout("element", e);
				currentTabDepth--;
			}
			os << tab() << "</" << name << ">" << std::endl;
		}

		void inoutfixarr(InoutName_T name, float* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize);

			os << tab() << "<" << name << ">" << std::endl;
			for (int i = 0; i < numElements; i++)
			{
				currentTabDepth++;
				float e = value[i];
				inout("element", e);
				currentTabDepth--;
			}
			os << tab() << "</" << name << ">" << std::endl;
		}

		void inoutfixarr(InoutName_T name, double* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize);

			os << tab() << "<" << name << ">" << std::endl;
			for (int i = 0; i < numElements; i++)
			{
				currentTabDepth++;
				double e = value[i];
				inout("element", e);
				currentTabDepth--;
			}
			os << tab() << "</" << name << ">" << std::endl;
		}

		void inoutfixarr(InoutName_T name, std::string* value, int numElements) override
        {
			os << tab() << "<" << name << ">" << std::endl;
			for (int i = 0; i < numElements; i++)
			{
				currentTabDepth++;
				std::string e = value[i];
				inout("element", e);
				currentTabDepth--;
			}
			os << tab() << "</" << name << ">" << std::endl;
		}

        int beginList(InoutName_T name, int size) override
        {
            //buf->WriteInt(size);
            os << tab() << "<" << name << ">" << std::endl;
            currentTabDepth++;
            return size;
        }

        void endList(InoutName_T name) override
        {
            currentTabDepth--;
            os << tab() << "</" << name << ">" << std::endl;

        }

    };
}
#endif
