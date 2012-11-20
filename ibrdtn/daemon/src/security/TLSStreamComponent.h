/*
 * TLSStreamComponent.h
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Stephen Roettger <roettger@ibr.cs.tu-bs.de>
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

#ifndef TLSSTREAMCOMPONENT_H_
#define TLSSTREAMCOMPONENT_H_

#include "Component.h"
#include "core/EventReceiver.h"

namespace dtn {

namespace security {

/*!
 * \brief This class acts as an event receiver for SecurityCertificateManager Events and forwards them to the TLSStream class.
 */
class TLSStreamComponent: public dtn::core::EventReceiver, public dtn::daemon::Component {
public:
	///The default constructor
	TLSStreamComponent();
	///The default destructor
	virtual ~TLSStreamComponent();

	/*!
	 * \brief callback function that is called if an event happened.
	 * \param evt the event
	 */
	virtual void raiseEvent(const dtn::core::Event *evt) throw ();

    /* functions from Component */
    virtual void initialize();
	virtual void startup();
	virtual void terminate();
	virtual const std::string getName() const;
};

}

}

#endif /* TLSSTREAMCOMPONENT_H_ */
