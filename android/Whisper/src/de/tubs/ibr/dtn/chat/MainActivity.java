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
import android.content.pm.ApplicationInfo;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.view.MenuItemCompat;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import de.tubs.ibr.dtn.api.BundleID;
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
	RosterProvider,
	ChatServiceListener {

	private ChatServiceHelper service_helper = null;
	private final String TAG = "MainActivity";
	private Boolean hasLargeLayout = false;
	private String selectedBuddy = null;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		service_helper = new ChatServiceHelper(this, this);
		
		super.onCreate(savedInstanceState);
	    setContentView(R.layout.main_layout);

	    // Check that the activity is using the layout version with
	    // the fragment_container FrameLayout
	    hasLargeLayout = (findViewById(R.id.chat_fragment) != null);

	    RosterFragment roster = getRosterFragment();
		if (roster == null) {
			roster = new RosterFragment();
			
		    // clear the back stack
		    FragmentManager fm = this.getSupportFragmentManager();
		    for(int i = 0; i < fm.getBackStackEntryCount(); ++i) {    
		        fm.popBackStack();
		    }
		    	
			// add chat fragment to the activity
			getSupportFragmentManager()
				.beginTransaction()
				.replace(R.id.fragment_container, roster)
				.commit();
	    }
	}
	
	public RosterFragment getRosterFragment() {
		try {
			return (RosterFragment)getSupportFragmentManager().findFragmentById(R.id.fragment_container);
		} catch (ClassCastException e) {
			return null;
		}
	}
	
	public ChatFragment getChatFragment() {
		// search for the chat fragment in the main view (portait)
		try {
			return (ChatFragment)getSupportFragmentManager().findFragmentById(R.id.fragment_container);
		} catch (ClassCastException e) { }

		// search for the chat fragment in the fragmented view (landscape)
		try {
		    if (getSupportFragmentManager().findFragmentById(R.id.chat_fragment) != null) {
		    	return (ChatFragment)getSupportFragmentManager().findFragmentById(R.id.chat_fragment);
		    }
		} catch (ClassCastException e) {
			return null;
		}
		
		return null;
	}

	@Override
	protected void onDestroy() {
		service_helper = null;
	    super.onDestroy();
	}

	@Override
	protected void onStart() {
		super.onStart();
		service_helper.bind();
		
		// set the correct behavior of the roster
		RosterFragment roster = getRosterFragment();
		if (roster != null) {
			roster.setPersistantSelection(hasLargeLayout);
		}
		
	    if (getIntent() != null) {
		    if ( getIntent().getAction().equals(ChatService.ACTION_OPENCHAT) &&
					getIntent().getCategories().contains("android.intent.category.DEFAULT") ) {
				// the activity is called by a notification
				this.onBuddySelected( getIntent().getStringExtra("buddy") );
				this.setIntent(null);
			}
	    }
	}

	@Override
	protected void onStop() {
		super.onStop();
		if (service_helper != null) {
			service_helper.unbind();
		}
	}
	
	@Override
	protected void onNewIntent(Intent intent) {
		super.onNewIntent(intent);
		
		// do not do any futher processing if the intent is not defined
		if (intent == null) return;
		
		// set the new intent for this activity
		this.setIntent(intent);
		
		if (service_helper.isConnected()) {
		    if (getIntent() != null) {
			    if ( getIntent().getAction().equals(ChatService.ACTION_OPENCHAT) &&
						getIntent().getCategories().contains("android.intent.category.DEFAULT") ) {
					// the activity is called by a notification
					this.onBuddySelected( getIntent().getStringExtra("buddy") );
					this.setIntent(null);
				}
		    }
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.buddy_menu, menu);
	    MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemPreferences), MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
	    
	    if (0 != (getApplicationInfo().flags & ApplicationInfo.FLAG_DEBUGGABLE)) {
	    	menu.findItem(R.id.itemDebugNotification).setVisible(true);
	    } else {
	    	menu.findItem(R.id.itemDebugNotification).setVisible(false);
	    }
	    
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
	    
	    case R.id.itemDebugNotification:
	    	try {
				service_helper.getService().startDebug(ChatService.Debug.NOTIFICATION);
			} catch (ServiceNotConnectedException e) {
				Log.e(TAG, "Debug error", e);
			}
	    	return true;
    
	    default:
	        return super.onOptionsItemSelected(item);
	    }
	}
	
	@Override
	protected void onRestoreInstanceState(Bundle savedInstanceState) {
		super.onRestoreInstanceState(savedInstanceState);
		if (savedInstanceState.containsKey("selectedBuddy")) {
			this.onBuddySelected(savedInstanceState.getString("selectedBuddy"));
		}
	}

	@Override
	protected void onSaveInstanceState(Bundle outState) {
		super.onSaveInstanceState(outState);
		ChatFragment chat = getChatFragment();
		if ((chat != null) && chat.isVisible()) {
			if (chat.getSelectedBuddy() != null) {
				outState.putString("selectedBuddy", chat.getSelectedBuddy());
			}
		}
	}

	public void onBuddySelected(String buddyId) {
		this.selectedBuddy = buddyId;
		
		// select the buddy on the roster
        RosterFragment roster = getRosterFragment();
		if (roster != null) {
			// select buddy on buddy list
			roster.onBuddySelected(buddyId);
		}
		
        // Call a method in the ChatFragment to update its content
        if ((buddyId != null) && !this.hasLargeLayout) {
    		
		    // clear the back stack
		    FragmentManager fm = this.getSupportFragmentManager();
		    for(int i = 0; i < fm.getBackStackEntryCount(); ++i) {    
		        fm.popBackStack();
		    }
		    
            FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
            transaction.setTransition(FragmentTransaction.TRANSIT_NONE);
            transaction.setCustomAnimations(R.anim.slide_in_right, R.anim.slide_out_left, R.anim.slide_in_left, R.anim.slide_out_right);

            // Replace whatever is in the fragment_container view with this fragment,
            // and add the transaction to the back stack so the user can navigate back
            ChatFragment chat = new ChatFragment();
            
            Bundle args = new Bundle();
            args.putString("buddyId", buddyId);
            chat.setArguments(args);
            
            transaction.replace(R.id.fragment_container, chat);
            transaction.addToBackStack(null);

            // Commit the transaction
            transaction.commit();
        } else {
	    	ChatFragment chat = this.getChatFragment();
	    	if (chat != null) {
	    		chat.onBuddySelected(buddyId);
	    	}
        }
    	
    	// update the content views
    	this.onContentChanged(buddyId);
	}
	
	public Roster getRoster() throws ServiceNotConnectedException {
		if (this.service_helper == null) throw new ServiceNotConnectedException();
		return this.service_helper.getService().getRoster();
	}

	public void onClearMessages(String buddyId) {
		try {
			// load buddy from roster
			Buddy buddy = this.getRoster().getBuddy( buddyId );
			this.getRoster().clearMessages(buddy);
		} catch (ServiceNotConnectedException e) {
			Log.e(TAG, "clear messages failed", e);
		}
	}

	public void onMessage(String buddyId, String text) {
		try {
			// load buddy from roster
			Buddy buddy = this.getRoster().getBuddy( buddyId );
			Message msg = new Message(null, false, new Date(), new Date(), text);
			msg.setBuddy(buddy);
			
			Log.i(TAG, "send text to " + buddy.getNickname() + ": " + msg.getPayload());
			
			// store the message in the database
			this.getRoster().storeMessage(msg);
			
			// send the message
			new SendChatMessageTask().execute(msg);
		} catch (ServiceNotConnectedException e) {
			Log.e(TAG, "failed to send message", e);
		}
	}

	public void onSaveMessage(String buddyId, String msg) {
		if (buddyId == null) return;

		try {
			// load buddy from roster
			Buddy buddy = this.getRoster().getBuddy( buddyId );
			
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
					BundleID id = MainActivity.this.service_helper.getService().sendMessage(msgs[i]);
					
					// set sent id of the message
					msgs[i].setSentId(id.toString());
					
					// update message into the database
					MainActivity.this.getRoster().reportSent(msgs[i]);
					
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

	public void onContentChanged(String buddyId) {
	    RosterFragment roster = getRosterFragment();
	    if (roster != null) {
	    	roster.onContentChanged(buddyId);
	    }
	    
	    ChatFragment chat = getChatFragment();
	    if (chat != null) {
	    	chat.onContentChanged(buddyId);
	    }
	}

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
		
	    RosterFragment roster = getRosterFragment();
	    if (roster != null) {
	    	roster.onServiceConnected(service);
	    }
	    
	    ChatFragment chat = getChatFragment();
	    if (chat != null) {
	    	chat.onServiceConnected(service);
	    }
	}

	public void onServiceDisconnected() {
	    RosterFragment roster = getRosterFragment();
	    if (roster != null) {
	    	roster.onServiceDisconnected();
	    }
	    
	    ChatFragment chat = getChatFragment();
	    if (chat != null) {
	    	chat.onServiceDisconnected();
	    }
	}

	public String getSelectedBuddy() {
		return this.selectedBuddy;
	}
}
