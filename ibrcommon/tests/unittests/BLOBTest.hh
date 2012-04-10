/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        BLOBTest.hh
/// @brief       CPPUnit-Tests for class BLOB
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef BLOBTEST_HH
#define BLOBTEST_HH
class BLOBTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'BLOB' ===*/
		/*=== BEGIN tests for class 'Reference' ===*/
		void testOperatorMultiply();
//		void testEnter();
//		void testLeave();
//		void testClear();
//		void testGetSize();
		/*=== END   tests for class 'Reference' ===*/
		/*=== END   tests for class 'BLOB' ===*/

		/*=== BEGIN tests for class 'StringBLOB' ===*/
		void testStringBLOBCreate();
		/*=== END   tests for class 'StringBLOB' ===*/

		/*=== BEGIN tests for class 'TmpFileBLOB' ===*/
		void testTmpFileBLOBCreate();
		/*=== END   tests for class 'TmpFileBLOB' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(BLOBTest);
			CPPUNIT_TEST(testOperatorMultiply);
//			CPPUNIT_TEST(testEnter);
//			CPPUNIT_TEST(testLeave);
//			CPPUNIT_TEST(testClear);
//			CPPUNIT_TEST(testGetSize);
			CPPUNIT_TEST(testStringBLOBCreate);
			CPPUNIT_TEST(testTmpFileBLOBCreate);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* BLOBTEST_HH */
