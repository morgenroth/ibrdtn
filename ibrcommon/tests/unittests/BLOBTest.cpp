/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        BLOBTest.cpp
/// @brief       CPPUnit-Tests for class BLOB
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

#include "BLOBTest.hh"
#include <ibrcommon/data/BLOB.h>
#include <ibrcommon/data/File.h>
#include <ibrcommon/thread/MutexLock.h>

CPPUNIT_TEST_SUITE_REGISTRATION(BLOBTest);

/*========================== tests below ==========================*/

/*=== BEGIN tests for class 'BLOB' ===*/
/*=== BEGIN tests for class 'Reference' ===*/
void BLOBTest::testOperatorMultiply()
{
	ibrcommon::BLOB::changeProvider(new ibrcommon::MemoryBLOBProvider(), true);

	/* test signature () */
	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
	ibrcommon::BLOB::iostream stream = ref.iostream();
	(*stream) << "0123456789";
}

//void BLOBTest::testEnter()
//{
//	ibrcommon::BLOB::changeProvider(new ibrcommon::MemoryBLOBProvider(), true);
//
//	/* test signature () */
//	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
//	ref.enter();
//	try {
//		ref.trylock();
//		ref.leave();
//		CPPUNIT_FAIL("lock on BLOB reference not working");
//	} catch (const ibrcommon::MutexException&) {
//	}
//	ref.leave();
//}
//
//void BLOBTest::testLeave()
//{
//	ibrcommon::BLOB::changeProvider(new ibrcommon::MemoryBLOBProvider(), true);
//
//	/* test signature () */
//	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
//	ref.enter();
//	ref.leave();
//	try {
//		ref.trylock();
//		ref.leave();
//	} catch (const ibrcommon::MutexException&) {
//		CPPUNIT_FAIL("lock on BLOB reference not working");
//	}
//}
//
//void BLOBTest::testClear()
//{
//	ibrcommon::BLOB::changeProvider(new ibrcommon::MemoryBLOBProvider(), true);
//
//	/* test signature () */
//	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
//	ibrcommon::BLOB::iostream stream = ref.iostream();
//	(*stream) << "0123456789";
//	CPPUNIT_ASSERT_EQUAL((size_t)10, stream.size());
//	stream.clear();
//	CPPUNIT_ASSERT_EQUAL((size_t)0, stream.size());
//}
//
//void BLOBTest::testGetSize()
//{
//	ibrcommon::BLOB::changeProvider(new ibrcommon::MemoryBLOBProvider(), true);
//
//	/* test signature () const */
//	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
//	ibrcommon::BLOB::iostream stream = ref.iostream();
//	(*stream) << "0123456789";
//	CPPUNIT_ASSERT_EQUAL((size_t)10, stream.size());
//}

/*=== END   tests for class 'Reference' ===*/
/*=== END   tests for class 'BLOB' ===*/

/*=== BEGIN tests for class 'StringBLOB' ===*/
void BLOBTest::testStringBLOBCreate()
{
	ibrcommon::BLOB::changeProvider(new ibrcommon::MemoryBLOBProvider(), true);

	/* test signature () */
	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
}

/*=== END   tests for class 'StringBLOB' ===*/

/*=== BEGIN tests for class 'TmpFileBLOB' ===*/
void BLOBTest::testTmpFileBLOBCreate()
{
	ibrcommon::File tmppath("/tmp");
	ibrcommon::BLOB::changeProvider(new ibrcommon::FileBLOBProvider(tmppath), true);

	/* test signature () */
	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
}

/*=== END   tests for class 'TmpFileBLOB' ===*/

void BLOBTest::setUp()
{
}

void BLOBTest::tearDown()
{
}

