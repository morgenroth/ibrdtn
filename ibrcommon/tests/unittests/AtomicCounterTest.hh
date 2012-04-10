/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        AtomicCounterTest.hh
/// @brief       CPPUnit-Tests for class AtomicCounter
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef ATOMICCOUNTERTEST_HH
#define ATOMICCOUNTERTEST_HH
class AtomicCounterTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'AtomicCounter' ===*/
		void testValue();
		void testWait();
		void testUnblockAll();
		void testOperatorPostincrement();
		void testOperatorPostdecrement();
		/*=== END   tests for class 'AtomicCounter' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(AtomicCounterTest);
			CPPUNIT_TEST(testValue);
			CPPUNIT_TEST(testWait);
			CPPUNIT_TEST(testUnblockAll);
			CPPUNIT_TEST(testOperatorPostincrement);
			CPPUNIT_TEST(testOperatorPostdecrement);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* ATOMICCOUNTERTEST_HH */
