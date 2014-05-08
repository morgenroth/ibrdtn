package de.tubs.ibr.dtn.keyexchange;

import java.util.LinkedList;
import java.util.List;

import de.tubs.ibr.dtn.R;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

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
        
        holder.imageIcon.setImageDrawable(new BitmapDrawable(null, createIdenticon(holder)));

        holder.textCommit.setText(createCommit(holder));

        return convertView;
	}
	
	private Bitmap createIdenticon(ViewHolder holder) {
		byte[] hash = holder.hashValue;

		// Übernommen von : http://www.davidhampgonsalves.com/Identicons
		Bitmap identicon = Bitmap.createBitmap(5, 5, Bitmap.Config.ARGB_8888);
	    
		float[] hsv = {Integer.valueOf(hash[3]+128).floatValue()*360.0f/255.0f, 0.8f, 0.8f};
	    int foreground = Color.HSVToColor(hsv);
	    int background = Color.parseColor("#f0f0f0");

	    for(int x=0 ; x < 5 ; x++) {
	        //Enforce horizontal symmetry
	        int i = x < 3 ? x : 4 - x;
	        for(int y=0 ; y < 5; y++) {
	        	int pixelColor;
	        	//toggle pixels based on bit being on/off
	        	if((hash[i] >> y & 1) == 1)
	                pixelColor = foreground;
	        	else
	                pixelColor = background;
	        	identicon.setPixel(x, y, pixelColor);
	        }
	    }
	    
		Bitmap bmpWithBorder = Bitmap.createBitmap(48, 48, identicon.getConfig());
		Canvas canvas = new Canvas(bmpWithBorder);
		canvas.drawColor(background);
		identicon = Bitmap.createScaledBitmap(identicon, 46, 46, false);
		canvas.drawBitmap(identicon, 1, 1, null);

	    return bmpWithBorder;
	}
	
	private String createCommit(ViewHolder holder) {
		String commit = "";
		
		commit += Utils.PGP_WORD_LIST[holder.hashValue[0]+128][0] + ", ";
		commit += Utils.PGP_WORD_LIST[holder.hashValue[1]+128][1] + ", ";
		commit += Utils.PGP_WORD_LIST[holder.hashValue[2]+128][0];
		
		return commit;
	}
}
