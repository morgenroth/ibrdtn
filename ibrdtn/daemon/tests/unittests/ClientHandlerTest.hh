/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        ClientHandlerTest.hh
/// @brief       CPPUnit-Tests for class ClientHandler
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef CLIENTHANDLERTEST_HH
#define CLIENTHANDLERTEST_HH
class ClientHandlerTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'ClientHandler' ===*/
		void testOperatorShiftRight();
		void testOperatorShiftLeft();
		void testGetPeer();
		void testQueue();
		void testReceived();
		void testRun();
		void testFinally();
		/*=== END   tests for class 'ClientHandler' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(ClientHandlerTest);
			CPPUNIT_TEST(testOperatorShiftRight);
			CPPUNIT_TEST(testOperatorShiftLeft);
			CPPUNIT_TEST(testGetPeer);
			CPPUNIT_TEST(testQueue);
			CPPUNIT_TEST(testReceived);
			CPPUNIT_TEST(testRun);
			CPPUNIT_TEST(testFinally);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* CLIENTHANDLERTEST_HH */
