/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        NeighborRoutingExtensionTest.hh
/// @brief       CPPUnit-Tests for class NeighborRoutingExtension
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef NEIGHBORROUTINGEXTENSIONTEST_HH
#define NEIGHBORROUTINGEXTENSIONTEST_HH
class NeighborRoutingExtensionTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'NeighborRoutingExtension' ===*/
		void testNotify();
		void testRun();
		void test__cancellation();
		/*=== END   tests for class 'NeighborRoutingExtension' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(NeighborRoutingExtensionTest);
			CPPUNIT_TEST(testNotify);
			CPPUNIT_TEST(testRun);
			CPPUNIT_TEST(test__cancellation);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* NEIGHBORROUTINGEXTENSIONTEST_HH */
