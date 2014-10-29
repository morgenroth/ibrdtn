package de.tubs.ibr.dtn.chat;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import android.annotation.SuppressLint;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.preference.PreferenceManager;
import android.support.v4.content.AsyncTaskLoader;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Roster;
import de.tubs.ibr.dtn.chat.service.ChatService;

@SuppressLint("SimpleDateFormat")
public class RosterLoader extends AsyncTaskLoader<Cursor> {

	private ChatService mService = null;
	private Cursor mData = null;
	private Boolean mObserving = false;

	public RosterLoader(Context context, ChatService service) {
		super(context);
		mService = service;
	}

	@Override
	public Cursor loadInBackground() {
		SQLiteDatabase db = mService.getRoster().getDatabase();
		
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getContext());
		if (prefs.getBoolean("hideOffline", false)) {
			Calendar cal = Calendar.getInstance();
			cal.add(Calendar.HOUR, -1);
			
			Date offline_limit = cal.getTime();
			SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
			
			// load online buddies
			return db.query(Roster.TABLE_NAME_ROSTER, RosterAdapter.PROJECTION,
					Buddy.LASTSEEN + " >= ? OR " + Buddy.PINNED + " > 0", new String[] { dateFormat.format(offline_limit) }, null, null, Buddy.NICKNAME);
		} else {
			// load all buddies
			return db.query(Roster.TABLE_NAME_ROSTER, RosterAdapter.PROJECTION,
					null, null, null, null, Buddy.NICKNAME);
		}
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
			
	        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getContext());
	        prefs.unregisterOnSharedPreferenceChangeListener(mPrefListener);
	        
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
        IntentFilter filter = new IntentFilter(Roster.NOTIFY_ROSTER_CHANGED);
        getContext().registerReceiver(_receiver, filter);
        
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getContext());
        prefs.registerOnSharedPreferenceChangeListener(mPrefListener);
        
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
	
	private OnSharedPreferenceChangeListener mPrefListener = new OnSharedPreferenceChangeListener() {
		@Override
		public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
			if ("hideOffline".equals(key)) {
				onContentChanged();
			}
		}
	};
	
	private BroadcastReceiver _receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (Roster.NOTIFY_ROSTER_CHANGED.equals(intent.getAction())) {
            	onContentChanged();
            }
        }
    };
}
