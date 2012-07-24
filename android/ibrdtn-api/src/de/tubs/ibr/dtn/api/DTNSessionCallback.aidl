/*
 * DTNSessionCallback.aidl
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
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
 
package de.tubs.ibr.dtn.api;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.Event;
import de.tubs.ibr.dtn.api.Bundle;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.Block;
import de.tubs.ibr.dtn.api.TransferMode;
import android.os.ParcelFileDescriptor;

interface DTNSessionCallback {
	void startBundle(in de.tubs.ibr.dtn.api.Bundle bundle);
	void endBundle();
	
	TransferMode startBlock(in de.tubs.ibr.dtn.api.Block block);
	void endBlock();
	
	/**
	 * This method is called if callback mode is set to FILEDESCRIPTOR
	 *
	 * Asks for a file descriptor to store the payload
	 * of the block previously announced by startBlock()
	 */
	ParcelFileDescriptor fd();
	
	/**
	 * This method is called if callback mode is set to FILEDESCRIPTOR
	 */
	void progress(long current, long length);
	
	/**
	 * This method is called if callback mode is set to SIMPLE
	 * 
	 * Hand-over the full payload as byte array. 
	 */
	void payload(in byte[] data);
	
	/**
	 * This method is called if callback mode is set to PASSTHROUGH
	 * 
	 * Hand-over a part of the base64 encoded payload data. 
	 */
	void characters(String data);
}