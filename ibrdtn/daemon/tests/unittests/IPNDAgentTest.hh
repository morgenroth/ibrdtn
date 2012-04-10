/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        IPNDAgentTest.hh
/// @brief       CPPUnit-Tests for class IPNDAgent
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef IPNDAGENTTEST_HH
#define IPNDAGENTTEST_HH
class IPNDAgentTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'IPNDAgent' ===*/
		void testBind();
		void testSendAnnoucement();
		void test__cancellation();
		/*=== END   tests for class 'IPNDAgent' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(IPNDAgentTest);
			CPPUNIT_TEST(testBind);
			CPPUNIT_TEST(testSendAnnoucement);
			CPPUNIT_TEST(test__cancellation);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* IPNDAGENTTEST_HH */
