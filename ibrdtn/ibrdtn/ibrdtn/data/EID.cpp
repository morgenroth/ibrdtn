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

		EID::Scheme EID::resolveScheme(const std::string &s)
		{
			if (DEFAULT_SCHEME == s) {
				return SCHEME_DTN;
			}
			else if (CBHE_SCHEME == s) {
				return SCHEME_CBHE;
			}
			else {
				return SCHEME_EXTENDED;
			}
		}

		void EID::extractCBHE(const std::string &ssp, Number &node, Number &app)
		{
			char delimiter = '\0';

			size_t n = 0;
			size_t a = 0;

			std::stringstream ss(ssp);
			ss >> n;
			ss.get(delimiter);
			ss >> a;

			node = n;
			app = a;
		}

		EID::EID()
		: _scheme(SCHEME_DTN), _scheme_ext(DEFAULT_SCHEME), _ssp("none"), _cbhe_node(0), _cbhe_application(0)
		{
		}

		EID::EID(const std::string &scheme, const std::string &ssp)
		 : _scheme(SCHEME_DTN), _scheme_ext(scheme), _ssp(ssp), _cbhe_node(0), _cbhe_application(0)
		{
			dtn::utils::Utils::trim(_scheme_ext);
			dtn::utils::Utils::trim(_ssp);

			// resolve scheme
			_scheme = resolveScheme(_scheme_ext);

			// extract CBHE numbers
			if (_scheme == SCHEME_CBHE) {
				extractCBHE(ssp, _cbhe_node, _cbhe_application);
			}

			// TODO: checks for illegal characters
		}

		EID::EID(const std::string &orig_value)
		: _scheme(SCHEME_DTN), _scheme_ext(DEFAULT_SCHEME), _ssp("none"), _cbhe_node(0), _cbhe_application(0)
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
				_scheme_ext = value.substr(0, delimiter);

				// the ssp is everything else
				size_t startofssp = delimiter + 1;
				_ssp = value.substr(startofssp, value.length() - startofssp);

				// do syntax check
				if (_scheme_ext.length() == 0) {
					throw dtn::InvalidDataException("scheme is empty!");
				}

				if (_ssp.length() == 0) {
					throw dtn::InvalidDataException("SSP is empty!");
				}

				// resolve scheme
				_scheme = resolveScheme(_scheme_ext);

				// IPN conversion
				if (_scheme == SCHEME_CBHE) {
					extractCBHE(_ssp, _cbhe_node, _cbhe_application);
				}
			} catch (const std::exception&) {
				_scheme = SCHEME_DTN;
				_scheme_ext = DEFAULT_SCHEME;
				_ssp = "none";
			}
		}

		EID::EID(const dtn::data::Number &node, const dtn::data::Number &application)
		 : _scheme(SCHEME_DTN), _scheme_ext(EID::DEFAULT_SCHEME), _ssp("none"), _cbhe_node(node), _cbhe_application(application)
		{
			if (node == 0)	return;

			std::stringstream ss_ssp;

			// add node ID
			ss_ssp << node.get<size_t>();

			// add node ID if larger than zero
			if (application > 0) {
				ss_ssp << "." << application.get<size_t>();
			}

			_ssp = ss_ssp.str();
			_scheme = SCHEME_CBHE;
			_scheme_ext = CBHE_SCHEME;
		}

		EID::~EID()
		{
		}

		EID& EID::operator=(const EID &other)
		{
			_scheme = other._scheme;
			_ssp = other._ssp;
			_scheme_ext = other._scheme_ext;
			_cbhe_node = other._cbhe_node;
			_cbhe_application = other._cbhe_application;
			return *this;
		}

		bool EID::operator==(const EID &other) const
		{
			if (_scheme != other._scheme) return false;

			// optimized comparison when using CBHE scheme
			if (_scheme == SCHEME_CBHE) {
				if (_cbhe_node != other._cbhe_node) return false;
				return (_cbhe_application == other._cbhe_application);
			}

			return (_ssp == other._ssp) && (_scheme_ext == other._scheme_ext);
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
			// optimized comparison when using CBHE scheme
			if ((_scheme == SCHEME_CBHE) && (_scheme == other._scheme)) {
				if (_cbhe_node < other._cbhe_node) return true;
				if (_cbhe_node != other._cbhe_node) return false;

				return (_cbhe_application < other._cbhe_application);
			}

			if (_scheme_ext < other._scheme_ext) return true;
			if (_scheme_ext != other._scheme_ext) return false;

			return _ssp < other._ssp;
		}

		bool EID::operator>(const EID &other) const
		{
			return other < (*this);
		}

		std::string EID::getString() const
		{
			return _scheme_ext + ":" + _ssp;
		}

		std::string EID::getApplication() const throw ()
		{
			if (_scheme == EID::SCHEME_CBHE)
			{
				if (_cbhe_application > 0) {
					return _cbhe_application.toString();
				} else {
					return "";
				}
			}

			size_t first_char = 0;
			const char delimiter = '/';

			// first char not "/", e.g. "//node1" -> 2
			first_char = _ssp.find_first_not_of(delimiter);

			// only "/" ? thats bad!
			if (first_char == std::string::npos) {
				return "";
				//throw dtn::InvalidDataException("wrong eid format, ssp: " + _ssp);
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
			if (_scheme == EID::SCHEME_CBHE)
			{
				return _cbhe_node.toString();
			}

			size_t first_char = 0;
			const char delimiter = '/';

			// first char not "/", e.g. "//node1" -> 2
			first_char = _ssp.find_first_not_of(delimiter);

			// only "/" ? thats bad!
			if (first_char == std::string::npos) {
				return "none";
				// throw dtn::InvalidDataException("wrong eid format, ssp: " + _ssp);
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
			return _scheme_ext;
		}

		const std::string& EID::getSSP() const
		{
			return _ssp;
		}

		EID EID::getNode() const throw ()
		{
			return _scheme_ext + ":" + getHost();
		}

		bool EID::hasApplication() const
		{
			// with a compressed bundle header we have another delimiter
			if (_scheme == EID::SCHEME_CBHE)
			{
				return (_cbhe_application > 0);
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
			return ((_scheme == SCHEME_CBHE) || ((_scheme == SCHEME_DTN) && (_ssp == "none")));
		}

		bool EID::isNone() const
		{
			return (_scheme == SCHEME_DTN) && (_ssp == "none");
		}

		std::string EID::getDelimiter() const
		{
			if (_scheme == EID::SCHEME_CBHE) {
				return ".";
			} else {
				return "/";
			}
		}

		EID::Compressed EID::getCompressed() const
		{
			if (isCompressable())
			{
				return make_pair(_cbhe_node, _cbhe_application);
			}

			return make_pair(0, 0);
		}
	}
}
