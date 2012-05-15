package de.tubs.ibr.dtn.daemon;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import android.app.ListActivity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.R;
import android.os.AsyncTask;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.widget.ListAdapter;
import android.widget.SimpleAdapter;

public class NeighborList extends ListActivity {
	
	private DTNService service = null;
	private List<Map<String, String>> _data = new LinkedList<Map<String, String>>();
	
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			NeighborList.this.service = DTNService.Stub.asInterface(service);
			Log.i("IBR-DTN", "NeighborList: service connected");
			
			(new LoadNeighborList()).execute();
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.i("IBR-DTN", "NeighborList: service disconnected");
			service = null;
		}
	};
	
    @Override
	protected void onPause() {
        // Detach our existing connection.
		unbindService(mConnection);
		
		super.onPause();
	}

	@Override
	protected void onResume() {
		// Establish a connection with the service.  We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		bindService(new Intent(DTNService.class.getName()), mConnection, Context.BIND_AUTO_CREATE);
  		
		super.onResume();
	}
    
	private class LoadNeighborList extends AsyncTask<String, Integer, Boolean> {
		protected Boolean doInBackground(String... data)
		{
			try {
		        // query all neighbors
				List<String> neighbors = service.getNeighbors();
			
				// clear all data
				_data.clear();
				
				if (neighbors != null) {
					for (String n : neighbors)
					{
						HashMap<String, String> m = new HashMap<String, String>();
						m.put("eid", n);
						_data.add( m );
					}
				}
		        
				return true;
			} catch (RemoteException e) {
				return false;
			}
		}

		protected void onProgressUpdate(Integer... progress) {
		}

		protected void onPostExecute(Boolean result)
		{
	        // Now create a new list adapter bound to the cursor.
	        // SimpleListAdapter is designed for binding to a Cursor.
	        ListAdapter adapter = new SimpleAdapter(
	        		NeighborList.this,
	                _data,
	                R.layout.neighborlist_item,
	                new String[] { "eid" },
	                new int[] { R.id.text1 }
	        );

	        // Bind to our new adapter.
	        setListAdapter(adapter);
		}
	}
}
