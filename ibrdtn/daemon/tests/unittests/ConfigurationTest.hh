/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        ConfigurationTest.hh
/// @brief       CPPUnit-Tests for class Configuration
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "Configuration.h"
#include <ibrcommon/data/File.h>

#ifndef CONFIGURATIONTEST_HH
#define CONFIGURATIONTEST_HH
class ConfigurationTest : public CppUnit::TestFixture {
	public:
		/*=== BEGIN tests for class 'Configuration' ===*/
		void testParams();
		void testGetNodename();
		void testGetPath();
		void testDoAPI();
		void testGetAPIInterface();
		void testGetAPISocket();
		void testVersion();
		void testGetStorage();
		void testGetLimit();
		/*=== BEGIN tests for class 'Discovery' ===*/
		void testDiscoveryEnabled();
		void testDiscoveryAnnounce();
		void testDiscoveryShortbeacon();
		void testDiscoveryVersion();
		void testDiscoveryAddress();
		void testDiscoveryPort();
		void testDiscoveryInterval();
		/*=== END   tests for class 'Discovery' ===*/

		/*=== BEGIN tests for class 'Debug' ===*/
		void testDebugLevel();
		void testDebugEnabled();
		void testDebugQuiet();
		/*=== END   tests for class 'Debug' ===*/

		/*=== BEGIN tests for class 'Logger' ===*/
		void testQuiet();
		void testOptions();
		void testOutput();
		/*=== END   tests for class 'Logger' ===*/

		/*=== BEGIN tests for class 'Network' ===*/
		void testGetInterfaces();
		void testGetStaticNodes();
		void testGetStaticRoutes();
		void testGetRoutingExtension();
		void testDoForwarding();
		void testDoAcceptNonsingleton();
		void testGetTCPOptionNoDelay();
		void testGetTCPChunkSize();
		/*=== END   tests for class 'Network' ===*/

		void testGetDiscovery();
		void testGetDebug();
		void testGetLogger();
		void testGetNetwork();
		/*=== END   tests for class 'Configuration' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(ConfigurationTest);
			CPPUNIT_TEST(testParams);
			CPPUNIT_TEST(testGetNodename);
			CPPUNIT_TEST(testGetPath);
			CPPUNIT_TEST(testDoAPI);
			CPPUNIT_TEST(testGetAPIInterface);
			CPPUNIT_TEST(testGetAPISocket);
			CPPUNIT_TEST(testVersion);
			CPPUNIT_TEST(testGetStorage);
			CPPUNIT_TEST(testGetLimit);
			CPPUNIT_TEST(testDiscoveryEnabled);
			CPPUNIT_TEST(testDiscoveryAnnounce);
			CPPUNIT_TEST(testDiscoveryShortbeacon);
			CPPUNIT_TEST(testDiscoveryVersion);
			CPPUNIT_TEST(testDiscoveryAddress);
			CPPUNIT_TEST(testDiscoveryPort);
			CPPUNIT_TEST(testDiscoveryInterval);
			CPPUNIT_TEST(testDebugLevel);
			CPPUNIT_TEST(testDebugEnabled);
			CPPUNIT_TEST(testDebugQuiet);
			CPPUNIT_TEST(testQuiet);
			CPPUNIT_TEST(testOptions);
			CPPUNIT_TEST(testOutput);
			CPPUNIT_TEST(testGetInterfaces);
			CPPUNIT_TEST(testGetStaticNodes);
			CPPUNIT_TEST(testGetStaticRoutes);
			CPPUNIT_TEST(testGetRoutingExtension);
			CPPUNIT_TEST(testDoForwarding);
			CPPUNIT_TEST(testDoAcceptNonsingleton);
			CPPUNIT_TEST(testGetTCPOptionNoDelay);
			CPPUNIT_TEST(testGetTCPChunkSize);
			CPPUNIT_TEST(testGetDiscovery);
			CPPUNIT_TEST(testGetDebug);
			CPPUNIT_TEST(testGetLogger);
			CPPUNIT_TEST(testGetNetwork);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* CONFIGURATIONTEST_HH */
