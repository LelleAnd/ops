/**
*
* Copyright (C) 2017 Lennart Andersson.
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

#include <ostream>

#include "fixed_string.h"

namespace strings {

	// Stream operator
	inline std::ostream& operator<< (std::ostream& os, const basic_fixed_string& str)
	{
		os << str.c_str();
		return os;
	}

	template<size_t N>
	fixed_string<N> ToLower(fixed_string<N> str)
	{
		for (typename fixed_string<N>::size_type i = 0; i < str.size(); ++i) {
			str[i] = ::tolower(str[i]);
		}
		return str;
	}

	template<size_t N>
	fixed_string<N> ToUpper(fixed_string<N> str)
	{
		for (typename fixed_string<N>::size_type i = 0; i < str.size(); ++i) {
			str[i] = ::toupper(str[i]);
		}
		return str;
	}

	template<size_t N>
	fixed_string<N> Trim(fixed_string<N> str)
	{
		typename fixed_string<N>::size_type pos1 = str.find_first_not_of(' ');
		typename fixed_string<N>::size_type pos2 = str.find_last_not_of(' ');
		if ((pos1 == pos2) && (pos1 == fixed_string<N>::npos)) return "";
		return str.substr(pos1 == fixed_string<N>::npos ? 0 : pos1,
			pos2 == fixed_string<N>::npos ? str.length() - 1 : pos2 - pos1 + 1);
	}

} //namespace
