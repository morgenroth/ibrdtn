package de.tubs.ibr.dtn.sharebox.ui;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.support.v4.content.AsyncTaskLoader;
import android.util.Log;
import de.tubs.ibr.dtn.sharebox.DtnService;
import de.tubs.ibr.dtn.sharebox.data.Database;
import de.tubs.ibr.dtn.sharebox.data.Download;

public class DownloadLoader extends AsyncTaskLoader<Cursor> {
    private final static String TAG = "DownloadLoader";
    
    private DtnService mService = null;
    private Cursor mData = null;
    private Boolean mObserving = false;
    
    public DownloadLoader(Context context, DtnService service) {
        super(context);
        if (service == null) throw new RuntimeException("The service should not be null!");
        mService = service;
    }
    
    @Override
    public Cursor loadInBackground() {
        SQLiteDatabase db = mService.getDatabase().getDB();

        try {
            return db.query(Database.TABLE_NAMES[0], DownloadAdapter.PROJECTION,
                        null, null, null, null, Download.TIMESTAMP + " DESC");
        } catch (Exception e) {
            // message not found
            Log.e(TAG, "getMessage() failed", e);
        }
        
        return null;
    }
    
    @Override
    public void onCanceled(Cursor data) {
        // Attempt to cancel the current asynchronous load.
        super.onCanceled(data);

        // The load has been canceled, so we should release the resources
        // associated with 'data'.
        releaseResources(data);
    }

    @Override
    public void deliverResult(Cursor data) {
        if (isReset()) {
            // The Loader has been reset; ignore the result and invalidate the
            // data.
            releaseResources(data);
            return;
        }

        // Hold a reference to the old data so it doesn't get garbage collected.
        // We must protect it until the new data has been delivered.
        Cursor oldData = mData;
        mData = data;

        if (isStarted()) {
            // If the Loader is in a started state, deliver the results to the
            // client. The superclass method does this for us.
            super.deliverResult(data);
        }

        // Invalidate the old data as we don't need it any more.
        if (oldData != null && oldData != data) {
            releaseResources(oldData);
        }
    }

    @Override
    protected void onReset() {
        // Ensure the loader has been stopped.
        onStopLoading();

        // At this point we can release the resources associated with 'mData'.
        if (mData != null) {
            releaseResources(mData);
            mData = null;
        }

        // The Loader is being reset, so we should stop monitoring for changes.
        if (mObserving) {
            getContext().unregisterReceiver(_receiver);
            mObserving = false;
        }
    }

    @Override
    protected void onStartLoading() {
        if (mData != null) {
            // Deliver any previously loaded data immediately.
            deliverResult(mData);
        }

        // Begin monitoring the underlying data source.
        IntentFilter filter = new IntentFilter(Database.NOTIFY_DATABASE_UPDATED);
        getContext().registerReceiver(_receiver, filter);
        mObserving = true;

        if (takeContentChanged() || mData == null) {
            // When the observer detects a change, it should call
            // onContentChanged()
            // on the Loader, which will cause the next call to
            // takeContentChanged()
            // to return true. If this is ever the case (or if the current data
            // is
            // null), we force a new load.
            forceLoad();
        }
    }

    @Override
    protected void onStopLoading() {
        // The Loader is in a stopped state, so we should attempt to cancel the
        // current load (if there is one).
        cancelLoad();

        // Note that we leave the observer as is. Loaders in a stopped state
        // should still monitor the data source for changes so that the Loader
        // will know to force a new load if it is ever started again.
    }

    private void releaseResources(Cursor data) {
        if (data != null) data.close();
    }
    
    private BroadcastReceiver _receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (Database.NOTIFY_DATABASE_UPDATED.equals(intent.getAction())) {
                onContentChanged();
            }
        }
    };
}
