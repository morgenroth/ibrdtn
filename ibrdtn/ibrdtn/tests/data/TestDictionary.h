/*
 * TestDictionary.h
 *
 *  Created on: 23.06.2010
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TESTDICTIONARY_H_
#define TESTDICTIONARY_H_

#include <ibrdtn/data/Dictionary.h>

class TestDictionary : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (TestDictionary);
	CPPUNIT_TEST (mainTest);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void mainTest(void);

};

#endif /* TESTDICTIONARY_H_ */
