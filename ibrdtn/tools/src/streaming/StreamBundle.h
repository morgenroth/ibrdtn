/*
 * StreamBundle.h
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

#include <ibrdtn/data/Bundle.h>
#include <ibrcommon/data/BLOB.h>

#ifndef STREAMBUNDLE_H_
#define STREAMBUNDLE_H_

class StreamBundle : public dtn::data::Bundle
{
public:
	StreamBundle();
	StreamBundle(const dtn::data::Bundle &b);
	virtual ~StreamBundle();

	/**
	 * Append data to the payload.
	 */
	void append(const char* data, size_t length);

	/**
	 * deletes the hole payload of the bundle
	 */
	void clear();

	/**
	 * returns the size of the current payload
	 * @return
	 */
	size_t size();

private:
	// reference to the BLOB where all data is stored until transmission
	ibrcommon::BLOB::Reference _ref;
};

#endif /* STREAMBUNDLE_H_ */
