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

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.support.v4.app.ListFragment;
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

public class NeighborListFragment extends ListFragment {

    private static final String TAG = "NeighborListFragment";
    private DTNService service = null;
    private NeighborListAdapter mAdapter = null;

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder service) {
            NeighborListFragment.this.service = DTNService.Stub.asInterface(service);
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service connected");

            IntentFilter filter = new IntentFilter(de.tubs.ibr.dtn.Intent.NEIGHBOR);
            filter.addCategory(Intent.CATEGORY_DEFAULT);
            getActivity().registerReceiver(_receiver, filter);
            refreshView();
        }

        public void onServiceDisconnected(ComponentName name) {
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service disconnected");
            service = null;
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

	private BroadcastReceiver _receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.NEIGHBOR)) {
                refreshView();
            }
        }
    };

    private void refreshView() {
        if (NeighborListFragment.this.service != null) {
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "refreshing");
            (new LoadNeighborList()).execute();
        }
    }

    @Override
    public void onPause() {
        // Detach our existing connection.
        getActivity().unregisterReceiver(_receiver);
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

    private class LoadNeighborList extends AsyncTask<String, Integer, List<Node>> {
        protected List<Node> doInBackground(String... data)
        {
            try {
                // query all neighbors
                List<Node> neighbors = service.getNeighbors();
                return neighbors;
            } catch (RemoteException e) {
                return null;
            }
        }

        protected void onPostExecute(List<Node> neighbors)
        {
            if (neighbors == null)
                return;

            synchronized (mAdapter) {
                // clear all data
                mAdapter.clear();

                for (Node n : neighbors) {
                    mAdapter.add(n);
                }
                
                mAdapter.notifyDataSetChanged();
            }
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
    };
}
