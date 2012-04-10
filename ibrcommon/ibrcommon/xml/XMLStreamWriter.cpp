/*
 * XMLStreamWriter.cpp
 *
 *  Created on: 01.06.2011
 *      Author: morgenro
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
	};

	int XMLStreamWriter::__close_callback(void * context)
	{
		//XMLStreamWriter *writer = static_cast<XMLStreamWriter*>(context);
		return 0;
	};

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

	void XMLStreamWriter::addData(const char *data, const size_t len)
	{
		if (xmlTextWriterWriteRawLen(_writer, BAD_CAST data, len) < 0)
		{
			throw ibrcommon::Exception("XMLStreamWriter: Error at xmlTextWriterWriteRaw\n");
		}
	}
}
