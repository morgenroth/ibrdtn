package de.tubs.ibr.dtn.chat;

import java.util.Collections;
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
//	private Roster roster = null;
	private List<Buddy> buddies = null;

	public RosterView(Context context, Roster roster)
	{
		this.inflater = LayoutInflater.from(context);
//		this.roster = roster;
		this.buddies = roster;
		
		refresh();
		
		IntentFilter i = new IntentFilter(Roster.REFRESH);
		context.registerReceiver(notify_receiver, i);
	}
	
	public class ViewHolder
	{
		public TextView text;
		public TextView bottomText;
		public Buddy buddy;
		public ImageView icon;
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
		return buddies.size();
	}

	public Object getItem(int position) {
		return buddies.get(position);
	}

	public long getItemId(int position) {
		return position;
	}
	
	public void refresh()
	{
		Log.d(TAG, "refresh requested...");
		Collections.sort(this.buddies);
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
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		
		holder.buddy = this.buddies.get(position);
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
		
		return convertView;
	}
}
