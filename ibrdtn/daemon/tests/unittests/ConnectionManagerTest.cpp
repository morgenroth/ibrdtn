/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        ConnectionManagerTest.cpp
/// @brief       CPPUnit-Tests for class ConnectionManager
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

#include "ConnectionManagerTest.hh"



CPPUNIT_TEST_SUITE_REGISTRATION(ConnectionManagerTest);

/*========================== tests below ==========================*/

/*=== BEGIN tests for class 'ConnectionManager' ===*/
void ConnectionManagerTest::testAddConnection()
{
	/* test signature (const dtn::core::Node &n) */
	CPPUNIT_FAIL("not implemented");
}

void ConnectionManagerTest::testAddConvergenceLayer()
{
	/* test signature (ConvergenceLayer *cl) */
	CPPUNIT_FAIL("not implemented");
}

void ConnectionManagerTest::testQueue()
{
	/* test signature (const dtn::data::EID &eid, const dtn::data::BundleID &b) */
	/* test signature (const ConvergenceLayer::Job &job) */
	/* test signature (const dtn::core::Node &node, const ConvergenceLayer::Job &job) */
	CPPUNIT_FAIL("not implemented");
}

void ConnectionManagerTest::testRaiseEvent()
{
	/* test signature (const dtn::core::Event *evt) */
	CPPUNIT_FAIL("not implemented");
}

void ConnectionManagerTest::testGetNeighbors()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void ConnectionManagerTest::testIsNeighbor()
{
	/* test signature (const dtn::core::Node&) */
	CPPUNIT_FAIL("not implemented");
}

void ConnectionManagerTest::testDiscovered()
{
	/* test signature (dtn::core::Node &node) */
	CPPUNIT_FAIL("not implemented");
}

void ConnectionManagerTest::testCheck_discovered()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

/*=== END   tests for class 'ConnectionManager' ===*/

void ConnectionManagerTest::setUp()
{
}

void ConnectionManagerTest::tearDown()
{
}

