package de.tubs.ibr.dtn.api;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.DTNSessionCallback;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.CallbackMode;
import android.os.ParcelFileDescriptor;

interface DTNSession {
	/**
	 * Send a string as a bundle to the given destination.
	 */
	boolean send(in SingletonEndpoint destination, int lifetime, String data);
	
	/**
	 * Send a string as a bundle to the given destination.
	 */
	boolean sendGroup(in GroupEndpoint destination, int lifetime, String data);
	
	/**
	 * Send the content of a file descriptor as bundle
	 */
	boolean sendFileDescriptor(in SingletonEndpoint destination, int lifetime, in ParcelFileDescriptor fd, long length);
	
	/**
	 * Send the content of a file descriptor as bundle
	 */
	boolean sendGroupFileDescriptor(in GroupEndpoint destination, int lifetime, in ParcelFileDescriptor fd, long length);
	
	/**
	 * query a given bundle id
	 */
	boolean query(DTNSessionCallback cb, in CallbackMode mode, in BundleID id);
	boolean queryNext(DTNSessionCallback cb, in CallbackMode mode);
	
	/**
	 * set a bundle as delivered
	 */
	boolean delivered(in BundleID id);
}
