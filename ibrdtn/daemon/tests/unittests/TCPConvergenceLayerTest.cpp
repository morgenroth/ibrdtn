/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        TCPConvergenceLayerTest.cpp
/// @brief       CPPUnit-Tests for class TCPConvergenceLayer
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

#include "TCPConvergenceLayerTest.hh"



CPPUNIT_TEST_SUITE_REGISTRATION(TCPConvergenceLayerTest);

/*========================== tests below ==========================*/

/*=== BEGIN tests for class 'TCPConvergenceLayer' ===*/
/*=== BEGIN tests for class 'TCPConnection' ===*/
void TCPConvergenceLayerTest::testTCPConnectionShutdown()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testTCPConnectionGetHeader()
{
	/* test signature () const */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testTCPConnectionGetNode()
{
	/* test signature () const */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testTCPConnectionGetDiscoveryProtocol()
{
	/* test signature () const */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testTCPConnectionQueue()
{
	/* test signature (const dtn::data::BundleID &bundle) */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testTCPConnectionOperatorShiftRight()
{
	/* test signature (TCPConvergenceLayer::TCPConnection &conn, dtn::data::Bundle &bundle) */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testTCPConnectionOperatorShiftLeft()
{
	/* test signature (TCPConvergenceLayer::TCPConnection &conn, const dtn::data::Bundle &bundle) */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testTCPConnectionRejectTransmission()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testTCPConnectionRun()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testTCPConnectionFinally()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testTCPConnectionClearQueue()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testTCPConnectionKeepalive()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

/*=== END   tests for class 'TCPConnection' ===*/

/*=== BEGIN tests for class 'Server' ===*/
void TCPConvergenceLayerTest::testServerQueue()
{
	/* test signature (const dtn::core::Node &n, const ConvergenceLayer::Job &job) */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testServerRaiseEvent()
{
	/* test signature (const dtn::core::Event *evt) */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testServerAccept()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testServerListen()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testServerShutdown()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testServerConnectionUp()
{
	/* test signature (TCPConvergenceLayer::TCPConnection *conn) */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testServerConnectionDown()
{
	/* test signature (TCPConvergenceLayer::TCPConnection *conn) */
	CPPUNIT_FAIL("not implemented");
}

/*=== END   tests for class 'Server' ===*/

void TCPConvergenceLayerTest::testGetDiscoveryProtocol()
{
	/* test signature () const */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testInitialize()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testStartup()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testTerminate()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testUpdate()
{
	/* test signature (std::string &name, std::string &data) */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testOnInterface()
{
	/* test signature (const ibrcommon::vinterface &net) const */
	CPPUNIT_FAIL("not implemented");
}

void TCPConvergenceLayerTest::testQueue()
{
	/* test signature (const dtn::core::Node &n, const ConvergenceLayer::Job &job) */
	CPPUNIT_FAIL("not implemented");
}

/*=== END   tests for class 'TCPConvergenceLayer' ===*/

void TCPConvergenceLayerTest::setUp()
{
}

void TCPConvergenceLayerTest::tearDown()
{
}

