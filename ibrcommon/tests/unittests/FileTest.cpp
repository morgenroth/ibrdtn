/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        FileTest.cpp
/// @brief       CPPUnit-Tests for class File
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

/*
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "FileTest.hh"
#include <ibrcommon/data/File.h>
#include <stdlib.h>

CPPUNIT_TEST_SUITE_REGISTRATION(FileTest);

/*========================== tests below ==========================*/

/*=== BEGIN tests for class 'File' ===*/
void FileTest::testGetType()
{
	/* test signature () const */
	ibrcommon::File resolve("/etc/hosts");
	CPPUNIT_ASSERT_EQUAL((unsigned char)DT_REG, resolve.getType());
}

void FileTest::testGetFiles()
{
	/* test signature (list<File> &files) */
	CPPUNIT_FAIL("not implemented");
}

void FileTest::testIsSystem()
{
	/* test signature () const */
	ibrcommon::File f1("/etc/..");
	CPPUNIT_ASSERT(f1.isSystem());

	ibrcommon::File f2("/etc");
	CPPUNIT_ASSERT(!f2.isSystem());
}

void FileTest::testIsDirectory()
{
	/* test signature () const */
	ibrcommon::File etc("/etc");
	CPPUNIT_ASSERT(etc.isDirectory());

	ibrcommon::File resolve("/etc/resolv.conf");
	CPPUNIT_ASSERT(!resolve.isDirectory());
}

void FileTest::testGetPath()
{
	/* test signature () const */
	CPPUNIT_FAIL("not implemented");
}

void FileTest::testRemove()
{
	/* test signature (bool recursive = false) */
	CPPUNIT_FAIL("not implemented");
}

void FileTest::testGet()
{
	/* test signature (string filename) const */
	CPPUNIT_FAIL("not implemented");
}

void FileTest::testGetParent()
{
	/* test signature () const */
	CPPUNIT_FAIL("not implemented");
}

void FileTest::testExists()
{
	/* test signature () const */
	CPPUNIT_FAIL("not implemented");
}

void FileTest::testUpdate()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void FileTest::testSize()
{
	/* test signature () const */
	CPPUNIT_FAIL("not implemented");
}

void FileTest::testCreateDirectory()
{
	/* test signature (File &path) */
	CPPUNIT_FAIL("not implemented");
}

void FileTest::testGetBasename()
{
	CPPUNIT_ASSERT_EQUAL(std::string(".."), ibrcommon::File("test/..").getBasename());
	CPPUNIT_ASSERT_EQUAL(std::string("."), ibrcommon::File("test/.").getBasename());
	CPPUNIT_ASSERT_EQUAL(std::string("test"), ibrcommon::File("./../blah/../test").getBasename());
}

/*=== END   tests for class 'File' ===*/

void FileTest::setUp()
{
}

void FileTest::tearDown()
{
}

