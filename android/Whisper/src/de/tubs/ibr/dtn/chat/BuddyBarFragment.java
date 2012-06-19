package de.tubs.ibr.dtn.chat;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Roster;
import de.tubs.ibr.dtn.chat.service.ChatService;

public class BuddyBarFragment extends Fragment {

	private final String TAG = "BuddyBarFragment";
	private String buddyId = null;
	private ChatService service = null;
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.buddybar_view, container, false);
		return v;
	}
	
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			BuddyBarFragment.this.service = ((ChatService.LocalBinder)service).getService();
			refresh();
			Log.i(TAG, "service connected");
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.i(TAG, "service disconnected");
			BuddyBarFragment.this.service = null;
		}
	};
	
	@Override
	public void onDestroy() {
	    if (mConnection != null) {
	    	// Detach our existing connection.
	    	getActivity().unbindService(mConnection);
	    }

	    super.onDestroy();
	}
	
	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		super.onActivityCreated(savedInstanceState);
		
		if (savedInstanceState != null) {
			// restore buddy id
			if (savedInstanceState.containsKey("buddyId")) {
				buddyId = savedInstanceState.getString("buddyId");
			}
		}
	}

	@Override
	public void onSaveInstanceState(Bundle outState) {
		super.onSaveInstanceState(outState);
		
		if (buddyId != null) {
			// save buddy id
			outState.putString("buddyId", buddyId);
		}
	}

	@Override
	public void onPause() {
		getActivity().unregisterReceiver(notify_receiver);
		super.onPause();
	}

	@Override
	public void onResume() {
		super.onResume();
		
		IntentFilter i = new IntentFilter(Roster.REFRESH);
		getActivity().registerReceiver(notify_receiver, i);
		
		// refresh visible data
		refresh();
	}
	
	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		
		// Establish a connection with the service.  We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		getActivity().bindService(new Intent(activity, ChatService.class), mConnection, Context.BIND_AUTO_CREATE);
	}
	
	private BroadcastReceiver notify_receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent i) {
			if (i.getStringExtra("buddy").equals(buddyId)) {
				BuddyBarFragment.this.refresh();
			}
		}
	};
	
	public void setBuddy(String buddyId) {
		this.buddyId = buddyId;
		refresh();
	}
	
	private void refresh()
	{
		Buddy buddy = null;
		Roster roster = null;
		
		ImageView iconTitleBar = (ImageView) getView().findViewById(R.id.buddybar_icon);
		TextView labelTitleBar = (TextView) getView().findViewById(R.id.buddybar_title);
		TextView bottomtextTitleBar = (TextView) getView().findViewById(R.id.buddybar_text);
		
		if ((buddyId != null) && (service != null)) {
			roster = this.service.getRoster();
			
			// load buddy from roster
			buddy = roster.get( buddyId );
			
			if (buddy == null) {
				Log.e(TAG, "Error buddy not found: " + getActivity().getIntent().getStringExtra("buddy"));
				return;
			}

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
			
			labelTitleBar.setText( presence_nick );
			bottomtextTitleBar.setText( presence_text );
			iconTitleBar.setImageResource(presence_icon);
			return;
		}
		
		labelTitleBar.setText( getActivity().getResources().getString(R.string.no_buddy_selected) );
		bottomtextTitleBar.setText( "" );
		iconTitleBar.setImageResource(R.drawable.offline);
	}
}
