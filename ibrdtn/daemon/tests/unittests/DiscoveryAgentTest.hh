/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        DiscoveryAgentTest.hh
/// @brief       CPPUnit-Tests for class DiscoveryAgent
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef DISCOVERYAGENTTEST_HH
#define DISCOVERYAGENTTEST_HH
class DiscoveryAgentTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'DiscoveryAgent' ===*/
		void testReceived();
		void testAddService();
		/*=== END   tests for class 'DiscoveryAgent' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(DiscoveryAgentTest);
			CPPUNIT_TEST(testReceived);
			CPPUNIT_TEST(testAddService);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* DISCOVERYAGENTTEST_HH */
