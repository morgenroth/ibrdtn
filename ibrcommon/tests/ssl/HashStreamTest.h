/*
 * HashStreamTest.h
 *
 *  Created on: 12.07.2010
 *      Author: morgenro
 */

#ifndef HASHSTREAMTEST_H_
#define HASHSTREAMTEST_H_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>

class HashStreamTest : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (HashStreamTest);
	CPPUNIT_TEST (hmacstream_test01);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void hmacstream_test01();

private:
	std::string getHex(std::istream &stream);
};

#endif /* HASHSTREAMTEST_H_ */
