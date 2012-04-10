/*
 * TestCompressedPayloadBlock.h
 *
 *  Created on: 20.04.2011
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ibrdtn/data/CompressedPayloadBlock.h>

#ifndef TESTCOMPRESSEDPAYLOADBLOCK_H_
#define TESTCOMPRESSEDPAYLOADBLOCK_H_

class TestCompressedPayloadBlock : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (TestCompressedPayloadBlock);
	CPPUNIT_TEST (compressTest);
	CPPUNIT_TEST (extractTest);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void compressTest(void);
	void extractTest(void);
};

#endif /* TESTCOMPRESSEDPAYLOADBLOCK_H_ */
