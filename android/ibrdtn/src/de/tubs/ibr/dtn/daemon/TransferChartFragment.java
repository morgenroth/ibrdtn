package de.tubs.ibr.dtn.daemon;

import android.content.Context;
import android.os.Bundle;
import de.tubs.ibr.dtn.R;

public class TransferChartFragment extends StatsChartFragment {
    
    private Integer[] mChartMap = { 13, 11, 6, 10 };
    private int[] mChartColors = { R.color.green, R.color.yellow, R.color.red, R.color.blue };
    
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
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
