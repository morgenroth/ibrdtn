package de.tubs.ibr.dtn.dtalkie;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.database.Cursor;
import android.os.Bundle;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.support.v4.app.ListFragment;
import android.support.v4.app.LoaderManager;
import android.support.v4.content.Loader;
import android.support.v4.view.MenuItemCompat;
import android.support.v4.widget.CursorAdapter;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.ListView;
import de.tubs.ibr.dtn.dtalkie.db.Message;
import de.tubs.ibr.dtn.dtalkie.db.MessageDatabase;
import de.tubs.ibr.dtn.dtalkie.db.MessageDatabase.Folder;
import de.tubs.ibr.dtn.dtalkie.service.TalkieService;
import de.tubs.ibr.dtn.dtalkie.service.Utils;

public class MessageFragment extends ListFragment implements LoaderManager.LoaderCallbacks<Cursor> {
    
    private static final int MESSAGE_LOADER_ID = 1;
    
    @SuppressWarnings("unused")
    private final String TAG = "MessageFragment";
    
    private CursorAdapter mAdapter = null;
    private TalkieService mService = null;
    private Boolean mBound = false;
    
    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder service) {
            mService = ((TalkieService.LocalBinder)service).getService();
            
            // check possible errors
            switch ( mService.getServiceError() ) {
            case NO_ERROR:
                break;
                
            case SERVICE_NOT_FOUND:
                Utils.showInstallServiceDialog(getActivity());
                return;
                
            case PERMISSION_NOT_GRANTED:
                Utils.showReinstallDialog(getActivity());
                return;
            }
            
            // initialize the loaders
            getLoaderManager().initLoader(MESSAGE_LOADER_ID,  null, MessageFragment.this);
        }

        public void onServiceDisconnected(ComponentName name) {
            getLoaderManager().destroyLoader(MESSAGE_LOADER_ID);
            mService = null;
        }
    };
    
    /**
     * Create a new instance of DetailsFragment, initialized to
     * show the buddy with buddyId
     */
    public static MessageFragment newInstance() {
        MessageFragment f = new MessageFragment();
        return f;
    }
    
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        
        // enable options menu
        setHasOptionsMenu(true);
        
        // enable context menu
        registerForContextMenu(getListView());
        
        mAdapter = new MessageAdapter(getActivity(), null, new MessageAdapter.ColumnsMap());
        setListAdapter(mAdapter);
        
        // Start out with a progress indicator.
        setListShown(false);
    }
    
    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.main_menu, menu);
        
        MenuItem autoplay = menu.findItem(R.id.itemAutoPlay);
        
        MenuItemCompat.setShowAsAction(autoplay, MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
        MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemClearList), MenuItemCompat.SHOW_AS_ACTION_NEVER | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
    }
    
    @Override
    public void onPrepareOptionsMenu(Menu menu) {
        MenuItem autoplay = menu.findItem(R.id.itemAutoPlay);
        
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());

        // restore autoplay option
        autoplay.setIcon(prefs.getBoolean("autoplay", false) ? R.drawable.ic_autoplay_pause : R.drawable.ic_autoplay);
        autoplay.setChecked(prefs.getBoolean("autoplay", false));
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
        
        switch (item.getItemId()) {
            case R.id.itemAutoPlay:
            {
                Editor edit = prefs.edit();
                Boolean newvalue = (!prefs.getBoolean("autoplay", false));
                edit.putBoolean("autoplay", newvalue);
                item.setChecked(newvalue);
                item.setIcon(newvalue ? R.drawable.ic_autoplay_pause : R.drawable.ic_autoplay);
                edit.commit();
                return true;
            }
                
            case R.id.itemClearList:
                MessageDatabase db = mService.getDatabase();
                db.clear(Folder.INBOX);
                return true;
            
            default:
                return super.onOptionsItemSelected(item);
        }
    }
    
    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
        super.onCreateContextMenu(menu, v, menuInfo);
        
        MenuInflater inflater = getActivity().getMenuInflater();
        inflater.inflate(R.menu.message_menu, menu);
    }
    
    @Override
    public boolean onContextItemSelected(MenuItem item) {
        AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
        
        if (info.targetView instanceof MessageItem) {
            MessageItem mitem = (MessageItem)info.targetView;
            Message m = mitem.getMessage();

            switch (item.getItemId())
            {
            case R.id.itemDelete:
                if (mService != null) {
                    mService.getDatabase().remove(Folder.INBOX, m.getId());
                }
                return true;
            default:
                return super.onContextItemSelected(item);
            }
        }
    
        return super.onContextItemSelected(item);
    }

    @Override
    public void onListItemClick(ListView l, View v, int position, long id) {
        MessageItem mitem = (MessageItem)v;
        if (mitem != null) {
            // play the selected message
            Intent play_i = new Intent(getActivity(), TalkieService.class);
            play_i.setAction(TalkieService.ACTION_PLAY);
            play_i.putExtra("folder", Folder.INBOX.toString());
            play_i.putExtra("message", mitem.getMessage().getId());
            getActivity().startService(play_i);
        }
    }
    
    public OnSharedPreferenceChangeListener mPrefListener = new OnSharedPreferenceChangeListener() {
        @Override
        public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
            if ("autoplay".equals(key)) {
                if (prefs.getBoolean(key, false)) {
                    Intent play_i = new Intent(getActivity(), TalkieService.class);
                    play_i.setAction(TalkieService.ACTION_PLAY_NEXT);
                    getActivity().startService(play_i);
                }
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mBound = false;
        
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
        prefs.registerOnSharedPreferenceChangeListener(mPrefListener);
    }

    @Override
    public void onDestroy() {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
        prefs.unregisterOnSharedPreferenceChangeListener(mPrefListener);
        
        if (mBound) {
            getLoaderManager().destroyLoader(MESSAGE_LOADER_ID);
            getActivity().unbindService(mConnection);
            mBound = false;
        }
        
        super.onDestroy();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        
        if (!mBound) {
            getActivity().bindService(new Intent(getActivity(), TalkieService.class), mConnection, Context.BIND_AUTO_CREATE);
            mBound = true;
        }
    }

    @Override
    public Loader<Cursor> onCreateLoader(int id, Bundle args) {
        // Now create and return a CursorLoader that will take care of
        // creating a Cursor for the data being displayed.
        return new MessageListLoader(getActivity(), mService, Folder.INBOX);
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
