/*
 * MessageView.java
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

import java.text.DateFormat;
import java.util.LinkedList;
import java.util.List;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Message;
import de.tubs.ibr.dtn.chat.core.Roster;

public class MessageView extends BaseAdapter {

	private final static String TAG = "MessageView";
	private LayoutInflater inflater = null;
	private Roster roster = null;
	private Buddy buddy = null;
	private List<Message> messages = null;

	public MessageView(Context context, Roster roster, Buddy buddy)
	{
		this.inflater = LayoutInflater.from(context);
		this.roster = roster;
		this.buddy = buddy;
		
		if (this.roster != null) {
			this.messages = this.roster.getMessages(this.buddy);
		} else {
			this.messages = new LinkedList<Message>();
		}
		
		IntentFilter i = new IntentFilter(Roster.REFRESH);
		context.registerReceiver(notify_receiver, i);
	}
	
	public class ViewHolder
	{
		public TextView label;
		public TextView text;
		public Message msg;
	}
	
	private BroadcastReceiver notify_receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent i) {
			if (buddy == null) return;
			if (i.getStringExtra("buddy").equals(buddy.getEndpoint())) {
				MessageView.this.refresh();
			}
		}
	};

	protected void onDestroy(Context context) {
		context.unregisterReceiver(notify_receiver);
	}
	
	public void refresh()
	{
		if (this.roster != null) {
			this.messages = this.roster.getMessages(this.buddy);
		}
		
		Log.d(TAG, "refresh requested...");
		this.notifyDataSetChanged();
	}

	public int getCount() {
		return this.messages.size();
	}

	public Object getItem(int position) {
		return this.messages.get(position);
	}

	public long getItemId(int position) {
		return position;
	}
	
	public View getView(int position, View convertView, ViewGroup parent) {
		ViewHolder holder;
		
		if (convertView == null)
		{
			convertView = this.inflater.inflate(R.layout.chat_item, null, true);
			holder = new ViewHolder();
			holder.label = (TextView) convertView.findViewById(R.id.label);
			holder.text = (TextView) convertView.findViewById(R.id.text);
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		
		holder.msg = this.messages.get(position);
		String date = DateFormat.getDateTimeInstance().format(holder.msg.getCreated());
		
		ImageView imageAvatarRight = (ImageView) convertView.findViewById(R.id.imageAvatarRight);
		ImageView imageAvatarLeft = (ImageView) convertView.findViewById(R.id.imageAvatar);

		if (holder.msg.isIncoming())
		{
			holder.label.setGravity(Gravity.LEFT);
			holder.text.setGravity(Gravity.LEFT);
			imageAvatarRight.setVisibility(View.GONE);
			imageAvatarLeft.setVisibility(View.VISIBLE);
		}
		else
		{
			holder.label.setGravity(Gravity.RIGHT);
			holder.text.setGravity(Gravity.RIGHT);
			imageAvatarRight.setVisibility(View.VISIBLE);
			imageAvatarLeft.setVisibility(View.GONE);
		}
		
		holder.label.setText(date);
		
		holder.text.setText(holder.msg.getPayload());
		return convertView;
	}	
}
