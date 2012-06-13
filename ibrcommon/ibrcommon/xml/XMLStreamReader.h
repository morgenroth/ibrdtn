/*
 * XMLStreamReader.h
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
