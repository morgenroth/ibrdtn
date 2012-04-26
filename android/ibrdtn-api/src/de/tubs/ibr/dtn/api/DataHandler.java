package de.tubs.ibr.dtn.api;

import android.os.ParcelFileDescriptor;

public interface DataHandler {
	public void startBundle(Bundle bundle);
	public void endBundle();
	public void startBlock(Block block);
	public void endBlock(); 
	public void characters(String data);
	public void payload(byte[] data);
	public ParcelFileDescriptor fd();
	public void progress(long current, long length);
	public void finished(int startId);
}
