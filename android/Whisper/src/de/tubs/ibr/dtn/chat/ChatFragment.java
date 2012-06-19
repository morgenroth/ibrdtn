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
import android.support.v4.app.ListFragment;
import android.util.Log;
import android.widget.ListView;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Roster;
import de.tubs.ibr.dtn.chat.service.ChatService;

public class ChatFragment extends ListFragment {
	private final String TAG = "ChatFragment";
	private MessageView view = null;
	private String buddyId = null;
//	private static String visibleBuddy = null;
	private ChatService service = null;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
	    super.onCreate(savedInstanceState);
	}
	
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			ChatFragment.this.service = ((ChatService.LocalBinder)service).getService();
			refresh();
			Log.i(TAG, "service connected");
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.i(TAG, "service disconnected");
			ChatFragment.this.service = null;
		}
	};
	
	@Override
	public void onDestroy() {
		if (this.view != null) {
			this.view.onDestroy(getActivity());
			this.view = null;
		}
		
	    if (mConnection != null) {
	    	// Detach our existing connection.
	    	getActivity().unbindService(mConnection);
	    }

	    super.onDestroy();
	}
	
	@Override
	public void onPause() {
		getActivity().unregisterReceiver(notify_receiver);
//		visibleBuddy = null;
		super.onPause();
	}

	@Override
	public void onResume() {
		super.onResume();
		
		IntentFilter i = new IntentFilter(Roster.REFRESH);
		getActivity().registerReceiver(notify_receiver, i);
		
//		// set the current visible buddy
//		ChatFragment.visibleBuddy = buddyId;

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

	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
	    this.getListView().setTranscriptMode(ListView.TRANSCRIPT_MODE_ALWAYS_SCROLL);
	    this.getListView().setStackFromBottom(true);
	    this.getListView().setEmptyView(null);
	    
		super.onActivityCreated(savedInstanceState);
	}

	private BroadcastReceiver notify_receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent i) {
			if (i.getStringExtra("buddy").equals(buddyId)) {
				ChatFragment.this.refresh();
			}
		}
	};
	
//	public static Boolean isVisible(String buddyId)
//	{
//		if (visibleBuddy == null) return false;
//		return (visibleBuddy.equals(buddyId));
//	}

//	public void refreshCallback()
//	{
//		ChatFragment.this.getActivity().runOnUiThread(new Runnable() {
//			@Override
//			public void run() {
//				refresh();
//				((BaseAdapter)getListView().getAdapter()).notifyDataSetChanged();
//			}
//		});
//	}
	
	public void setBuddy(String buddyId) {
		if ((this.buddyId != buddyId) && (this.view != null)) {
			this.view.onDestroy(getActivity());
			this.view = null;
			this.setListAdapter(null);
		}
		
		this.buddyId = buddyId;
		
//		// set the current visible buddy
//		ChatFragment.visibleBuddy = buddyId;

		refresh();
	}
	
	private void refresh()
	{
		Buddy buddy = null;
		Roster roster = null;
		
		if ((buddyId != null) && (service == null)) {
			roster = this.service.getRoster();
			
			// load buddy from roster
			buddy = roster.get( buddyId );
			
			if (buddy == null) {
				Log.e(TAG, "Error buddy not found: " + getActivity().getIntent().getStringExtra("buddy"));
				return;
			}
		}
		
		if (this.view == null) {
			// activate message view
			this.view = new MessageView(getActivity(), roster, buddy);
			this.setListAdapter(this.view);
		} else {
			this.view.refresh();
		}
	}

//	@Override
//	public boolean onCreateOptionsMenu(Menu menu) {
//	    MenuInflater inflater = getMenuInflater();
//	    inflater.inflate(R.menu.message_menu, menu);
//	    MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemClearMessages), MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
//	    return true;
//	}
//
//	@Override
//	public boolean onOptionsItemSelected(MenuItem item) {
//	    switch (item.getItemId()) {
//	    	case R.id.itemClearMessages:
//	    		if (service != null) {	    		
//		    		// load buddy from roster
//		    		Buddy buddy = this.service.getRoster().get( buddyId );
//		    		this.service.getRoster().clearMessages(buddy);
//	    		}
//	    		return true;
//	    	
//	    	default:
//	    		return super.onOptionsItemSelected(item);
//	    }
//	}
}
