/*
 * TestBundleList.h
 *
 *  Created on: 02.06.2010
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ibrdtn/data/BundleList.h>

#ifndef TESTBUNDLELIST_H_
#define TESTBUNDLELIST_H_

using namespace std;

class TestBundleList: public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (TestBundleList);
	CPPUNIT_TEST (orderTest);
	CPPUNIT_TEST (containTest);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void orderTest(void);
	void containTest(void);

private:
	class DerivedBundleList : public dtn::data::BundleList
	{
	public:
		DerivedBundleList();
		virtual ~DerivedBundleList();

		void eventBundleExpired(const ExpiringBundle &b);

		int counter;
	};

	void genbundles(DerivedBundleList &l, int number, int offset, int max);

	DerivedBundleList *list;
};

#endif /* TESTBUNDLELIST_H_ */
