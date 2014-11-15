package de.tubs.ibr.dtn.api;

import android.os.ParcelFileDescriptor;

public abstract class SimpleDataHandler implements DataHandler {
	
	private Bundle mBundle = null;
	
	protected abstract void onMessage(BundleID id, byte[] data);

	@Override
	public void startBundle(Bundle bundle) {
		mBundle = bundle;
	}

	@Override
	public void endBundle() {
		mBundle = null;
	}

	@Override
	public TransferMode startBlock(Block block) {
		if (block.type == 1 && block.length <= 4096) {
			return TransferMode.SIMPLE;
		} else {
			return TransferMode.NULL;
		}
	}

	@Override
	public void endBlock() {
	}

	@Override
	public void payload(byte[] data) {
		onMessage(new BundleID(mBundle), data);
	}

	@Override
	public ParcelFileDescriptor fd() {
		return null;
	}

	@Override
	public void progress(long current, long length) {
	}
}
