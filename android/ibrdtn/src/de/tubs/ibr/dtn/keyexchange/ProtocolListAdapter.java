package de.tubs.ibr.dtn.keyexchange;

import java.util.LinkedList;
import java.util.List;

import de.tubs.ibr.dtn.R;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class ProtocolListAdapter extends BaseAdapter {

	private LayoutInflater mInflater = null;
    private List<ProtocolContainer> mList = new LinkedList<ProtocolContainer>();
    private Context mContext = null;
    
    private class ViewHolder {
        public ImageView imageIcon;
        public TextView textName;
        public ProtocolContainer protocol;
    }
    
    public ProtocolListAdapter(Context context) {
    	this.mContext = context;
        this.mInflater = LayoutInflater.from(context);
    }
    
    public void swapList(List<ProtocolContainer> data) {
    	mList = data;
    	notifyDataSetChanged();
    }

    public void add(ProtocolContainer p) {
        mList.add(p);
    }

    public void clear() {
        mList.clear();
    }

	public void remove(int position) {
        mList.remove(position);
    }
    
	@Override
	public int getCount() {
		return mList.size();
	}

	@Override
	public Object getItem(int position) {
		return mList.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		ViewHolder holder;

        if (convertView == null) {
            convertView = this.mInflater.inflate(R.layout.protocollist_item, null, true);
            holder = new ViewHolder();
            holder.imageIcon = (ImageView) convertView.findViewById(R.id.imageIcon);
            holder.textName = (TextView) convertView.findViewById(R.id.textName);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        holder.protocol = mList.get(position);

        if (holder.protocol.isUsed()) {
            holder.imageIcon.setBackgroundColor(mContext.getResources().getColor(R.color.green));
            holder.imageIcon.setImageResource(R.drawable.ic_action_security_closed);
        } else {
            holder.imageIcon.setBackgroundColor(mContext.getResources().getColor(R.color.gray));
            holder.imageIcon.setImageResource(R.drawable.ic_action_security_open);
        }

        holder.textName.setText(holder.protocol.getProtocol().getTitle(mContext));
        
        if(position == 0 || position == 1) {
        	holder.textName.setTextColor(mContext.getResources().getColor(R.color.light_green));
        }
        if(position == 2 || position == 3) {
        	holder.textName.setTextColor(mContext.getResources().getColor(R.color.light_yellow));
        }
        if(position == 4 || position == 5) {
        	holder.textName.setTextColor(mContext.getResources().getColor(R.color.light_red));
        }

        return convertView;
	}

}
