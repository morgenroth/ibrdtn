/*
 * XMLStreamHandler.h
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
