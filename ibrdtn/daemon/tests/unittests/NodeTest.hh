/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        NodeTest.hh
/// @brief       CPPUnit-Tests for class Node
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef NODETEST_HH
#define NODETEST_HH
class NodeTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'Node' ===*/
		void testGetProtocol();
//		void testGetProtocolName();
//		void testGetSetType();
//		void testGetSetProtocol();
//		void testGetSetAddress();
//		void testGetSetPort();
//		void testGetSetDescription();
//		void testGetSetURI();
//		void testGetSetTimeout();
//		void testGetRoundTripTime();
//		void testDecrementTimeout();
//		void testOperatorEqual();
//		void testOperatorLessThan();
//		void testToString();
		/*=== END   tests for class 'Node' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(NodeTest);
			CPPUNIT_TEST(testGetProtocol);
//			CPPUNIT_TEST(testGetProtocolName);
//			CPPUNIT_TEST(testGetSetType);
//			CPPUNIT_TEST(testGetSetProtocol);
//			CPPUNIT_TEST(testGetSetAddress);
//			CPPUNIT_TEST(testGetSetPort);
//			CPPUNIT_TEST(testGetSetDescription);
//			CPPUNIT_TEST(testGetSetURI);
//			CPPUNIT_TEST(testGetSetTimeout);
//			CPPUNIT_TEST(testGetRoundTripTime);
//			CPPUNIT_TEST(testDecrementTimeout);
//			CPPUNIT_TEST(testOperatorEqual);
//			CPPUNIT_TEST(testOperatorLessThan);
//			CPPUNIT_TEST(testToString);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* NODETEST_HH */
