/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019-2020 Lennart Andersson.
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

#ifndef SerializableH
#define SerializableH

namespace ops
{
	class ArchiverInOut;

	class Serializable
	{
	public:
		virtual void serialize(ArchiverInOut* archiver) = 0;

		Serializable() = default;
		Serializable(const Serializable& l) = default;
		Serializable& operator= (const Serializable& l) = default;
		Serializable(Serializable&&) = default;
		Serializable& operator=(Serializable&&) = default;
		virtual ~Serializable() = default;
	};
}

#endif
