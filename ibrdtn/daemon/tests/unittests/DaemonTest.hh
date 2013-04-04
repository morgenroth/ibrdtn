 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef DAEMONTEST_HH
#define DAEMONTEST_HH
class DaemonTest : public CppUnit::TestFixture {
	private:

	public:
		/*=== BEGIN tests for dtnd ===*/
		void testUpDownUp();
		/*=== END   tests for dtnd ===*/


		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(DaemonTest);
			CPPUNIT_TEST(testUpDownUp);
		CPPUNIT_TEST_SUITE_END();
};
#endif
