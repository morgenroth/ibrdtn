/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        RetransmissionExtensionTest.hh
/// @brief       CPPUnit-Tests for class RetransmissionExtension
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef RETRANSMISSIONEXTENSIONTEST_HH
#define RETRANSMISSIONEXTENSIONTEST_HH
class RetransmissionExtensionTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'RetransmissionExtension' ===*/
		void testNotify();
		/*=== END   tests for class 'RetransmissionExtension' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(RetransmissionExtensionTest);
			CPPUNIT_TEST(testNotify);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* RETRANSMISSIONEXTENSIONTEST_HH */
