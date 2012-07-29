package de.tubs.ibr.dtn.chat;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.view.MenuItemCompat;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnKeyListener;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Roster;
import de.tubs.ibr.dtn.chat.service.ChatService;
import de.tubs.ibr.dtn.chat.service.ChatServiceHelper;
import de.tubs.ibr.dtn.chat.service.ChatServiceHelper.ChatServiceListener;
import de.tubs.ibr.dtn.chat.service.ChatServiceHelper.ServiceNotConnectedException;

public class ChatFragment extends Fragment implements ChatServiceListener, RosterFragment.OnBuddySelectedListener {
	private ChatServiceHelper service_helper = null;
	private final String TAG = "ChatFragment";
	
	private MessageView view = null;
	private String buddyId = null;
	
	private OnMessageListener mCallback = null;
	
    @Override
	public void onAttach(Activity activity) {
        super.onAttach(activity);
        
        // This makes sure that the container activity has implemented
        // the callback interface. If not, it throws an exception
        try {
            mCallback = (OnMessageListener) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString()
                    + " must implement OnMessageListener");
        }
	}

	// Container Activity must implement this interface
    public interface OnMessageListener {
        public void onMessage(String buddyId, String text);
        public void onSaveMessage(String buddyId, String text);
        public void onClearMessages(String buddyId);
    }
    
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.chat_fragment, container, false);
		
		if ((this.getArguments() != null) && this.getArguments().containsKey("large_layout")) {
			if (!this.getArguments().getBoolean("large_layout")) {
				LinearLayout layout = (LinearLayout)v.findViewById(R.id.chat_buddy_display);
				layout.setVisibility(View.VISIBLE);
			}
		}
		
		// set "enter" handler
		EditText textedit = (EditText) v.findViewById(R.id.textMessage);
		textedit.setEnabled(false);
		
		textedit.setOnKeyListener(new OnKeyListener() {
			@Override
			public boolean onKey(View v, int keycode, KeyEvent event) {
				if ((KeyEvent.KEYCODE_ENTER == keycode) && (event.getAction() == KeyEvent.ACTION_DOWN))
				{
					flushTextBox();
					return true;
				}
				return false;
			}
		});
		
		// set send handler
		ImageButton buttonSend = (ImageButton) v.findViewById(R.id.buttonSend);
		buttonSend.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				flushTextBox();
			}
		});
		
		return v;
	}

	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		super.onActivityCreated(savedInstanceState);
		
		// restore buddy id
		if (savedInstanceState != null) {
			// restore buddy id
			if (savedInstanceState.containsKey("buddyId")) {
				buddyId = savedInstanceState.getString("buddyId");
			}
		}
		
		// select right buddy
		Bundle args = this.getArguments();
		if ((args != null) && args.containsKey("buddyId")) {
			this.onBuddySelected(args.getString("buddyId"));
		}
		
		service_helper.bind();
	}
	
	private void restoreDraftMessage() {
		EditText text = (EditText) getView().findViewById(R.id.textMessage);
		text.setText("");
		
		if (this.buddyId == null) {
			text.setEnabled(false);
			return;
		}

		try {
			// load buddy from roster
			Buddy buddy = this.getRoster().get( this.buddyId );
			String msg = buddy.getDraftMessage();
			
			if (msg != null) text.append(msg);
	
			text.setEnabled(true);
			
			text.requestFocus();
		} catch (ServiceNotConnectedException e) {
			Log.e(TAG, "failed to restore draft message", e);
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
	
	private void flushTextBox()
	{
		EditText text = (EditText) getView().findViewById(R.id.textMessage);
		
		if (text.getText().length() > 0) {
			mCallback.onMessage(buddyId, text.getText().toString());
			
			// clear the text field
			text.setText("");
			text.requestFocus();
		}
	}

	@Override
	public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
	    inflater.inflate(R.menu.message_menu, menu);
	    MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemClearMessages), MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
	}
	
	private DialogInterface.OnClickListener message_delete_dialog = new DialogInterface.OnClickListener() {
		@Override
		public void onClick(DialogInterface dialog, int which) {
			mCallback.onClearMessages(buddyId);
		} 
	};

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    switch (item.getItemId()) {
    		case R.id.itemClearMessages:
    			AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
    			builder.setMessage(getActivity().getResources().getString(R.string.question_delete_messages));
    			builder.setPositiveButton(getActivity().getResources().getString(android.R.string.yes), message_delete_dialog);
    			builder.setNegativeButton(getActivity().getResources().getString(android.R.string.no), null);
    			builder.show();
	    		return true;
	    	
	    	default:
	    		return super.onOptionsItemSelected(item);
	    }
	}

	@Override
	public void onContentChanged(String buddyId) {
		onContentChanged();
	}

	@Override
	public void onServiceConnected(ChatService service) {
		restoreDraftMessage();
		onContentChanged();
	}

	@Override
	public void onServiceDisconnected() {
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		service_helper = new ChatServiceHelper(getActivity(), this);		
		super.onCreate(savedInstanceState);
	}

	@Override
	public void onDestroy() {
		if (this.view != null) {
			this.view.onDestroy(getActivity());
			this.view = null;
		}
		
		service_helper.unbind();
	    super.onDestroy();
	}
	
	@Override
	public void onPause() {
		// set the current buddy to invisible
		ChatService.setInvisible(this.buddyId);
		
		if (buddyId != null) {
			EditText text = (EditText) getView().findViewById(R.id.textMessage);
			mCallback.onSaveMessage(buddyId, text.getText().toString());
		}
		
		super.onPause();
	}

	@Override
	public void onResume() {
		super.onResume();
		
		// set the current visible buddy
		ChatService.setVisible(this.buddyId);

		// refresh visible data
		onContentChanged();
	}
	
	public Roster getRoster() throws ServiceNotConnectedException {
		return this.service_helper.getService().getRoster();
	}
	
	private void onContentChanged()
	{
		Buddy buddy = null;
		Roster roster = null;
		
		try {
			if (buddyId != null) {
				roster = this.getRoster();
				
				// load buddy from roster
				buddy = roster.get( buddyId );
				
				if (buddy == null) {
					Log.e(TAG, "Error buddy not found: " + getActivity().getIntent().getStringExtra("buddy"));
					return;
				}
				
			    String presence_tag = buddy.getPresence();
			    String presence_nick = buddy.getNickname();
			    String presence_text = buddy.getEndpoint();
			    int presence_icon = R.drawable.online;
			    
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
				
				// if the presence is older than 60 minutes then mark the buddy as offline
				if (!buddy.isOnline())
				{
					presence_icon = R.drawable.offline;
				}

				ImageView iconTitleBar = (ImageView) this.getView().findViewById(R.id.buddy_icon);
				TextView labelTitleBar = (TextView) this.getView().findViewById(R.id.buddy_nickname);
				TextView bottomtextTitleBar = (TextView) this.getView().findViewById(R.id.buddy_statusmessage);
			
				labelTitleBar.setText( presence_nick );
				bottomtextTitleBar.setText( presence_text );
				iconTitleBar.setImageResource(presence_icon);
			}
			
			if (this.view == null) {
				// activate message view
				this.view = new MessageView(getActivity(), roster, buddy);
				
				ListView lv = (ListView)this.getActivity().findViewById(R.id.list_messages);
				lv.setAdapter(this.view);
				
				restoreDraftMessage();
			} else {
				this.view.refresh();
			}
		} catch (ServiceNotConnectedException e) {
			Log.e(TAG, "update failed", e);
		}
	}

	@Override
	public void onBuddySelected(String buddyId) {
		if ((this.buddyId != buddyId) && (this.view != null)) {
			this.view.onDestroy(getActivity());
			this.view = null;
			ListView lv = (ListView)this.getActivity().findViewById(R.id.list_messages);
			lv.setAdapter(null);
			
			if (this.buddyId != null) {
				EditText text = (EditText) getView().findViewById(R.id.textMessage);
				mCallback.onSaveMessage(this.buddyId, text.getText().toString());
			}
		}
		
		this.setHasOptionsMenu(buddyId != null);
		this.buddyId = buddyId;
		
		// set the current visible buddy
		ChatService.setVisible(this.buddyId);
	
		onContentChanged();
	}
}
