/*
 * NeighborListFragment.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

package de.tubs.ibr.dtn.daemon.api;

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
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.ListView;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.Node;
import de.tubs.ibr.dtn.daemon.data.NeighborListAdapter;
import de.tubs.ibr.dtn.daemon.data.NeighborListLoader;
import de.tubs.ibr.dtn.service.DaemonService;

public class SelectNeighborFragment extends ListFragment implements
	LoaderManager.LoaderCallbacks<List<Node>> {
	
	// The loader's unique id. Loader ids are specific to the Activity or
	// Fragment in which they reside.
	private static final int LOADER_ID = 1;

    private static final String TAG = "SelectNeighborFragment";
    
    private DTNService mService = null;
    private NeighborListAdapter mAdapter = null;

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder s) {
    		mService = DTNService.Stub.asInterface(s);
        	if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service connected");

        	// initialize the loader
            getLoaderManager().initLoader(LOADER_ID,  null, SelectNeighborFragment.this);
        }

        public void onServiceDisconnected(ComponentName name) {
        	getLoaderManager().destroyLoader(LOADER_ID);
        	
        	mService = null;
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service disconnected");
        }
    };

    @Override
	public void onListItemClick(ListView l, View v, int position, long id) {
        NeighborListAdapter nla = (NeighborListAdapter)this.getListAdapter();
        Node n = (Node)nla.getItem(position);
        
        Intent data = new Intent(de.tubs.ibr.dtn.Intent.NEIGHBOR_SELECTED);
        data.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_NODE, n);
        
        // select the item
        getActivity().setResult(0, data);
        getActivity().finish();
        
        // call super-method
        super.onListItemClick(l, v, position, id);
	}

    @Override
    public void onPause() {
        // Detach our existing connection.
        getActivity().unbindService(mConnection);
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();

        // Establish a connection with the service. We use an explicit
        // class name because we want a specific service implementation that
        // we know will be running in our own process (and thus won't be
        // supporting component replacement by other applications).
        Intent bindIntent = DaemonService.createDtnServiceIntent(getActivity());
        getActivity().bindService(bindIntent, mConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        
        // We have a menu item to show in action bar.
        setHasOptionsMenu(true);

        setEmptyText(getActivity().getResources().getString(R.string.list_no_neighbors));

        // create a new list adapter
        mAdapter = new NeighborListAdapter(getActivity());

        // set listview adapter
        setListAdapter(mAdapter);
        
        // Start out with a progress indicator.
        setListShown(false);
    }
    
    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
        inflater.inflate(R.menu.neighbor_menu, menu);
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.itemDiscovery: {
                // start discovery
                final Intent discoIntent = new Intent(getActivity(), DaemonService.class);
                discoIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_START_DISCOVERY);
                discoIntent.putExtra(DaemonService.EXTRA_DISCOVERY_DURATION, 120L);
                getActivity().startService(discoIntent);
            }

            default:
                return super.onOptionsItemSelected(item);
        }
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
