package de.tubs.ibr.dtn.daemon;

import android.content.Context;
import android.os.Bundle;
import de.tubs.ibr.dtn.R;

public class StorageChartFragment extends StatsChartFragment {
    
    private Integer[] mChartMap = { 14 };
    private int[] mChartColors = { R.color.blue };
    
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
        protected int getDataMapPosition(int position) {
            return mChartMap[position];
        }

        @Override
        protected int getDataRows() {
            return mChartMap.length;
        }

        @Override
        protected int getDataColor(int position) {
            return mChartColors[position];
        }
    };
}
