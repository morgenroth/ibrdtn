package de.tubs.ibr.dtn.chat;

import android.app.Activity;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import de.tubs.ibr.dtn.chat.RosterView.ViewHolder;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Roster;
import de.tubs.ibr.dtn.chat.service.ChatService;
import de.tubs.ibr.dtn.chat.service.ChatServiceHelper.ChatServiceListener;
import de.tubs.ibr.dtn.chat.service.ChatServiceHelper.ServiceNotConnectedException;

public class RosterFragment extends Fragment implements ChatServiceListener {
	private final String TAG = "RosterFragment";
	private RosterView view = null;

	private OnBuddySelectedListener mCallback = null;
	private boolean persistantSelection = false;
	private RosterProvider rProvider = null;
	
	private ListView lv = null;

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
        
        // get connection to the roster provider
        try {
        	rProvider = (RosterProvider) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString()
                    + " must implement RosterProvider");
        }
	}
    
    @Override
	public void onDetach() {
		super.onDetach();
		rProvider = null;
		mCallback = null;
	}

    private synchronized RosterView getRosterView() {
    	if (this.view == null) {
    		this.view = new RosterView(getActivity(), this);
    	}
    	
    	return this.view;
    }
    
	@Override
	public void onResume() {
		super.onResume();
		this.onContentChanged(null);
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		this.view = null;
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.roster_fragment, container, false);
		this.lv = (ListView)v.findViewById(R.id.list_buddies);
		
		this.lv.setOnCreateContextMenuListener(this);
		this.lv.setAdapter(null);
		
		View roster_view = View.inflate(this.getActivity(), R.layout.roster_me, null);
		this.lv.addHeaderView(roster_view, null, true);
		
		this.lv.setAdapter(getRosterView());
		this.lv.setOnItemClickListener(_click_listener);
		
		return v;
	}

	// Container Activity must implement this interface
    public interface OnBuddySelectedListener {
        public void onBuddySelected(String buddyId);
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
		ViewHolder holder = (ViewHolder)info.targetView.getTag();

		try {
			Buddy buddy = holder.buddy;
			if (buddy == null) return false;
	
			switch (item.getItemId())
			{
			case R.id.itemDelete:
				mCallback.onBuddySelected(null);
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
	
	@Override
	public void onContentChanged(String buddyId) {
		if (this.getActivity() == null) return;
		
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this.getActivity());
	    String presence_tag = prefs.getString("presencetag", "auto");
	    String presence_nick = prefs.getString("editNickname", "Nobody");
	    String presence_text = prefs.getString("statustext", "");

	    if (presence_text.length() == 0) {
	    	presence_text = "<" + getResources().getString(R.string.no_status_message) + ">";
	    }
	    
	    ImageView iconPresence = (ImageView)lv.findViewById(R.id.me_icon);
	    TextView textNick = (TextView)lv.findViewById(R.id.me_nickname);
	    TextView textMessage = (TextView)lv.findViewById(R.id.me_statusmessage);
	    
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

		getRosterView().setShowOffline(!prefs.getBoolean("hideOffline", false));
		if (!persistantSelection) getRosterView().setSelected(null);
		
		// set roster to RosterView
		try {
			getRosterView().onRosterChanged(this.rProvider.getRoster());
		} catch (ServiceNotConnectedException e) {	}
		
		// forward content update
		getRosterView().onContentChanged(buddyId);
	}
	
	private OnItemClickListener _click_listener = new OnItemClickListener() {
		@Override
		public void onItemClick(AdapterView<?> arg0, View v, int arg2, long arg3) {
			ViewHolder holder = (ViewHolder)v.getTag();
			
			if (holder == null) {
				showMeDialog();
			} else if (holder.buddy != null) {
				mCallback.onBuddySelected(holder.buddy.getEndpoint());
			}
		}
	};
	
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
				
				RosterFragment.this.onContentChanged(null);
			}
		});
		
		dialog.show(getActivity().getSupportFragmentManager(), "me");
	}
	
	public Roster getRoster() throws ServiceNotConnectedException {
		if (this.rProvider == null) throw new ServiceNotConnectedException();
		return this.rProvider.getRoster();
	}
	
	public void setPersistantSelection(Boolean val) {
		persistantSelection = val;
	}
	
	public void onBuddySelected(String buddyId) {
		if (persistantSelection) getRosterView().setSelected(buddyId);
	}

	@Override
	public void onServiceConnected(ChatService service) {
		this.onContentChanged(null);
	}

	@Override
	public void onServiceDisconnected() {
	}
}
