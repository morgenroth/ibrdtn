package de.tubs.ibr.dtn.chat.core;

import java.text.DateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.List;

import de.tubs.ibr.dtn.chat.core.Roster.RefreshCallback;

import android.content.Context;
import de.tubs.ibr.dtn.chat.R;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.TextView;

public class Buddy implements Comparable<Buddy> {

	private String nickname = null;
	private String endpoint = null;
	private Date lastseen = null;
	private String presence = null;
	private String status = null;

	private Roster roster = null;
	private List<Message> messages = null;
	private SmartListAdapter listadapter = null;
	private RefreshCallback refreshcallback = null;
	
	public Buddy(Roster roster, String nickname, String endpoint, String presence, String status)
	{
		this.nickname = nickname;
		this.endpoint = endpoint;
		this.roster = roster;
		this.lastseen = null;
		this.presence = presence;
		this.status = status;
	}

	public Date getLastSeen() {
		return lastseen;
	}

	public void setLastSeen(Date lastseen) {
		this.lastseen = lastseen;
	}
	
	public Boolean isOnline()
	{
		Calendar cal = Calendar.getInstance();
		cal.add(Calendar.HOUR, -1);
		
		if (getLastSeen() == null)
		{
			return false;
		}
		else  if ( cal.getTime().after(getLastSeen()) )
		{
			return false;
		}
		
		return true;
	}
	
	public Date getExpiration()
	{
		Calendar cal = Calendar.getInstance();
		cal.add(Calendar.HOUR, 1);
		cal.add(Calendar.SECOND, 30);
		return cal.getTime();
	}
	
	public String getPresence() {
		return presence;
	}

	public void setPresence(String presence) {
		this.presence = presence;
		update();
	}

	public String getStatus() {
		return status;
	}

	public void setStatus(String status) {
		this.status = status;
		update();
	}

	public String getEndpoint() {
		return endpoint;
	}

	public void setEndpoint(String endpoint) {
		this.endpoint = endpoint;
	}

	public String getNickname() {
		return nickname;
	}

	public void setNickname(String nickname) {
		this.nickname = nickname;
		update();
	}
	
	private void update() {
		if (listadapter != null) {
			listadapter.refresh();
		}
	}

	@Override
	public int compareTo(Buddy another) {
		if (isOnline() == another.isOnline())
			return this.nickname.compareToIgnoreCase(another.nickname);
		
		if (isOnline()) return -1;
		
		return 1;
	}
	
	public class ViewHolder
	{
		public TextView label;
		public TextView text;
		public Message msg;
	}
	
	public ListAdapter getListAdapter(Context context)
	{
		if (messages == null)
		{
			messages = roster.getMessages(this);
			listadapter = new SmartListAdapter(context, messages);
		}
		
		return listadapter;
	}
	
	public void setRefreshCallback(RefreshCallback cb) {
		this.refreshcallback = cb;
	}
	
	public void addMessage(Message msg)
	{
		messages.add(msg);
		update();
	}
	
	public void clearMessages()
	{
		messages.clear();
		update();
	}
	
	private class SmartListAdapter extends BaseAdapter {
		
		private LayoutInflater inflater = null;
		private List<Message> list = null;
		
		public SmartListAdapter(Context context, List<Message> list)
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
				convertView = this.inflater.inflate(R.layout.chat_item, null, true);
				holder = new ViewHolder();
				holder.label = (TextView) convertView.findViewById(R.id.label);
				holder.text = (TextView) convertView.findViewById(R.id.text);
				convertView.setTag(holder);
			} else {
				holder = (ViewHolder) convertView.getTag();
			}
			
			holder.msg = list.get(position);
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
		
		public void refresh()
		{
			if (refreshcallback != null)
			{
				refreshcallback.refreshCallback();
			}
		}
	};
}
