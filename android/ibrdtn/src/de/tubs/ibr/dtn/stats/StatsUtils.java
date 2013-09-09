package de.tubs.ibr.dtn.stats;

import java.util.ArrayList;
import java.util.HashMap;

import android.content.Context;
import android.database.Cursor;
import android.text.format.DateUtils;
import android.text.format.Time;

import com.jjoe64.graphview.GraphView.GraphViewData;

import de.tubs.ibr.dtn.daemon.StatsListAdapter;
import de.tubs.ibr.dtn.daemon.StatsListAdapter.RowType;

public class StatsUtils {
    public static void generateTimestampLabels(Context context, Double min_timestamp, Double max_timestamp, ArrayList<String> labels) {
        if (max_timestamp > min_timestamp) {
            // adjust labels
            Double step_value = (max_timestamp - min_timestamp) / 6.0;
            
            for (Double i = min_timestamp; i < max_timestamp; i += step_value) {
                String date_text = StatsUtils.formatTimeStampString(context, Double.valueOf(i * 1000.0).longValue());
                labels.add(date_text);
            }
        }
        
        String date_text = StatsUtils.formatTimeStampString(context, Double.valueOf(max_timestamp * 1000.0).longValue());
        labels.add(date_text);
    }
    
    public static void convertData(Context context, Cursor stats, HashMap<String, ArrayList<GraphViewData>> series) {
        ConvergenceLayerStatsEntry.ColumnsMap cmap = new ConvergenceLayerStatsEntry.ColumnsMap();
        
        // move before the first position
        stats.moveToPosition(-1);
        
        HashMap<String, ConvergenceLayerStatsEntry> last_map = new HashMap<String, ConvergenceLayerStatsEntry>();

        while (stats.moveToNext()) {
            ArrayList<GraphViewData> series_data = null;
            
            // create an entry object
            ConvergenceLayerStatsEntry e = new ConvergenceLayerStatsEntry(context, stats, cmap);
            
            // generate a data key for this series
            String key = e.getConvergenceLayer() + "|" + e.getDataTag();
            
            if (last_map.containsKey(key)) {
                // add a new series is there is none
                if (!series.containsKey(key)) {
                    series_data = new ArrayList<GraphViewData>();
                    series.put(key, series_data);
                } else {
                    series_data = series.get(key);
                }

                ConvergenceLayerStatsEntry last_entry = last_map.get(key);
                
                Double timestamp = Double.valueOf(e.getTimestamp().getTime()) / 1000.0;
                
                Double last_timestamp = Double.valueOf(last_entry.getTimestamp().getTime()) / 1000.0;
                Double timestamp_diff = timestamp - last_timestamp;

                Long last_value = last_entry.getDataValue();

                GraphViewData p = null;

                if (e.getDataValue() < last_value) {
                    p = new GraphViewData(timestamp, 0);
                } else {
                    p = new GraphViewData(timestamp, (e.getDataValue() - last_value) / timestamp_diff);
                }

                series_data.add(p);
            }
            
            // store the last entry for the next round
            last_map.put( key, e );
        }
    }
    
    public static void convertData(Context context, Cursor stats, ArrayList<ArrayList<GraphViewData>> data, StatsListAdapter adapter) {
        int charts_count = adapter.getDataRows();
        
        // add one array for each data-set to display 
        for (int i = 0; i < charts_count; i++) {
            data.add( new ArrayList<GraphViewData>() );
        }
        
        StatsEntry.ColumnsMap cmap = new StatsEntry.ColumnsMap();

        // move before the first position
        stats.moveToPosition(-1);
        
        StatsEntry last_entry = null;

        while (stats.moveToNext()) {
            // create an entry object
            StatsEntry e = new StatsEntry(context, stats, cmap);
            
            if (last_entry != null) {
                Double timestamp = Double.valueOf(e.getTimestamp().getTime()) / 1000.0;
                
                for (int i = 0; i < charts_count; i++) {
                    int position = adapter.getDataMapPosition(i);
                    
                    Double value = StatsListAdapter.getRowDouble(position, e);
                    GraphViewData p = null;
                    
                    if (StatsListAdapter.getRowType(position).equals(RowType.RELATIVE)) {
                        Double last_timestamp = Double.valueOf(last_entry.getTimestamp().getTime()) / 1000.0;
                        Double timestamp_diff = timestamp - last_timestamp;
                        
                        Double last_value = StatsListAdapter.getRowDouble(position, last_entry);
                        
                        if (value < last_value) {
                            p = new GraphViewData(timestamp, 0);
                        } else {
                            p = new GraphViewData(timestamp, (value - last_value) / timestamp_diff);
                        }
                    } else {
                        p = new GraphViewData(timestamp, value);
                    }
                    
                    data.get(i).add(p);
                }
            }
            
            // store the last entry for the next round
            last_entry = e;
        }
    }
    
    public static String formatTimeStampString(Context context, long when) {
        return formatTimeStampString(context, when, false);
    }

    public static String formatTimeStampString(Context context, long when, boolean fullFormat) {
        Time then = new Time();
        then.set(when);
        Time now = new Time();
        now.setToNow();

        // Basic settings for formatDateTime() we want for all cases.
        int format_flags = DateUtils.FORMAT_ABBREV_ALL;

        // If the message is from a different year, show the date and year.
        if (then.year != now.year) {
            format_flags |= DateUtils.FORMAT_SHOW_YEAR | DateUtils.FORMAT_SHOW_DATE;
        } else if (then.yearDay != now.yearDay) {
            // If it is from a different day than today, show only the date.
            format_flags |= DateUtils.FORMAT_SHOW_DATE;
        } else {
            // Otherwise, if the message is from today, show the time.
            format_flags |= DateUtils.FORMAT_SHOW_TIME;
        }

        // If the caller has asked for full details, make sure to show the date
        // and time no matter what we've determined above (but still make showing
        // the year only happen if it is a different year from today).
        if (fullFormat) {
            format_flags |= (DateUtils.FORMAT_SHOW_DATE | DateUtils.FORMAT_SHOW_TIME);
        }

        return DateUtils.formatDateTime(context, when, format_flags);
    }
}
