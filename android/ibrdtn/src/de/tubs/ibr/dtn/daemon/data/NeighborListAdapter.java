package de.tubs.ibr.dtn.daemon.data;

import java.util.LinkedList;
import java.util.List;

import android.content.Context;
import android.graphics.PorterDuff.Mode;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.Node;

public class NeighborListAdapter extends BaseAdapter {
    private LayoutInflater mInflater = null;
    private List<Node> mList = new LinkedList<Node>();
    private Context mContext = null;

    private class ViewHolder {
        public ImageView imageIcon;
        public TextView textName;
        public Node node;
    }

    public NeighborListAdapter(Context context) {
    	this.mContext = context;
        this.mInflater = LayoutInflater.from(context);
    }
    
    public void swapList(List<Node> data) {
    	mList = data;
    	notifyDataSetChanged();
    }

    public int getCount() {
        return mList.size();
    }

    public void add(Node n) {
        mList.add(n);
    }

    public void clear() {
        mList.clear();
    }

	public void remove(int position) {
        mList.remove(position);
    }

    public Object getItem(int position) {
        return mList.get(position);
    }

    public long getItemId(int position) {
        return position;
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder;

        if (convertView == null) {
            convertView = this.mInflater.inflate(R.layout.neighborlist_item, null, true);
            holder = new ViewHolder();
            holder.imageIcon = (ImageView) convertView.findViewById(R.id.imageIcon);
            holder.textName = (TextView) convertView.findViewById(R.id.textName);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        holder.node = mList.get(position);
        
        int image_color = R.color.node_default;

        if (holder.node.type.equals("NODE_P2P")) {
            holder.imageIcon.setImageResource(R.drawable.ic_p2p);
            image_color = R.color.node_p2p;
        } else if (holder.node.type.equals("NODE_DISCOVERED")) {
            holder.imageIcon.setImageResource(R.drawable.ic_wifi);
            image_color = R.color.node_discovered;
        } else if (holder.node.type.equals("NODE_CONNECTED")) {
            holder.imageIcon.setImageResource(R.drawable.ic_node);
            image_color = R.color.node_connected;
        } else if (holder.node.type.equals("NODE_INTERNET")) {
            holder.imageIcon.setImageResource(R.drawable.ic_world);
            image_color = R.color.node_internet;
        } else {
            holder.imageIcon.setImageResource(R.drawable.ic_node);
            image_color = R.color.node_default;
        }
        
        holder.imageIcon.setColorFilter(mContext.getResources().getColor(image_color), Mode.SRC_IN);

        holder.textName.setText(holder.node.endpoint.toString());

        return convertView;
    }  
}
