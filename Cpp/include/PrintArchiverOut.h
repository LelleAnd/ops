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

#ifndef PrintArchiverOutH
#define PrintArchiverOutH

#include <vector>
#include <string>
#include <iostream>

#include "ArchiverInOut.h"
#include "OPSObject.h"


namespace ops
{
    using namespace std;

    //Pure virtual interface.

    class PrintArchiverOut : public ArchiverInOut
    {
        std::ostream& os;
        int currentTabDepth;
        static constexpr int tabSize = 3;

    private:

        std::string tab()
        {
            std::string ret("");
            for (int i = 0; i < currentTabDepth; i++) ret += ("   ");
            return ret;
        }
    public:

        PrintArchiverOut(std::ostream& os_, int startTabDepth = 0) : os(os_), currentTabDepth(startTabDepth)
        {

        }

		// Returns true if it's an output archiver
		virtual bool isOut() override { return true; }

		void close()
        {
        }

        void printObject(InoutName_T name, Serializable* obj)
        {
            os << "\n________________Begin Object___________________" << "\n\n";
            inout(name, obj);
            os << "\n_________________End Object____________________" << "\n";
        }

        virtual void inout(InoutName_T name, bool& value) override
        {
            os << tab() << name << " = " << value << "\n";
        }

        virtual void inout(InoutName_T name, char& value) override
        {
            os << tab() << name << " = " << (int)value << "\n";
        }

        virtual void inout(InoutName_T name, int& value) override
        {
            os << tab() << name << " = " << value << "\n";
        }

        virtual void inout(InoutName_T name, int16_t& value) override
        {
            os << tab() << name << " = " << value << "\n";
        }

        virtual void inout(InoutName_T name, int64_t& value) override
        {
            os << tab() << name << " = " << value << "\n";
        }

        virtual void inout(InoutName_T name, float& value) override
        {
            os << tab() << name << " = " << value << "\n";
        }

        virtual void inout(InoutName_T name, double& value) override
        {
            os << tab() << name << " = " << value << "\n";
        }

        virtual void inout(InoutName_T name, std::string& value) override
        {
            os << tab() << name << " = " << value << "\n";
        }

		virtual void inoutfixstring(InoutName_T name, char* value, int& size, int max_size, int idx) override
		{
			UNUSED(size)
			UNUSED(max_size)
			os << tab() << name;
			if (idx > 0) os << "[" << idx << "]";
			os << " = " << value << "\n";
		}

		virtual Serializable* inout(InoutName_T name, Serializable* value, int element) override
        {
			UNUSED(element)
			OPSObject* opsO = dynamic_cast<OPSObject*> (value);
			if (opsO != nullptr)
			{
				os << tab() << name << " type = " << opsO->getTypeString() << "\n";
				currentTabDepth++;
				value->serialize(this);
				currentTabDepth--;
			}
            return value;
        }

        virtual Serializable* inout(InoutName_T name, Serializable* value) override
        {
            OPSObject* opsO = dynamic_cast<OPSObject*> (value);

            if (opsO != nullptr)
            {
                os << tab() << name << " type = " << opsO->getTypeString() << "\n";
                currentTabDepth++;
                value->serialize(this);
                currentTabDepth--;
            }
            return value;
        }

        virtual void inout(InoutName_T name, Serializable& value) override
        {
            OPSObject* opsO = dynamic_cast<OPSObject*> (&value);

            if (opsO != nullptr)
            {
                os << tab() << name << " type = " << opsO->getTypeString() << "\n";
                currentTabDepth++;
                value.serialize(this);
                currentTabDepth--;
            }
        }

        virtual void inout(InoutName_T name, char* buffer, int bufferSize) override
        {
			UNUSED(buffer)
			UNUSED(bufferSize)
			os << tab() << name << " ... TODO \n";
        }

        virtual void inout(InoutName_T name, std::vector<bool>& value) override
        {
            if (value.size() > 0)
            {
                std::string value0 = value[0] ? "true" : "false";
                std::string valueX = value[value.size() - 1] ? "true" : "false";

				os << tab() << name << "(size = " << value.size() << ") = [ " << value0;
				if (value.size() > 1) {
					os << " ... " << valueX;
				}
				os << " ]" << endl;
			}
        }

        virtual void inout(InoutName_T name, std::vector<char>& value) override
        {
			if (value.size() > 0) {
				os << tab() << name << "(size = " << value.size() << ") = [ " << (int)value[0];
				if (value.size() > 1) {
					os << " ... " << (int)value[value.size() - 1];
				}
				os << " ]" << endl;
			}
        }

