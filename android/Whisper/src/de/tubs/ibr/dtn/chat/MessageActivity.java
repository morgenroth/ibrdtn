/*
 * MessageActivity.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
package de.tubs.ibr.dtn.chat;

import android.app.ActionBar;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.view.MenuItemCompat;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.Window;
import android.widget.ImageView;
import android.widget.TextView;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.service.ChatService;
import de.tubs.ibr.dtn.chat.service.Utils;

public class MessageActivity extends FragmentActivity {
	
	private final String TAG = "MessageActivity";
	private String buddyId = null;
	private ChatService service = null;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
	    super.onCreate(savedInstanceState);
	    
	    if (android.os.Build.VERSION.SDK_INT < Utils.ANDROID_API_ACTIONBAR) {
	    	requestWindowFeature(Window.FEATURE_NO_TITLE);
	    }
	    
	    setContentView(R.layout.chat_main);
	    
		// get ID of the buddy
	    setBuddy(getIntent().getStringExtra("buddy"));
	}
	
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			MessageActivity.this.service = ((ChatService.LocalBinder)service).getService();
			refresh();
			Log.i(TAG, "service connected");
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.i(TAG, "service disconnected");
			MessageActivity.this.service = null;
		}
	};
	
	@Override
	protected void onDestroy() {
	    if (mConnection != null) {
	    	// Detach our existing connection.
	    	unbindService(mConnection);
	    }
	    
	    super.onDestroy();
	}
	
	@Override
	public void onResume() {
		refresh();
		super.onResume();
	}
	
	@Override
	protected void onNewIntent(Intent intent) {
		super.onNewIntent(intent);
		setBuddy(intent.getStringExtra("buddy"));
	}
	
	private void setBuddy(String buddyId) {
		this.buddyId = buddyId;
		Log.d(TAG, "set buddy to " + buddyId);
		
		Fragment fragment = this.getSupportFragmentManager().findFragmentById(R.id.chat_fragment);
		if (fragment instanceof ChatFragment) {
			ChatFragment chat = (ChatFragment)fragment;
			chat.setBuddy( buddyId );
		}
		
		fragment = this.getSupportFragmentManager().findFragmentById(R.id.input_fragment);
		if (fragment instanceof InputFragment) {
			InputFragment input = (InputFragment)fragment;
			input.setBuddy( buddyId );
		}
	}
	
	@Override
	protected void onPostCreate(Bundle savedInstanceState) {		
	    super.onPostCreate(savedInstanceState);
	    
		// Establish a connection with the service.  We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		bindService(new Intent(MessageActivity.this, ChatService.class), mConnection, Context.BIND_AUTO_CREATE);
	}
	
	private void refresh()
	{
		if (buddyId == null) return;
		if (service == null) return;
		
		// load buddy from roster
		Buddy buddy = this.service.getRoster().get( buddyId );

	    String presence_tag = buddy.getPresence();
	    String presence_nick = buddy.getNickname();
	    String presence_text = buddy.getEndpoint();
	    int presence_icon = R.drawable.offline;
	    
		if (buddy.getStatus() != null)
		{
			if (buddy.getStatus().length() > 0) { 
				presence_text = buddy.getStatus();
			} else {
				presence_text = buddy.getEndpoint();
			}
		}
		
		if ((presence_tag != null) && buddy.isOnline())
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
		
		this.setTitle(getResources().getString(R.string.conversation_with) + " " + presence_nick);
		
	    if (android.os.Build.VERSION.SDK_INT >= Utils.ANDROID_API_ACTIONBAR) {
	    	ActionBar actionbar = getActionBar();
	    	actionbar.setTitle(presence_nick);
	    	actionbar.setSubtitle(presence_text);
	    	if (android.os.Build.VERSION.SDK_INT >= Utils.ANDROID_API_ACTIONBAR_SETICON) {
	    		actionbar.setIcon(presence_icon);
	    	}
	    } else {
			ImageView iconTitleBar = (ImageView) findViewById(R.id.actionbar_icon);
			TextView labelTitleBar = (TextView) findViewById(R.id.actionbar_title);
			TextView bottomtextTitleBar = (TextView) findViewById(R.id.actionbar_text);
		
			labelTitleBar.setText( presence_nick );
			bottomtextTitleBar.setText( presence_text );
			iconTitleBar.setImageResource(presence_icon);
	    }
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.message_menu, menu);
	    MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemClearMessages), MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
	    return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    switch (item.getItemId()) {
	    	case R.id.itemClearMessages:
	    		if (service != null) {	    		
		    		// load buddy from roster
		    		Buddy buddy = MessageActivity.this.service.getRoster().get( buddyId );
		    		this.service.getRoster().clearMessages(buddy);
	    		}
	    		return true;
	    	
	    	default:
	    		return super.onOptionsItemSelected(item);
	    }
	}
}
