package de.tubs.ibr.dtn.daemon.data;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;

import android.annotation.SuppressLint;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.support.v4.content.AsyncTaskLoader;
import android.util.Log;
import de.tubs.ibr.dtn.service.DaemonService;
import de.tubs.ibr.dtn.stats.ConvergenceLayerStatsEntry;
import de.tubs.ibr.dtn.stats.StatsDatabase;

@SuppressLint("SimpleDateFormat")
public class ConvergenceLayerStatsLoader extends AsyncTaskLoader<Cursor> {
    
    private static final String TAG = "ConvergenceLayerStatsLoader";
    
    private DaemonService mService = null;
    private Boolean mStarted = false;
    private Cursor mData = null;
    private String mConvergenceLayer = null;

    public ConvergenceLayerStatsLoader(Context context, DaemonService service, String convergencecayer) {
        super(context);
        mService = service;
        mConvergenceLayer = convergencecayer;
        setUpdateThrottle(250);
    }
    
    @Override
    public void deliverResult(Cursor data) {
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
        
        IntentFilter filter = new IntentFilter(StatsDatabase.NOTIFY_DATABASE_UPDATED);
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
    public Cursor loadInBackground() {
        SQLiteDatabase db = mService.getStatsDatabase().getDB();
        
        final DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        
        // generate a time limit (24 hours)
        Calendar cal = Calendar.getInstance();
        cal.roll(Calendar.DATE, -1);
        String timestamp_limit = formatter.format(cal.getTime());

        try {
            if (mConvergenceLayer == null) {
                // limit to specific download
                return db.query(
                        StatsDatabase.TABLE_NAMES[1],
                        ConvergenceLayerStatsEntry.PROJECTION,
                        ConvergenceLayerStatsEntry.TIMESTAMP + " >= ?",
                        new String[] { timestamp_limit },
                        null, null, ConvergenceLayerStatsEntry.TIMESTAMP + " ASC");
            } else {
                // limit to specific download
                return db.query(
                        StatsDatabase.TABLE_NAMES[1],
                        ConvergenceLayerStatsEntry.PROJECTION,
                        ConvergenceLayerStatsEntry.TIMESTAMP + " >= ? AND " + ConvergenceLayerStatsEntry.CONVERGENCE_LAYER + " = ?",
                        new String[] { timestamp_limit, mConvergenceLayer },
                        null, null, ConvergenceLayerStatsEntry.TIMESTAMP + " ASC");
            }
        } catch (Exception e) {
            Log.e(TAG, "loadInBackground() failed", e);
        }
        
        return null;
    }

    private BroadcastReceiver _receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            onContentChanged();
        }
    };
}
