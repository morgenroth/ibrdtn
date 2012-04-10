/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        StaticRoutingExtensionTest.hh
/// @brief       CPPUnit-Tests for class StaticRoutingExtension
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef STATICROUTINGEXTENSIONTEST_HH
#define STATICROUTINGEXTENSIONTEST_HH
class StaticRoutingExtensionTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'StaticRoutingExtension' ===*/
		/*=== BEGIN tests for class 'StaticRoute' ===*/
		void testMatch();
		void testGetDestination();
		/*=== END   tests for class 'StaticRoute' ===*/

		void testNotify();
		/*=== END   tests for class 'StaticRoutingExtension' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(StaticRoutingExtensionTest);
			CPPUNIT_TEST(testMatch);
			CPPUNIT_TEST(testGetDestination);
			CPPUNIT_TEST(testNotify);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* STATICROUTINGEXTENSIONTEST_HH */
