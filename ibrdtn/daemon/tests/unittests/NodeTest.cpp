/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        NodeTest.cpp
/// @brief       CPPUnit-Tests for class Node
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

#include "NodeTest.hh"
#include "core/Node.h"


CPPUNIT_TEST_SUITE_REGISTRATION(NodeTest);

/*========================== tests below ==========================*/

/*=== BEGIN tests for class 'Node' ===*/
void NodeTest::testGetProtocol()
{
	dtn::core::Node n(dtn::data::EID("dtn://test"));

	n.add( dtn::core::Node::URI(dtn::core::Node::NODE_STATIC_LOCAL, dtn::core::Node::CONN_TCPIP, "ip=0.0.0.0;port=1234;", 0, 10) );
	n.add( dtn::core::Node::URI(dtn::core::Node::NODE_DISCOVERED, dtn::core::Node::CONN_TCPIP, "ip=0.0.0.0;port=1234;", 0, 20) );
	n.add( dtn::core::Node::URI(dtn::core::Node::NODE_CONNECTED, dtn::core::Node::CONN_TCPIP, "ip=0.0.0.0;port=1234;") );

	std::list<dtn::core::Node::URI> items = n.get(dtn::core::Node::CONN_TCPIP);

	std::list<dtn::core::Node::URI>::iterator iter = items.begin();

	CPPUNIT_ASSERT_EQUAL((*iter).type, dtn::core::Node::NODE_DISCOVERED);
	++iter;

	CPPUNIT_ASSERT_EQUAL((*iter).type, dtn::core::Node::NODE_STATIC_LOCAL);
	++iter;

	CPPUNIT_ASSERT_EQUAL((*iter).type, dtn::core::Node::NODE_CONNECTED);
}

//void NodeTest::testGetProtocolName()
//{
//	/* test signature (Node::Protocol proto) */
//	CPPUNIT_FAIL("not implemented");
//}
//
//void NodeTest::testGetSetType()
//{
//	/* test signature () const  -- originally from Function 'getType' */
//	/* test signature (Node::Type type)  -- originally from Function 'setType' */
//	CPPUNIT_FAIL("not implemented");
//}
//
//void NodeTest::testGetSetProtocol()
//{
//	/* test signature (Node::Protocol protocol)  -- originally from Function 'setProtocol' */
//	/* test signature () const  -- originally from Function 'getProtocol' */
//	CPPUNIT_FAIL("not implemented");
//}
//
//void NodeTest::testGetSetAddress()
//{
//	/* test signature (std::string address)  -- originally from Function 'setAddress' */
//	/* test signature () const  -- originally from Function 'getAddress' */
//	CPPUNIT_FAIL("not implemented");
//}
//
//void NodeTest::testGetSetPort()
//{
//	/* test signature (unsigned int port)  -- originally from Function 'setPort' */
//	/* test signature () const  -- originally from Function 'getPort' */
//	CPPUNIT_FAIL("not implemented");
//}
//
//void NodeTest::testGetSetDescription()
//{
//	/* test signature (std::string description)  -- originally from Function 'setDescription' */
//	/* test signature () const  -- originally from Function 'getDescription' */
//	CPPUNIT_FAIL("not implemented");
//}
//
//void NodeTest::testGetSetURI()
//{
//	/* test signature (std::string uri)  -- originally from Function 'setURI' */
//	/* test signature () const  -- originally from Function 'getURI' */
//	CPPUNIT_FAIL("not implemented");
//}
//
//void NodeTest::testGetSetTimeout()
//{
//	/* test signature (int timeout)  -- originally from Function 'setTimeout' */
//	/* test signature () const  -- originally from Function 'getTimeout' */
//	CPPUNIT_FAIL("not implemented");
//}
//
//void NodeTest::testGetRoundTripTime()
//{
//	/* test signature () const */
//	CPPUNIT_FAIL("not implemented");
//}
//
//void NodeTest::testDecrementTimeout()
//{
//	/* test signature (int step) */
//	CPPUNIT_FAIL("not implemented");
//}
//
//void NodeTest::testOperatorEqual()
//{
//	/* test signature (const Node &other) const */
//	CPPUNIT_FAIL("not implemented");
//}
//
//void NodeTest::testOperatorLessThan()
//{
//	/* test signature (const Node &other) const */
//	CPPUNIT_FAIL("not implemented");
//}
//
//void NodeTest::testToString()
//{
//	/* test signature () const */
//	CPPUNIT_FAIL("not implemented");
//}

/*=== END   tests for class 'Node' ===*/

void NodeTest::setUp()
{
}

void NodeTest::tearDown()
{
}

