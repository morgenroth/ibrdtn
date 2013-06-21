package de.tubs.ibr.dtn.daemon;

import android.annotation.SuppressLint;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.stats.StatsEntry;

public abstract class StatsListAdapter extends BaseAdapter {
    private LayoutInflater mInflater = null;
    private StatsEntry mData = null;
    private Context mContext = null;
    
    public class StatsData {
        public String title;
        public String value;
    }

    private class ViewHolder {
        public ImageView imageIcon;
        public TextView textName;
        public StatsData data;
    }

    public StatsListAdapter(Context context) {
        this.mContext = context;
        this.mInflater = LayoutInflater.from(context);
    }
    
    public void swapList(StatsEntry data) {
        mData = data;
        notifyDataSetChanged();
    }

    protected abstract int getDataMapPosition(int position);
    protected abstract int getDataRows();
    protected abstract int getDataColor(int position);

    public Object getItem(int position) {
        StatsData e = new StatsData();
        
        if (mData == null) return null;
        
        int mappos = getDataMapPosition(position);
        
        e.title = getRowTitle(mContext, mappos);
        e.value = getRowString(mappos, mData);
        
        return e;
    }
    
    @Override
    public int getCount() {
        if (mData == null) return 0;
        return getDataRows();
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

        holder.data = (StatsData)getItem(position);

        holder.imageIcon.setBackgroundColor(mContext.getResources().getColor(getDataColor(position)));
        holder.imageIcon.setImageResource(R.drawable.ic_log);

        holder.textName.setText(holder.data.title + ": " + holder.data.value);

        return convertView;
    }
    
    @SuppressLint("DefaultLocale")
    private static String humanReadableByteCount(long bytes, boolean si) {
        int unit = si ? 1000 : 1024;
        if (bytes < unit) return bytes + " B";
        int exp = (int) (Math.log(bytes) / Math.log(unit));
        String pre = (si ? "kMGTPE" : "KMGTPE").charAt(exp-1) + (si ? "" : "i");
        return String.format("%.1f %sB", bytes / Math.pow(unit, exp), pre);
    }
    
    @SuppressLint("DefaultLocale")
    public static String getRowString(int position, StatsEntry data) {
        Object value = getRowData(position, data);
        
        if (value == null) {
            return "-";
        }
        
        if (position == 0) {
            // special case "seconds"
            return String.format("%d s", (Long)value);
        }
        else if (position == 14) {
            // special case "bytes"
            return humanReadableByteCount((Long)value, true);
        }
        
        if (value instanceof String) {
            return (String)value;
        } else if (value instanceof Double) {
            return String.format("%f", (Double)value);
        } else {
            return value.toString();
        }
    }
    
    public static Double getRowDouble(int position, StatsEntry data) {
        Object value = getRowData(position, data);
        
        if (value == null) {
            return 0.0;
        }
        
        if (value instanceof Double) {
            return (Double)value;
        } else {
            return Double.valueOf(value.toString());
        }
    }
    
    public enum RowType {
        ABSOLUTE,
        RELATIVE
    };
    
    private final static RowType[] mRowTypes = {
        RowType.RELATIVE,
        RowType.ABSOLUTE,
        RowType.RELATIVE,
        
        RowType.ABSOLUTE,
        RowType.ABSOLUTE,
        RowType.RELATIVE,
        
        RowType.RELATIVE,
        RowType.RELATIVE,
        RowType.RELATIVE,
        RowType.RELATIVE,
        RowType.RELATIVE,
        RowType.RELATIVE,
        RowType.ABSOLUTE,
        RowType.RELATIVE,
        
        RowType.ABSOLUTE
    };
    
    private final static int[] mRowTitles = {
      R.string.stats_title_uptime,
      R.string.stats_title_neighbors,
      R.string.stats_title_timestamp,
      
      R.string.stats_title_clock_offset,
      R.string.stats_title_clock_rating,
      R.string.stats_title_clock_adjustment,
      
      R.string.stats_title_transfers_aborted,
      R.string.stats_title_bundles_expired,
      R.string.stats_title_bundles_generated,
      R.string.stats_title_bundles_queued,
      R.string.stats_title_bundles_received,
      R.string.stats_title_transfers_requeued,
      R.string.stats_title_bundles_stored,
      R.string.stats_title_transfers_completed,
      
      R.string.stats_title_storage_size
    };
    
    public static RowType getRowType(int position) {
        return mRowTypes[position];
    }
    
    public static String getRowTitle(Context context, int position) {
        return context.getResources().getString(mRowTitles[position]);
    }
    
    public static Object getRowData(int row, StatsEntry data) {
        switch (row) {
            case 0:
                return data.getUptime();
                
            case 1:
                return data.getNeighbors();

            case 2:
                return data.getTimestamp();

            case 3:
                return data.getClockOffset();

            case 4:
                return data.getClockRating();

            case 5:
                return data.getClockAdjustments();

            case 6:
                return data.getBundleAborted();

            case 7:
                return data.getBundleExpired();

            case 8:
                return data.getBundleGenerated();

            case 9:
                return data.getBundleQueued();

            case 10:
                return data.getBundleReceived();

            case 11:
                return data.getBundleRequeued();

            case 12:
                return data.getBundleStored();

            case 13:
                return data.getBundleTransmitted();
                
            case 14:
                return data.getStorageSize();
        }
        return null;
    }
}
