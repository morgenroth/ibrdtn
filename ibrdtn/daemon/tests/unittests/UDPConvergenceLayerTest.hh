/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        UDPConvergenceLayerTest.hh
/// @brief       CPPUnit-Tests for class UDPConvergenceLayer
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef UDPCONVERGENCELAYERTEST_HH
#define UDPCONVERGENCELAYERTEST_HH
class UDPConvergenceLayerTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'UDPConvergenceLayer' ===*/
		void testGetDiscoveryProtocol();
		void testQueue();
		void testOperatorShiftRight();
		void test__cancellation();
		/*=== END   tests for class 'UDPConvergenceLayer' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(UDPConvergenceLayerTest);
			CPPUNIT_TEST(testGetDiscoveryProtocol);
			CPPUNIT_TEST(testQueue);
			CPPUNIT_TEST(testOperatorShiftRight);
			CPPUNIT_TEST(test__cancellation);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* UDPCONVERGENCELAYERTEST_HH */
