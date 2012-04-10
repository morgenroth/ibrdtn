/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        refcnt_ptrTest.hh
/// @brief       CPPUnit-Tests for class refcnt_ptr
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef REFCNT_PTRTEST_HH
#define REFCNT_PTRTEST_HH
class refcnt_ptrTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'refcnt_ptr' ===*/
		void testSelfAssignment();
		/*=== END   tests for class 'refcnt_ptr' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(refcnt_ptrTest);
		CPPUNIT_TEST(testSelfAssignment);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* REFCNT_PTRTEST_HH */
