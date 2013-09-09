package de.tubs.ibr.dtn.ping;

import java.util.List;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.ListFragment;
import android.support.v4.app.LoaderManager;
import android.support.v4.content.Loader;
import android.view.View;
import android.widget.ListView;
import de.tubs.ibr.dtn.api.Node;

public class NeighborChooserFragment extends ListFragment implements
    LoaderManager.LoaderCallbacks<List<Node>> {
    
    // The loader's unique id. Loader ids are specific to the Activity or
    // Fragment in which they reside.
    private static final int LOADER_ID = 1;
    
    private NeighborListAdapter mAdapter = null;
    private PingService mService = null;
    private Boolean mBound = false;
    
    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder service) {
            mService = ((PingService.LocalBinder)service).getService();
            
            // initialize the loaders
            getLoaderManager().initLoader(LOADER_ID,  null, NeighborChooserFragment.this);
        }

        public void onServiceDisconnected(ComponentName name) {
            getLoaderManager().destroyLoader(LOADER_ID);
            mService = null;
        }
    };
    
    @Override
    public void onListItemClick(ListView l, View v, int position, long id) {
        NeighborListAdapter nla = (NeighborListAdapter)this.getListAdapter();
        Node n = (Node)nla.getItem(position);
        
        Intent data = new Intent();
        data.putExtra("endpoint", n.endpoint.toString());
        
        // select the item
        getActivity().setResult(0, data);
        getActivity().finish();
        
        // call super-method
        super.onListItemClick(l, v, position, id);
    }
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mBound = false;
    }

    @Override
    public void onDestroy() {
        if (mBound) {
            getLoaderManager().destroyLoader(LOADER_ID);
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
            getActivity().bindService(new Intent(getActivity(), PingService.class), mConnection, Context.BIND_AUTO_CREATE);
            mBound = true;
        }
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        setEmptyText(getActivity().getResources().getString(R.string.list_no_neighbors));

        // create a new list adapter
        mAdapter = new NeighborListAdapter(getActivity());

        // set listview adapter
        setListAdapter(mAdapter);
        
        // Start out with a progress indicator.
        setListShown(false);
    }

    @Override
    public Loader<List<Node>> onCreateLoader(int id, Bundle args) {
        return new NeighborListLoader(getActivity(), mService);
    }

    @Override
    public void onLoadFinished(Loader<List<Node>> loader, List<Node> neighbors) {
        synchronized (mAdapter) {
            mAdapter.swapList(neighbors);
        }
        
        // The list should now be shown.
        if (isResumed()) {
            setListShown(true);
        } else {
            setListShownNoAnimation(true);
        }
    }

    @Override
    public void onLoaderReset(Loader<List<Node>> loader) {
    }
}
