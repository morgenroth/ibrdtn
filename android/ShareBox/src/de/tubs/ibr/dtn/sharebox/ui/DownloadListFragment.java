package de.tubs.ibr.dtn.sharebox.ui;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.os.Bundle;
import android.os.IBinder;
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
import de.tubs.ibr.dtn.sharebox.DtnService;
import de.tubs.ibr.dtn.sharebox.R;
import de.tubs.ibr.dtn.sharebox.data.Database;
import de.tubs.ibr.dtn.sharebox.data.Download;
import de.tubs.ibr.dtn.sharebox.data.Utils;

public class DownloadListFragment extends ListFragment implements LoaderManager.LoaderCallbacks<Cursor> {

    private static final int DOWNLOAD_LOADER_ID = 1;
    
    @SuppressWarnings("unused")
    private final String TAG = "DownloadListFragment";
    
    private CursorAdapter mAdapter = null;
    private DtnService mService = null;
    private Boolean mBound = false;
    
    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder service) {
            mService = ((DtnService.LocalBinder)service).getService();
            
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
            getLoaderManager().initLoader(DOWNLOAD_LOADER_ID,  null, DownloadListFragment.this);
        }

        public void onServiceDisconnected(ComponentName name) {
            getLoaderManager().destroyLoader(DOWNLOAD_LOADER_ID);
            mService = null;
        }
    };
    
    /**
     * Create a new instance of DetailsFragment, initialized to
     * show the buddy with buddyId
     */
    public static DownloadListFragment newInstance() {
        DownloadListFragment f = new DownloadListFragment();
        return f;
    }
    
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        
        // enable options menu
        setHasOptionsMenu(true);
        
        // enable context menu
        registerForContextMenu(getListView());
        
        mAdapter = new DownloadAdapter(getActivity(), null, new DownloadAdapter.ColumnsMap());
        setListAdapter(mAdapter);
        
        // Start out with a progress indicator.
        setListShown(false);
    }
    
    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.main_menu, menu);
        MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemClearList), MenuItemCompat.SHOW_AS_ACTION_NEVER | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.itemClearList:
                Database db = mService.getDatabase();
                db.clear();
                return true;
            
            default:
                return super.onOptionsItemSelected(item);
        }
    }
    
    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
        super.onCreateContextMenu(menu, v, menuInfo);
        
        MenuInflater inflater = getActivity().getMenuInflater();
        inflater.inflate(R.menu.item_menu, menu);
    }
    
    @Override
    public boolean onContextItemSelected(MenuItem item) {
        AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
        
        if (info.targetView instanceof DownloadItem) {
            DownloadItem di = (DownloadItem)info.targetView;
            Download d = di.getObject();

            switch (item.getItemId())
            {
            case R.id.itemDelete:
                Intent rejectIntent = new Intent(getActivity(), DtnService.class);
                rejectIntent.setAction(DtnService.REJECT_DOWNLOAD_INTENT);
                rejectIntent.putExtra(DtnService.PARCEL_KEY_BUNDLE_ID, d.getBundleId());
                getActivity().startService(rejectIntent);
                return true;
                
            default:
                return super.onContextItemSelected(item);
            }
        }
    
        return super.onContextItemSelected(item);
    }

    @Override
    public void onListItemClick(ListView l, View v, int position, long id) {
        DownloadItem item = (DownloadItem)v;
        if (item != null) {
            if (item.getObject().isPending()) {
                Intent dialogIntent = new Intent(getActivity(), PendingDialog.class);
                dialogIntent.putExtra(DtnService.PARCEL_KEY_BUNDLE_ID, item.getObject().getBundleId());
                dialogIntent.putExtra(DtnService.PARCEL_KEY_LENGTH, item.getObject().getLength());
                getActivity().startActivity(dialogIntent);
            }
        }
    }
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mBound = false;
    }

    @Override
    public void onDestroy() {
        if (mBound) {
            getLoaderManager().destroyLoader(DOWNLOAD_LOADER_ID);
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
            getActivity().bindService(new Intent(getActivity(), DtnService.class), mConnection, Context.BIND_AUTO_CREATE);
            mBound = true;
        }
    }

    @Override
    public Loader<Cursor> onCreateLoader(int id, Bundle args) {
        // Now create and return a CursorLoader that will take care of
        // creating a Cursor for the data being displayed.
        return new DownloadLoader(getActivity(), mService);
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
