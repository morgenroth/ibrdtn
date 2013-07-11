/*
 * EID.cpp
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

#include "ibrdtn/data/EID.h"
#include "ibrdtn/utils/Utils.h"
#include <sstream>
#include <iostream>

namespace dtn
{
	namespace data
	{
		const std::string EID::DEFAULT_SCHEME = "dtn";
		const std::string EID::CBHE_SCHEME = "ipn";

		EID::EID()
		: _scheme(DEFAULT_SCHEME), _ssp("none")
		{
		}

		EID::EID(const std::string &scheme, const std::string &ssp)
		 : _scheme(scheme), _ssp(ssp)
		{
			dtn::utils::Utils::trim(_scheme);
			dtn::utils::Utils::trim(_ssp);

			// TODO: checks for illegal characters
		}

		EID::EID(const std::string &orig_value)
		: _scheme(DEFAULT_SCHEME), _ssp("none")
		{
			if (orig_value.length() == 0) {
				throw dtn::InvalidDataException("given EID is empty!");
			}

			std::string value = orig_value;
			dtn::utils::Utils::trim(value);

			try {
				// search for the delimiter
				size_t delimiter = value.find_first_of(":");

				// jump to default eid if the format is wrong
				if (delimiter == std::string::npos)
					throw dtn::InvalidDataException("wrong EID format: " + value);

				// the scheme is everything before the delimiter
				_scheme = value.substr(0, delimiter);

				// the ssp is everything else
				size_t startofssp = delimiter + 1;
				_ssp = value.substr(startofssp, value.length() - startofssp);

				// do syntax check
				if (_scheme.length() == 0) {
					throw dtn::InvalidDataException("scheme is empty!");
				}

				if (_ssp.length() == 0) {
					throw dtn::InvalidDataException("SSP is empty!");
				}
			} catch (const std::exception&) {
				_scheme = DEFAULT_SCHEME;
				_ssp = "none";
			}
		}

		EID::EID(const dtn::data::Number &node, const dtn::data::Number &application)
		 : _scheme(EID::DEFAULT_SCHEME), _ssp("none")
		{
			if (node == 0)	return;

			std::stringstream ss_ssp;
			ss_ssp << node.toString() << "." << application.toString();
			_ssp = ss_ssp.str();
			_scheme = CBHE_SCHEME;
		}

		EID::~EID()
		{
		}

		EID& EID::operator=(const EID &other)
		{
			_ssp = other._ssp;
			_scheme = other._scheme;
			return *this;
		}

		bool EID::operator==(const EID &other) const
		{
			return (_ssp == other._ssp) && (_scheme == other._scheme);
		}

		bool EID::operator==(const std::string &other) const
		{
			return ((*this) == EID(other));
		}

		bool EID::operator!=(const EID &other) const
		{
			return !((*this) == other);
		}

		EID EID::operator+(const std::string& suffix) const
		{
			return EID(getString() + suffix);
		}

		bool EID::sameHost(const std::string& other) const
		{
			return ( EID(other) == getNode() );
		}

		bool EID::sameHost(const EID &other) const
		{
			return ( other.getNode() == getNode() );
		}

		bool EID::operator<(const EID &other) const
		{
			if (_scheme < other._scheme) return true;
			if (_scheme != other._scheme) return false;

			return _ssp < other._ssp;
		}

		bool EID::operator>(const EID &other) const
		{
			return other < (*this);
		}

		std::string EID::getString() const
		{
			return _scheme + ":" + _ssp;
		}

		std::string EID::getApplication() const throw ()
		{
			size_t first_char = 0;
			char delimiter = '.';

			if (_scheme != EID::CBHE_SCHEME)
			{
				// with a uncompressed bundle header we have another delimiter
				delimiter = '/';

				// first char not "/", e.g. "//node1" -> 2
				first_char = _ssp.find_first_not_of(delimiter);

				// only "/" ? thats bad!
				if (first_char == std::string::npos) {
					return "";
					//throw dtn::InvalidDataException("wrong eid format, ssp: " + _ssp);
				}
			}

			// start of application part
			size_t application_start = _ssp.find_first_of(delimiter, first_char);

			// no application part available
			if (application_start == std::string::npos)
				return "";
			
			// return the application part
			return _ssp.substr(application_start + 1, _ssp.length() - application_start - 1);
		}

		std::string EID::getHost() const throw ()
		{
			size_t first_char = 0;
			char delimiter = '.';

			if (_scheme != EID::CBHE_SCHEME)
			{
				// with a uncompressed bundle header we have another delimiter
				delimiter = '/';

				// first char not "/", e.g. "//node1" -> 2
				first_char = _ssp.find_first_not_of(delimiter);

				// only "/" ? thats bad!
				if (first_char == std::string::npos) {
					return "none";
					// throw dtn::InvalidDataException("wrong eid format, ssp: " + _ssp);
				}
			}

			// start of application part
			size_t application_start = _ssp.find_first_of(delimiter, first_char);

			// no application part available
			if (application_start == std::string::npos)
				return _ssp;

			// return the node part
			return _ssp.substr(0, application_start);
		}

		const std::string& EID::getScheme() const
		{
			return _scheme;
		}

		const std::string& EID::getSSP() const
		{
			return _ssp;
		}

		EID EID::getNode() const throw ()
		{
			return _scheme + ":" + getHost();
		}

		bool EID::hasApplication() const
		{
			// with a compressed bundle header we have another delimiter
			if (_scheme == EID::CBHE_SCHEME)
			{
				return (_ssp.find_first_of(".") != std::string::npos);
			}

			// first char not "/", e.g. "//node1" -> 2
			size_t first_char = _ssp.find_first_not_of("/");

			// only "/" ? thats bad!
			if (first_char == std::string::npos)
				throw dtn::InvalidDataException("wrong eid format, ssp: " + _ssp);

			// start of application part
			size_t application_start = _ssp.find_first_of("/", first_char);

			// no application part available
			if (application_start == std::string::npos)
				return false;

			// return the application part
			return true;
		}

		bool EID::isCompressable() const
		{
			return ((_scheme == DEFAULT_SCHEME) && (_ssp == "none")) || (_scheme == EID::CBHE_SCHEME);
		}

		bool EID::isNone() const
		{
			return (_scheme == DEFAULT_SCHEME) && (_ssp == "none");
		}

		std::string EID::getDelimiter() const
		{
			if (_scheme == EID::CBHE_SCHEME) {
				return ".";
			} else {
				return "/";
			}
		}

		EID::Compressed EID::getCompressed() const
		{
			dtn::data::Number node = 0;
			dtn::data::Number app = 0;

			if (isCompressable())
			{
				std::stringstream ss_node(getHost());
				node.read(ss_node);

				if (hasApplication())
				{
					std::stringstream ss_app(getApplication());
					app.read(ss_app);
				}
			}

			return make_pair(node, app);
		}
	}
}
