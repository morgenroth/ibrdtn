/*
 * PayloadConfidentialBlockTest.h
 *
 *  Created on: 01.02.2011
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <string>

#ifndef PAYLOADCONFIDENTIALBLOCKTEST_H_
#define PAYLOADCONFIDENTIALBLOCKTEST_H_

class PayloadConfidentialBlockTest : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (PayloadConfidentialBlockTest);
	CPPUNIT_TEST (encryptTest);
	CPPUNIT_TEST (decryptTest);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void encryptTest(void);
	void decryptTest(void);

private:
	std::string getHex(std::istream &stream);
	std::string _testdata;
};

#endif /* PAYLOADCONFIDENTIALBLOCKTEST_H_ */
