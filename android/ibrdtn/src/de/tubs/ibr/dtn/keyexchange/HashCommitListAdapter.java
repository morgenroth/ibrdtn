package de.tubs.ibr.dtn.keyexchange;

import java.util.LinkedList;
import java.util.List;

import android.content.Context;
import android.graphics.drawable.BitmapDrawable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import de.tubs.ibr.dtn.R;

public class HashCommitListAdapter extends BaseAdapter {
	private LayoutInflater mInflater = null;
    private List<byte[]> mList = new LinkedList<byte[]>();
    
    private class ViewHolder {
        public ImageView imageIcon;
        public TextView textCommit;
        public byte[] hashValue;
    }
    
    public HashCommitListAdapter(Context context) {
        this.mInflater = LayoutInflater.from(context);
    }
    
    public void add(byte[] p) {
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
            convertView = this.mInflater.inflate(R.layout.hash_commit_list_item, null, true);
            holder = new ViewHolder();
            holder.imageIcon = (ImageView) convertView.findViewById(R.id.imageIcon);
            holder.textCommit = (TextView) convertView.findViewById(R.id.textName);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        holder.hashValue = mList.get(position);
        
        holder.imageIcon.setImageDrawable(new BitmapDrawable(null, Utils.createIdenticon(holder.hashValue)));

        holder.textCommit.setText(createCommit(holder));

        return convertView;
	}
	
	private String createCommit(ViewHolder holder) {
		String commit = "";
		
		commit += Utils.PGP_WORD_LIST[holder.hashValue[0]+128][0] + ", ";
		commit += Utils.PGP_WORD_LIST[holder.hashValue[1]+128][1] + ", ";
		commit += Utils.PGP_WORD_LIST[holder.hashValue[2]+128][0];
		
		return commit;
	}
}
