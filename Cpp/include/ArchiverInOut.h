/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2019-2020 Lennart Andersson.
 *
 * This notice apply to all source files, *.cpp, *.h, *.java, and *.cs in this directory
 * and all its subdirectories if nothing else is explicitly stated within the source file itself.
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

#ifndef ops_ArchiverInOutH
#define ops_ArchiverInOutH

#include <vector>
#include <exception>
#include <string>

#include "OPSTypeDefs.h"
#include "Serializable.h"

namespace ops
{
    namespace exceptions
    {
        class ArchiverException : public std::exception
        {
        private:
			ExceptionMessage_T message;
        public:
            explicit ArchiverException(ExceptionMessage_T m)
            {
				message = "ArchiverException: ";
				message += m;
            }
			ArchiverException(ExceptionMessage_T m, InoutName_T name)
			{
				message = "ArchiverException: ";
				message += m;
				message += name;
			}
			const char* what() const noexcept override { return message.c_str(); }
		};
    }
    using namespace exceptions;

    class ArchiverInOut
    {
    public:
        virtual ~ArchiverInOut();

		// Returns true if it's an output archiver
		virtual bool isOut() = 0;

        virtual void inout(InoutName_T name, bool& value) = 0;
        virtual void inout(InoutName_T name, char& value) = 0;
        virtual void inout(InoutName_T name, int& value) = 0;
        virtual void inout(InoutName_T name, int16_t& value) = 0;
        virtual void inout(InoutName_T name, int64_t& value) = 0;
        virtual void inout(InoutName_T name, float& value) = 0;
        virtual void inout(InoutName_T name, double& value) = 0;
        virtual void inout(InoutName_T name, std::string& value) = 0;
        virtual void inout(InoutName_T name, Serializable& value) = 0;

		virtual void inoutfixstring(InoutName_T name, char* value, int& size, int max_size, int idx) = 0;

		template<size_t N>
		void inout(InoutName_T name, strings::fixed_string<N>& value, int idx = 0)
		{
			int size = (int)value.size();
			inoutfixstring(name, &value[0], size, (int)value.max_size(), idx);
			if (!isOut()) value.resize(); // recalculate size for an input archiver
		}

		template <class T>
		void inoutenum(InoutName_T name, T& value)
		{
			int16_t tmp = static_cast<int16_t>(value);
			inout(name, tmp);
			value = static_cast<T>(tmp);
		}

		virtual void inout(InoutName_T name, char* buffer, int bufferSize) = 0;

		virtual Serializable* inout(InoutName_T name, Serializable* value) = 0;

        virtual Serializable* inout(InoutName_T name, Serializable* value, int element) = 0;

        virtual void inout(InoutName_T name, std::vector<bool>& value) = 0;
        virtual void inout(InoutName_T name, std::vector<char>& value) = 0;
        virtual void inout(InoutName_T name, std::vector<int>& value) = 0;
        virtual void inout(InoutName_T name, std::vector<int16_t>& value) = 0;
        virtual void inout(InoutName_T name, std::vector<int64_t>& value) = 0;
        virtual void inout(InoutName_T name, std::vector<float>& value) = 0;
        virtual void inout(InoutName_T name, std::vector<double>& value) = 0;
        virtual void inout(InoutName_T name, std::vector<std::string>& value) = 0;

		template<size_t N>
		void inout(InoutName_T name, std::vector<strings::fixed_string<N>>& vec)
		{
			const int size = beginList(name, (int)vec.size());
			if ((int)vec.size() != size) {
				vec.clear();
				vec.reserve(size);
				vec.resize(size);
				for (int i = 0; i < size; i++) {
					inout("element", vec[i], i);
				}
			} else {
				for (int i = 0; i < size; i++) {
					inout("element", vec[i], i);
				}
			}
			endList(name);
		}

		template <class T>
		void inoutenum(InoutName_T name, std::vector<T>& vec)
		{
			const int size = beginList(name, (int)vec.size());
			if ((int)vec.size() != size) {
				vec.clear();
				vec.reserve(size);
				vec.resize(size);
				for (int i = 0; i < size; i++) {
					inoutenum<T>("element", vec[i]); ///TODO , i);
				}
			} else {
				for (int i = 0; i < size; i++) {
					inoutenum<T>("element", vec[i]); ///TODO , i);
				}
			}
			endList(name);
		}

