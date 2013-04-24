/*
 * StaticRoutingExension.h
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

#ifndef STATICROUTINGEXTENSION_H_
#define STATICROUTINGEXTENSION_H_

#include "routing/StaticRoute.h"
#include "routing/RoutingExtension.h"
#include <ibrdtn/data/MetaBundle.h>
#include <ibrcommon/thread/Queue.h>
#include <ibrcommon/thread/Mutex.h>

namespace dtn
{
	namespace routing
	{
		class StaticRoutingExtension : public RoutingExtension, public ibrcommon::JoinableThread
		{
		public:
			StaticRoutingExtension();
			virtual ~StaticRoutingExtension();

			void notify(const dtn::core::Event *evt) throw ();
			void componentUp() throw ();
			void componentDown() throw ();

		protected:
			void run() throw ();
			void __cancellation() throw ();

		private:
			class EIDRoute : public StaticRoute
			{
			public:
				EIDRoute(const dtn::data::EID &match, const dtn::data::EID &nexthop, size_t expiretime = 0);
				virtual ~EIDRoute();

				bool match(const dtn::data::EID &eid) const;
				const dtn::data::EID& getDestination() const;

				/**
				 * Describe this route as a one-line-string.
				 * @return
				 */
				const std::string toString() const;

				size_t getExpiration() const;

			private:
				const dtn::data::EID _nexthop;
				const dtn::data::EID _match;
				const size_t expiretime;
			};

			class Task
			{
			public:
				virtual ~Task() {};
				virtual std::string toString() = 0;
			};

			class SearchNextBundleTask : public Task
			{
			public:
				SearchNextBundleTask(const dtn::data::EID &eid);
				virtual ~SearchNextBundleTask();

				virtual std::string toString();

				const dtn::data::EID eid;
			};

			class ProcessBundleTask : public Task
			{
			public:
				ProcessBundleTask(const dtn::data::MetaBundle &meta, const dtn::data::EID &origin);
				virtual ~ProcessBundleTask();

				virtual std::string toString();

				const dtn::data::MetaBundle bundle;
				const dtn::data::EID origin;
			};

			class ClearRoutesTask : public Task
			{
			public:
				ClearRoutesTask();
				virtual ~ClearRoutesTask();
				virtual std::string toString();
			};

			class RouteChangeTask : public Task
			{
			public:
				enum CHANGE_TYPE
				{
					ROUTE_ADD = 0,
					ROUTE_DEL = 1
				};

				RouteChangeTask(CHANGE_TYPE type, StaticRoute *route);
				virtual ~RouteChangeTask();

				virtual std::string toString();

				CHANGE_TYPE type;
				StaticRoute *route;
			};

			class ExpireTask : public Task
			{
			public:
				ExpireTask(size_t timestamp);
				virtual ~ExpireTask();

				virtual std::string toString();

				size_t timestamp;
			};

			/**
			 * hold queued tasks for later processing
			 */
			ibrcommon::Queue<StaticRoutingExtension::Task* > _taskqueue;

			/**
			 * static list of routes
			 */
			std::list<StaticRoute*> _routes;
			ibrcommon::Mutex _expire_lock;
			size_t next_expire;
		};
	}
}

#endif /* STATICROUTINGEXTENSION_H_ */
