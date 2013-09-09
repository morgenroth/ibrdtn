/*
 * XMLStreamWriter.cpp
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

#include "ibrcommon/xml/XMLStreamWriter.h"
#include "ibrcommon/Exceptions.h"

namespace ibrcommon
{
	int XMLStreamWriter::__write_callback(void * context, const char * buffer, int len)
	{
		XMLStreamWriter *writer = static_cast<XMLStreamWriter*>(context);
		writer->_stream.write(buffer, len);
		return len;
	}

	int XMLStreamWriter::__close_callback(void*)
	{
		//XMLStreamWriter *writer = static_cast<XMLStreamWriter*>(context);
		return 0;
	}

	XMLStreamWriter::XMLStreamWriter(std::ostream &stream)
	 : _stream(stream)
	{
		_out_buf = xmlOutputBufferCreateIO(__write_callback, __close_callback, this, NULL);
		_writer = xmlNewTextWriter(_out_buf);
	}

	XMLStreamWriter::~XMLStreamWriter()
	{
		xmlFreeTextWriter(_writer);
	}

	void XMLStreamWriter::startDocument(const char *encoding)
	{
		if (xmlTextWriterStartDocument(_writer, NULL, encoding, NULL) < 0) {
			throw ibrcommon::Exception("XMLStreamWriter: Error at xmlTextWriterStartDocument\n");
		}
	}

	void XMLStreamWriter::endDocument()
	{
		if (xmlTextWriterEndDocument(_writer) < 0) {
			throw ibrcommon::Exception("XMLStreamWriter: Error at xmlTextWriterEndDocument\n");
		}
	}

	void XMLStreamWriter::startElement(const std::string &name)
	{
		if (xmlTextWriterStartElement(_writer, BAD_CAST name.c_str()) < 0) {
			throw ibrcommon::Exception("XMLStreamWriter: Error at xmlTextWriterStartElement\n");
		}
	}

	void XMLStreamWriter::addAttribute(const std::string &name, const std::string &value)
	{
		if (xmlTextWriterWriteAttribute(_writer, BAD_CAST name.c_str(), BAD_CAST value.c_str()) < 0) {
			throw ibrcommon::Exception("XMLStreamWriter: Error at xmlTextWriterWriteAttribute\n");
		}
	}

	void XMLStreamWriter::addComment(const std::string &comment)
	{
		if (xmlTextWriterWriteComment(_writer, BAD_CAST comment.c_str()) < 0)
		{
			throw ibrcommon::Exception("XMLStreamWriter: Error at xmlTextWriterWriteComment\n");
		}
	}

	void XMLStreamWriter::endElement()
	{
		if (xmlTextWriterEndElement(_writer) < 0)
		{
			throw ibrcommon::Exception("XMLStreamWriter: Error at xmlTextWriterEndElement\n");
		}
	}

	void XMLStreamWriter::addData(const std::string &data)
	{
		if (xmlTextWriterWriteRaw(_writer, BAD_CAST data.c_str()) < 0)
		{
			throw ibrcommon::Exception("XMLStreamWriter: Error at xmlTextWriterWriteRaw\n");
		}
	}

	void XMLStreamWriter::addData(const char *data, const int len)
	{
		if (xmlTextWriterWriteRawLen(_writer, BAD_CAST data, len) < 0)
		{
			throw ibrcommon::Exception("XMLStreamWriter: Error at xmlTextWriterWriteRaw\n");
		}
	}
}
