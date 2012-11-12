/*
 * NeighborList.java
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

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import android.app.ListActivity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.widget.ListAdapter;
import android.widget.SimpleAdapter;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.R;

public class NeighborList extends ListActivity {
	
	private DTNService service = null;
	private List<Map<String, String>> _data = new LinkedList<Map<String, String>>();
	
	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName name, IBinder service) {
			NeighborList.this.service = DTNService.Stub.asInterface(service);
			Log.i("IBR-DTN", "NeighborList: service connected");
			
			IntentFilter filter = new IntentFilter(de.tubs.ibr.dtn.Intent.NEIGHBOR);
			filter.addCategory(Intent.CATEGORY_DEFAULT);
			NeighborList.this.registerReceiver(_receiver, filter);
			refreshView();
		}

		public void onServiceDisconnected(ComponentName name) {
			Log.i("IBR-DTN", "NeighborList: service disconnected");
			service = null;
		}
	};
	
	private BroadcastReceiver _receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.NEIGHBOR)) {
				refreshView();
			}
		}	
	};
	
	private void refreshView() {
		if (NeighborList.this.service != null) {
			(new LoadNeighborList()).execute();
		}
	}
	
    @Override
	protected void onPause() {
        // Detach our existing connection.
    	NeighborList.this.unregisterReceiver(_receiver);
		unbindService(mConnection);
		super.onPause();
	}

	@Override
	protected void onResume() {
		super.onResume();
		
		// Establish a connection with the service.  We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		bindService(new Intent(DTNService.class.getName()), mConnection, Context.BIND_AUTO_CREATE);
	}
    
	private class LoadNeighborList extends AsyncTask<String, Integer, List<String>> {
		protected List<String> doInBackground(String... data)
		{
			try {
		        // query all neighbors
				List<String> neighbors = service.getNeighbors();
				return neighbors;
			} catch (RemoteException e) {
				return null;
			}
		}

		protected void onPostExecute(List<String> neighbors)
		{
			if (neighbors == null) return;
			
	        // Now create a new list adapter bound to the cursor.
	        // SimpleListAdapter is designed for binding to a Cursor.
	        ListAdapter adapter = new SimpleAdapter(
	        		NeighborList.this,
	                _data,
	                R.layout.neighborlist_item,
	                new String[] { "eid" },
	                new int[] { R.id.textName }
	        );
	        
	        synchronized(_data) {
				// clear all data
				_data.clear();
				
				for (String n : neighbors)
				{
					HashMap<String, String> m = new HashMap<String, String>();
					m.put("eid", n);
					_data.add( m );
				}

				// Bind to our new adapter.
				setListAdapter(adapter);
	        }
		}
	}
}
