/*
 * LogActivity.java
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

import android.app.ListActivity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import de.tubs.ibr.dtn.DTNService;
import android.graphics.Color;
import de.tubs.ibr.dtn.R;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class LogActivity extends ListActivity {
	
	private final static String TAG = "LogActivity"; 
	private SmartListAdapter adapter = null;
	private DTNService service = null;
	
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			LogActivity.this.service = DTNService.Stub.asInterface(service);
			Log.i(TAG, "service connected");
			
			(new LoadDataTask()).execute();
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.i(TAG, "service disconnected");
			service = null;
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		// create a new smart list adapter
		adapter = new SmartListAdapter(this);
		
		// set listview adapter
		getListView().setAdapter(adapter);
		
		// Establish a connection with the service.  We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		bindService(new Intent(DTNService.class.getName()), mConnection, Context.BIND_AUTO_CREATE);
	}

	@Override
	protected void onDestroy() {
        // Detach our existing connection.
		unbindService(mConnection);
		
		super.onDestroy();
	}
    
	private class LoadDataTask extends AsyncTask<String, Integer, Boolean> {
		protected Boolean doInBackground(String... data)
		{
			try {
		        // query all logs
				List<String> logs = service.getLog();
				
				// clear all data
				LogActivity.this.adapter.clear();
				
				if (logs != null) {
					for (String l : logs) {
						LogActivity.this.adapter.add(new LogMessage(l));
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
	        // refresh data view
	        LogActivity.this.adapter.refresh();
	        
	        getListView().setSelection(adapter.getCount() - 1);
		}
	}
	
	private class ViewHolder
	{
		public ImageView imageMark;
		public TextView textDate;
		public TextView textTag;
		public TextView textLog;
		public LogMessage msg;
	}
	
	private class SmartListAdapter extends BaseAdapter {
		
		private LayoutInflater inflater = null;
		private List<LogMessage> list = new LinkedList<LogMessage>();

		public SmartListAdapter(Context context)
		{
			this.inflater = LayoutInflater.from(context);
		}
		
		public int getCount() {
			return list.size();
		}
		
		public void clear() {
			list.clear();
		}
		
		public void add(LogMessage msg) {
			list.add(msg);
		}

		public Object getItem(int position) {
			return list.get(position);
		}

		public long getItemId(int position) {
			return position;
		}
		
		public View getView(int position, View convertView, ViewGroup parent) {
			ViewHolder holder;
			
			if (convertView == null)
			{
				convertView = this.inflater.inflate(R.layout.log_item, null, true);
				holder = new ViewHolder();
				holder.imageMark = (ImageView) convertView.findViewById(R.id.imageMark);
				holder.textDate = (TextView) convertView.findViewById(R.id.textDate);
				holder.textTag = (TextView) convertView.findViewById(R.id.textTag);
				holder.textLog = (TextView) convertView.findViewById(R.id.textLog);
				convertView.setTag(holder);
			} else {
				holder = (ViewHolder) convertView.getTag();
			}
			
			holder.msg = list.get(position);
			
			if (holder.msg.tag.equals("EMERGENCY")) {
				holder.imageMark.setImageLevel(1);
				holder.textTag.setTextColor(getResources().getColor(R.color.light_red));
			}
			else if (holder.msg.tag.equals("ALERT")) {
				holder.imageMark.setImageLevel(2);
				holder.textTag.setTextColor(getResources().getColor(R.color.light_blue));
			}
			else if (holder.msg.tag.equals("CRITICAL")) {
				holder.imageMark.setImageLevel(1);
				holder.textTag.setTextColor(getResources().getColor(R.color.light_red));
			}
			else if (holder.msg.tag.equals("ERROR")) {
				holder.imageMark.setImageLevel(1);
				holder.textTag.setTextColor(getResources().getColor(R.color.light_red));
			}
			else if (holder.msg.tag.equals("WARNING")) {
				holder.imageMark.setImageLevel(3);
				holder.textTag.setTextColor(getResources().getColor(R.color.light_violett));
			}
			else if (holder.msg.tag.equals("NOTICE")) {
				holder.imageMark.setImageLevel(5);
				holder.textTag.setTextColor(getResources().getColor(R.color.light_yellow));
			}
			else if (holder.msg.tag.equals("INFO")) {
				holder.imageMark.setImageLevel(4);
				holder.textTag.setTextColor(getResources().getColor(R.color.light_green));
			}
			else {
				holder.imageMark.setImageLevel(0);
				holder.textTag.setTextColor(Color.WHITE);
			}
			
			holder.textDate.setText(holder.msg.date);
			holder.textLog.setText(holder.msg.msg);
			holder.textTag.setText(holder.msg.tag);
			
			return convertView;
		}
		
		public void refresh()
		{
			notifyDataSetChanged();
		}
	};
}
