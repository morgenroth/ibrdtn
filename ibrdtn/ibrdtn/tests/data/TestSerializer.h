/*
 * TestSerializer.h
 *
 *  Created on: 30.07.2010
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TESTSERIALIZER_H_
#define TESTSERIALIZER_H_

class TestSerializer : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (TestSerializer);
	CPPUNIT_TEST (serializer_separate01);
	CPPUNIT_TEST (serializer_cbhe01);
	CPPUNIT_TEST (serializer_cbhe02);
	CPPUNIT_TEST (serializer_bundle_length);
	CPPUNIT_TEST (serializer_fragment_one);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void serializer_separate01(void);
	void serializer_cbhe01(void);
	void serializer_cbhe02(void);

	void serializer_bundle_length(void);

	void serializer_fragment_one(void);
};

#endif /* TESTSERIALIZER_H_ */
