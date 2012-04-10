/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        QueueTest.hh
/// @brief       CPPUnit-Tests for class Queue
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef QUEUETEST_HH
#define QUEUETEST_HH
class QueueTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'Queue' ===*/
		/*=== END   tests for class 'Queue' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(QueueTest);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* QUEUETEST_HH */
