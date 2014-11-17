/*
 * BundleFilter.h
 *
 * Copyright (C) 2014 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <jm@m-network.de>
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

#include <ibrdtn/data/Number.h>
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/Block.h>
#include <ibrdtn/data/PrimaryBlock.h>
#include <ibrdtn/data/EID.h>
#include <routing/RoutingExtension.h>
#include <core/Node.h>

#ifndef BUNDLEFILTER_H_
#define BUNDLEFILTER_H_

namespace dtn
{
	namespace core
	{
		class FilterException : public ibrcommon::Exception
		{
		public:
			FilterException(std::string what) throw () : ibrcommon::Exception(what)
			{
			};
		};

		class FilterContext {
		public:
			FilterContext();
			virtual ~FilterContext();

			void setMetaBundle(const dtn::data::MetaBundle &data);
			const dtn::data::MetaBundle& getMetaBundle() const throw (FilterException);

			void setBundle(const dtn::data::Bundle &data);
			const dtn::data::Bundle& getBundle() const throw (FilterException);

			void setPrimaryBlock(const dtn::data::PrimaryBlock &data);
			const dtn::data::PrimaryBlock& getPrimaryBlock() const throw (FilterException);

			const dtn::data::BundleID& getBundleID() const throw (FilterException);

			void setBlock(const dtn::data::Block &block, const dtn::data::Number &size);
			const dtn::data::Block& getBlock() const throw (FilterException);
			dtn::data::Number getBlockLength() const throw (FilterException);

			void setPeer(const dtn::data::EID &endpoint);
			const dtn::data::EID& setPeer() const throw (FilterException);

			void setProtocol(const dtn::core::Node::Protocol &protocol);
			dtn::core::Node::Protocol getProtocol() const throw (FilterException);

			void setRouting(const dtn::routing::RoutingExtension &routing);
			const std::string getRoutingTag() const throw (FilterException);

		private:
			const dtn::data::MetaBundle *_metabundle;
			const dtn::data::Bundle *_bundle;
			const dtn::data::PrimaryBlock *_primaryblock;
			const dtn::data::Block *_block;
			dtn::data::Number _block_length;
			const dtn::data::EID *_peer;
			dtn::core::Node::Protocol _protocol;
			const dtn::routing::RoutingExtension *_routing;
		};

		class BundleFilter {
		public:
			enum ACTION {
				ACCEPT,
				REJECT,
				DROP,
				PASS,
				SKIP
			};

			enum TABLE {
				INPUT,
				OUTPUT,
				ROUTING
			};

			BundleFilter();
			virtual ~BundleFilter();

			/**
			 * Evaluates a context and results in ACCEPT, REJECT, or DROP directive
			 */
			virtual ACTION evaluate(const FilterContext &context) const throw ();

			/**
			 * Filters a bundle with a context. The bundle may be modified during
			 * the processing.
			 */
			virtual ACTION filter(const FilterContext &context, dtn::data::Bundle &bundle) const throw ();

			/**
			 * append a bundle filter at the end of the chain
			 */
			BundleFilter* append(BundleFilter *filter);

		private:
			BundleFilter *_next;
		};

		class AcceptFilter : public BundleFilter
		{
		public:
			virtual ~AcceptFilter() {};
			virtual ACTION evaluate(const FilterContext&) const throw () { return ACCEPT; };
			virtual ACTION filter(const FilterContext&, dtn::data::Bundle&) const throw () { return ACCEPT; };
		};

		class DropFilter : public BundleFilter
		{
		public:
			virtual ~DropFilter() {};
			virtual ACTION evaluate(const FilterContext&) const throw () { return DROP; };
			virtual ACTION filter(const FilterContext&, dtn::data::Bundle&) const throw () { return DROP; };
		};

		class RejectFilter : public BundleFilter
		{
		public:
			virtual ~RejectFilter() {};
			virtual ACTION evaluate(const FilterContext&) const throw () { return REJECT; };
			virtual ACTION filter(const FilterContext&, dtn::data::Bundle&) const throw () { return REJECT; };
		};
	} /* namespace core */
} /* namespace dtn */

#endif /* BUNDLEFILTER_H_ */
