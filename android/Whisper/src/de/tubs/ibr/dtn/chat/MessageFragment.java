package de.tubs.ibr.dtn.chat;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.Fragment;
import android.support.v4.view.MenuItemCompat;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.service.ChatService;
import de.tubs.ibr.dtn.chat.service.Utils;

public class MessageFragment extends Fragment {
	
	private static final int MESSAGE_LOADER_ID = 1;
	private static final int BUDDY_LOADER_ID = 2;
	
	@SuppressWarnings("unused")
	private final String TAG = "MessageFragment";
	
	private ChatService mService = null;
	private boolean mBound = false;
	
	private MessageComposer mComposer = null;
	private MessageList mMessageList = null;
	private BuddyDisplay mBuddyDisplay = null;
	private MenuItem mVoiceMenu = null;
	
	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName name, IBinder service) {
			mService = ((ChatService.LocalBinder)service).getService();

			mMessageList.setChatService(mService);
			if (mBuddyDisplay != null) mBuddyDisplay.setChatService(mService);
			
        	// initialize the loaders
            getLoaderManager().initLoader(MESSAGE_LOADER_ID,  null, mMessageList);
            if (mBuddyDisplay != null) getLoaderManager().initLoader(BUDDY_LOADER_ID,  null, mBuddyDisplay);
            
            // restore draft message
            restoreDraftMessage();
            
            if (mVoiceMenu != null) {
                Buddy b = mService.getRoster().getBuddy(getShownBuddy());
                if (Utils.isVoiceRecordingSupported(getActivity()) && b.hasVoice()) {
                    mVoiceMenu.setVisible(true);
                } else {
                    mVoiceMenu.setVisible(false);
                }
            }
		}

		public void onServiceDisconnected(ComponentName name) {
			getLoaderManager().destroyLoader(MESSAGE_LOADER_ID);
			getLoaderManager().destroyLoader(BUDDY_LOADER_ID);
			mService = null;
		}
	};
	
    /**
     * Create a new instance of DetailsFragment, initialized to
     * show the buddy with buddyId
     */
    public static MessageFragment newInstance(Long buddyId) {
    	MessageFragment f = new MessageFragment();

        // Supply buddyId input as an argument.
        Bundle args = new Bundle();
        if (buddyId != null) args.putLong(ChatService.EXTRA_BUDDY_ID, buddyId);
        f.setArguments(args);

        return f;
    }

    public Long getShownBuddy() {
        Long id = getArguments().getLong(ChatService.EXTRA_BUDDY_ID, -1);
        if (id == -1) return null;
        return id;
    }
    
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.message_fragment, container, false);
		
		mComposer = (MessageComposer) v.findViewById(R.id.message_composer);
		mMessageList = (MessageList) v.findViewById(R.id.list_messages);
		mBuddyDisplay = (BuddyDisplay) v.findViewById(R.id.buddy_display);
		
		return v;
	}

	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		super.onActivityCreated(savedInstanceState);
		
		// enable options menu
		setHasOptionsMenu(true);
		
		// assign buddy id to sub-components
		Long buddyId = getShownBuddy();
		mComposer.setBuddyId(buddyId);
		mMessageList.setBuddyId(buddyId);
		if (mBuddyDisplay != null) mBuddyDisplay.setBuddyId(buddyId);
		
		mComposer.setEnabled(false);
	}
	
	private void restoreDraftMessage() {
		Long buddyId = getShownBuddy();
		
		if (buddyId != null) {
			// load buddy from roster
			String msg = mService.getRoster().getDraftMessage( buddyId );
			
			mComposer.setMessage(msg);
			mComposer.setEnabled(true);
		}
	}
	
    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.message_menu, menu);
        MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemVoiceMessage), MenuItemCompat.SHOW_AS_ACTION_ALWAYS | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
        
        mVoiceMenu = menu.findItem(R.id.itemVoiceMessage);
        if (mService != null) {
            Buddy b = mService.getRoster().getBuddy(getShownBuddy());
            if (Utils.isVoiceRecordingSupported(getActivity()) && b.hasVoice()) {
                mVoiceMenu.setVisible(true);
            } else {
                mVoiceMenu.setVisible(false);
            }
        }
        
        MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemClearMessages), MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
    }
	
	private DialogInterface.OnClickListener message_delete_dialog = new DialogInterface.OnClickListener() {
		public void onClick(DialogInterface dialog, int which) {
			if (mService == null)
				return;
			
			Long buddyId = getShownBuddy();
			
			if (buddyId != null) {
				// clear message of the buddy
				mService.getRoster().clearMessages(buddyId);
			}
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
	    		
    		case R.id.itemVoiceMessage:
    		    startVoiceRecording();
    		    return true;
	    	
	    	default:
	    		return super.onOptionsItemSelected(item);
	    }
	}
	
	@Override
	public void onStart() {
		super.onStart();
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
	}

	@Override
	public void onDestroy() {
		// unbind from service
		if (mBound) {
			getActivity().unbindService(mConnection);
			getLoaderManager().destroyLoader(MESSAGE_LOADER_ID);
			getLoaderManager().destroyLoader(BUDDY_LOADER_ID);
			mBound = false;
		}
		
	    super.onDestroy();
	}

	@Override
	public void onStop() {
		super.onStop();
	}

	@Override
	public void onPause() {
		Long buddyId = getShownBuddy();
		
		if (buddyId != null) {
			if (mService != null) {
				if (mComposer.getMessage().length() > 0)
					mService.getRoster().setDraftMessage(buddyId, mComposer.getMessage());
				else
					mService.getRoster().setDraftMessage(buddyId, null);
			}
		}
		
		// unregister to message updates
		getActivity().unregisterReceiver(mNotificationReceiver);
		
		super.onPause();
	}

	@Override
	public void onResume() {
		super.onResume();
		
		Long buddyId = getShownBuddy();
		
		if ((buddyId != null) && !mBound) {
			// Establish a connection with the service.  We use an explicit
			// class name because we want a specific service implementation that
			// we know will be running in our own process (and thus won't be
			// supporting component replacement by other applications).
			getActivity().bindService(new Intent(getActivity(), ChatService.class), mConnection, Context.BIND_AUTO_CREATE);
			mBound = true;
		}
		
		// register to notifications
		IntentFilter filter = new IntentFilter(ChatService.ACTION_NEW_MESSAGE);
		filter.setPriority(10);
		getActivity().registerReceiver(mNotificationReceiver, filter);
	}
	
	private BroadcastReceiver mNotificationReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (ChatService.ACTION_NEW_MESSAGE.equals(intent.getAction())) {
				Long buddyId = intent.getLongExtra(ChatService.EXTRA_BUDDY_ID, -1);
				if (getShownBuddy() == buddyId) abortBroadcast();
			}
		}
	};
	
	private void startVoiceRecording() {
        if (mService != null) {
            Buddy b = mService.getRoster().getBuddy(getShownBuddy());
            if (!b.hasVoice()) return;
            
            final Intent i = new Intent("de.tubs.ibr.dtn.dtalkie.RECORD_MESSAGE");
            i.addCategory(Intent.CATEGORY_DEFAULT);
            i.putExtra("destination", b.getVoiceEndpoint());
            i.putExtra("singleton", true);
            startActivity(i);
        }
	}
}
