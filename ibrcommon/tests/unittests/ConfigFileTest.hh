/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        ConfigFileTest.hh
/// @brief       CPPUnit-Tests for class ConfigFile
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef CONFIGFILETEST_HH
#define CONFIGFILETEST_HH
class ConfigFileTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'ConfigFile' ===*/
		void testRead();
		void testReadInto();
		void testAdd();
		void testRemove();
		void testKeyExists();
		void testOperatorShiftLeft();
		void testOperatorShiftRight();
		void testT_as_string();
		void testString_as_T();
		void testTrim();
		/*=== END   tests for class 'ConfigFile' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(ConfigFileTest);
			CPPUNIT_TEST(testRead);
			CPPUNIT_TEST(testReadInto);
			CPPUNIT_TEST(testAdd);
			CPPUNIT_TEST(testRemove);
			CPPUNIT_TEST(testKeyExists);
			CPPUNIT_TEST(testOperatorShiftLeft);
			CPPUNIT_TEST(testOperatorShiftRight);
			CPPUNIT_TEST(testT_as_string);
			CPPUNIT_TEST(testString_as_T);
			CPPUNIT_TEST(testTrim);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* CONFIGFILETEST_HH */
