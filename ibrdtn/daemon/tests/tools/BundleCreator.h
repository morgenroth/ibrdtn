/*
 * BundleCreator.h
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 * Written-by: Matthias Myrtus
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef BUNDLECREATOR_H_
#define BUNDLECREATOR_H_

#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/EID.h"

class BundleCreator{
public:
	enum Position {FRONT, BACK};

	/*
	 * Constructor
	 * @param the path to the foldes where the Payloadfiles should be placed
	 */
	BundleCreator(string path): _path(path){};
	virtual ~BundleCreator(){};

	/*
	 * Creates a Bundle
	 * @param Reference to the resulting Bundleobject.
	 * @param sourceID consigns the sourceID of the bundle.
	 * @param destID consigns the destinationID of the bundle.
	 * @param blocksBeforePayload consigns the number of Blocks before the Payloadblock.
	 * @param blocksAfterPayload consigns the number of Blocks after the Payloadblock
	 */
	void createBundle(dtn::data::Bundle &bundle,string souceID = "dtn:alice/app3", string destID = "dtn:bob/app2", int payloadsize, int blocksBeforePayload = 0, int blocksAfterPayload = 0);

	/*
	 * Creates a Bundle and uses the consigned Fileobject when the Payloadblock is generated.
	 *
	 * @param Reference to the resulting Bundleobject.
	 * @param Fileobject which is used to create the Payloadblock
	 * @param sourceID consigns the sourceID of the bundle.
	 * @param destID consigns the destinationID of the bundle.
	 * @param blocksBeforePayload consigns the number of Blocks before the Payloadblock.
	 * @param blocksAfterPayload consigns the number of Blocks after the Payloadblock
	 */
	void createBundle(dtn::data::Bundle &bundle, ibrcommon::File &file, string souceID = "dtn:alice/app3", string destID = "dtn:bob/app2", int blocksBeforePayload = 0, int blocksAfterPayload = 0);

	/*
	 * Creates a bundle and splits it up into a specified number of fragments. The resulting bundlearray contains
	 * the original bundle at the first position followed by the fragments.
	 *
	 * @param bundle is pointing to an array where the resulting BundleFragments are being placed. The Array has to be initialized (size = fragments +1).
	 * @param fragments consigns the number of fragments to generate.
	 * @param payloadsize consigns the size od the Payload (in letters).
	 * @param sourceID consigns the sourceID of the bundle.
	 * @param destID consigns the destinationID of the bundle.
	 * @param blocksBeforePayload consigns the number of Blocks before the Payloadblock.
	 * @param blocksAfterPayload consigns the number of Blocks after the Payloadblock
	 */
	void createFragment(dtn::data::Bundle *bundle, int fragments, int payloadsize, string souceID = "dtn:alice/app3", string destID = "dtn:bob/app2", int blocksBeforePayload = 0, int blocksAfterPayload = 0);

private:
	void getFile();
	void addPayloadBlock(dtn::data::Bundle &bundle,int size = 1000, int fragmentnumber = -1, int fragments = 1);
	void addBlock(dtn::data::Bundle &bundle, int blocknumber, Position pos);
	void addBlocks(dtn::data::Bundle &bundle, int quantity, Position pos);

	string _path;
};


#endif /* BUNDLECREATOR_H_ */
