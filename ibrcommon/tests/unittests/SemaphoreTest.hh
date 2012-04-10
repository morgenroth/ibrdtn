/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        SemaphoreTest.hh
/// @brief       CPPUnit-Tests for class Semaphore
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef SEMAPHORETEST_HH
#define SEMAPHORETEST_HH
class SemaphoreTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'Semaphore' ===*/
		void testWait();
		void testPost();
		/*=== END   tests for class 'Semaphore' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(SemaphoreTest);
			CPPUNIT_TEST(testWait);
			CPPUNIT_TEST(testPost);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* SEMAPHORETEST_HH */
