/*
 * Dictionary.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "ibrdtn/data/Dictionary.h"
#include "ibrdtn/data/SDNV.h"
#include "ibrdtn/data/Exceptions.h"
#include "ibrdtn/data/Bundle.h"
#include <map>
#include <stdexcept>
#include <string.h>
#include <iostream>
#include <vector>

using namespace std;

namespace dtn
{
namespace data
{
	Dictionary::Dictionary()
	{
	}

	/**
	 * create a dictionary with all EID of the given bundle
	 */
	Dictionary::Dictionary(const dtn::data::Bundle &bundle)
	{
		// rebuild the dictionary
		add(bundle.destination);
		add(bundle.source);
		add(bundle.reportto);
		add(bundle._custodian);

		// add EID of all secondary blocks
		for (Bundle::const_iterator iter = bundle.begin(); iter != bundle.end(); ++iter)
		{
			const Block &b = (**iter);
			add( b.getEIDList() );
		}
	}

	Dictionary::Dictionary(const Dictionary &d)
	{
		this->operator=(d);
	}

	/**
	 * assign operator
	 */
	Dictionary& Dictionary::operator=(const Dictionary &d)
	{
		_bytestream.str(d._bytestream.str());
		return (*this);
	}

	Dictionary::~Dictionary()
	{
	}

	size_t Dictionary::get(const std::string value) const
	{
		std::string bytes = _bytestream.str();
		const char *bytebegin = bytes.c_str();
		const char *bytepos = bytebegin;
		const char *byteend = bytebegin + bytes.length() + 1;

		if (bytes.length() <= 0) return std::string::npos;

		while (bytepos < byteend)
		{
			std::string dictstr(bytepos);

			if (dictstr == value)
			{
				return bytepos - bytebegin;
			}

			bytepos += dictstr.length() + 1;
		}

		return std::string::npos;
	}

	bool Dictionary::exists(const std::string value) const
	{
		std::string bytes = _bytestream.str();
		const char *bytepos = bytes.c_str();
		const char *byteend = bytepos + bytes.length();

		if (bytes.length() <= 0) return false;

		while (bytepos < byteend)
		{
			std::string dictstr(bytepos);

			if (dictstr == value)
			{
				return true;
			}

			bytepos += dictstr.length() + 1;
		}

		return false;
	}

	void Dictionary::add(const std::string value)
	{
		if (!exists(value))
		{
			_bytestream << value << '\0';
		}
	}

	void Dictionary::add(const EID &eid)
	{
		add(eid.getScheme());
		add(eid.getSSP());
	}

	void Dictionary::add(const list<EID> &eids)
	{
		list<EID>::const_iterator iter = eids.begin();

		while (iter != eids.end())
		{
			add(*iter);
			++iter;
		}
	}

	EID Dictionary::get(size_t scheme, size_t ssp)
	{
		char buffer[1024];

		_bytestream.seekg(scheme);
		_bytestream.get(buffer, 1024, '\0');
		string scheme_str(buffer);

		_bytestream.seekg(ssp);
		_bytestream.get(buffer, 1024, '\0');
		string ssp_str(buffer);

		return EID(scheme_str, ssp_str);
	}

	void Dictionary::clear()
	{
		_bytestream.str("");
	}

	size_t Dictionary::getSize() const
	{
		return _bytestream.str().length();
	}

	pair<size_t, size_t> Dictionary::getRef(const EID &eid) const
	{
		const string scheme = eid.getScheme();
		const string ssp = eid.getSSP();
		return make_pair(get(scheme), get(ssp));
	}

	std::ostream &operator<<(std::ostream &stream, const dtn::data::Dictionary &obj)
	{
		dtn::data::SDNV length(obj.getSize());
		stream << length;
		stream << obj._bytestream.str();

		return stream;
	}

	std::istream &operator>>(std::istream &stream, dtn::data::Dictionary &obj)
	{
		dtn::data::SDNV length;
		stream >> length;

		// if the dictionary size if zero throw a exception
		if (length.getValue() <= 0)
			throw dtn::InvalidDataException("Dictionary size is zero!");

		obj._bytestream.str("");
		std::vector<char> data(length.getValue());
		stream.read(&data[0], data.size());
		obj._bytestream.write(&data[0], data.size());

		return stream;
	}
}
}
