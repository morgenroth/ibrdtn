/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        SQLiteConfigureTest.hh
/// @brief       CPPUnit-Tests for class SQLiteConfigure
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef SQLITECONFIGURETEST_HH
#define SQLITECONFIGURETEST_HH
class SQLiteConfigureTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'SQLiteConfigure' ===*/
		void testConfigure();
		/*=== END   tests for class 'SQLiteConfigure' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(SQLiteConfigureTest);
			CPPUNIT_TEST(testConfigure);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* SQLITECONFIGURETEST_HH */
