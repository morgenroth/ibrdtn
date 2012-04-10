/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        SyslogStreamTest.hh
/// @brief       CPPUnit-Tests for class SyslogStream
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef SYSLOGSTREAMTEST_HH
#define SYSLOGSTREAMTEST_HH
class SyslogStreamTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'SyslogStream' ===*/
		void testGetStream();
		void testOpen();
		void testSetPriority();
		/*=== END   tests for class 'SyslogStream' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(SyslogStreamTest);
			CPPUNIT_TEST(testGetStream);
			CPPUNIT_TEST(testOpen);
			CPPUNIT_TEST(testSetPriority);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* SYSLOGSTREAMTEST_HH */
