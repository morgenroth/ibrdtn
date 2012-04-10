/*
 * XMLStreamReader.h
 *
 *  Created on: 01.06.2011
 *      Author: morgenro
 */

#ifndef XMLSTREAMREADER_H_
#define XMLSTREAMREADER_H_

#include "ibrcommon/xml/XMLStreamHandler.h"
#include <libxml/parser.h>
#include <libxml/encoding.h>
#include <iostream>

namespace ibrcommon
{
	class XMLStreamReader
	{
	public:
		XMLStreamReader(XMLStreamHandler &callback);
		virtual ~XMLStreamReader();
		static void __startDocument(void *data);
		static void __endDocument(void *data);
		static void __startElement(void *data, const xmlChar *fullname, const xmlChar **ats);
		static void __endElement(void *data, const xmlChar *fullname);
		static void __characters(void *data, const xmlChar *ch, int len);
		void parse(std::istream &stream);

	private:
		XMLStreamHandler &_callback;
	};
}

#endif /* XMLSTREAMREADER_H_ */
