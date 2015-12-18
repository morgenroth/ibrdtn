package de.tubs.ibr.dtn.daemon.data;

import java.util.LinkedList;
import java.util.List;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.daemon.LogMessage;

public class LogListAdapter extends BaseAdapter {
    private LayoutInflater inflater = null;
    private static final int MAX_LINES = 600;
    private List<LogMessage> mList = new LinkedList<LogMessage>();
    private Context context;

    private class ViewHolder {
        public ImageView imageMark;
        public TextView textDate;
        public TextView textTag;
        public TextView textLog;
        public LogMessage msg;
    }

    public LogListAdapter(Context context) {
        this.context = context;
        this.inflater = LayoutInflater.from(context);
    }

    public int getCount() {
        return mList.size();
    }

    public void add(LogMessage msg) {
        if (mList.size() > MAX_LINES) {
        	mList.remove(0);
        }
        mList.add(msg);
        notifyDataSetChanged();
    }

    public void remove(int position) {
        mList.remove(position);
    }
    
    public void clear() {
    	mList.clear();
    }

    public Object getItem(int position) {
        return mList.get(position);
    }

    public long getItemId(int position) {
        return position;
    }

    public synchronized View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder;

        if (convertView == null) {
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

        holder.msg = mList.get(position);

        if (holder.msg.level.equals("E")) {
            holder.imageMark.setImageLevel(1);
            holder.textTag.setTextColor(context.getResources().getColor(R.color.mark_1));
        } else if (holder.msg.level.equals("W")) {
            holder.imageMark.setImageLevel(3);
            holder.textTag.setTextColor(context.getResources().getColor(R.color.mark_3));
        } else if (holder.msg.level.equals("I")) {
            holder.imageMark.setImageLevel(4);
            holder.textTag.setTextColor(context.getResources().getColor(R.color.mark_4));
        } else if (holder.msg.level.equals("D")) {
            holder.imageMark.setImageLevel(5);
            holder.textTag.setTextColor(context.getResources().getColor(R.color.mark_5));
        } else {
            holder.imageMark.setImageLevel(0);
            holder.textTag.setTextColor(context.getResources().getColor(R.color.mark_0));
        }

        holder.textDate.setText(holder.msg.date);
        holder.textLog.setText(holder.msg.msg);

        //holder.textTag.setText(tag + " (" + holder.msg.tag + ")");
        holder.textTag.setText(holder.msg.tag);

        return convertView;
    }
}
