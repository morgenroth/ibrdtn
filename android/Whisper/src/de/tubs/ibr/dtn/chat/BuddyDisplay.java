package de.tubs.ibr.dtn.chat;

import android.content.Context;
import android.os.Bundle;
import android.support.v4.app.LoaderManager;
import android.support.v4.content.Loader;
import android.util.AttributeSet;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.service.ChatService;

public class BuddyDisplay extends LinearLayout implements
	LoaderManager.LoaderCallbacks<Buddy> {
	
	private ChatService mService = null;
	private Long mBuddyId = null;
	
	ImageView iconTitleBar = null;
	TextView labelTitleBar = null;
	TextView bottomtextTitleBar = null;
	
	public BuddyDisplay(Context context) {
		super(context);
	}
	
    public BuddyDisplay(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        
		iconTitleBar = (ImageView) findViewById(R.id.buddy_icon);
		labelTitleBar = (TextView) findViewById(R.id.buddy_nickname);
		bottomtextTitleBar = (TextView) findViewById(R.id.buddy_statusmessage);
    }
    
    public void setBuddyId(Long buddyId) {
    	mBuddyId = buddyId;
    }
    
    public void setChatService(ChatService service) {
    	mService = service;
    }
    
	@Override
	public Loader<Buddy> onCreateLoader(int id, Bundle args) {
		return new BuddyLoader(getContext(), mService, mBuddyId);
	}

	@Override
	public void onLoadFinished(Loader<Buddy> loader, Buddy data) {
		if (data == null) return;
	    String presence_tag = data.getPresence();
	    String presence_nick = data.getNickname();
	    String presence_text = data.getEndpoint();
	    int presence_icon = R.drawable.online;
	    
		if (data.getStatus() != null)
		{
			if (data.getStatus().length() > 0) { 
				presence_text = data.getStatus();
			} else {
				presence_text = data.getEndpoint();
			}
		}
		
		if ((presence_tag != null) && data.isOnline())
		{
			if (presence_tag.equalsIgnoreCase("unavailable"))
			{
				presence_icon = R.drawable.offline;
			}
			else if (presence_tag.equalsIgnoreCase("xa"))
			{
				presence_icon = R.drawable.xa;
			}
			else if (presence_tag.equalsIgnoreCase("away"))
			{
				presence_icon = R.drawable.away;
			}
			else if (presence_tag.equalsIgnoreCase("dnd"))
			{
				presence_icon = R.drawable.busy;
			}
			else if (presence_tag.equalsIgnoreCase("chat"))
			{
				presence_icon = R.drawable.online;
			}
		}
		
		// if the presence is older than 60 minutes then mark the buddy as offline
		if (!data.isOnline())
		{
			presence_icon = R.drawable.offline;
		}

		labelTitleBar.setText( presence_nick );
		bottomtextTitleBar.setText( presence_text );
		iconTitleBar.setImageResource(presence_icon);
	}

	@Override
	public void onLoaderReset(Loader<Buddy> loader) {
	}
}
