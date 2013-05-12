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

package de.tubs.ibr.dtn.daemon;

import java.util.LinkedList;
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
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.Node;
import de.tubs.ibr.dtn.service.DaemonService;

public class NeighborListFragment extends ListFragment implements
	LoaderManager.LoaderCallbacks<List<Node>> {
	
	// The loader's unique id. Loader ids are specific to the Activity or
	// Fragment in which they reside.
	private static final int LOADER_ID = 1;

    private static final String TAG = "NeighborListFragment";
    
    private Object mServiceLock = new Object();
    private DTNService mService = null;
    private NeighborListAdapter mAdapter = null;

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder s) {
        	synchronized(mServiceLock) {
        		mService = DTNService.Stub.asInterface(s);
        	}
        	if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service connected");

        	// initialize the loader
            NeighborListFragment.this.getLoaderManager().initLoader(LOADER_ID,  null, NeighborListFragment.this);
        }

        public void onServiceDisconnected(ComponentName name) {
        	NeighborListFragment.this.getLoaderManager().destroyLoader(LOADER_ID);
        	
            synchronized(mServiceLock) {
            	mService = null;
            }
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service disconnected");
        }
    };

    @Override
	public void onListItemClick(ListView l, View v, int position, long id) {
		NeighborListAdapter nla = (NeighborListAdapter)this.getListAdapter();
		Node n = (Node)nla.getItem(position);
		
        // initiate connection via intent
        final Intent intent = new Intent(getActivity(), DaemonService.class);
        intent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_INITIATE_CONNECTION);
        intent.putExtra("endpoint", n.endpoint.toString());
        getActivity().startService(intent);
    	
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
        getActivity().bindService(new Intent(DTNService.class.getName()), mConnection,
                Context.BIND_AUTO_CREATE);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        setEmptyText(getActivity().getResources().getString(R.string.list_no_neighbors));

        // create a new list adapter
        mAdapter = new NeighborListAdapter(getActivity());

        // set listview adapter
        setListAdapter(mAdapter);
    }

    private class NeighborListAdapter extends BaseAdapter {
        private LayoutInflater inflater = null;
        private List<Node> list = new LinkedList<Node>();

        private class ViewHolder {
            public ImageView imageIcon;
            public TextView textName;
            public Node node;
        }

        public NeighborListAdapter(Context context) {
            this.inflater = LayoutInflater.from(context);
        }

        public int getCount() {
            return list.size();
        }

        public void add(Node n) {
            list.add(n);
        }

        public void clear() {
            list.clear();
        }

        @SuppressWarnings("unused")
		public void remove(int position) {
            list.remove(position);
        }

        public Object getItem(int position) {
            return list.get(position);
        }

        public long getItemId(int position) {
            return position;
        }

        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;

            if (convertView == null) {
                convertView = this.inflater.inflate(R.layout.neighborlist_item, null, true);
                holder = new ViewHolder();
                holder.imageIcon = (ImageView) convertView.findViewById(R.id.imageIcon);
                holder.textName = (TextView) convertView.findViewById(R.id.textName);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            holder.node = list.get(position);

            if (holder.node.type.equals("NODE_P2P")) {
                holder.imageIcon.setBackgroundColor(getResources().getColor(R.color.blue));
                holder.imageIcon.setImageResource(R.drawable.ic_p2p);
            } else if (holder.node.type.equals("NODE_DISCOVERED")) {
                holder.imageIcon.setBackgroundColor(getResources().getColor(R.color.blue));
                holder.imageIcon.setImageResource(R.drawable.ic_wifi);
            } else if (holder.node.type.equals("NODE_CONNECTED")) {
                holder.imageIcon.setBackgroundColor(getResources().getColor(R.color.green));
                holder.imageIcon.setImageResource(R.drawable.ic_node);
            } else if (holder.node.type.equals("NODE_INTERNET")) {
                holder.imageIcon.setBackgroundColor(getResources().getColor(R.color.dark_gray));
                holder.imageIcon.setImageResource(R.drawable.ic_world);
            } else {
                holder.imageIcon.setBackgroundColor(getResources().getColor(R.color.yellow));
                holder.imageIcon.setImageResource(R.drawable.ic_node);
            }

            holder.textName.setText(holder.node.endpoint.toString());

            return convertView;
        }  
    }

	@Override
	public Loader<List<Node>> onCreateLoader(int id, Bundle args) {
		return new NeighborListLoader(getActivity(), mService);
	}

	@Override
	public void onLoadFinished(Loader<List<Node>> loader, List<Node> neighbors) {
        synchronized (mAdapter) {
            // clear all data
            mAdapter.clear();

            for (Node n : neighbors) {
                mAdapter.add(n);
            }
            
            mAdapter.notifyDataSetChanged();
        }
	}

	@Override
	public void onLoaderReset(Loader<List<Node>> loader) {
		mAdapter.clear();
	};
}
