/**
*
* Copyright (C) 2017-2021 Lennart Andersson.
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

#include <cstring>
#ifndef FIXED_NO_STD_STRING
	#include <string>
#endif
#include <exception>

// noexcept and default specifiers requires a c++11 compiler.
#if __cplusplus >= 201103L		// Value according to standard for full C++11 conformity
	#define FIXED_C11_DETECTED
#elif defined(_MSC_VER) && (_MSC_VER >= 1900)
	// VS2015 still defines _cplusplus to 199711L but supports the features we need.
	// VS2013 an earlier also defines _cplusplus to 199711L but does not support the features.
	#define FIXED_C11_DETECTED
#endif
#ifndef FIXED_C11_DETECTED
#error C++11 Compiler required
#endif

#if __cplusplus >= 201402L		// Value according to standard for full C++14 conformity
	#define FIXED_C14_DETECTED
#elif defined(_MSC_VER) && (_MSC_VER >= 1915)
	#if _MSVC_LANG >= 201402L
		#define FIXED_C14_DETECTED
	#endif
#endif

#if __cplusplus >= 201703L		// Value according to standard for full C++17 conformity
	#define FIXED_C17_DETECTED
#elif defined(_MSC_VER) && (_MSC_VER >= 1915)
	#if _MSVC_LANG >= 201703L
		#define FIXED_C17_DETECTED
	#endif
#endif

#ifdef FIXED_C17_DETECTED
#define FIXED_IF_CONSTEXPR if constexpr
#else
#define FIXED_IF_CONSTEXPR if
#endif

namespace ops { namespace strings {

	class basic_fixed_string
	{
	public:
		typedef size_t size_type;
		// Capacity:
		virtual size_type size() const noexcept = 0;
		virtual size_type length() const noexcept = 0;

		// String operations
		virtual const char* data() const noexcept = 0;
		virtual const char* c_str() const noexcept = 0;

		virtual ~basic_fixed_string() = default;
	};

	typedef enum { truncate_string, throw_exception } overrun_policy_t;

	template <size_t N, overrun_policy_t POLICY = throw_exception>
	class fixed_string : public basic_fixed_string
	{
	public:
		typedef basic_fixed_string::size_type size_type;
	private:
        char _array[N + 1]{ 0 };
        size_type _size{ 0 };
	public:
		// Exceptions:
		struct index_out_of_range : public std::exception {
			const char* what() const noexcept override { return "Index too large"; }
		};
		struct size_out_of_range : public std::exception {
			const char* what() const noexcept override { return "String too large"; }
		};

		// Constructors:
		fixed_string() noexcept { }
		fixed_string(char* s) { append(s, strlen(s)); }
		fixed_string(const char* s) { append(s, strlen(s)); }
		fixed_string(char* s, size_type len) { size_type sz = strlen(s); append(s, (sz < len) ? sz : len); }
		fixed_string(const char* s, size_type len) { const size_type sz = strlen(s); append(s, (sz < len) ? sz : len); }
#ifndef FIXED_NO_STD_STRING
		fixed_string(const std::string s) { append(s.c_str(), s.size()); }
#endif

		template<size_t M, overrun_policy_t POL>
		fixed_string(const fixed_string<M, POL>& str) { append(str.c_str(), str.size()); }

		// Construction from any type that have c_str() and size() methods
		template<typename T>
		fixed_string(const T& str) { append(str.c_str(), str.size()); }

		// all the special members can be defaulted
		fixed_string(fixed_string const&) = default;
		fixed_string(fixed_string&&) = default;
		fixed_string& operator=(fixed_string&&) = default;
		fixed_string& operator=(fixed_string const&) = default;
		~fixed_string() = default;

		// Iterators:
		// ...

		// Capacity:
		size_type size() const noexcept override { return _size; }
		size_type length() const noexcept override { return _size; }
		size_type max_size() const noexcept { return N; }
		void resize() noexcept
		{
			_array[N] = '\0';	// make sure the array is null terminated
			_size = strlen(&_array[0]); 
		}
		void resize(size_type n) { resize(n, '\0'); }
		void resize(size_type n, char c)
		{
			if (n > N) {
				if (POLICY == throw_exception) throw size_out_of_range();
				n = N;
			}
			for (size_type i = _size; i < n; ++i) _array[i] = c;
			_size = n; 
			_array[_size] = '\0';
		}

		void clear() noexcept { _array[0] = '\0'; _size = 0; }
		bool empty() const noexcept { return _size == 0; }

		// Element access:
		char& operator[] (size_type pos)
		{
			// if pos is equal to size, return ref to null char (according to standard for std::string)
			if (pos > _size) throw index_out_of_range();
			return _array[pos];
		}
		char& at(size_type pos)
		{
			if (pos >= _size) throw index_out_of_range();
			return _array[pos];
		}

		// Modifiers:
		fixed_string& operator+= (const fixed_string& str) { return append(str.c_str(), str.size()); }
#ifndef FIXED_NO_STD_STRING
		fixed_string& operator+= (const std::string& str) { return append(str.c_str(), str.size()); }
#endif
		fixed_string& operator+= (const char* s) { return append(s, strlen(s)); }
		fixed_string& operator+= (char c)
		{
			if (_size == N) {
#if defined(_MSC_VER) && (_MSC_VER == 1900)
#pragma warning( disable : 4127)
#endif
				FIXED_IF_CONSTEXPR (POLICY == throw_exception) throw size_out_of_range();
			} else {
				_array[_size] = c;
				_size++;
				_array[_size] = '\0';
			}
			return *this;
		}

		fixed_string& append(const fixed_string& str) { return append(str.c_str(), str.size()); }
		fixed_string& append(const char* s) { return append(s, strlen(s)); }
		fixed_string& append(const char* s, size_type len)
		{
			if (len > 0) {
				if ((_size + len) > N) {
#if defined(_MSC_VER) && (_MSC_VER == 1900)
#pragma warning( disable : 4127)
#endif
					FIXED_IF_CONSTEXPR (POLICY == throw_exception) {
						throw size_out_of_range();
					} else {
						len = N - _size;
					}
				}
				memcpy(&_array[_size], s, len);
				_size += len;
			}
			_array[_size] = '\0';	// Needed if len == 0, since it can be from constructors with null string
			return *this;
		}

		template<size_t M, overrun_policy_t POL>
		fixed_string& operator+= (const fixed_string<M, POL>& str) { return append(str.c_str(), str.size()); }

		template<size_t M, overrun_policy_t POL>
		fixed_string& append(const fixed_string<M, POL>& str) { return append(str.c_str(), str.size()); }

#ifndef FIXED_NO_STD_STRING
		// Implicit conversion operator
		operator std::string() const { return std::string(_array); }
#endif

		// String operations
		const char* data() const noexcept override { return &_array[0]; }
		const char* c_str() const noexcept override { return &_array[0]; }

		size_type find(const fixed_string& str, size_type pos = 0) const
		{
			return find(str.c_str(), pos);
		}
		size_type find(const char* s, size_type pos = 0) const noexcept
		{
			if (pos > _size) return npos;
			const char* ptr = strstr(&_array[pos], s);
			if (ptr == nullptr) return npos;
			return (size_type)(ptr - &_array[0]);
		}
		size_type find(char c, size_type pos = 0) const noexcept
		{
			if (pos > _size) return npos;
			const char* ptr = strchr(&_array[pos], c);
			if (ptr == nullptr) return npos;
			return (size_type)(ptr - &_array[0]);
		}

		size_type find_first_of(char c, size_type pos = 0) const
		{
			if (pos < _size) {
				for (size_type i = pos; i < _size; ++i) {
					if (_array[i] == c) return i;
				}
			}
			return npos;
		}

		size_type find_first_of(const char* s, size_type pos = 0) const
		{
			if (pos >= _size) return npos;
			const char* ptr = strpbrk(&_array[pos], s);
			if (ptr == nullptr) return npos;
			return (size_type)(ptr - &_array[0]);
		}

		size_type find_first_not_of(char c, size_type pos = 0) const
		{
			if (pos < _size) {
				for (size_type i = pos; i < _size; ++i) {
					if (_array[i] != c) return i;
				}
			}
			return npos;
		}

		size_type find_last_of(char c, size_type pos = npos) const
		{
			if (_size > 0) {
				if ((pos == npos) || (pos >= _size)) pos = _size - 1;
				for (size_type i = pos; i > 0; --i) {
					if (_array[i] == c) return i;
				}
				if (_array[0] == c) return 0;
			}
			return npos;
		}

		size_type find_last_not_of(char c, size_type pos = npos) const
		{
			if (_size > 0) {
				if ((pos == npos) || (pos >= _size)) pos = _size - 1;
				for (size_type i = pos; i > 0; --i) {
					if (_array[i] != c) return i;
				}
				if (_array[0] != c) return 0;
			}
			return npos;
		}

#ifndef FIXED_NO_STD_STRING
		std::string substr(size_type pos = 0, size_type len = npos) const
		{
			if (pos > _size) throw index_out_of_range();
			size_type avail = _size - pos;
			if (len > avail) len = avail;
			return std::string(&_array[pos], len);
		}
#else
		fixed_string substr(size_type pos = 0, size_type len = npos) const
		{
			if (pos > _size) throw index_out_of_range();
			const size_type avail = _size - pos;
			if (len > avail) len = avail;
			return fixed_string(&_array[pos], len);
		}
#endif

		template<size_t M, overrun_policy_t POL = POLICY>
		fixed_string<M, POL> substr(size_type pos = 0, size_type len = npos) const
		{
			if (pos >= _size) throw index_out_of_range();
			return fixed_string<M, POL>(&_array[pos], len);
		}

		static const size_type npos = static_cast<size_type>(-1);
	};

	// Relational operators
	template<size_t M, size_t N, overrun_policy_t POLICYM, overrun_policy_t POLICYN>
	bool operator== (const fixed_string<M, POLICYM>& lhs, const fixed_string<N, POLICYN>& rhs) { return strcmp(lhs.c_str(), rhs.c_str()) == 0; }
	template<size_t M, overrun_policy_t POLICY>
	bool operator== (const char*   lhs, const fixed_string<M, POLICY>& rhs) { return strcmp(lhs, rhs.c_str()) == 0; }
	template<size_t M, overrun_policy_t POLICY>
	bool operator== (const fixed_string<M, POLICY>& lhs, const char*   rhs) { return strcmp(lhs.c_str(), rhs) == 0; }

	template<size_t M, size_t N, overrun_policy_t POLICYM, overrun_policy_t POLICYN>
	bool operator!= (const fixed_string<M, POLICYM>& lhs, const fixed_string<N, POLICYN>& rhs) { return strcmp(lhs.c_str(), rhs.c_str()) != 0; }
	template<size_t M, overrun_policy_t POLICY>
	bool operator!= (const char*   lhs, const fixed_string<M, POLICY>& rhs) { return strcmp(lhs, rhs.c_str()) != 0; }
	template<size_t M, overrun_policy_t POLICY>
	bool operator!= (const fixed_string<M, POLICY>& lhs, const char*   rhs) { return strcmp(lhs.c_str(), rhs) != 0; }

	template <size_t M, size_t N, overrun_policy_t POLICYM, overrun_policy_t POLICYN>
	bool operator<  (const fixed_string<M, POLICYM>& lhs, const fixed_string<N, POLICYN>& rhs) { return strcmp(lhs.c_str(), rhs.c_str()) < 0; }
	template <size_t M, overrun_policy_t POLICY>
	bool operator<  (const char*   lhs, const fixed_string<M, POLICY>& rhs) { return strcmp(lhs, rhs.c_str()) < 0; }
	template <size_t M, overrun_policy_t POLICY>
	bool operator<  (const fixed_string<M, POLICY>& lhs, const char*   rhs) { return strcmp(lhs.c_str(), rhs) < 0; }

	template <size_t M, size_t N, overrun_policy_t POLICYM, overrun_policy_t POLICYN>
	bool operator<=  (const fixed_string<M, POLICYM>& lhs, const fixed_string<N, POLICYN>& rhs) { return strcmp(lhs.c_str(), rhs.c_str()) <= 0; }
	template <size_t M, overrun_policy_t POLICY>
	bool operator<=  (const char*   lhs, const fixed_string<M, POLICY>& rhs) { return strcmp(lhs, rhs.c_str()) <= 0; }
	template <size_t M, overrun_policy_t POLICY>
	bool operator<=  (const fixed_string<M, POLICY>& lhs, const char*   rhs) { return strcmp(lhs.c_str(), rhs) <= 0; }

	template <size_t M, size_t N, overrun_policy_t POLICYM, overrun_policy_t POLICYN>
	bool operator>  (const fixed_string<M, POLICYM>& lhs, const fixed_string<N, POLICYN>& rhs) { return strcmp(lhs.c_str(), rhs.c_str()) > 0; }
	template <size_t M, overrun_policy_t POLICY>
	bool operator>  (const char*   lhs, const fixed_string<M, POLICY>& rhs) { return strcmp(lhs, rhs.c_str()) > 0; }
	template <size_t M, overrun_policy_t POLICY>
	bool operator>  (const fixed_string<M, POLICY>& lhs, const char*   rhs) { return strcmp(lhs.c_str(), rhs) > 0; }

	template <size_t M, size_t N, overrun_policy_t POLICYM, overrun_policy_t POLICYN>
	bool operator>=  (const fixed_string<M, POLICYM>& lhs, const fixed_string<N, POLICYN>& rhs) { return strcmp(lhs.c_str(), rhs.c_str()) >= 0; }
	template <size_t M, overrun_policy_t POLICY>
	bool operator>=  (const char*   lhs, const fixed_string<M, POLICY>& rhs) { return strcmp(lhs, rhs.c_str()) >= 0; }
	template <size_t M, overrun_policy_t POLICY>
	bool operator>=  (const fixed_string<M, POLICY>& lhs, const char*   rhs) { return strcmp(lhs.c_str(), rhs) >= 0; }

	// operator +
	template <size_t M, size_t N, overrun_policy_t POLICY>
	fixed_string<M + N> operator+ (const fixed_string<M, POLICY>& lhs, const fixed_string<N, POLICY>& rhs)
	{
		fixed_string<M + N, POLICY> res;
		res += lhs;
		res += rhs;
		return res;
	}

#ifndef FIXED_NO_STD_STRING
	template <size_t M, overrun_policy_t POLICY>
	std::string operator+ (const std::string& lhs, const fixed_string<M, POLICY>& rhs) { return lhs + rhs.substr(); }
	template <size_t M, overrun_policy_t POLICY>
	std::string operator+ (const fixed_string<M, POLICY>& lhs, const std::string& rhs) { return lhs.substr() + rhs; }
#endif

	template <size_t N>
	using fixed_string_trunc = fixed_string<N, truncate_string>;

}} //namespace