        virtual void inout(InoutName_T name, std::vector<int>& value) override
        {
			if (value.size() > 0) {
				os << tab() << name << "(size = " << value.size() << ") = [ " << value[0];
				if (value.size() > 1) {
					os << " ... " << value[value.size() - 1];
				}
				os << " ]" << endl;
			}
        }

        virtual void inout(InoutName_T name, std::vector<int16_t>& value) override
        {
			if (value.size() > 0) {
				os << tab() << name << "(size = " << value.size() << ") = [ " << value[0];
				if (value.size() > 1) {
					os << " ... " << value[value.size() - 1];
				}
				os << " ]" << endl;
			}
        }

        virtual void inout(InoutName_T name, std::vector<int64_t>& value) override
        {
			if (value.size() > 0) {
				os << tab() << name << "(size = " << value.size() << ") = [ " << value[0];
				if (value.size() > 1) {
					os << " ... " << value[value.size() - 1];
				}
				os << " ]" << endl;
			}
        }

        virtual void inout(InoutName_T name, std::vector<float>& value) override
        {
            if (value.size() > 0) {
				os << tab() << name << "(size = " << value.size() << ") = [ " << value[0];
				if (value.size() > 1) {
					os << " ... " << value[value.size() - 1];
				}
				os << " ]" << endl;
			}
            //            os << tab() << name << " = [" << endl;
            //            for (unsigned int i = 0; i < value.size(); i++)
            //            {
            //                currentTabDepth++;
            //                float e = value[i];
            //                inout(std::string(""), e);
            //                currentTabDepth--;
            //            }
            //            os << tab() << "]" << endl;
        }

        virtual void inout(InoutName_T name, std::vector<double>& value) override
        {
			if (value.size() > 0) {
				os << tab() << name << "(size = " << value.size() << ") = [ " << value[0];
				if (value.size() > 1) {
					os << " ... " << value[value.size() - 1];
				}
				os << " ]" << endl;
			}
        }

        virtual void inout(InoutName_T name, std::vector<std::string>& value) override
        {
			if (value.size() > 0) {
				os << tab() << name << "(size = " << value.size() << ") = [ " << value[0];
				if (value.size() > 1) {
					os << " ... " << value[value.size() - 1];
				}
				os << " ]" << endl;
			}
        }

		virtual void inoutfixarr(InoutName_T name, bool* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize)
			std::string value0 = value[0] ? "true" : "false";
			std::string valueX = value[numElements - 1] ? "true" : "false";
			os << tab() << name << "(size = " << numElements << ") = [ " << value0 << " ... " << valueX << " ]" << endl;
		}

		virtual void inoutfixarr(InoutName_T name, char* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize)
			os << tab() << name << "(size = " << numElements << ") = [ " << (int)value[0] << " ... " << (int)value[numElements - 1] << " ]" << endl;
		}

		virtual void inoutfixarr(InoutName_T name, int* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize)
			os << tab() << name << "(size = " << numElements << ") = [ " << value[0] << " ... " << value[numElements - 1] << " ]" << endl;
		}

		virtual void inoutfixarr(InoutName_T name, int16_t* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize)
			os << tab() << name << "(size = " << numElements << ") = [ " << value[0] << " ... " << value[numElements - 1] << " ]" << endl;
		}

		virtual void inoutfixarr(InoutName_T name, int64_t* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize)
			os << tab() << name << "(size = " << numElements << ") = [ " << value[0] << " ... " << value[numElements - 1] << " ]" << endl;
		}

		virtual void inoutfixarr(InoutName_T name, float* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize)
			os << tab() << name << "(size = " << numElements << ") = [ " << value[0] << " ... " << value[numElements - 1] << " ]" << endl;
		}

		virtual void inoutfixarr(InoutName_T name, double* value, int numElements, int totalSize) override
		{
			UNUSED(totalSize)
			os << tab() << name << "(size = " << numElements << ") = [ " << value[0] << " ... " << value[numElements - 1] << " ]" << endl;
		}

		virtual void inoutfixarr(InoutName_T name, std::string* value, int numElements) override
		{
			os << tab() << name << "(size = " << numElements << ") = [ " << value[0] << " ... " << value[numElements - 1] << " ]" << endl;
		}
		
		int beginList(InoutName_T name, int size) override
        {
            os << tab() << name << " = " << endl;
            currentTabDepth++;
            return size;
        }

        void endList(InoutName_T name) override
        {
			UNUSED(name)
            currentTabDepth--;
        }

    };

}
#endif
