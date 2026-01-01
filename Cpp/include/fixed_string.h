/**
*
* Copyright (C) 2017-2025 Lennart Andersson.
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

// constexpr usage with non-empty bodies require at least a c++14 compiler.
#if __cplusplus >= 201402L		// Value according to standard for full C++14 conformity
	#define FIXED_C14_DETECTED
#elif defined(_MSC_VER) && (_MSC_VER >= 1915)
	#if _MSVC_LANG >= 201402L
		#define FIXED_C14_DETECTED
	#endif
#endif
#ifndef FIXED_C14_DETECTED
#error C++14 Compiler required
#endif

#if __cplusplus >= 201703L		// Value according to standard for full C++17 conformity
	#define FIXED_C17_DETECTED
#elif defined(_MSC_VER) && (_MSC_VER >= 1915)
	#if _MSVC_LANG >= 201703L
		#define FIXED_C17_DETECTED
	#endif
#endif

#if __cplusplus >= 202002L		// Value according to standard for full C++20 conformity
	#define FIXED_C20_DETECTED
#elif defined(_MSC_VER) && (_MSC_VER >= 1916)
	#if _MSVC_LANG >= 202002L
		#define FIXED_C20_DETECTED
	#endif
#endif

#if __cplusplus >= 202302L		// Value according to standard for full C++23 conformity
	#define FIXED_C23_DETECTED
#elif defined(_MSC_VER) && (_MSC_VER >= 1916)
	#if _MSVC_LANG >= 202302L
		#define FIXED_C23_DETECTED
	#endif
#endif

#ifdef FIXED_C17_DETECTED
	#define FIXED_IF_CONSTEXPR if constexpr
#else
	#define FIXED_IF_CONSTEXPR if
#endif

namespace ops { namespace strings {

#ifdef FIXED_C17_DETECTED
	// strlen() isn't available in constexpr-land, but there is another one
	constexpr size_t constexpr_strlen(const char* s)
	{
		return std::char_traits<char>::length(s);
	}
#else
	#define constexpr_strlen strlen
#endif

    enum class  overrun_policy_t { truncate_string, throw_exception };

    template <size_t N, overrun_policy_t POLICY = overrun_policy_t::throw_exception>
    class fixed_string
    {
        using u16_or_siz_type = typename std::conditional<N<65536, uint16_t, size_t>::type;
        using local_size_type = typename std::conditional<N<256, uint8_t, u16_or_siz_type>::type;
    public:
        using size_type = size_t;
    private:
        local_size_type _size{0};
        char _array[N + 1]{0};
    public:
		// Exceptions:
		struct index_out_of_range : public std::exception {
			const char* what() const noexcept override { return "Index too large"; }
		};
		struct size_out_of_range : public std::exception {
			const char* what() const noexcept override { return "String too large"; }
		};

		// Constructors:
		constexpr fixed_string() noexcept { }
		constexpr fixed_string(char* s) { append(s, constexpr_strlen(s)); }
		constexpr fixed_string(const char* s) { append(s, constexpr_strlen(s)); }
        constexpr fixed_string(const char* s1, const char* s2, const char sep = '\0') {
            append(s1, constexpr_strlen(s1));
            if (sep != '\0') { operator+=(sep); }
            append(s2, constexpr_strlen(s2));
        }
		constexpr fixed_string(char* s, size_type len) { size_type sz = constexpr_strlen(s); append(s, (sz < len) ? sz : len); }
		constexpr fixed_string(const char* s, size_type len) { const size_type sz = constexpr_strlen(s); append(s, (sz < len) ? sz : len); }
#ifndef FIXED_NO_STD_STRING
		constexpr fixed_string(const std::string& s) { append(s.c_str(), s.size()); }
#ifdef FIXED_C17_DETECTED
		constexpr fixed_string(const std::string_view& s) { append(s.data(), s.size()); }
#endif
#endif

		template<size_t M, overrun_policy_t POL>
		constexpr fixed_string(const fixed_string<M, POL>& str) { append(str.c_str(), str.size()); }

		// Construction from any type that have c_str() and size() methods
		template<typename T>
		constexpr fixed_string(const T& str) { append(str.c_str(), str.size()); }

		// all the special members can be defaulted
		fixed_string(fixed_string const&) = default;
		fixed_string(fixed_string&&) = default;
		fixed_string& operator=(fixed_string&&) = default;
		fixed_string& operator=(fixed_string const&) = default;
		~fixed_string() = default;

		// Iterators:
		// ...

		// Capacity:
		constexpr size_type size() const noexcept { return _size; }
		constexpr size_type length() const noexcept { return _size; }
		constexpr size_type max_size() const noexcept { return N; }
		void resize() noexcept
		{
			_array[N] = '\0';	// make sure the array is null terminated
			_size = (local_size_type)strlen(&_array[0]);
		}
		void resize(size_type n) { resize(n, '\0'); }
		void resize(size_type n, char c)
		{
			if (n > N) {
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && (!defined(FIXED_C17_DETECTED))
#pragma warning( disable : 4127)
#endif
				FIXED_IF_CONSTEXPR (POLICY == overrun_policy_t::throw_exception) {
					throw size_out_of_range();
				} else {
					n = N;
				}
			}
			for (size_type i = _size; i < n; ++i) _array[i] = c;
			_size = (local_size_type)n;
			_array[_size] = '\0';
		}

		void clear() noexcept { _array[0] = '\0'; _size = 0; }
		constexpr bool empty() const noexcept { return _size == 0; }

		// Element access:
		constexpr char& operator[] (size_type pos)
		{
			// if pos is equal to size, return ref to null char (according to standard for std::string)
			if (pos > _size) throw index_out_of_range();
			return _array[pos];
		}
		constexpr char& at(size_type pos)
		{
			if (pos >= _size) throw index_out_of_range();
			return _array[pos];
		}

		// Modifiers:
		constexpr fixed_string& operator+= (const fixed_string& str) { return append(str.c_str(), str.size()); }
#ifndef FIXED_NO_STD_STRING
		constexpr fixed_string& operator+= (const std::string& str) { return append(str.c_str(), str.size()); }
#endif
		constexpr fixed_string& operator+= (const char* s) { return append(s, constexpr_strlen(s)); }
		constexpr fixed_string& operator+= (char c)
		{
			if (_size == N) {
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && (!defined(FIXED_C17_DETECTED))
#pragma warning( disable : 4127)
#endif
				FIXED_IF_CONSTEXPR (POLICY == overrun_policy_t::throw_exception) throw size_out_of_range();
			} else {
				_array[_size] = c;
				_size++;
				_array[_size] = '\0';
			}
			return *this;
		}

        constexpr fixed_string& append(const fixed_string& str) { return append(str.c_str(), str.size()); }
        constexpr fixed_string& append(const char* s) { return append(s, constexpr_strlen(s)); }
        constexpr fixed_string& append(const char* s, size_type len)
        {
            if (len > 0) {
                if ((_size + len) > N) {
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && (!defined(FIXED_C17_DETECTED))
#pragma warning( disable : 4127)
#endif
                    FIXED_IF_CONSTEXPR (POLICY == overrun_policy_t::throw_exception) {
                        throw size_out_of_range();
                    } else {
                        len = N - _size;
                    }
                }
#if defined(__cpp_if_consteval)
                // When C+23 is available we can control which code to use in compile-time/run-time
                if consteval {
                    // memcpy() isn't available in constexpr land, so do a manual copy
                    for (size_t i = 0; i < len; ++i) {
                        _array[_size + i] = s[i];
                    }
                } else {
                    memcpy(&_array[_size], s, len);
                }
#elif defined(FIXED_C17_DETECTED)
                // memcpy() isn't available in constexpr land, so do a manual copy
                for (size_t i = 0; i < len; ++i) {
                    _array[_size + i] = s[i];
                }
#else
                memcpy(&_array[_size], s, len);
#endif
                _size += (local_size_type)len;
            }
            _array[_size] = '\0';	// Needed if len == 0, since it can be from constructors with null string
            return *this;
        }

		template<size_t M, overrun_policy_t POL>
#ifdef __GNUC__
		// Stop inlining for GCC to avoid erroneous warnings in copy loop in above append(const char*,size_t) at 
		// least up to gcc-14.2 with optimization -O2 or higher
        __attribute__((noinline))
#endif
		constexpr fixed_string& operator+= (const fixed_string<M, POL>& str) { return append(str.c_str(), str.size()); }

		template<size_t M, overrun_policy_t POL>
		constexpr fixed_string& append(const fixed_string<M, POL>& str) { return append(str.c_str(), str.size()); }

#ifndef FIXED_NO_STD_STRING
		// Implicit conversion operator
		operator std::string() const { return std::string(_array); }
#ifdef FIXED_C17_DETECTED
        constexpr operator std::string_view() const { return std::string_view(_array, _size); }
#endif
#endif

		// String operations
		constexpr const char* data() const noexcept { return &_array[0]; }
		constexpr const char* c_str() const noexcept { return &_array[0]; }

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
		constexpr fixed_string<M, POL> substr(size_type pos = 0, size_type len = npos) const
		{
			if (pos >= _size) throw index_out_of_range();
			return fixed_string<M, POL>(&_array[pos], len);
		}

		constexpr static size_type npos = static_cast<size_type>(-1);
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
	constexpr fixed_string<M + N> operator+ (const fixed_string<M, POLICY>& lhs, const fixed_string<N, POLICY>& rhs)
	{
		return fixed_string<M + N, POLICY>(lhs.data(), rhs.data());
	}

#ifndef FIXED_NO_STD_STRING
	template <size_t M, overrun_policy_t POLICY>
	std::string operator+ (const std::string& lhs, const fixed_string<M, POLICY>& rhs) { return lhs + rhs.substr(); }
	template <size_t M, overrun_policy_t POLICY>
	std::string operator+ (const fixed_string<M, POLICY>& lhs, const std::string& rhs) { return lhs.substr() + rhs; }
#endif

	// -----------------------------------------------------------------------------

	template <size_t N>
	using fixed_string_trunc = fixed_string<N, overrun_policy_t::truncate_string>;

	// -----------------------------------------------------------------------------
	// Helpers to simplify creation of fixed strings with exact size of string.
	//
	// Example of usage in C++ 17 and later:
	//   constexpr auto fs6 = ops::strings::make_fixed_string("Hej Hopp");
	//   static_assert(fs6.max_size() == 8);
	//   static_assert(fs6.size() == 8);
	//   static_assert(std::string_view(fs6.c_str()) == "Hej Hopp");

	template <int Size>
	constexpr auto make_fixed_string(const char (&cstr)[Size]) -> fixed_string<Size - 1>
	{
		return fixed_string<Size - 1>(cstr);
	}

	template <int Size>
	constexpr auto make_fixed_string_trunc(const char (&cstr)[Size]) -> fixed_string_trunc<Size - 1>
	{
		return fixed_string_trunc<Size - 1>(cstr);
	}

    template <int N, int M>
    constexpr auto make_fixed_string_trunc(const fixed_string_trunc<N>& fst1, const fixed_string_trunc<M>& fst2, const char sep)
        -> fixed_string_trunc<N + M + 1>
    {
        return fixed_string_trunc<N + M + 1>(fst1.data(), fst2.data(), sep);
    }

} // namespace strings
} // namespace ops
