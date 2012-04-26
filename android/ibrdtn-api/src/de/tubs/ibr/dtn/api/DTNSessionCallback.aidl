package de.tubs.ibr.dtn.api;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.Event;
import de.tubs.ibr.dtn.api.Bundle;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.Block;
import android.os.ParcelFileDescriptor;

interface DTNSessionCallback {
	void startBundle(in de.tubs.ibr.dtn.api.Bundle bundle);
	void endBundle();
	
	void startBlock(in de.tubs.ibr.dtn.api.Block block);
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