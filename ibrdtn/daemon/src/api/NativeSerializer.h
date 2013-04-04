/*
 * NativeSerializer.h
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
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

#ifndef NATIVESERIALIZER_H_
#define NATIVESERIALIZER_H_

#include "api/NativeSerializerCallback.h"
#include <ibrdtn/data/PrimaryBlock.h>
#include <ibrdtn/data/Bundle.h>
#include <streambuf>
#include <vector>
#include <iostream>

namespace dtn
{
	namespace api
	{
		class NativeCallbackStream : public std::basic_streambuf<char, std::char_traits<char> > {
		public:
			NativeCallbackStream(NativeSerializerCallback &cb);
			virtual ~NativeCallbackStream();

		protected:
			virtual int sync();
			virtual std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type = std::char_traits<char>::eof());

		private:
			// output buffer
			std::vector<char> _output_buf;

			NativeSerializerCallback &_callback;
		};

		class NativeSerializer {
		public:
			enum DataMode {
				BUNDLE_HEADER,
				BUNDLE_FULL,
				BUNDLE_INFO
			};

			NativeSerializer(NativeSerializerCallback &cb, DataMode mode);
			virtual ~NativeSerializer();

			NativeSerializer &operator<<(const dtn::data::Bundle &obj);

		private:
			NativeSerializerCallback &_callback;
			DataMode _mode;
		};
	} /* namespace api */
} /* namespace dtn */
#endif /* NATIVESERIALIZER_H_ */
