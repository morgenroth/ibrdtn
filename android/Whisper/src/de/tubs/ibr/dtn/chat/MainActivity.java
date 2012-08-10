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

import java.util.Date;

import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.view.MenuItemCompat;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Message;
import de.tubs.ibr.dtn.chat.core.Roster;
import de.tubs.ibr.dtn.chat.service.ChatService;
import de.tubs.ibr.dtn.chat.service.ChatServiceHelper;
import de.tubs.ibr.dtn.chat.service.ChatServiceHelper.ChatServiceListener;
import de.tubs.ibr.dtn.chat.service.ChatServiceHelper.ServiceNotConnectedException;
import de.tubs.ibr.dtn.chat.service.Utils;

public class MainActivity extends FragmentActivity 
	implements RosterFragment.OnBuddySelectedListener,
	ChatFragment.OnMessageListener,
	ChatServiceListener {

	private ChatServiceHelper service_helper = null;
	private final String TAG = "MainActivity";
	private Boolean hasLargeLayout = false;
	private String selectBuddy = null;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		service_helper = new ChatServiceHelper(this, this);
		
		super.onCreate(savedInstanceState);
	    setContentView(R.layout.main_layout);

	    // Check that the activity is using the layout version with
	    // the fragment_container FrameLayout
	    if (findViewById(R.id.fragment_container) != null) {
	    	// remind that this is a large layout
	    	hasLargeLayout = false;

	    	// However, if we're being restored from a previous state,
	    	// then we don't need to do anything and should return or else
	    	// we could end up with overlapping fragments.
	    	if (savedInstanceState != null) {
	    		return;
	    	}

			// Create an instance of the roster fragment
			RosterFragment roster = new RosterFragment();

			// In case this activity was started with special instructions from an Intent,
			// pass the Intent's extras to the fragment as arguments
			Bundle args = getIntent().getExtras();
			if (args == null) {
				args = new Bundle();
			}
			args.putBoolean("persistantSelection", false);
			roster.setArguments(args);

			// Add the fragment to the 'fragment_container' FrameLayout
			getSupportFragmentManager().beginTransaction().add(R.id.fragment_container, roster).commit();
	    }
	    else
	    {
	    	hasLargeLayout = true;
	    }

		if ((getIntent() != null) && getIntent().hasExtra("buddy")) {
			selectBuddy = getIntent().getStringExtra("buddy");
		}
	}

	@Override
	protected void onDestroy() {
		service_helper = null;
	    super.onDestroy();
	}

	@Override
	protected void onPause() {
		if (service_helper != null) {
			service_helper.unbind();
		}
		
		super.onPause();
	}

	@Override
	protected void onResume() {
		super.onResume();
		
		service_helper.bind();
	}
	
	@Override
	protected void onNewIntent(Intent intent) {
		super.onNewIntent(intent);
		
		// get ID of the buddy
		if ((intent != null) && intent.hasExtra("buddy")) {
			selectBuddy = intent.getStringExtra("buddy");
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
	
	public void selectBuddy(String buddyId) {
    	// get the roster list
		RosterFragment rosterFrag = (RosterFragment)
                getSupportFragmentManager().findFragmentById(R.id.roster_fragment);
		
		if (rosterFrag != null) {
			// select buddy on buddy list
			rosterFrag.onBuddySelected(buddyId);
		} else {
			onBuddySelected(buddyId);
		}
	}

	@Override
	public void onBuddySelected(String buddyId) {
        // The user selected the buddy from the RosterFragment
        // Do something here to display that chat
		ChatFragment chatFrag = (ChatFragment)
                getSupportFragmentManager().findFragmentById(R.id.chat_fragment);

        if (chatFrag != null) {
            // If chat frag is available, we're in two-pane layout...

            // Call a method in the ChatFragment to update its content
        	chatFrag.onBuddySelected(buddyId);
        } else {
            // Otherwise, we're in the one-pane layout and must swap frags...
        	Object chatFragCandidate = getSupportFragmentManager().findFragmentById(R.id.fragment_container);
        	if (chatFragCandidate instanceof ChatFragment) {
	    		chatFrag = (ChatFragment)chatFragCandidate;
	    		if (chatFrag != null) {
	                // Call a method in the ChatFragment to update its content
	            	chatFrag.onBuddySelected(buddyId);
	            	return;
	    		}
        	}

            // Create fragment and give it an argument for the selected article
        	ChatFragment newFragment = new ChatFragment();
            Bundle args = new Bundle();
            args.putString("buddyId", buddyId);
            args.putBoolean("large_layout", this.hasLargeLayout);
            newFragment.setArguments(args);
        
            FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
            transaction.setTransition(FragmentTransaction.TRANSIT_NONE);
            transaction.setCustomAnimations(R.anim.slide_in_right, R.anim.slide_out_left, R.anim.slide_in_left, R.anim.slide_out_right);

            // Replace whatever is in the fragment_container view with this fragment,
            // and add the transaction to the back stack so the user can navigate back
            transaction.replace(R.id.fragment_container, newFragment);
            transaction.addToBackStack(null);

            // Commit the transaction
            transaction.commit();
        }
	}
	
	public Roster getRoster() throws ServiceNotConnectedException {
		return this.service_helper.getService().getRoster();
	}

	@Override
	public void onClearMessages(String buddyId) {
		try {
			// load buddy from roster
			Buddy buddy = this.getRoster().get( buddyId );
			this.getRoster().clearMessages(buddy);
		} catch (ServiceNotConnectedException e) {
			Log.e(TAG, "clear messages failed", e);
		}
	}

	@Override
	public void onMessage(String buddyId, String text) {
		try {
			// load buddy from roster
			Buddy buddy = this.getRoster().get( buddyId );
			Message msg = new Message(false, new Date(), new Date(), text);
			msg.setBuddy(buddy);
			
			Log.i(TAG, "send text to " + buddy.getNickname() + ": " + msg.getPayload());
			
			// send the message
			new SendChatMessageTask().execute(msg);
			
			// store the message in the database
			this.getRoster().storeMessage(msg);
		} catch (ServiceNotConnectedException e) {
			Log.e(TAG, "failed to send message", e);
		}
	}

	@Override
	public void onSaveMessage(String buddyId, String msg) {
		if (buddyId == null) return;

		try {
			// load buddy from roster
			Buddy buddy = this.getRoster().get( buddyId );
			
			if (msg.length() > 0)
				buddy.setDraftMessage( msg );
			else
				buddy.setDraftMessage( null );
			
			this.getRoster().store(buddy);
		} catch (ServiceNotConnectedException e) {
			Log.e(TAG, "failed to save draft message", e);
		}
	}
	
	private class SendChatMessageTask extends AsyncTask<Message, Integer, Integer> {
		protected Integer doInBackground(Message... msgs) {
			int count = msgs.length;
			int totalSize = 0;
			for (int i = 0; i < count; i++)
			{
				try {
					MainActivity.this.service_helper.getService().sendMessage(msgs[i]);
					
					// update total size
					totalSize += msgs[i].getPayload().length();
					
					// publish the progress
					publishProgress((int) ((i / (float) count) * 100));
				} catch (Exception e) {
					Log.e(TAG, "could not send the message", e);
				}
			}
			return totalSize;
		}
	
		protected void onProgressUpdate(Integer... progress) {
			//setProgressPercent(progress[0]);
		}
	
		protected void onPostExecute(Integer result) {
			//showDialog("Downloaded " + result + " bytes");
		}
	}

	@Override
	public void onContentChanged(String buddyId) {
	}

	@Override
	public void onServiceConnected(ChatService service) {
		try {
			// check possible errors
			switch ( MainActivity.this.service_helper.getService().getServiceError() ) {
			case NO_ERROR:
				break;
				
			case SERVICE_NOT_FOUND:
				Utils.showInstallServiceDialog(MainActivity.this);
				break;
				
			case PERMISSION_NOT_GRANTED:
				Utils.showReinstallDialog(MainActivity.this);
				break;
			}
		} catch (ServiceNotConnectedException e) {
			Log.e(TAG, "failure while checking for service error", e);
		}
		
		if (selectBuddy != null) {
			this.selectBuddy(selectBuddy);
			selectBuddy = null;
		}
	}

	@Override
	public void onServiceDisconnected() {
	}
}
