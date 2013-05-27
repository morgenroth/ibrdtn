/*
 * TestExtensionBlock.h
 *
 *  Created on: 16.01.2013
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TESTEXTENSIONBLOCK_H_
#define TESTEXTENSIONBLOCK_H_

class TestExtensionBlock : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (TestExtensionBlock);
	CPPUNIT_TEST (deserializeUnknownBlock);
	CPPUNIT_TEST_SUITE_END ();

	static void hexdump(const unsigned char &c);
public:
	void setUp (void);
	void tearDown (void);

protected:
	void deserializeUnknownBlock(void);
};

#endif /* TESTEXTENSIONBLOCK_H_ */
