package de.tubs.ibr.dtn.daemon;

import java.util.LinkedList;
import java.util.List;

import android.app.ListActivity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Color;
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
import android.widget.ListView;
import android.widget.TextView;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.R;

public class LogActivity extends ListActivity {
	
	private final static String TAG = "LogActivity"; 
	private SmartListAdapter adapter = null;
	private DTNService service = null;

	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			LogActivity.this.service = DTNService.Stub.asInterface(service);
			Log.i(TAG, "service connected");
			
			refreshView();
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
		
		getListView().setTranscriptMode(ListView.TRANSCRIPT_MODE_NORMAL);
		getListView().setStackFromBottom(true);

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
	
	private void refreshView() {
		if (LogActivity.this.service != null) {
			(new LoadDataTask()).execute();
		}
	}
	
	@Override
	protected void onPause() {
		this.unregisterReceiver(_receiver);
		super.onPause();
	}

	@Override
	protected void onResume() {
		super.onResume();
		IntentFilter filter = new IntentFilter(de.tubs.ibr.dtn.Intent.EVENT);
		filter.addCategory(Intent.CATEGORY_DEFAULT);
		this.registerReceiver(_receiver, filter);
		refreshView();
	}

	private BroadcastReceiver _receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.EVENT)) {
				refreshView();
			}
		}	
	};
    
	private class LoadDataTask extends AsyncTask<String, Integer, List<String>> {
		protected List<String> doInBackground(String... data)
		{
			try {
		        // query all logs
				List<String> logs = service.getLog();
		        
				return logs;
			} catch (RemoteException e) {
				return null;
			}
		}

		protected void onProgressUpdate(Integer... progress) {
		}

		protected void onPostExecute(List<String> logs)
		{
			if (logs == null) return;
			LogActivity.this.adapter.refresh(logs);
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
		
		public synchronized void refresh(List<String> logs) {
			// clear all data
			clear();
			
			for (String l : logs) {
				add(new LogMessage(l));
			}
			
	        // refresh data view
	        refresh();
		}
		
		public void refresh()
		{
			notifyDataSetChanged();
		}
	};
}