		virtual void inoutfixarr(InoutName_T name, bool* value, int numElements, int totalSize) = 0;
		virtual void inoutfixarr(InoutName_T name, char* value, int numElements, int totalSize) = 0;
		virtual void inoutfixarr(InoutName_T name, int* value, int numElements, int totalSize) = 0;
		virtual void inoutfixarr(InoutName_T name, int16_t* value, int numElements, int totalSize) = 0;
		virtual void inoutfixarr(InoutName_T name, int64_t* value, int numElements, int totalSize) = 0;
		virtual void inoutfixarr(InoutName_T name, float* value, int numElements, int totalSize) = 0;
		virtual void inoutfixarr(InoutName_T name, double* value, int numElements, int totalSize) = 0;
		virtual void inoutfixarr(InoutName_T name, std::string* value, int numElements) = 0;

		template<size_t N>
		void inoutfixarr(InoutName_T name, strings::fixed_string<N>* value, int numElements)
		{
			const int size = beginList(name, numElements);
			if (size != numElements) throw ops::ArchiverException("Illegal size of fix array received. name: ", name);
			for (int i = 0; i < size; i++) {
				inout("element", value[i], i);
			}
			endList(name);
		}

		template <class T>
		void inoutenum(InoutName_T name, T* value, int numElements)
		{
			const int size = beginList(name, numElements);
			if (size != numElements) throw ops::ArchiverException("Illegal size of fix array received. name: ", name);
			for (int i = 0; i < size; i++) {
				inoutenum("element", value[i]); ///TODO , i);
			}
			endList(name);
		}

        template <class SerializableType> 
		void inout(InoutName_T name, std::vector<SerializableType>& vec, SerializableType prototype)
        {
            const int size = beginList(name, (int)vec.size());
            if ((int) vec.size() < size)
            {
                vec.clear();
                vec.reserve(size);
                vec.resize(size, prototype);
                for (int i = 0; i < size; i++)
                {
                    inout("element", vec[i]);
                }
            }
            else
            {
                for (int i = 0; i < size; i++)
                {
                    inout("element", vec[i]);
                }
            }
            endList(name);
        }

        template <class SerializableType> 
		void inout(InoutName_T name, std::vector<SerializableType*>& vec)
        {
            const int size = beginList(name, (int)vec.size());
            if ((int) vec.size() < size)
            {
                vec.clear();
                vec.reserve(size);
                for (int i = 0; i < size; i++)
                {
                    vec.push_back((SerializableType*) inout(name, (Serializable*) nullptr, i));
                }
            }
            else
            {
                for (int i = 0; i < size; i++)
                {
                    vec[i] = (SerializableType*) inout(name, vec[i], i);
                }
            }
            endList(name);
        }

        template <class SerializableType> 
		void inoutfixarr(InoutName_T name, SerializableType** value, int numElements)
        {
            const int size = beginList(name, numElements);
            if (size != numElements) throw ops::ArchiverException("Illegal size of fix array received. name: ", name);
            for (int i = 0; i < size; i++) {
                value[i] = (SerializableType*) inout(name, value[i], i);
            }
            endList(name);
        }

        template <class SerializableType> 
		void inoutfixarr(InoutName_T name, SerializableType* value, int numElements)
        {
            const int size = beginList(name, numElements);
            if (size != numElements) throw ops::ArchiverException("Illegal size of fix array received. name: ", name);
            for (int i = 0; i < size; i++) {
                inout("element", value[i]);
            }
            endList(name);
        }

    protected:
        virtual int beginList(InoutName_T name, int i) = 0;
        virtual void endList(InoutName_T name) = 0;
    };

	template <typename T>
	void cloneVectorPtr(std::vector<T*>& dst, const std::vector<T*> src)
	{
		for (unsigned int i = 0; i < dst.size(); i++) {
			if (dst[i]) delete dst[i];
		}
		dst.resize(src.size());
		for (unsigned int i = 0; i < src.size(); i++) {
			if (dst.size() >= i + 1) {
				dst[i] = (T*)src[i]->clone();
			} else {
				dst.push_back((T*)src[i]->clone());
			}
		}
	}

	template <typename T, unsigned int N>
	void cloneFixArrPtr(T* dst[], T* const src[])
	{
		for (unsigned int i = 0; i < N; i++) {
			if (dst[i]) delete dst[i];
			dst[i] = (T*)src[i]->clone();
		}
	}

}

#endif
