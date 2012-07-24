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
import de.tubs.ibr.dtn.chat.service.ChatService;
import de.tubs.ibr.dtn.chat.service.Utils;

public class MainActivity extends FragmentActivity {
	private final String TAG = "MainActivity";
	private ChatService service = null;
	private String _open_buddy = null;
	
	private void selectBuddy(String buddyId) {
		Fragment fragment = this.getSupportFragmentManager().findFragmentById(R.id.roster_fragment);
		if (fragment != null) {
			RosterFragment roster = (RosterFragment)fragment;
			roster.selectBuddy(buddyId);
		}
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		if (getIntent() != null) {
			_open_buddy = getIntent().getStringExtra("buddy");
		}
		
	    super.onCreate(savedInstanceState);
	    
	    setContentView(R.layout.roster_main);
	    
		// Establish a connection with the service.  We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		bindService(new Intent(MainActivity.this, ChatService.class), mConnection, Context.BIND_AUTO_CREATE);
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

		// get ID of the buddy
		if (_open_buddy != null) {
	    	selectBuddy(_open_buddy);
	    	_open_buddy = null;
		}
	}

	@Override
	protected void onNewIntent(Intent intent) {
		// get ID of the buddy
		if (intent != null) {
			_open_buddy = intent.getStringExtra("buddy");
		}
		super.onNewIntent(intent);
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
