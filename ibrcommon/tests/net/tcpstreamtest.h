/*
 * tcpstreamtest.h
 *
 *  Created on: 22.03.2010
 *      Author: morgenro
 */

#ifndef TCPSTREAMTEST_H_
#define TCPSTREAMTEST_H_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "ibrcommon/net/tcpserver.h"
#include "ibrcommon/net/tcpstream.h"
#include "ibrcommon/thread/Thread.h"
#include <ibrcommon/net/vinterface.h>
#include "ibrcommon/thread/Conditional.h"

using namespace std;

class tcpstreamtest : public CPPUNIT_NS :: TestFixture
{
	class StreamChecker : public ibrcommon::JoinableThread
	{
	public:
		StreamChecker(int chars = 10);
		~StreamChecker();

		virtual void run();
		virtual void __cancellation();

	private:
		ibrcommon::tcpserver _srv;
		int _chars;
	};

	CPPUNIT_TEST_SUITE (tcpstreamtest);
	CPPUNIT_TEST (baseTest);
	CPPUNIT_TEST_SUITE_END ();

	public:
		void setUp (void);
		void tearDown (void);

	protected:
		void baseTest (void);
		void runTest (void);

	private:
		StreamChecker *_checker;
};

#endif /* TCPSTREAMTEST_H_ */
