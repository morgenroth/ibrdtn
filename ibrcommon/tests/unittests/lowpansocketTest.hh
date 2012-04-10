/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        lowpansocketTest.hh
/// @brief       CPPUnit-Tests for class lowpansocket
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef LOWPANSOCKETTEST_HH
#define LOWPANSOCKETTEST_HH
class lowpansocketTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'lowpansocket' ===*/
		/*=== BEGIN tests for class 'peer' ===*/
		void testSend();
		/*=== END   tests for class 'peer' ===*/

		void testReceive();
		void testGetPeer();
		/*=== END   tests for class 'lowpansocket' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(lowpansocketTest);
			CPPUNIT_TEST(testSend);
			CPPUNIT_TEST(testReceive);
			CPPUNIT_TEST(testGetPeer);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* LOWPANSOCKETTEST_HH */
