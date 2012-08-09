package de.tubs.ibr.dtn.chat;

import android.app.Activity;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v4.app.ListFragment;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import de.tubs.ibr.dtn.chat.RosterView.ViewHolder;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Roster;
import de.tubs.ibr.dtn.chat.service.ChatService;
import de.tubs.ibr.dtn.chat.service.ChatServiceHelper;
import de.tubs.ibr.dtn.chat.service.ChatServiceHelper.ChatServiceListener;
import de.tubs.ibr.dtn.chat.service.ChatServiceHelper.ServiceNotConnectedException;

public class RosterFragment extends ListFragment implements ChatServiceListener {
	
	private ChatServiceHelper service_helper = null;
	private final String TAG = "RosterFragment";
	private RosterView view = null;

	private String selectedBuddy = null;
	private OnBuddySelectedListener mCallback = null;
	private boolean persistantSelection = true;
	
	private boolean paused = false;
	
    @Override
	public void onAttach(Activity activity) {
        super.onAttach(activity);
        
        // This makes sure that the container activity has implemented
        // the callback interface. If not, it throws an exception
        try {
            mCallback = (OnBuddySelectedListener) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString()
                    + " must implement OnBuddySelectedListener");
        }
	}
    
    @Override
	public void onCreate(Bundle savedInstanceState) {
		service_helper = new ChatServiceHelper(getActivity(), this);
		paused = false;
		
		super.onCreate(savedInstanceState);
		
		if ((this.getArguments() != null) && this.getArguments().containsKey("persistantSelection")) {
			this.persistantSelection = this.getArguments().getBoolean("persistantSelection");
		} else {
			this.persistantSelection = true;
		}
	}

	// Container Activity must implement this interface
    public interface OnBuddySelectedListener {
        public void onBuddySelected(String buddyId);
    }

	@Override
	public void onDestroy() {
		if (this.view != null) {
			this.view.onDestroy(getActivity());
			this.view = null;
		}
	    
		if (service_helper != null) {
			service_helper.unbind();
			service_helper = null;
		}
	    super.onDestroy();
	}

	@Override
	public void onPause() {
		paused = true;
		super.onPause();
	}

	@Override
	public void onResume() {
		paused = false;
		onContentChanged();
		super.onResume();
	}
	
	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		super.onActivityCreated(savedInstanceState);
		
		this.getListView().setOnCreateContextMenuListener(this);
		
		this.setListAdapter(null);
		
		View v = View.inflate(this.getActivity(), R.layout.roster_me, null);
		this.getListView().addHeaderView(v, null, true);
		
		if (this.view != null) {
			this.setListAdapter(this.view);
		}
		
		service_helper.bind();
	}

	@Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
		super.onCreateContextMenu(menu, v, menuInfo);
		
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) menuInfo;
		if (info.position == 0) return;
		
		MenuInflater inflater = getActivity().getMenuInflater();
		inflater.inflate(R.menu.buddycontext_menu, menu);
	}
	
	public Roster getRoster() throws ServiceNotConnectedException {
		return this.service_helper.getService().getRoster();
	}
	
	@Override
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
		ViewHolder holder = (ViewHolder)info.targetView.getTag();

		try {
			Buddy buddy = holder.buddy;
			if (buddy == null) return false;
	
			switch (item.getItemId())
			{
			case R.id.itemDelete:
				this.onBuddySelected(null);
				this.getRoster().remove(buddy);
				return true;
			default:
				return super.onContextItemSelected(item);
			}
		} catch (ServiceNotConnectedException e) {
			Log.e(TAG, "context menu failed", e);
			return super.onContextItemSelected(item);
		}
	}
	
	private void onContentChanged()
	{
		if (this.getView() == null) return;
		
		if ((this.view != null) && !paused)
		{
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this.getActivity());
		    String presence_tag = prefs.getString("presencetag", "auto");
		    String presence_nick = prefs.getString("editNickname", "Nobody");
		    String presence_text = prefs.getString("statustext", "");

		    if (presence_text.length() == 0) {
		    	presence_text = "<" + getResources().getString(R.string.no_status_message) + ">";
		    }
		    
		    ImageView iconPresence = (ImageView)getListView().findViewById(R.id.me_icon);
		    TextView textNick = (TextView)getListView().findViewById(R.id.me_nickname);
		    TextView textMessage = (TextView)getListView().findViewById(R.id.me_statusmessage);
		    
		    int presence_icon = R.drawable.online;
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
			iconPresence.setImageResource(presence_icon);
			textNick.setText(presence_nick);
			textMessage.setText(presence_text);
    
			view.setShowOffline(!prefs.getBoolean("hideOffline", false));
			view.refresh();
		}
	}
	
	@Override
	public void onListItemClick(ListView l, View v, int position, long id) {
		ViewHolder holder = (ViewHolder)v.getTag();
		
		if (holder == null) {
			showMeDialog();
		} else if (holder.buddy != null) {
			onBuddySelected(holder.buddy.getEndpoint());
		}
		
		super.onListItemClick(l, v, position, id);
	}
	
	private void showMeDialog() {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
	    String presence_tag = prefs.getString("presencetag", "auto");
	    String presence_text = prefs.getString("statustext", "");
	    
		MeDialog dialog = MeDialog.newInstance(presence_tag, presence_text);

		dialog.setOnChangeListener(new MeDialog.OnChangeListener() {
			@Override
			public void onStateChanged(String presence, String message) {
				Log.d(TAG, "state changed: " + presence + ", " + message);
				SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
				Editor edit = prefs.edit();
				edit.putString("presencetag", presence);
				edit.putString("statustext", message);
				edit.commit();
				
				RosterFragment.this.onContentChanged();
			}
		});
		
		dialog.show(getActivity().getSupportFragmentManager(), "me");
	}
	
	public void selectBuddy(String buddyId) {
		// select the list item
		this.selectedBuddy = buddyId;
		if ((this.view != null) && persistantSelection) this.view.setSelected(this.selectedBuddy);
	}
	
	public void onBuddySelected(String buddyId) {
		// select the list item
		selectBuddy(buddyId);

		if (buddyId != null) {
			mCallback.onBuddySelected(buddyId);
		}
	}

	@Override
	public void onContentChanged(String buddyId) {
		this.onContentChanged();
	}

	@Override
	public void onServiceConnected(ChatService service) {
		try {
			// activate roster view
			this.view = new RosterView(getActivity(), this.getRoster());
			this.setListAdapter(this.view);
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this.getActivity());
			this.view.setShowOffline(!prefs.getBoolean("hideOffline", false));
			this.view.setSelected(selectedBuddy);
			this.onContentChanged();
		} catch (ServiceNotConnectedException e) {
			Log.e(TAG, "failure while checking for service error", e);
		}
	}

	@Override
	public void onServiceDisconnected() {
	}
}
