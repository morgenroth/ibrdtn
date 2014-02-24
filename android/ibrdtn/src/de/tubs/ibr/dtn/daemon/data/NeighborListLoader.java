package de.tubs.ibr.dtn.daemon.data;

import java.util.LinkedList;
import java.util.List;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.RemoteException;
import android.support.v4.content.AsyncTaskLoader;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.api.Node;

public class NeighborListLoader extends AsyncTaskLoader<List<Node>> {
	
	private DTNService mService = null;
	private Boolean mStarted = false;
	private List<Node> mData = null;

	public NeighborListLoader(Context context, DTNService service) {
		super(context);
		mService = service;
		setUpdateThrottle(250);
	}
	
	@Override
	public void deliverResult(List<Node> data) {
		if (isReset()) {
			mData = null;
			return;
		}
		
		mData = data;
		
		if (isStarted()) {
			super.deliverResult(data);
		}
	}

	@Override
	protected void onReset() {
		onStopLoading();
		
		if (mStarted) {
			// unregister from intent receiver
			getContext().unregisterReceiver(_receiver);
			mStarted = false;
		}
	}

	@Override
	protected void onStartLoading() {
		if (mData != null) {
			this.deliverResult(mData);
		}
		
        IntentFilter filter = new IntentFilter(de.tubs.ibr.dtn.Intent.NEIGHBOR);
        filter.addCategory(Intent.CATEGORY_DEFAULT);
        getContext().registerReceiver(_receiver, filter);
        mStarted = true;
        
        if (this.takeContentChanged() || mData == null) {
        	forceLoad();
        }
	}

	@Override
	protected void onStopLoading() {
		cancelLoad();
	}

	@Override
	public List<Node> loadInBackground() {
		try {
			return mService.getNeighbors();
		} catch (RemoteException e) {
			e.printStackTrace();
			return new LinkedList<Node>();
		}
	}

	private BroadcastReceiver _receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (de.tubs.ibr.dtn.Intent.NEIGHBOR.equals(intent.getAction())) {
            	onContentChanged();
            }
        }
    };
}
