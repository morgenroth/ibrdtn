/*
 * StaticRegexRoute.cpp
 *
 *  Created on: 28.11.2012
 *      Author: morgenro
 */

#include "routing/StaticRegexRoute.h"
#include "routing/StaticRouteChangeEvent.h"
#include <ibrcommon/Logger.h>
#include <typeinfo>

namespace dtn
{
	namespace routing
	{
		StaticRegexRoute::StaticRegexRoute(const std::string &regex, const dtn::data::EID &dest)
			: _dest(dest), _regex_str(regex), _invalid(false), _expire(0)
		{
			if ( regcomp(&_regex, regex.c_str(), 0) )
			{
				IBRCOMMON_LOGGER_TAG("StaticRegexRoute", error) << "Could not compile regex: " << regex << IBRCOMMON_LOGGER_ENDL;
				_invalid = true;
			}
		}

		StaticRegexRoute::~StaticRegexRoute()
		{
			if (!_invalid) {
				regfree(&_regex);
			}
		}

		StaticRegexRoute::StaticRegexRoute(const StaticRegexRoute &obj)
			: _dest(obj._dest), _regex_str(obj._regex_str), _invalid(obj._invalid)
		{
			if ( regcomp(&_regex, _regex_str.c_str(), 0) )
			{
				_invalid = true;
			}
		}

		StaticRegexRoute& StaticRegexRoute::operator=(const StaticRegexRoute &obj)
		{
			if (!_invalid)
			{
				regfree(&_regex);
			}

			_dest = obj._dest;
			_regex_str = obj._regex_str;
			_invalid = obj._invalid;

			if (!_invalid)
			{
				if ( regcomp(&_regex, obj._regex_str.c_str(), 0) )
				{
					IBRCOMMON_LOGGER_TAG("StaticRegexRoute", error) << "Could not compile regex: " << _regex_str << IBRCOMMON_LOGGER_ENDL;
					_invalid = true;
				}
			}

			return *this;
		}

		bool StaticRegexRoute::match(const dtn::data::EID &eid) const
		{
			if (_invalid) return false;

			const std::string dest = eid.getString();

			// test against the regular expression
			int reti = regexec(&_regex, dest.c_str(), 0, NULL, 0);

			if( !reti )
			{
				// the expression match
				return true;
			}
			else if( reti == REG_NOMATCH )
			{
				// the expression not match
				return false;
			}
			else
			{
				char msgbuf[100];
				regerror(reti, &_regex, msgbuf, sizeof(msgbuf));
				IBRCOMMON_LOGGER_TAG("StaticRegexRoute", error) << "Regex match failed: " << std::string(msgbuf) << IBRCOMMON_LOGGER_ENDL;
				return false;
			}
		}

		const dtn::data::EID& StaticRegexRoute::getDestination() const
		{
			return _dest;
		}

		const std::string StaticRegexRoute::toString() const
		{
			std::stringstream ss;
			ss << _regex_str << " => " << _dest.getString();
			return ss.str();
		}

		const dtn::data::Timestamp& StaticRegexRoute::getExpiration() const
		{
			return _expire;
		}

		void StaticRegexRoute::raiseExpired() const
		{
			dtn::routing::StaticRouteChangeEvent::raiseEvent(dtn::routing::StaticRouteChangeEvent::ROUTE_EXPIRED, _dest, _regex_str);
		}

		bool StaticRegexRoute::equals(const StaticRoute &route) const
		{
			try {
				const StaticRegexRoute &r = dynamic_cast<const StaticRegexRoute&>(route);
				return (_regex_str == r._regex_str) && (_dest == r._dest);
			} catch (const std::bad_cast&) {
				return false;
			}
		}
	}
}
