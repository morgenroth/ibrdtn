/*
 * Event.h
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

#ifndef EVENT_H_
#define EVENT_H_

#include <string>

namespace dtn
{
	namespace core
	{
		class Event
		{
		public:
			virtual ~Event() = 0;

			/**
			 * Get the name of this event.
			 */
			virtual const std::string getName() const = 0;

			/**
			 * Get a string representation of this event.
			 */
			virtual std::string toString() const;

			/**
			 * Get a describing message for this event
			 */
			virtual std::string getMessage() const = 0;

			/**
			 * If this event should be logged, this method
			 * returns true
			 */
			bool isLoggable() const;

			/**
			 * Contains the priority of this event.
			 */
			const int prio;

		protected:
			/**
			 * Constructor of Event
			 * Accepts a priority lower or higher than zero.
			 */
			Event(int prio = 0);

			/**
			 * Specify if this event should be logged
			 */
			void setLoggable(bool val);

		private:
			bool _loggable;
		};

		class EventProcessor {
		public:
			virtual ~EventProcessor() { };
			virtual void process(const Event *evt) = 0;
		};
	}
}

#endif /* EVENT_H_ */
