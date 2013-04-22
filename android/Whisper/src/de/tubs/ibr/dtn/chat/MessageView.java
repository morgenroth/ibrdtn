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

import java.util.LinkedList;
import java.util.List;

import android.content.Context;
import android.text.format.DateUtils;
import android.text.format.Time;
import android.util.Log;
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
	private Buddy buddy = null;
	private List<Message> messages = null;
	private Context context = null;

	public MessageView(Context context, Buddy buddy)
	{
		this.context = context;
		this.inflater = LayoutInflater.from(context);
		this.buddy = buddy;
		this.messages = new LinkedList<Message>();
	}
	
	public class ViewHolder
	{
		public TextView label;
		public TextView text;
		public ImageView pending;
		public ImageView delivered;
		public ImageView details;
		public Message msg;
	}
	
	public void refresh(Roster roster)
	{
		this.messages = roster.getMessages(this.buddy);
		
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
		
		Message msg = this.messages.get(position);
		
		// discard view if the layout does not match
		if (convertView != null) {
			holder = (ViewHolder) convertView.getTag();
			if (holder.msg.isIncoming() != msg.isIncoming()) {
				convertView = null;
				holder = null;
			}
		}
		
		if (convertView == null) {
			if (msg.isIncoming()) {
				convertView = this.inflater.inflate(R.layout.chat_item_recv, null, true);
			} else {
				convertView = this.inflater.inflate(R.layout.chat_item_send, null, true);
			}
			
			holder = new ViewHolder();
			holder.label = (TextView) convertView.findViewById(R.id.label);
			holder.text = (TextView) convertView.findViewById(R.id.text);
			holder.pending = (ImageView) convertView.findViewById(R.id.pending_indicator);
			holder.delivered = (ImageView) convertView.findViewById(R.id.delivered_indicator);
			holder.details = (ImageView) convertView.findViewById(R.id.details_indicator);
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		
		holder.msg = msg;
		
		String date = MessageView.formatTimeStampString(this.context, holder.msg.getCreated().getTime());
		
		//String date = DateFormat.getDateTimeInstance().format(holder.msg.getCreated());

		// set details invisible
		holder.details.setVisibility(View.GONE);
		
		if (!holder.msg.isIncoming())
		{
			if (holder.msg.getFlags() == 0) {
				holder.pending.setVisibility(View.VISIBLE);
				holder.delivered.setVisibility(View.GONE);
			} else if ((holder.msg.getFlags() & 2) > 0) {
				holder.pending.setVisibility(View.GONE);
				holder.delivered.setVisibility(View.VISIBLE);
			} else {
				holder.pending.setVisibility(View.GONE);
				holder.delivered.setVisibility(View.GONE);
			}
		}
		
		holder.label.setText(date);
		holder.text.setText(holder.msg.getPayload());
		return convertView;
	}
	
    public static String formatTimeStampString(Context context, long when) {
        return formatTimeStampString(context, when, false);
    }

    public static String formatTimeStampString(Context context, long when, boolean fullFormat) {
        Time then = new Time();
        then.set(when);
        Time now = new Time();
        now.setToNow();

        // Basic settings for formatDateTime() we want for all cases.
        int format_flags = DateUtils.FORMAT_ABBREV_ALL;

        // If the message is from a different year, show the date and year.
        if (then.year != now.year) {
            format_flags |= DateUtils.FORMAT_SHOW_YEAR | DateUtils.FORMAT_SHOW_DATE;
        } else if (then.yearDay != now.yearDay) {
            // If it is from a different day than today, show only the date.
            format_flags |= DateUtils.FORMAT_SHOW_DATE;
        } else {
            // Otherwise, if the message is from today, show the time.
            format_flags |= DateUtils.FORMAT_SHOW_TIME;
        }

        // If the caller has asked for full details, make sure to show the date
        // and time no matter what we've determined above (but still make showing
        // the year only happen if it is a different year from today).
        if (fullFormat) {
            format_flags |= (DateUtils.FORMAT_SHOW_DATE | DateUtils.FORMAT_SHOW_TIME);
        }

        return DateUtils.formatDateTime(context, when, format_flags);
    }
}
