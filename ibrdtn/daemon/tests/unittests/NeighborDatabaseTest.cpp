/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        NeighborDatabaseTest.cpp
/// @brief       CPPUnit-Tests for class NeighborDatabase
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

#include "NeighborDatabaseTest.hh"



CPPUNIT_TEST_SUITE_REGISTRATION(NeighborDatabaseTest);

/*========================== tests below ==========================*/

/*=== BEGIN tests for class 'NeighborDatabase' ===*/
/*=== BEGIN tests for class 'NeighborEntry' ===*/
void NeighborDatabaseTest::testNeighborEntryUpdateLastSeen()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void NeighborDatabaseTest::testNeighborEntryUpdateBundles()
{
	/* test signature (const ibrcommon::BloomFilter &bf) */
	CPPUNIT_FAIL("not implemented");
}

/*=== END   tests for class 'NeighborEntry' ===*/

void NeighborDatabaseTest::testUpdateLastSeen()
{
	/* test signature (const dtn::data::EID &eid) */
	CPPUNIT_FAIL("not implemented");
}

void NeighborDatabaseTest::testUpdateBundles()
{
	/* test signature (const dtn::data::EID &eid, const ibrcommon::BloomFilter &bf) */
	CPPUNIT_FAIL("not implemented");
}

void NeighborDatabaseTest::testKnowBundle()
{
	/* test signature (const dtn::data::EID &eid, const dtn::data::BundleID &bundle) */
	CPPUNIT_FAIL("not implemented");
}

void NeighborDatabaseTest::testGetSetAvailable()
{
	/* test signature (const dtn::data::EID &eid)  -- originally from Function 'setAvailable' */
	/* test signature () const  -- originally from Function 'getAvailable' */
	CPPUNIT_FAIL("not implemented");
}

void NeighborDatabaseTest::testSetUnavailable()
{
	/* test signature (const dtn::data::EID &eid) */
	CPPUNIT_FAIL("not implemented");
}

void NeighborDatabaseTest::testGet()
{
	/* test signature (const dtn::data::EID &eid) */
	CPPUNIT_FAIL("not implemented");
}

/*=== END   tests for class 'NeighborDatabase' ===*/

void NeighborDatabaseTest::setUp()
{
}

void NeighborDatabaseTest::tearDown()
{
}

