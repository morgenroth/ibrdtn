/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        udpsocketTest.cpp
/// @brief       CPPUnit-Tests for class udpsocket
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 

#include "udpsocketTest.hh"



CPPUNIT_TEST_SUITE_REGISTRATION(udpsocketTest);

/*========================== tests below ==========================*/

/*=== BEGIN tests for class 'udpsocket' ===*/
/*=== BEGIN tests for class 'peer' ===*/
void udpsocketTest::testSend()
{
	/* test signature (const char *data, const size_t length) */
	CPPUNIT_FAIL("not implemented");
}

/*=== END   tests for class 'peer' ===*/

void udpsocketTest::testReceive()
{
	/* test signature (char* data, size_t maxbuffer) */
	/* test signature (char* data, size_t maxbuffer, std::string &address) */
	CPPUNIT_FAIL("not implemented");
}

void udpsocketTest::testGetPeer()
{
	/* test signature (const std::string address, const unsigned int port) */
	CPPUNIT_FAIL("not implemented");
}

/*=== END   tests for class 'udpsocket' ===*/

void udpsocketTest::setUp()
{
}

void udpsocketTest::tearDown()
{
}

