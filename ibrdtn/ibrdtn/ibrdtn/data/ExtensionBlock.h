/*
 * ExtensionBlock.h
 *
 *  Created on: 26.05.2010
 *      Author: morgenro
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
