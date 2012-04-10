/*
 * BundleCreator.cpp
 *
 *  Created on: 24.09.2010
 *      Author: Myrtus
 */
#include "tests/tools/BundleCreator.h"

string BundleCreator::getFile(){
	string result = path;
	result << "/TestBlockXXXXXX";
	char *temp = strdup(result.c_str());
	fd = mkstemp(temp);
	if (fd == -1){
		free(temp);
		throw ibrcommon::IOException("Could not create file.");
	}
	close(fd);
	result = temp;
	free(temp);
	return result;
}

void BundleCreator::addPayloadBlock(dtn::data::Bundle &bundle,int size = 1000, int fragmentnumber = -1, int fragments = 1){
	fstream payloadfile;
	string filepath;
	int add,rest = size % fragments;

	path = getFile();
	payloadfile.open(filepath,ios::out|ios::binary);

	for (int i = 1; i<=fragments; i++){
		if(rest > 0){
			rest--;
			add = 1;
		}
		else {add = 0;}

		for(int j = 0; j < (size/fragments)+add;j++){
			if (fragmentnumber == -1)
				payloadfile << i;
			else
				payloadfile << fragmentnumber;
		}
	}
	payloadfile.close();

	ibrcommon::File file = ibrcommon::File(filepath);
	if(!file.exists()){
		cerr << "Datei existiert nicht"<<endl;
	}
	ibrcommon::BLOB::Reference payloadblob = ibrcommon::FileBLOB::create(file);
	bundle.push_front(payloadblob);
}

void BundleCreator::addBlock(dtn::data::Bundle &bundle, int blocknumber, Position pos){
	dtn::data::ExtensionBlock &block;
	if(pos = BundleCreator::FRONT)
		block = bundle.push_front<dtn::data::ExtensionBlock>();
	else
		block = bundle.push_back<dtn::data::ExtensionBlock>();

	ibrcommon::BLOB::Reference blob = block.getBLOB();
	blob.enter();
	(*blob) << blocknumber;
	blob.leave();
}

void BundleCreator::addBlocks(dtn::data::Bundle &bundle, int quantity, Position pos){
	for (int i = 0; i<quantity;i++){
		addBlock(bundle,i,pos);
	}
}

void BundleCreator::createBundle(dtn::data::Bundle &bundle,string souceID = "dtn:alice/app3", string destID = "dtn:bob/app2", int payloadsize, int blocksBeforePayload = 0, int blocksAfterPayload = 0){
	bundle._source = dtn::data::EID(sourceID);
	bundle._destination = dtn::data::EID(destID);
	bundle._procflags += dtn::data::Bundle::PRIORITY_BIT1;
	bundle._procflags += dtn::data::Bundle::CUSTODY_REQUESTED;
	bundle._reportto = dtn::data::EID(source);
	addPayloadBlock(bundle);
	addBlocks(bundle,blocksBeforePayload,Position::FRONT);
	addBlocks(bundle,blocksAfterPayload,Position::BACK);
}

void BundleCreator::createBundle(dtn::data::Bundle &bundle, ibrcommon::File &file, string souceID = "dtn:alice/app3", string destID = "dtn:bob/app2", int blocksBeforePayload = 0, int blocksAfterPayload = 0){
	bundle._source = dtn::data::EID(sourceID);
	bundle._destination = dtn::data::EID(destID);
	bundle._procflags += dtn::data::Bundle::PRIORITY_BIT1;
	bundle._procflags += dtn::data::Bundle::CUSTODY_REQUESTED;
	bundle._reportto = dtn::data::EID(source);
	ibrcommon::BLOB::Reference payloadblob = ibrcommon::FileBLOB::create(file);
	bundle.push_front(payloadblob);
	addBlocks(bundle,blocksBeforePayload,Position::FRONT);
	addBlocks(bundle,blocksAfterPayload,Position::Back);
}

void BundleCreator::createFragment(dtn::data::Bundle *bundle, int fragments, int payloadsize, string souceID = "dtn:alice/app3", string destID = "dtn:bob/app2", int blocksBeforePayload = 0, int blocksAfterPayload = 0){
	if(fragments = 0){
		throw("Error in function createFragment: argument fragments mustn't be 0");
	}
	int add,rest = payloadsize % fragments;
	bundle[0]._source = dtn::data::EID(sourceID);
	bundle[0]._destination = dtn::data::EID(destID);
	bundle[0]._procflags += dtn::data::Bundle::PRIORITY_BIT1;
	bundle[0]._procflags += dtn::data::Bundle::CUSTODY_REQUESTED;
	bundle[0]._reportto = dtn::data::EID(source);
	addPayloadBlock(bundle[0],payloadsize,(-1),number);
	for(int i = 1;  i < fragments+1; i++){
		bundle[i]._source = dtn::data::EID(sourceID);
		bundle[i]._destination = dtn::data::EID(destID);
		bundle[i]._procflags += dtn::data::Bundle::FRAGMENT;
		bundle[i]._procflags += dtn::data::Bundle::PRIORITY_BIT1;
		bundle[i]._procflags += dtn::data::Bundle::CUSTODY_REQUESTED;
		bundle[i]._reportto = dtn::data::EID(source);
		if(rest > 0){
			addPayloadblock(bundle[i],(payloadsize/fragments)+1,i);
			rest--;
		}
		else
			addPayloadblock(bundle[i],(payloadsize/fragments),i);
	}
}


