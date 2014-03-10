package de.tubs.ibr.dtn.chat;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.support.v4.content.AsyncTaskLoader;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Roster;
import de.tubs.ibr.dtn.chat.service.ChatService;

public class BuddyLoader extends AsyncTaskLoader<Buddy> {

	private ChatService mService = null;
	private Long mBuddyId = null;
	private Buddy mData = null;
	private Boolean mObserving = false;
	
	public BuddyLoader(Context context, ChatService service, Long buddyid) {
		super(context);
		mService = service;
		mBuddyId = buddyid;
	}

	@Override
	public Buddy loadInBackground() {
		return mService.getRoster().getBuddy(mBuddyId);
	}
	
	@Override
	public void onCanceled(Buddy data) {
		// Attempt to cancel the current asynchronous load.
		super.onCanceled(data);

		// The load has been canceled, so we should release the resources
		// associated with 'data'.
		releaseResources(data);
	}

	@Override
	public void deliverResult(Buddy data) {
		if (isReset()) {
			// The Loader has been reset; ignore the result and invalidate the
			// data.
			releaseResources(data);
			return;
		}

		// Hold a reference to the old data so it doesn't get garbage collected.
		// We must protect it until the new data has been delivered.
		Buddy oldData = mData;
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
			getContext().unregisterReceiver(mReceiver);
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
        getContext().registerReceiver(mReceiver, filter);
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

	private void releaseResources(Buddy data) {
	}
	
	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
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
