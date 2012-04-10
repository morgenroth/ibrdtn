/*
 * Base64StreamTest.h
 *
 *  Created on: 16.06.2011
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef BASE64STREAMTEST_H_
#define BASE64STREAMTEST_H_

class Base64StreamTest : public CppUnit::TestFixture {
	private:
		void compare(std::istream &s1, std::istream &s2);

	public:
		/*=== BEGIN tests for class 'Base64StreamTest' ===*/
		void testEncode();
		void testDecode();
		void testReader();
		void testFileReference();
		/*=== END   tests for class 'Base64StreamTest' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(Base64StreamTest);
		CPPUNIT_TEST(testEncode);
		CPPUNIT_TEST(testDecode);
		CPPUNIT_TEST(testReader);
		CPPUNIT_TEST(testFileReference);
		CPPUNIT_TEST_SUITE_END();
};

#endif /* BASE64STREAMTEST_H_ */
