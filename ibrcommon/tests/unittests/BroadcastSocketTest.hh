/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        BroadcastSocketTest.hh
/// @brief       CPPUnit-Tests for class BroadcastSocket
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef BROADCASTSOCKETTEST_HH
#define BROADCASTSOCKETTEST_HH
class BroadcastSocketTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'BroadcastSocket' ===*/
		void testBind();
		/*=== END   tests for class 'BroadcastSocket' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(BroadcastSocketTest);
			CPPUNIT_TEST(testBind);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* BROADCASTSOCKETTEST_HH */
