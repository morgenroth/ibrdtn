/*
 * RosterView.java
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
package de.tubs.ibr.dtn.chat;

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Roster;

public class RosterView extends BaseAdapter {

	private final static String TAG = "RosterView";
	private LayoutInflater inflater = null;
	private List<Buddy> buddies = null;
	private List<Buddy> buddies_filtered = new LinkedList<Buddy>();
	private String selectedBuddy = null;
	
	private Boolean showOffline = true;

	public RosterView(Context context, Roster roster)
	{
		this.inflater = LayoutInflater.from(context);
		this.buddies = roster;
		
		refresh();
		
		IntentFilter i = new IntentFilter(Roster.REFRESH);
		context.registerReceiver(notify_receiver, i);
	}
	
	private void filterBuddies() {
		buddies_filtered.clear();
		
		for (Buddy b : buddies) {
			if (showOffline) {
				buddies_filtered.add(b);
			} else {
				if (b.isOnline() || b.getEndpoint().equals(selectedBuddy)) {
					buddies_filtered.add(b);
				}
			}
		}
	}
	
	public void setShowOffline(Boolean val) {
		if (this.showOffline != val) {
			this.showOffline = val;
			filterBuddies();
			this.notifyDataSetChanged();
		}
	}
	
	public class ViewHolder
	{
		public TextView text;
		public TextView bottomText;
		public Buddy buddy;
		public ImageView hinticon;
		public ImageView icon;
		public View layout;
	}
	
	private BroadcastReceiver notify_receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent i) {
			RosterView.this.refresh();
		}
	};

	protected void onDestroy(Context context) {
		context.unregisterReceiver(notify_receiver);
	}
	
	public int getCount() {
		return buddies_filtered.size();
	}

	public Object getItem(int position) {
		return buddies_filtered.get(position);
	}

	public long getItemId(int position) {
		return position;
	}
	
	public synchronized void setSelected(String buddyId) {
		this.selectedBuddy = buddyId;
		filterBuddies();
		this.notifyDataSetChanged();
	}
	
	public void refresh()
	{
		Log.d(TAG, "refresh requested...");
		Collections.sort(this.buddies);
		filterBuddies();
		this.notifyDataSetChanged();
	}
	
	public View getView(int position, View convertView, ViewGroup parent) {
		ViewHolder holder;
		
		if (convertView == null)
		{
			convertView = this.inflater.inflate(R.layout.roster_item, null, true);
			holder = new ViewHolder();
			holder.text = (TextView) convertView.findViewById(R.id.label);
			holder.bottomText = (TextView) convertView.findViewById(R.id.bottomtext);
			holder.icon = (ImageView) convertView.findViewById(R.id.icon);
			holder.hinticon = (ImageView) convertView.findViewById(R.id.hinticon);
			holder.layout = convertView.findViewById(R.id.roster_item_layout);
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		
		holder.buddy = this.buddies_filtered.get(position);
		holder.icon.setImageResource(R.drawable.online);
		holder.text.setText(holder.buddy.toString());
	
		String presence = holder.buddy.getPresence();
			
		if (presence != null)
		{
			if (presence.equalsIgnoreCase("unavailable"))
			{
				holder.icon.setImageResource(R.drawable.offline);
			}
			else if (presence.equalsIgnoreCase("xa"))
			{
				holder.icon.setImageResource(R.drawable.xa);
			}
			else if (presence.equalsIgnoreCase("away"))
			{
				holder.icon.setImageResource(R.drawable.away);
			}
			else if (presence.equalsIgnoreCase("dnd"))
			{
				holder.icon.setImageResource(R.drawable.busy);
			}
			else if (presence.equalsIgnoreCase("chat"))
			{
				holder.icon.setImageResource(R.drawable.online);
			}
		}
		
		// if the presence is older than 60 minutes then mark the buddy as offline
		if (!holder.buddy.isOnline())
		{
			holder.icon.setImageResource(R.drawable.offline);
		}
		
		if (holder.buddy.getStatus() != null)
		{
			if (holder.buddy.getStatus().length() > 0) { 
				holder.bottomText.setText(holder.buddy.getStatus());
			} else {
				holder.bottomText.setText(holder.buddy.getEndpoint());
			}
		}
		else
		{
			holder.bottomText.setText(holder.buddy.getEndpoint());
		}
		
		if (holder.buddy.hasDraft()) {
			holder.hinticon.setVisibility(View.VISIBLE);
			holder.hinticon.setImageResource(R.drawable.ic_draft);
		} else {
			holder.hinticon.setVisibility(View.GONE);
		}
		
		if (selectedBuddy != null) {
			if (selectedBuddy.equals(holder.buddy.getEndpoint())) {
				holder.layout.setBackgroundColor(convertView.getResources().getColor(R.color.chat_active));
				convertView.setActivated(true);
				holder.hinticon.setVisibility(View.VISIBLE);
				holder.hinticon.setImageResource(R.drawable.ic_selected);
			} else {
				convertView.setActivated(false);
				holder.layout.setBackgroundColor(convertView.getResources().getColor(android.R.color.transparent));
			}
		} else {
			holder.layout.setBackgroundColor(convertView.getResources().getColor(android.R.color.transparent));
		}
		
		return convertView;
	}
}
