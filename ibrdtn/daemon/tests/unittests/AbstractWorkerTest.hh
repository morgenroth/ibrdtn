/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        AbstractWorkerTest.hh
/// @brief       CPPUnit-Tests for class AbstractWorker
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef ABSTRACTWORKERTEST_HH
#define ABSTRACTWORKERTEST_HH
class AbstractWorkerTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'AbstractWorker' ===*/
		void testInitialize();
		void testTransmit();
		void testShutdown();
		/*=== END   tests for class 'AbstractWorker' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(AbstractWorkerTest);
			CPPUNIT_TEST(testInitialize);
			CPPUNIT_TEST(testTransmit);
			CPPUNIT_TEST(testShutdown);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* ABSTRACTWORKERTEST_HH */
