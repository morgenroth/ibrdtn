/*
 * TestTrackingBlock.cpp
 *
 *  Created on: 16.01.2013
 *      Author: morgenro
 */

#include "data/TestTrackingBlock.h"
#include <ibrdtn/data/TrackingBlock.h>
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/Serializer.h>
#include <ibrcommon/data/BLOB.h>
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION (TestTrackingBlock);

void TestTrackingBlock::setUp()
{
}

void TestTrackingBlock::tearDown()
{
}

void TestTrackingBlock::createTest(void)
{
	dtn::data::Bundle b;
	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();

	// generate some test data
	{
		ibrcommon::BLOB::iostream stream = ref.iostream();
		for (int i = 0; i < 10000; ++i)
		{
			(*stream) << "0123456789";
		}
	}

	// add a payload block
	b.push_back(ref);

	dtn::data::TrackingBlock &tracking = b.push_front<dtn::data::TrackingBlock>();

	dtn::data::EID hops[3] = { dtn::data::EID("dtn://hop1"), dtn::data::EID("dtn://hop2"), dtn::data::EID("dtn://hop3") };

	tracking.append(hops[0]);
	tracking.append(hops[1]);
	tracking.append(hops[2]);

	std::stringstream ss;
	dtn::data::DefaultSerializer ds(ss);

	// serialize the bundle
	ds << b;

	dtn::data::DefaultDeserializer dds(ss);
	dtn::data::Bundle dest;

	// deserialize the bundle
	dds >> dest;

	const dtn::data::TrackingBlock &dest_tracking = b.find<dtn::data::TrackingBlock>();

	const dtn::data::TrackingBlock::tracking_list &list = dest_tracking.getTrack();

	CPPUNIT_ASSERT_EQUAL(list.size(), (size_t)3);

	int i = 0;
	for (dtn::data::TrackingBlock::tracking_list::const_iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		const dtn::data::TrackingBlock::TrackingEntry &entry = (*iter);
		CPPUNIT_ASSERT_EQUAL(entry.endpoint.getString(), hops[i].getString());
		i++;
	}
}
