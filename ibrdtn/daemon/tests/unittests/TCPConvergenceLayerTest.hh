/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        TCPConvergenceLayerTest.hh
/// @brief       CPPUnit-Tests for class TCPConvergenceLayer
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TCPCONVERGENCELAYERTEST_HH
#define TCPCONVERGENCELAYERTEST_HH
class TCPConvergenceLayerTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'TCPConvergenceLayer' ===*/
		/*=== BEGIN tests for class 'TCPConnection' ===*/
		void testTCPConnectionShutdown();
		void testTCPConnectionGetHeader();
		void testTCPConnectionGetNode();
		void testTCPConnectionGetDiscoveryProtocol();
		void testTCPConnectionQueue();
		void testTCPConnectionOperatorShiftRight();
		void testTCPConnectionOperatorShiftLeft();
		void testTCPConnectionRejectTransmission();
		void testTCPConnectionRun();
		void testTCPConnectionFinally();
		void testTCPConnectionClearQueue();
		void testTCPConnectionKeepalive();
		/*=== END   tests for class 'TCPConnection' ===*/

		/*=== BEGIN tests for class 'Server' ===*/
		void testServerQueue();
		void testServerRaiseEvent();
		void testServerAccept();
		void testServerListen();
		void testServerShutdown();
		void testServerConnectionUp();
		void testServerConnectionDown();
		/*=== END   tests for class 'Server' ===*/

		void testGetDiscoveryProtocol();
		void testInitialize();
		void testStartup();
		void testTerminate();
		void testUpdate();
		void testOnInterface();
		void testQueue();
		/*=== END   tests for class 'TCPConvergenceLayer' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(TCPConvergenceLayerTest);
			CPPUNIT_TEST(testTCPConnectionShutdown);
			CPPUNIT_TEST(testTCPConnectionGetHeader);
			CPPUNIT_TEST(testTCPConnectionGetNode);
			CPPUNIT_TEST(testTCPConnectionGetDiscoveryProtocol);
			CPPUNIT_TEST(testTCPConnectionQueue);
			CPPUNIT_TEST(testTCPConnectionOperatorShiftRight);
			CPPUNIT_TEST(testTCPConnectionOperatorShiftLeft);
			CPPUNIT_TEST(testTCPConnectionRejectTransmission);
			CPPUNIT_TEST(testTCPConnectionRun);
			CPPUNIT_TEST(testTCPConnectionFinally);
			CPPUNIT_TEST(testTCPConnectionClearQueue);
			CPPUNIT_TEST(testTCPConnectionKeepalive);
			CPPUNIT_TEST(testServerQueue);
			CPPUNIT_TEST(testServerRaiseEvent);
			CPPUNIT_TEST(testServerAccept);
			CPPUNIT_TEST(testServerListen);
			CPPUNIT_TEST(testServerShutdown);
			CPPUNIT_TEST(testServerConnectionUp);
			CPPUNIT_TEST(testServerConnectionDown);
			CPPUNIT_TEST(testGetDiscoveryProtocol);
			CPPUNIT_TEST(testInitialize);
			CPPUNIT_TEST(testStartup);
			CPPUNIT_TEST(testTerminate);
			CPPUNIT_TEST(testUpdate);
			CPPUNIT_TEST(testOnInterface);
			CPPUNIT_TEST(testQueue);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* TCPCONVERGENCELAYERTEST_HH */
