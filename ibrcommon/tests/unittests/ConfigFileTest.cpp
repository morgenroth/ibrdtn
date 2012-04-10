/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        ConfigFileTest.cpp
/// @brief       CPPUnit-Tests for class ConfigFile
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 

#include "ConfigFileTest.hh"



CPPUNIT_TEST_SUITE_REGISTRATION(ConfigFileTest);

/*========================== tests below ==========================*/

/*=== BEGIN tests for class 'ConfigFile' ===*/
void ConfigFileTest::testRead()
{
	/* test signature ( const string& key ) const */
	/* test signature ( const string& key, const T& value ) const */
	CPPUNIT_FAIL("not implemented");
}

void ConfigFileTest::testReadInto()
{
	/* test signature ( T& var, const string& key ) const */
	/* test signature ( T& var, const string& key, const T& value ) const */
	CPPUNIT_FAIL("not implemented");
}

void ConfigFileTest::testAdd()
{
	/* test signature ( string key, const T& value ) */
	CPPUNIT_FAIL("not implemented");
}

void ConfigFileTest::testRemove()
{
	/* test signature ( const string& key ) */
	CPPUNIT_FAIL("not implemented");
}

void ConfigFileTest::testKeyExists()
{
	/* test signature ( const string& key ) const */
	CPPUNIT_FAIL("not implemented");
}

void ConfigFileTest::testOperatorShiftLeft()
{
	/* test signature ( std::ostream& os, const ConfigFile& cf ) */
	CPPUNIT_FAIL("not implemented");
}

void ConfigFileTest::testOperatorShiftRight()
{
	/* test signature ( std::istream& is, ConfigFile& cf ) */
	CPPUNIT_FAIL("not implemented");
}

void ConfigFileTest::testT_as_string()
{
	/* test signature ( const T& t ) */
	CPPUNIT_FAIL("not implemented");
}

void ConfigFileTest::testString_as_T()
{
	/* test signature ( const string& s ) */
	CPPUNIT_FAIL("not implemented");
}

void ConfigFileTest::testTrim()
{
	/* test signature ( string& s ) */
	CPPUNIT_FAIL("not implemented");
}

/*=== END   tests for class 'ConfigFile' ===*/

void ConfigFileTest::setUp()
{
}

void ConfigFileTest::tearDown()
{
}

