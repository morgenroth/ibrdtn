/*
 * XMLStreamHandler.h
 *
 *  Created on: 01.06.2011
 *      Author: morgenro
 */

#ifndef XMLSTREAMHANDLER_H_
#define XMLSTREAMHANDLER_H_

#include <string>
#include <map>

namespace ibrcommon
{
	class XMLStreamHandler
	{
	public:
		virtual ~XMLStreamHandler() {};
		virtual void startDocument() = 0;
		virtual void endDocument() = 0;
		virtual void startElement(const std::string &name, const std::map<std::string, std::string> &attr) = 0;
		virtual void endElement(const std::string &name) = 0;
		virtual void characters(const char *ch, int len) = 0;
	};
}

#endif /* XMLSTREAMHANDLER_H_ */
