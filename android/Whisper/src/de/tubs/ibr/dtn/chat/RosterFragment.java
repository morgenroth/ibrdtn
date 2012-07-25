package de.tubs.ibr.dtn.chat;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.support.v4.app.Fragment;
import android.support.v4.app.ListFragment;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.ListView;
import de.tubs.ibr.dtn.chat.RosterView.ViewHolder;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.service.ChatService;

public class RosterFragment extends ListFragment {
	private final String TAG = "RosterFragment";
	private RosterView view = null;
	private ChatService service = null;
	private String selectedBuddy = null;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
	    super.onCreate(savedInstanceState);
	    
		// Establish a connection with the service.  We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		getActivity().bindService(new Intent(this.getActivity(), ChatService.class), mConnection, Context.BIND_AUTO_CREATE);
	}
	
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			RosterFragment.this.service = ((ChatService.LocalBinder)service).getService();
			
			// activate roster view
			RosterFragment.this.view = new RosterView(getActivity(), RosterFragment.this.service.getRoster());
			RosterFragment.this.setListAdapter(RosterFragment.this.view);
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(RosterFragment.this.getActivity());
			RosterFragment.this.view.setShowOffline(!prefs.getBoolean("hideOffline", false));
			RosterFragment.this.view.setSelected(selectedBuddy);
			RosterFragment.this.refresh();
			
			Log.i(TAG, "service connected");
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.i(TAG, "service disconnected");
			RosterFragment.this.service = null;
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
	public void onResume() {
		refresh();
		super.onResume();
	}
	
	@Override
	public void onViewCreated(View view, Bundle savedInstanceState) {
		this.getListView().setOnCreateContextMenuListener(this);
		super.onViewCreated(view, savedInstanceState);
	}

	@Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
		super.onCreateContextMenu(menu, v, menuInfo);
		
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) menuInfo;
		if (info.position == 0) return;
		
		MenuInflater inflater = getActivity().getMenuInflater();
		inflater.inflate(R.menu.buddycontext_menu, menu);
	}
	
	@Override
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();

		Buddy buddy = this.service.getRoster().get(info.position);
		if (buddy == null) return false;

		switch (item.getItemId())
		{
		case R.id.itemDelete:
			this.selectBuddy(null);
			this.service.getRoster().remove(buddy);
			return true;
		default:
			return super.onContextItemSelected(item);
		}
	}
	
	private void refresh()
	{
		if (this.view != null)
		{
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this.getActivity());
		    String presence_tag = prefs.getString("presencetag", "auto");
		    String presence_nick = prefs.getString("editNickname", "Nobody");
		    String presence_text = prefs.getString("statustext", "");

		    if (presence_text.length() == 0) {
		    	presence_text = "<" + getResources().getString(R.string.no_status_message) + ">";
		    }

		    view.setMe(new Buddy(presence_nick, null, presence_tag, presence_text, null));		    
			view.setShowOffline(!prefs.getBoolean("hideOffline", false));
			view.refresh();
		}
	}
	
	@Override
	public void onListItemClick(ListView l, View v, int position, long id) {
		ViewHolder holder = (ViewHolder)v.getTag();
		
		if (holder.buddy != null) {
			selectBuddy(holder.buddy.getEndpoint());
		} else {
			showMeDialog();
		}
		
		super.onListItemClick(l, v, position, id);
	}
	
	private void showMeDialog() {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
	    String presence_tag = prefs.getString("presencetag", "auto");
	    String presence_text = prefs.getString("statustext", "");
	    
		MeDialog dialog = MeDialog.newInstance(this.getActivity(), presence_tag, presence_text);

		dialog.setOnChangeListener(new MeDialog.OnChangeListener() {
			@Override
			public void onStateChanged(String presence, String message) {
				Log.d(TAG, "state changed: " + presence + ", " + message);
				SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
				Editor edit = prefs.edit();
				edit.putString("presencetag", presence);
				edit.putString("statustext", message);
				edit.commit();
				
				RosterFragment.this.refresh();
			}
		});
		
		dialog.show(getActivity().getSupportFragmentManager(), "me");
	}
	
	public void selectBuddy(String buddyId) {
    	Fragment fragment = getActivity().getSupportFragmentManager().findFragmentById(R.id.chat_fragment);
    	if ((fragment == null) || !fragment.isInLayout()) {
    		if (buddyId != null) {
    			Intent i = new Intent(getActivity(), MessageActivity.class);
	    		i.putExtra("buddy", buddyId);
	    		startActivity(i);
    		}
    	} else {
    		// select the list item
    		this.selectedBuddy = buddyId;
    		if (this.view != null) this.view.setSelected(this.selectedBuddy);
    		
    		ChatFragment chat = (ChatFragment)fragment;
    		chat.setBuddy(buddyId);
    		
    		InputFragment input = (InputFragment)getActivity().getSupportFragmentManager().findFragmentById(R.id.input_fragment);
    		input.setBuddy(buddyId);
    	}
	}
}
