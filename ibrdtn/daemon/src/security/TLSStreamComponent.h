/*
 * TLSStreamComponent.h
 *
 *  Created on: Apr 2, 2011
 *      Author: roettger
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
	virtual void raiseEvent(const dtn::core::Event *evt);

    /* functions from Component */
    virtual void initialize();
	virtual void startup();
	virtual void terminate();
	virtual const std::string getName() const;
};

}

}

#endif /* TLSSTREAMCOMPONENT_H_ */
