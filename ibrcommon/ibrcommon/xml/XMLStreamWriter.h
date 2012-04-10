/*
 * XMLStreamWriter.h
 *
 *  Created on: 01.06.2011
 *      Author: morgenro
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

		void addData(const char *data, const size_t len);

	private:
		std::ostream &_stream;
		xmlOutputBufferPtr _out_buf;
		xmlTextWriterPtr _writer;
	};
}

#endif /* XMLSTREAMWRITER_H_ */
