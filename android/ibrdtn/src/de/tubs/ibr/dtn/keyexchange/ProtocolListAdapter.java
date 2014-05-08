package de.tubs.ibr.dtn.keyexchange;

import java.util.LinkedList;
import java.util.List;

import de.tubs.ibr.dtn.R;
import android.content.Context;
import android.graphics.PorterDuff.Mode;
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
			convertView = this.mInflater.inflate(R.layout.protocol_list_item, null, true);
			holder = new ViewHolder();
			holder.imageIcon = (ImageView) convertView.findViewById(R.id.imageIcon);
			holder.textName = (TextView) convertView.findViewById(R.id.textName);
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}

		holder.protocol = mList.get(position);

		EnumProtocol p = holder.protocol.getProtocol();
		int protocol_color = R.color.light_red;
		
		if (p == EnumProtocol.QR || p == EnumProtocol.NFC) {
			protocol_color = R.color.green;
		}
		if (p == EnumProtocol.HASH || p == EnumProtocol.JPAKE) {
			protocol_color = R.color.light_yellow;
		}
		if (p == EnumProtocol.NONE || p == EnumProtocol.DH) {
			protocol_color = R.color.light_red;
		}

		if (holder.protocol.isUsed()) {
			holder.imageIcon.setImageResource(R.drawable.ic_action_security_closed);
			holder.imageIcon.setColorFilter(mContext.getResources().getColor(protocol_color), Mode.MULTIPLY);
		} else {
			holder.imageIcon.setImageResource(R.drawable.ic_action_security_open);
			holder.imageIcon.setColorFilter(null);
		}

		holder.textName.setText(holder.protocol.getProtocol().getTitle(mContext));
		holder.textName.setTextColor(mContext.getResources().getColor(protocol_color));

		return convertView;
	}

}
