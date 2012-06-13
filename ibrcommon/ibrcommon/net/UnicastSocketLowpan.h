/*
 * UnicastSocket.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef UNICASTSOCKETLOWPAN_H_
#define UNICASTSOCKETLOWPAN_H_

#include "ibrcommon/net/vinterface.h"
#include "ibrcommon/net/vaddress.h"
#include "ibrcommon/net/lowpansocket.h"

namespace ibrcommon
{
	class UnicastSocketLowpan : public ibrcommon::lowpansocket
	{
	public:
		UnicastSocketLowpan();
		virtual ~UnicastSocketLowpan();

		void bind(int panid, const vaddress &address);
		void bind(int panid, const vinterface &iface);
	};
}

#endif /* UNICASTSOCKETLOWPAN_H_ */
