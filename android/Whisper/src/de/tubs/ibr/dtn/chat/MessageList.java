package de.tubs.ibr.dtn.chat;

import android.content.Context;
import android.database.Cursor;
import android.os.Bundle;
import android.support.v4.app.LoaderManager;
import android.support.v4.content.Loader;
import android.support.v4.widget.CursorAdapter;
import android.util.AttributeSet;
import android.widget.ListView;
import de.tubs.ibr.dtn.chat.service.ChatService;

public class MessageList extends ListView implements
	LoaderManager.LoaderCallbacks<Cursor> {
	
	private ChatService mService = null;
	private CursorAdapter mAdapter = null;
	private Long mBuddyId = null;

	public MessageList(Context context) {
		super(context);
	}
	
    public MessageList(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
    
    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
		
		mAdapter = new MessageAdapter(getContext(), null, new MessageAdapter.ColumnsMap());
		setAdapter(mAdapter);
    }
    
    public void setBuddyId(Long buddyId) {
    	mBuddyId = buddyId;
    }
    
    public void setChatService(ChatService service) {
    	mService = service;
    }

	@Override
	public Loader<Cursor> onCreateLoader(int id, Bundle args) {
        // Now create and return a CursorLoader that will take care of
        // creating a Cursor for the data being displayed.
        return new MessageListLoader(getContext(), mService, mBuddyId);
	}

	@Override
	public void onLoadFinished(Loader<Cursor> loader, Cursor data) {
        // Swap the new cursor in.  (The framework will take care of closing the
        // old cursor once we return.)
        mAdapter.swapCursor(data);
	}

	@Override
	public void onLoaderReset(Loader<Cursor> arg0) {
        // This is called when the last Cursor provided to onLoadFinished()
        // above is about to be closed.  We need to make sure we are no
        // longer using it.
        mAdapter.swapCursor(null);
	}
}
