/*
 * ExtensionBlock.h
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

#ifndef EXTENSIONBLOCK_H_
#define EXTENSIONBLOCK_H_

#include "ibrdtn/data/Block.h"
#include <ibrcommon/data/BLOB.h>
#include <map>

namespace dtn
{
	namespace data
	{
		class ExtensionBlock : public Block
		{
		public:
			class Factory
			{
			public:
				virtual dtn::data::Block* create() = 0;

				static Factory& get(char type) throw (ibrcommon::Exception);

			protected:
				Factory(char type);
				virtual ~Factory();

			private:
				char _type;
			};

			ExtensionBlock();
			ExtensionBlock(ibrcommon::BLOB::Reference ref);
			virtual ~ExtensionBlock();

			ibrcommon::BLOB::Reference getBLOB() const;

			void setType(char type);

			virtual size_t getLength() const;
			virtual std::ostream &serialize(std::ostream &stream, size_t &length) const;
			virtual std::istream &deserialize(std::istream &stream, const size_t length);

		protected:
			class FactoryList
			{
			public:
				FactoryList();
				virtual ~FactoryList();

				Factory& get(char type) throw (ibrcommon::Exception);
				void add(char type, Factory *f) throw (ibrcommon::Exception);
				void remove(char type);

				static void initialize();

			private:
				std::map<char, Factory*> fmap;
			};

			static FactoryList *factories;

		private:
			ibrcommon::BLOB::Reference _blobref;
		};
	}
}

#endif /* EXTENSIONBLOCK_H_ */
