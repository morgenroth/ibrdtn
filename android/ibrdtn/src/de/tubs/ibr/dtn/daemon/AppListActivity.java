/*
 * AppListActivity.java
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

import java.util.List;

import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import de.tubs.ibr.dtn.R;

public class AppListActivity extends ListActivity {
	
	private static final String TAG = "AppListActivity";

	private SmartListAdapter smartAdapter = null;
	
    @Override
	protected void onPause() {
		super.onPause();
	}

	@Override
	protected void onResume() {
		(new LoadAppList()).execute();
		super.onResume();
	}
	
	public class ViewHolder
	{
		public TextView text;
		public TextView bottomText;
		public ImageView icon;
		public ResolveInfo info;
	}
	
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		super.onListItemClick(l, v, position, id);
		
		ViewHolder holder = (ViewHolder)v.getTag();
		
		Intent appStartIntent = getPackageManager().getLaunchIntentForPackage(holder.info.activityInfo.packageName);
		if (null != appStartIntent)
		{
		    startActivity(appStartIntent);
		}
	}
	
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.apps_main);
		
		final Intent marketIntent = new Intent(Intent.ACTION_VIEW);
		marketIntent.setData(Uri.parse("market://details?id=de.tubs.ibr.dtn"));
		
		final List<ResolveInfo> marketlist = getPackageManager().queryIntentActivities(marketIntent, 0);
		if (!marketlist.isEmpty())
		{
			ResolveInfo info = marketlist.get(0);
			
			ImageButton marketButton = (ImageButton)findViewById(R.id.iconTitleBar);
			marketButton.setVisibility(View.VISIBLE);
			marketButton.setImageDrawable( info.activityInfo.loadIcon(getPackageManager()) );
			
			marketButton.setOnClickListener(new OnClickListener() {
				public void onClick(View v) {
					final Intent marketIntent = new Intent(Intent.ACTION_VIEW);
					//marketIntent.setData(Uri.parse("market://search?q=de.tubs.ibr.dtn.*"));
					marketIntent.setData(Uri.parse("market://search?q=IBR-DTN+services&c=apps"));
					startActivity(marketIntent);
				}
			});
		}
	}
	
	private class SmartListAdapter extends BaseAdapter {
		
		private LayoutInflater inflater = null;
		private List<ResolveInfo> list = null;

		public SmartListAdapter(Context context, List<ResolveInfo> list)
		{
			this.inflater = LayoutInflater.from(context);
			this.list = list;
		}
		
		public int getCount() {
			return list.size();
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
				convertView = this.inflater.inflate(R.layout.app_item, null, true);
				holder = new ViewHolder();
				holder.text = (TextView) convertView.findViewById(R.id.label);
				holder.bottomText = (TextView) convertView.findViewById(R.id.bottomtext);
				holder.icon = (ImageView) convertView.findViewById(R.id.icon);
				convertView.setTag(holder);
			} else {
				holder = (ViewHolder) convertView.getTag();
			}
			
			holder.info = list.get(position);
			holder.text.setText( holder.info.activityInfo.loadLabel(getPackageManager()) );
			holder.icon.setImageDrawable( holder.info.activityInfo.loadIcon(getPackageManager()) );
			holder.bottomText.setText(holder.info.activityInfo.packageName);
			
//			SharedPreferences prefs = getSharedPreferences("registrations", Context.MODE_PRIVATE);
//			if (prefs.contains(holder.info.activityInfo.packageName)) {
//				holder.bottomText.setTextColor(getResources().getColor(R.color.light_green));
//			} else {
//				holder.bottomText.setTextColor(getResources().getColor(R.color.gray));
//			}
			
			return convertView;
		}
	};
    
	private class LoadAppList extends AsyncTask<String, Integer, List<ResolveInfo>> {
		protected List<ResolveInfo> doInBackground(String... data)
		{
			final Intent mainIntent = new Intent();
			mainIntent.setAction(de.tubs.ibr.dtn.Intent.DTNAPP);
			mainIntent.addCategory(Intent.CATEGORY_DEFAULT);
			
			final List<ResolveInfo> applist = getPackageManager().queryIntentActivities(mainIntent, 0);
			
			for (ResolveInfo info : applist)
			{
				Log.i(TAG, info.activityInfo.loadLabel(getPackageManager()) + ": " + info.toString());
			}
			
			return applist;
		}

		protected void onProgressUpdate(Integer... progress) {
		}

		protected void onPostExecute(List<ResolveInfo> result)
		{
			smartAdapter = new SmartListAdapter(AppListActivity.this, result);
			
	        // Bind to our new adapter.
			AppListActivity.this.setListAdapter(smartAdapter);
		}
	}
}
