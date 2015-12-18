package de.tubs.ibr.dtn.daemon;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Bundle;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.daemon.data.StatsListAdapter;
import de.tubs.ibr.dtn.stats.StatsUtils;

public class StorageChartFragment extends StatsChartFragment {
    
    private Integer[] mChartMap = { 12 };
    private int[] mChartColors = { R.color.stats_first };
    
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        
        // create a new data list adapter
        setStatsListAdapter(new DataListAdapter(getActivity()));
    }

    private class DataListAdapter extends StatsListAdapter {

        public DataListAdapter(Context context) {
            super(context);
        }

        @Override
        public int getDataMapPosition(int position) {
            return mChartMap[position];
        }

        @Override
        public int getDataRows() {
            return mChartMap.length;
        }

        @Override
        public int getDataColor(int position) {
            return mChartColors[position];
        }
    };
    
    @SuppressLint("DefaultLocale")
    @Override
    public String formatLabel(double value, boolean isValueX) {
        if (isValueX) return super.formatLabel(value, isValueX);
        return StatsUtils.formatByteString(Double.valueOf(value).intValue(), true);
    }
}
