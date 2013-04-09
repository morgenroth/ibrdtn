/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        FileTest.hh
/// @brief       CPPUnit-Tests for class File
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef FILETEST_HH
#define FILETEST_HH
class FileTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'File' ===*/
		void testGetType();
		void testGetFiles();
		void testIsSystem();
		void testIsDirectory();
		void testGetPath();
		void testRemove();
		void testGet();
		void testGetParent();
		void testExists();
		void testUpdate();
		void testSize();
		void testCreateDirectory();
		void testGetBasename();
		/*=== END   tests for class 'File' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(FileTest);
			CPPUNIT_TEST(testGetType);
//			CPPUNIT_TEST(testGetFiles);
			CPPUNIT_TEST(testIsSystem);
			CPPUNIT_TEST(testIsDirectory);
//			CPPUNIT_TEST(testGetPath);
//			CPPUNIT_TEST(testRemove);
//			CPPUNIT_TEST(testGet);
//			CPPUNIT_TEST(testGetParent);
//			CPPUNIT_TEST(testExists);
//			CPPUNIT_TEST(testUpdate);
//			CPPUNIT_TEST(testSize);
//			CPPUNIT_TEST(testCreateDirectory);
			CPPUNIT_TEST(testGetBasename);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* FILETEST_HH */
