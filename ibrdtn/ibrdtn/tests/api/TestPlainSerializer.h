/*
 * TestPlainSerializer.h
 *
 *  Created on: 04.10.2011
 *      Author: roettger
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TESTPLAINSERIALIZER_H_
#define TESTPLAINSERIALIZER_H_

class TestPlainSerializer : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (TestPlainSerializer);
	CPPUNIT_TEST (plain_serializer_inversion);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void plain_serializer_inversion(void);
};

#endif /* TESTPLAINSERIALIZER_H_ */
