/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        HTTPConvergenceLayerTest.hh
/// @brief       CPPUnit-Tests for class HTTPConvergenceLayer
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef HTTPCONVERGENCELAYERTEST_HH
#define HTTPCONVERGENCELAYERTEST_HH
class HTTPConvergenceLayerTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'HTTPConvergenceLayer' ===*/
		void testGetDiscoveryProtocol();
		void testQueue();
		void test__cancellation();
		/*=== END   tests for class 'HTTPConvergenceLayer' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(HTTPConvergenceLayerTest);
			CPPUNIT_TEST(testGetDiscoveryProtocol);
			CPPUNIT_TEST(testQueue);
			CPPUNIT_TEST(test__cancellation);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* HTTPCONVERGENCELAYERTEST_HH */
