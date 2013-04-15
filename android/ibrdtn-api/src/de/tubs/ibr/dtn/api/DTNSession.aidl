/*
 * DTNSession.aidl
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
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.DTNSessionCallback;
import de.tubs.ibr.dtn.api.BundleID;
import android.os.ParcelFileDescriptor;

interface DTNSession {
	/**
	 * Send a string as a bundle to the given destination.
	 * It returns the BundleID of the transmitted bundle or null
	 * if the send wasn't successful.
	 */
	BundleID send(in SingletonEndpoint destination, int lifetime, in byte[] data);
	
	/**
	 * Send a string as a bundle to the given destination.
	 * It returns the BundleID of the transmitted bundle or null
	 * if the send wasn't successful.
	 */
	BundleID sendGroup(in GroupEndpoint destination, int lifetime, in byte[] data);
	
	/**
	 * Send the content of a file descriptor as bundle
	 * It returns the BundleID of the transmitted bundle or null
	 * if the send wasn't successful.
	 */
	BundleID sendFileDescriptor(in SingletonEndpoint destination, int lifetime, in ParcelFileDescriptor fd, long length);
	
	/**
	 * Send the content of a file descriptor as bundle
	 * It returns the BundleID of the transmitted bundle or null
	 * if the send wasn't successful.
	 */
	BundleID sendGroupFileDescriptor(in GroupEndpoint destination, int lifetime, in ParcelFileDescriptor fd, long length);
	
	/**
	 * query a given bundle id
	 */
	boolean query(DTNSessionCallback cb, in BundleID id);
	boolean queryNext(DTNSessionCallback cb);
	
	/**
	 * set a bundle as delivered
	 */
	boolean delivered(in BundleID id);
}
