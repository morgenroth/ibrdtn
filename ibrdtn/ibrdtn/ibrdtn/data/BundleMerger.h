/*
 * BundleMerger.h
 *
 *  Created on: 25.01.2010
 *      Author: morgenro
 */

#ifndef BUNDLEMERGER_H_
#define BUNDLEMERGER_H_

#include "ibrcommon/data/BLOB.h"
#include "ibrdtn/data/Bundle.h"
#include <set>

namespace dtn
{
	namespace data
	{
		class BundleMerger
		{
		public:
			class Chunk
			{
			public:
				Chunk(size_t, size_t);
				virtual ~Chunk();

				bool operator<(const Chunk &other) const;

				size_t offset;
				size_t length;

				static bool isComplete(size_t length, const std::set<Chunk> &chunks);
			};

			class Container
			{
			public:
				Container(ibrcommon::BLOB::Reference &ref);
				virtual ~Container();
				bool isComplete();

				Bundle& getBundle();

				friend Container &operator<<(Container &c, dtn::data::Bundle &obj);

			private:
				bool contains(size_t offset, size_t length) const;
				void add(size_t offset, size_t length);

				dtn::data::Bundle _bundle;
				ibrcommon::BLOB::Reference _blob;
				bool _initialized;

				std::set<Chunk> _chunks;
				size_t _appdatalength;
			};

			static Container getContainer();
			static Container getContainer(ibrcommon::BLOB::Reference &ref);
		};
	}
}


#endif /* BUNDLEMERGER_H_ */
