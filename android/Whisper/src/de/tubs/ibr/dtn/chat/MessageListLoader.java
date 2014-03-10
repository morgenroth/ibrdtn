package de.tubs.ibr.dtn.chat;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.support.v4.content.AsyncTaskLoader;
import de.tubs.ibr.dtn.chat.core.Message;
import de.tubs.ibr.dtn.chat.core.Roster;
import de.tubs.ibr.dtn.chat.service.ChatService;

public class MessageListLoader extends AsyncTaskLoader<Cursor> {
	
	private ChatService mService = null;
	private Long mBuddyId = null;
	private Cursor mData = null;
	private Boolean mObserving = false;

	public MessageListLoader(Context context, ChatService service, Long buddyid) {
		super(context);
		if (service == null) throw new RuntimeException("The service should not be null!");
		mService = service;
		mBuddyId = buddyid;
	}

	@Override
	public Cursor loadInBackground() {
		SQLiteDatabase db = mService.getRoster().getDatabase();

		// load all buddies
		return db.query(Roster.TABLE_NAME_MESSAGES, MessageAdapter.PROJECTION,
				Message.BUDDY + " = ?", new String[] { mBuddyId.toString() }, null, null, null, null);
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
        IntentFilter filter = new IntentFilter(Roster.NOTIFY_ROSTER_CHANGED);
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
            if (Roster.NOTIFY_ROSTER_CHANGED.equals(intent.getAction())) {
            	Long changedBuddy = intent.getLongExtra(ChatService.EXTRA_BUDDY_ID, -1);
            	if (changedBuddy.equals(mBuddyId) && (changedBuddy >= 0)) {
            		onContentChanged();
            	}
            }
        }
    };
}
