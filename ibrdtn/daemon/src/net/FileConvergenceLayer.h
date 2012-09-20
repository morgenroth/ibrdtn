/*
 * FileConvergenceLayer.h
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

#include "Component.h"
#include "net/ConvergenceLayer.h"
#include "core/Node.h"
#include "core/EventReceiver.h"
#include <ibrdtn/data/BundleList.h>
#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/Queue.h>

#ifndef FILECONVERGENCELAYER_H_
#define FILECONVERGENCELAYER_H_

namespace dtn
{
	namespace net
	{

		class FileConvergenceLayer : public dtn::net::ConvergenceLayer, public dtn::daemon::IndependentComponent, public dtn::core::EventReceiver, public dtn::data::BundleList::Listener
		{
		public:
			FileConvergenceLayer();
			virtual ~FileConvergenceLayer();

			void raiseEvent(const dtn::core::Event *evt);

			dtn::core::Node::Protocol getDiscoveryProtocol() const;

			void open(const dtn::core::Node&);
			void queue(const dtn::core::Node &n, const ConvergenceLayer::Job &job);

			const std::string getName() const;

		protected:
			void componentUp();
			void componentRun();
			void componentDown();

			void __cancellation();

		private:
			class Task
			{
			public:
				enum Action
				{
					TASK_LOAD,
					TASK_STORE
				};

				Task(Action a, const dtn::core::Node &n);
				virtual ~Task();

				const Action action;
				const dtn::core::Node node;
			};

			class StoreBundleTask : public Task
			{
			public:
				StoreBundleTask(const dtn::core::Node &n, const ConvergenceLayer::Job &j);
				virtual ~StoreBundleTask();

				const ConvergenceLayer::Job job;
			};

			void replyHandshake(const dtn::data::Bundle &bundle, std::list<dtn::data::MetaBundle>&);

			ibrcommon::Mutex _blacklist_mutex;
			dtn::data::BundleList _blacklist;
			ibrcommon::Queue<Task*> _tasks;

			static ibrcommon::File getPath(const dtn::core::Node&);
			static std::list<dtn::data::MetaBundle> scan(const ibrcommon::File &path);
			static void load(const dtn::core::Node&);

		};

	} /* namespace net */
} /* namespace dtn */
#endif /* FILECONVERGENCELAYER_H_ */
