/*
 * MainActivity.java
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
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.IBinder;
import android.preference.PreferenceManager;
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
import de.tubs.ibr.dtn.chat.service.ChatService;
import de.tubs.ibr.dtn.chat.service.Utils;

public class MainActivity extends FragmentActivity {
	private final String TAG = "MainActivity";
	private ChatService service = null;
	private Boolean intent_handled;
	
	private void selectBuddy(String buddyId) {
		Fragment fragment = this.getSupportFragmentManager().findFragmentById(R.id.roster_fragment);
		if (fragment != null) {
			RosterFragment roster = (RosterFragment)fragment;
			roster.selectBuddy(buddyId);
		}
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		intent_handled = false;
		
	    super.onCreate(savedInstanceState);
	    
	    if (android.os.Build.VERSION.SDK_INT < Utils.ANDROID_API_ACTIONBAR) {
	    	requestWindowFeature(Window.FEATURE_NO_TITLE);
	    }
	    
	    setContentView(R.layout.roster_main);
	}

	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			MainActivity.this.service = ((ChatService.LocalBinder)service).getService();
			
			// check possible errors
			switch ( MainActivity.this.service.getServiceError() ) {
			case NO_ERROR:
				break;
				
			case SERVICE_NOT_FOUND:
				Utils.showInstallServiceDialog(MainActivity.this);
				break;
				
			case PERMISSION_NOT_GRANTED:
				Utils.showReinstallDialog(MainActivity.this);
				break;
			}

			Log.i(TAG, "service connected");
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.i(TAG, "service disconnected");
			MainActivity.this.service = null;
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
		super.onResume();
		refresh();
	}

	@Override
	protected void onPostCreate(Bundle savedInstanceState) {		
	    super.onPostCreate(savedInstanceState);
	    
		// Establish a connection with the service.  We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		bindService(new Intent(MainActivity.this, ChatService.class), mConnection, Context.BIND_AUTO_CREATE);
	}
	
	@Override
	protected void onStart() {
		super.onStart();
		
		// get ID of the buddy
		if ((getIntent() != null) && !intent_handled) {
			intent_handled = true;
		    String buddyId = getIntent().getStringExtra("buddy");
		    if (buddyId != null) { 
		    	selectBuddy(buddyId);
		    }
		}
	}

	@Override
	protected void onNewIntent(Intent intent) {
		// get ID of the buddy
		if (intent != null) {
		    String buddyId = intent.getStringExtra("buddy");
		    if (buddyId != null) selectBuddy(buddyId);
		}
		super.onNewIntent(intent);
	}

	private void refresh()
	{
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
		
	    String presence_tag = preferences.getString("presencetag", "auto");
	    String presence_nick = preferences.getString("editNickname", "Nobody");
	    String presence_text = preferences.getString("statustext", "");
	    int presence_icon = R.drawable.online;
	    
	    if (presence_text.length() == 0) {
	    	presence_text = "<" + getResources().getString(R.string.no_status_message) + ">";
	    }
	    
		if (presence_tag != null)
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
	    
	    if (android.os.Build.VERSION.SDK_INT >= Utils.ANDROID_API_ACTIONBAR) {
	    	ActionBar actionbar = getActionBar();
	    	actionbar.setTitle(presence_nick);
	    	actionbar.setSubtitle(presence_text);
	    	if (android.os.Build.VERSION.SDK_INT >= Utils.ANDROID_API_ACTIONBAR_SETICON) {
	    		actionbar.setIcon(presence_icon);
	    	}
	    } else {
		    ImageView icon = (ImageView)findViewById(R.id.actionbar_icon);
			TextView nicknameLabel = (TextView)findViewById(R.id.actionbar_title);
			TextView statusLabel = (TextView)findViewById(R.id.actionbar_text);
			
			icon.setImageResource(presence_icon);
			nicknameLabel.setText(presence_nick);
			statusLabel.setText(presence_text);
	    }
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.buddy_menu, menu);
	    MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemPreferences), MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
	    return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    // Handle item selection
	    switch (item.getItemId()) {
	    case R.id.itemPreferences:
	    {
			// Launch Preference activity
			Intent i = new Intent(MainActivity.this, Preferences.class);
			startActivity(i);
	        return true;
	    }
	    
	    default:
	        return super.onOptionsItemSelected(item);
	    }
	}
}
