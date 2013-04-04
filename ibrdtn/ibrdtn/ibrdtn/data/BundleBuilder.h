/*
 * BundleBuilder.h
 *
 *  Created on: 10.01.2013
 *      Author: morgenro
 */

#ifndef BUNDLEBUILDER_H_
#define BUNDLEBUILDER_H_

#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/Block.h>
#include <iterator>
#include <stdlib.h>

namespace dtn
{
	namespace data
	{
		class BundleBuilder {
		public:
			enum POSITION
			{
				FRONT,
				MIDDLE,
				END
			};

			BundleBuilder(Bundle &target, POSITION alignment = END, int pos = 0);
			virtual ~BundleBuilder();

			/**
			 * clear the content of the target bundle
			 */
			void clear();

			template <class T>
			T& insert(size_t procflags);

			POSITION getAlignment() const;

			dtn::data::Block &insert(dtn::data::ExtensionBlock::Factory &f, size_t procflags);

			/**
			 * Add a block to the bundle.
			 */
			dtn::data::Block& insert(dtn::data::block_t block_type, size_t procflags);

		private:
			Bundle *_target;

			POSITION _alignment;
			int _pos;
		};

		template <class T>
		T& BundleBuilder::insert(size_t procflags)
		{
			switch (_alignment)
			{
			case FRONT:
			{
				T &block = _target->push_front<T>();
				block._procflags = procflags & (~(dtn::data::Block::LAST_BLOCK) | block._procflags);
				return block;
			}

			case END:
			{
				T &block = _target->push_back<T>();
				block._procflags = procflags & (~(dtn::data::Block::LAST_BLOCK) | block._procflags);
				return block;
			}

			default:
				if(_pos <= 0) {
					T &block = _target->push_front<T>();
					block._procflags = procflags & (~(dtn::data::Block::LAST_BLOCK) | block._procflags);
					return block;
				}

				dtn::data::Bundle::iterator it = _target->begin();
				std::advance(it, _pos-1);

				T &block = (it == _target->end()) ? _target->push_back<T>() : _target->insert<T>(it);

				block._procflags = procflags & (~(dtn::data::Block::LAST_BLOCK) | block._procflags);
				return block;
			}
		}
	} /* namespace data */
} /* namespace dtn */
#endif /* BUNDLEBUILDER_H_ */
