package de.tubs.ibr.dtn.daemon;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.swig.NativeStats;

public class StatsListAdapter extends BaseAdapter {
    private LayoutInflater mInflater = null;
    private NativeStats mData = null;
    private Context mContext = null;
    
    public class StatsEntry {
        public String title;
        public String value;
    }

    private class ViewHolder {
        public ImageView imageIcon;
        public TextView textName;
        public StatsEntry data;
    }

    public StatsListAdapter(Context context) {
        this.mContext = context;
        this.mInflater = LayoutInflater.from(context);
    }
    
    public void swapList(NativeStats data) {
        mData = data;
        notifyDataSetChanged();
    }

    public int getCount() {
        return 14;
    }

    public Object getItem(int position) {
        StatsEntry e = new StatsEntry();
        
        switch (position) {
            case 0:
                // TODO: add localized string
                e.title = "Uptime";
                e.value = String.format("%d s", mData.getUptime());
                break;
                
            case 1:
                // TODO: add localized string
                e.title = "Neighbors";
                e.value = Long.valueOf(mData.getNeighbors()).toString();
                break;
                
            case 2:
                // TODO: add localized string
                e.title = "Bundles aborted";
                e.value = Long.valueOf(mData.getBundles_aborted()).toString();
                break;
                
            case 3:
                // TODO: add localized string
                e.title = "Bundles expired";
                e.value = Long.valueOf(mData.getBundles_expired()).toString();
                break;
                
            case 4:
                // TODO: add localized string
                e.title = "Bundles generated";
                e.value = Long.valueOf(mData.getBundles_generated()).toString();
                break;
                
            case 5:
                // TODO: add localized string
                e.title = "Bundles queued";
                e.value = Long.valueOf(mData.getBundles_queued()).toString();
                break;

            case 6:
                // TODO: add localized string
                e.title = "Bundles received";
                e.value = Long.valueOf(mData.getBundles_received()).toString();
                break;
                
            case 7:
                // TODO: add localized string
                e.title = "Bundles requeued";
                e.value = Long.valueOf(mData.getBundles_requeued()).toString();
                break;
                
            case 8:
                // TODO: add localized string
                e.title = "Bundles stored";
                e.value = Long.valueOf(mData.getBundles_stored()).toString();
                break;
                
            case 9:
                // TODO: add localized string
                e.title = "Bundles transmitted";
                e.value = Long.valueOf(mData.getBundles_transmitted()).toString();
                break;
                
            case 10:
                // TODO: add localized string
                e.title = "Timestamp";
                e.value = String.format("%d", mData.getTimestamp());
                break;
                
            case 11:
                // TODO: add localized string
                e.title = "Clock offset";
                e.value = String.format("%f s", mData.getTime_offset());
                break;
                
            case 12:
                // TODO: add localized string
                e.title = "Clock rating";
                e.value = String.format("%f", mData.getTime_rating());
                break;
                
            case 13:
                // TODO: add localized string
                e.title = "Clock adjustments";
                e.value = String.format("%d", mData.getTime_adjustments());
                break;
                
            default:
                e.title = "";
                e.value = "";
                break;
        }
        
        return e;
    }

    public long getItemId(int position) {
        return position;
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder;

        if (convertView == null) {
            convertView = this.mInflater.inflate(R.layout.stats_item, null, true);
            holder = new ViewHolder();
            holder.imageIcon = (ImageView) convertView.findViewById(R.id.imageIcon);
            holder.textName = (TextView) convertView.findViewById(R.id.textName);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        holder.data = (StatsEntry)getItem(position);

        holder.imageIcon.setBackgroundColor(mContext.getResources().getColor(R.color.dark_yellow));
        holder.imageIcon.setImageResource(R.drawable.ic_log);

        holder.textName.setText(holder.data.title + ": " + holder.data.value);

        return convertView;
    }  
}
