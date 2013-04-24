/*
 * NativeSerializerTest.h
 *
 *  Created on: 08.04.2013
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef NATIVESERIALIZERTEST_H_
#define NATIVESERIALIZERTEST_H_

class NativeSerializerTest : public CppUnit::TestFixture {
public:
	/*=== BEGIN tests for dtnd ===*/
	void serializeBundle();
	void callbackStreamTest();
	/*=== END   tests for dtnd ===*/

	void setUp();
	void tearDown();

	CPPUNIT_TEST_SUITE(NativeSerializerTest);
	CPPUNIT_TEST(serializeBundle);
	CPPUNIT_TEST(callbackStreamTest);
	CPPUNIT_TEST_SUITE_END();
};

#endif /* NATIVESERIALIZERTEST_H_ */
