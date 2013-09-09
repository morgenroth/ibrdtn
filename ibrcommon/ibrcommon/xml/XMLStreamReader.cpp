/*
 * XMLStreamReader.cpp
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

#include "ibrcommon/xml/XMLStreamReader.h"
#include "ibrcommon/Exceptions.h"
#include <string.h>

namespace ibrcommon
{
	XMLStreamReader::XMLStreamReader(XMLStreamHandler &callback)
	 : _callback(callback)
	{ }

	XMLStreamReader::~XMLStreamReader()
	{ }

	void XMLStreamReader::__startDocument(void *data)
	{
		XMLStreamHandler *cb = static_cast<XMLStreamHandler*>(data);
		cb->startDocument();
	}

	void XMLStreamReader::__endDocument(void *data)
	{
		XMLStreamHandler *cb = static_cast<XMLStreamHandler*>(data);
		cb->endDocument();
	}

	void XMLStreamReader::__startElement(void *data, const xmlChar *fullname, const xmlChar **ats)
	{
		XMLStreamHandler *cb = static_cast<XMLStreamHandler*>(data);
		std::map<std::string, std::string> attrs;

		if (ats != NULL)
		{
			int i = 0;
			while (ats[i] != NULL)
			{
				attrs[std::string((const char*)ats[i])] = std::string((const char*)ats[i+1]);
				i += 2;
			}
		}

		const std::string name((const char*)fullname);
		cb->startElement(name, attrs);
	}

	void XMLStreamReader::__endElement(void *data, const xmlChar *fullname)
	{
		XMLStreamHandler *cb = static_cast<XMLStreamHandler*>(data);
		const std::string name((const char*)fullname);
		cb->endElement(name);
	}

	void XMLStreamReader::__characters(void *data, const xmlChar *ch, int len)
	{
		XMLStreamHandler *cb = static_cast<XMLStreamHandler*>(data);
		cb->characters((const char*)ch, len);
	}

	void XMLStreamReader::parse(std::istream &stream)
	{
		xmlSAXHandler my_handler;
		::memset(&my_handler, '\0', sizeof(xmlSAXHandler));

		my_handler.startDocument = __startDocument;
		my_handler.endDocument = __endDocument;
		my_handler.startElement = __startElement;
		my_handler.endElement = __endElement;
		my_handler.characters = __characters;

		int res = 0, size = 1024;
		char chars[1024];
		xmlParserCtxtPtr ctx = xmlCreatePushParserCtxt(&my_handler, &_callback, chars, res, NULL);

		while (stream.good())
		{
			stream.read(chars, size);

			if (stream.eof())
			{
				xmlParseChunk(ctx, chars, static_cast<int>(stream.gcount()), 1);
			}
			else
			{
				xmlParseChunk(ctx, chars, static_cast<int>(stream.gcount()), 0);
			}
		}
	}

}
