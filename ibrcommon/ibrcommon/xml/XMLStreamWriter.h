/*
 * XMLStreamWriter.h
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

#ifndef XMLSTREAMWRITER_H_
#define XMLSTREAMWRITER_H_

#include <libxml/xmlwriter.h>
#include <iostream>

namespace ibrcommon
{
	class XMLStreamWriter
	{
	public:
		static int __write_callback(void * context, const char * buffer, int len);
		static int __close_callback(void * context);

		XMLStreamWriter(std::ostream &stream);
		virtual ~XMLStreamWriter();

		void startDocument(const char *encoding);

		void endDocument();

		void startElement(const std::string &name);

		void addAttribute(const std::string &name, const std::string &value);

		void addComment(const std::string &comment);

		void endElement();

		void addData(const std::string &data);

		void addData(const char *data, const int len);

	private:
		std::ostream &_stream;
		xmlOutputBufferPtr _out_buf;
		xmlTextWriterPtr _writer;
	};
}

#endif /* XMLSTREAMWRITER_H_ */
