package de.tubs.ibr.dtn.daemon;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.support.v4.content.AsyncTaskLoader;
import de.tubs.ibr.dtn.service.DaemonService;
import de.tubs.ibr.dtn.swig.NativeStats;

public class StatsLoader extends AsyncTaskLoader<NativeStats> {
    
    private DaemonService mService = null;
    private Boolean mStarted = false;
    private NativeStats mData = null;
    private Handler mHandler = null;

    public StatsLoader(Context context, DaemonService service) {
        super(context);
        mService = service;
        setUpdateThrottle(250);
    }
    
    @Override
    public void deliverResult(NativeStats data) {
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
            mHandler.removeCallbacks(_update);
            
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
        
        mHandler = new Handler();
        mHandler.postDelayed(_update, 1000);
        
        IntentFilter filter = new IntentFilter(de.tubs.ibr.dtn.Intent.EVENT);
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
    public NativeStats loadInBackground() {
        return mService.getStats();
    }
    
    private Runnable _update = new Runnable() {
        @Override
        public void run() {
            onContentChanged();
            
            if (mStarted)
                mHandler.postDelayed(_update, 1000);
        }
    };

    private BroadcastReceiver _receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            onContentChanged();
        }
    };
}
