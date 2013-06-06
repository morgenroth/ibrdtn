package de.tubs.ibr.dtn.chat;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ApplicationInfo;
import android.database.Cursor;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.app.ListFragment;
import android.support.v4.app.LoaderManager;
import android.support.v4.content.Loader;
import android.support.v4.view.MenuItemCompat;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.ListView;
import de.tubs.ibr.dtn.chat.service.ChatService;
import de.tubs.ibr.dtn.chat.service.Utils;

public class RosterFragment extends ListFragment implements LoaderManager.LoaderCallbacks<Cursor> {
    boolean mDualPane;
    Long mBuddyId = null;
	
    private final static int LOADER_ID = 1;
    
	@SuppressWarnings("unused")
	private final String TAG = "RosterFragment";
	
	private RosterAdapter mAdapter = null;

	private ChatService mService = null;
	private boolean mBound = false;
	
	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName name, IBinder service) {
			mService = ((ChatService.LocalBinder)service).getService();
			
			switch (mService.getServiceError()) {
			case NO_ERROR:
				break;
				
			case SERVICE_NOT_FOUND:
				Utils.showInstallServiceDialog(getActivity());
				break;
				
			case PERMISSION_NOT_GRANTED:
				Utils.showReinstallDialog(getActivity());
				break;
			}
			
			// load roster
			getLoaderManager().initLoader(LOADER_ID, null, RosterFragment.this);
		}

		public void onServiceDisconnected(ComponentName name) {
			getLoaderManager().destroyLoader(LOADER_ID);
			mService = null;
		}
	};
	
    @Override
	public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
	    inflater.inflate(R.menu.buddy_menu, menu);
	    MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemPreferences), MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
	    
	    if (0 != (getActivity().getApplicationInfo().flags & ApplicationInfo.FLAG_DEBUGGABLE)) {
	    	menu.findItem(R.id.itemDebugNotification).setVisible(true);
	    	menu.findItem(R.id.itemDebugBuddyAdd).setVisible(true);
	    	menu.findItem(R.id.itemDebugSendPresence).setVisible(true);
	    } else {
	    	menu.findItem(R.id.itemDebugNotification).setVisible(false);
	    	menu.findItem(R.id.itemDebugBuddyAdd).setVisible(false);
	    	menu.findItem(R.id.itemDebugSendPresence).setVisible(false);
	    }
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    // Handle item selection
	    switch (item.getItemId()) {
	    case R.id.itemPreferences:
	    {
			// Launch Preference activity
			Intent i = new Intent(getActivity(), Preferences.class);
			startActivity(i);
	        return true;
	    }
	    
	    case R.id.itemDebugNotification:
	    	if (mService != null)
				mService.startDebug(ChatService.Debug.NOTIFICATION);
	    	return true;
	    	
	    case R.id.itemDebugBuddyAdd:
	    	if (mService != null)
				mService.startDebug(ChatService.Debug.BUDDY_ADD);
	    	return true;
	    	
	    case R.id.itemDebugSendPresence:
            if (mService != null)
                mService.startDebug(ChatService.Debug.SEND_PRESENCE);
	        return true;
    
	    default:
	        return super.onOptionsItemSelected(item);
	    }
	}

	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		super.onActivityCreated(savedInstanceState);
		
		// enable options menu
		setHasOptionsMenu(true);
		
		// enable context menu
		registerForContextMenu(getListView());
		
        // Create an empty adapter we will use to display the loaded data.
        mAdapter = new RosterAdapter(getActivity(), null, new RosterAdapter.ColumnsMap());
        setListAdapter(mAdapter);
		
        // Check to see if we have a frame in which to embed the details
        // fragment directly in the containing UI.
        View messagesFrame = getActivity().findViewById(R.id.messages);
        mDualPane = messagesFrame != null && messagesFrame.getVisibility() == View.VISIBLE;

        if (savedInstanceState != null) {
            // Restore last state for checked position.
        	mBuddyId = savedInstanceState.getLong("buddyId", -1);
        	if (mBuddyId.equals(-1L)) mBuddyId = null;
        } else {
        	Intent i = getActivity().getIntent();
        	if (i != null)
        	{
        		Bundle extras = i.getExtras();
            	if (extras != null) {
            		mBuddyId = extras.getLong("buddyId", -1);
            		if (mBuddyId.equals(-1L)) mBuddyId = null;
            	}
        	}
        }

        if (mDualPane) {
            // In dual-pane mode, the list view highlights the selected item.
            getListView().setChoiceMode(ListView.CHOICE_MODE_SINGLE);
        }
        
        // Make sure our UI is in the correct state.
        if (mBuddyId != null) showMessages(mBuddyId);

        // Start out with a progress indicator.
        setListShown(false);
	}
	  
    public void showMessages(Long buddyId) {
    	mBuddyId = buddyId;
    	
        if (mDualPane) {
            // We can display everything in-place with fragments, so update
            // the list to highlight the selected item and show the data.
    		if (mAdapter != null)
    			mAdapter.setSelected(buddyId);
    		
            // Check what fragment is currently shown, replace if needed.
            MessageFragment messages = (MessageFragment)
                    getFragmentManager().findFragmentById(R.id.messages);
            if (messages == null || messages.getShownBuddy() != buddyId) {
                // Make new fragment to show this selection.
            	messages = MessageFragment.newInstance(buddyId);

                // Execute a transaction, replacing any existing fragment
                // with this one inside the frame.
                FragmentTransaction ft = getFragmentManager().beginTransaction();
                ft.replace(R.id.messages, messages);
                ft.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_FADE);
                ft.commit();
            }

        } else {
            // Otherwise we need to launch a new activity to display
            // the dialog fragment with selected text.
            Intent intent = new Intent();
            intent.setClass(getActivity(), MessageActivity.class);
            intent.putExtra("buddyId", buddyId);
            startActivity(intent);
        }
    }
    
	@Override
	public void onSaveInstanceState(Bundle outState) {
		super.onSaveInstanceState(outState);
		if (mBuddyId != null)
			outState.putLong("buddyId", mBuddyId);
		else
			outState.remove("buddyId");
	}

	@Override
	public void onListItemClick(ListView l, View v, int position, long id) {
		// get buddy of position
		RosterItem ritem = (RosterItem)v;
		if (ritem != null)
			showMessages(ritem.getBuddyId());
	}
    
	@Override
	public void onResume() {
		super.onResume();
		
		// Establish a connection with the service.  We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		if (!mBound) {
			getActivity().bindService(new Intent(getActivity(), ChatService.class), mConnection, Context.BIND_AUTO_CREATE);
			mBound = true;
		}
	}
	
	@Override
	public void onPause() {
		super.onPause();
	}

	@Override
	public void onStart() {
		super.onStart();
	}

	@Override
	public void onStop() {
		super.onStop();
	}

	@Override
	public void onDestroy() {
		// unbind from service
		if (mBound) {
			getActivity().unbindService(mConnection);
			getLoaderManager().destroyLoader(LOADER_ID);
			mBound = false;
		}
		
		super.onDestroy();
	}

	@Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
		super.onCreateContextMenu(menu, v, menuInfo);
		
		MenuInflater inflater = getActivity().getMenuInflater();
		inflater.inflate(R.menu.buddycontext_menu, menu);
	}
	
	@Override
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
		
		if (info.targetView instanceof RosterItem) {
			RosterItem ritem = (RosterItem)info.targetView;

			switch (item.getItemId())
			{
			case R.id.itemDelete:
				if (mService != null) {
					mService.getRoster().removeBuddy(ritem.getBuddyId());
				}
				return true;
			default:
				return super.onContextItemSelected(item);
			}
		}
		
		return super.onContextItemSelected(item);
	}

	@Override
	public Loader<Cursor> onCreateLoader(int id, Bundle args) {
        // Now create and return a CursorLoader that will take care of
        // creating a Cursor for the data being displayed.
        return new RosterLoader(getActivity(), mService);
	}
	
	@Override
	public void onLoadFinished(Loader<Cursor> loader, Cursor data) {
        // Swap the new cursor in.  (The framework will take care of closing the
        // old cursor once we return.)
        mAdapter.swapCursor(data);
        
        // The list should now be shown.
        if (isResumed()) {
            setListShown(true);
        } else {
            setListShownNoAnimation(true);
        }
	}
	
	@Override
	public void onLoaderReset(Loader<Cursor> loader) {
        // This is called when the last Cursor provided to onLoadFinished()
        // above is about to be closed.  We need to make sure we are no
        // longer using it.
        mAdapter.swapCursor(null);
	}
}
