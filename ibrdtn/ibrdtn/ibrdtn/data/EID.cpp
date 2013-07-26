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

		void EID::extractDTN(const std::string &ssp, std::string &node, std::string &application)
		{
			size_t first_char = 0;
			const char delimiter = '/';

			// first char not "/", e.g. "//node1" -> 2
			first_char = ssp.find_first_not_of(delimiter);

			// only "/" ? thats bad!
			if (first_char == std::string::npos) {
				node = "none";
				application = "";
				return;
			}

			// start of application part
			size_t application_start = ssp.find_first_of(delimiter, first_char);

			// no application part available
			if (application_start == std::string::npos) {
				node = ssp;
				application = "";
				return;
			}

			// set the node part
			node = ssp.substr(0, application_start);

			// set the application part
			application = ssp.substr(application_start + 1, ssp.length() - application_start - 1);
		}

		EID::EID()
		: _scheme_type(SCHEME_DTN), _scheme(DEFAULT_SCHEME), _ssp("none"), _application(), _cbhe_node(0), _cbhe_application(0)
		{
		}

		EID::EID(const Scheme scheme_type, const std::string &scheme, const std::string &ssp, const std::string &application)
		: _scheme_type(scheme_type), _scheme(scheme), _ssp(ssp), _application(application), _cbhe_node(0), _cbhe_application(0)
		{
			if (scheme_type == SCHEME_CBHE) {
				throw dtn::InvalidDataException("This constructor does not work for CBHE schemes");
			}
		}

		EID::EID(const std::string &scheme, const std::string &ssp)
		 : _scheme_type(SCHEME_EXTENDED), _scheme(scheme), _ssp(ssp), _application(), _cbhe_node(0), _cbhe_application(0)
		{
			dtn::utils::Utils::trim(_scheme);
			dtn::utils::Utils::trim(_ssp);

			// resolve scheme
			_scheme_type = resolveScheme(_scheme);

			switch (_scheme_type) {
			case SCHEME_CBHE:
				// extract CBHE numbers
				extractCBHE(ssp, _cbhe_node, _cbhe_application);
				break;

			case SCHEME_DTN:
				extractDTN(ssp, _ssp, _application);
				break;

			default:
				break;
			}
		}

		EID::EID(const std::string &orig_value)
		: _scheme_type(SCHEME_DTN), _scheme(DEFAULT_SCHEME), _ssp("none"), _application(), _cbhe_node(0), _cbhe_application(0)
		{
			try {
				if (orig_value.length() == 0) {
					throw dtn::InvalidDataException("given EID is empty!");
				}

				std::string value = orig_value;
				dtn::utils::Utils::trim(value);

				// search for the delimiter
				size_t delimiter = value.find_first_of(":");

				// jump to default eid if the format is wrong
				if (delimiter == std::string::npos)
					throw dtn::InvalidDataException("wrong EID format: " + value);

				// the scheme is everything before the delimiter
				_scheme = value.substr(0, delimiter);

				// the ssp is everything else
				size_t startofssp = delimiter + 1;
				const std::string ssp = value.substr(startofssp, value.length() - delimiter + 1);

				// do syntax check
				if (_scheme.length() == 0) {
					throw dtn::InvalidDataException("scheme is empty!");
				}

				if (ssp.length() == 0) {
					throw dtn::InvalidDataException("SSP is empty!");
				}

				// resolve scheme
				_scheme_type = resolveScheme(_scheme);

				switch (_scheme_type) {
				case SCHEME_CBHE:
					// extract CBHE numbers
					extractCBHE(ssp, _cbhe_node, _cbhe_application);
					break;

				case SCHEME_DTN:
					// extract DTN scheme node/application
					extractDTN(ssp, _ssp, _application);
					break;

				default:
					_ssp = ssp;
					break;
				}
			} catch (const std::exception&) {
				_scheme_type = SCHEME_DTN;
				_scheme = DEFAULT_SCHEME;
				_ssp = "none";
			}
		}

		EID::EID(const dtn::data::Number &node, const dtn::data::Number &application)
		 : _scheme_type(SCHEME_CBHE), _scheme(EID::CBHE_SCHEME), _ssp("none"), _application(), _cbhe_node(node), _cbhe_application(application)
		{
			// set dtn:none if the node is zero
			if (node == 0) {
				_scheme_type = SCHEME_DTN;
				_scheme = EID::DEFAULT_SCHEME;
			}
		}

		EID::~EID()
		{
		}

		bool EID::operator==(const EID &other) const
		{
			if (_scheme_type != other._scheme_type) return false;

			switch (_scheme_type) {
			case SCHEME_CBHE:
				return (_cbhe_node == other._cbhe_node) && (_cbhe_application == other._cbhe_application);

			case SCHEME_DTN:
				return (_ssp == other._ssp) && (_application == other._application);

			default:
				return (_scheme == other._scheme) && (_ssp == other._ssp);
			}
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
			return sameHost( EID(other) );
		}

		bool EID::sameHost(const EID &other) const
		{
			if (_scheme_type != other._scheme_type) return false;

			switch (_scheme_type) {
			case SCHEME_CBHE:
				return _cbhe_node == other._cbhe_node;

			case SCHEME_DTN:
				return _ssp == other._ssp;

			default:
				return (_scheme == other._scheme) && (_ssp == other._ssp);
			}
		}

		bool EID::operator<(const EID &other) const
		{
			if (_scheme_type < other._scheme_type) return true;
			if (_scheme_type != other._scheme_type) return false;

			switch (_scheme_type) {
			case SCHEME_CBHE:
				if (_cbhe_node < other._cbhe_node) return true;
				if (_cbhe_node != other._cbhe_node) return false;

				return (_cbhe_application < other._cbhe_application);

			case SCHEME_DTN:
				if (_ssp < other._ssp) return true;
				if (_ssp != other._ssp) return false;

				return (_application < other._application);

			default:
				if (_scheme < other._scheme) return true;
				if (_scheme != other._scheme) return false;

				return (_ssp < other._ssp);
			}
		}

		bool EID::operator>(const EID &other) const
		{
			return other < (*this);
		}

		std::string EID::getString() const
		{
			switch (_scheme_type) {
			case SCHEME_CBHE:
			{
				std::stringstream ss;
				ss << "ipn:" << _cbhe_node.get<size_t>();

				if (_cbhe_application > 0) {
					ss << "." << _cbhe_application.get<size_t>();
				}

				return ss.str();
			}

			case SCHEME_DTN:
				return "dtn:" + _ssp + "/" + _application;

			default:
				return _scheme + ":" + _ssp;
			}
		}

		std::string EID::getApplication() const throw ()
		{
			switch (_scheme_type) {
			case SCHEME_CBHE:
			{
				if (_cbhe_application > 0) {
					return _cbhe_application.toString();
				} else {
					return "";
				}
			}

			case SCHEME_DTN:
				return _application;

			default:
				return _ssp;
			}
		}

		std::string EID::getHost() const throw ()
		{
			switch (_scheme_type) {
			case SCHEME_CBHE:
				return _cbhe_node.toString();
			case SCHEME_DTN:
				return _ssp;
			default:
				return _ssp;
			}
		}

		const std::string EID::getScheme() const
		{
			return _scheme;
		}

		const std::string EID::getSSP() const
		{
			switch (_scheme_type) {
			case SCHEME_CBHE:
			{
				std::stringstream ss;
				ss << _cbhe_node.get<size_t>();

				if (_cbhe_application > 0) {
					ss << "." << _cbhe_application.get<size_t>();
				}

				return ss.str();
			}
			case SCHEME_DTN:
				return _ssp + "/" + _application;
			default:
				return _ssp;
			}
		}

		EID EID::getNode() const throw ()
		{
			switch (_scheme_type) {
			case SCHEME_CBHE:
				return EID(_cbhe_node, 0);
			case SCHEME_DTN:
				return EID(_scheme_type, _scheme, _ssp, "");
			default:
				return EID(_scheme_type, _scheme, _ssp, "");
			}
		}

		bool EID::hasApplication() const
		{
			switch (_scheme_type) {
			case SCHEME_CBHE:
				return (_cbhe_application > 0);
			case SCHEME_DTN:
				return _application != "";
			default:
				return true;
			}
		}

		bool EID::isCompressable() const
		{
			return ((_scheme_type == SCHEME_CBHE) || ((_scheme_type == SCHEME_DTN) && (_ssp == "none")));
		}

		bool EID::isNone() const
		{
			return (_scheme_type == SCHEME_DTN) && (_ssp == "none");
		}

		std::string EID::getDelimiter() const
		{
			if (_scheme_type == EID::SCHEME_CBHE) {
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
