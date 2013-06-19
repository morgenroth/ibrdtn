package de.tubs.ibr.dtn.daemon;

import java.util.ArrayList;

import android.content.Context;
import android.database.Cursor;
import android.os.Bundle;

import com.michaelpardo.chartview.widget.ChartView;
import com.michaelpardo.chartview.widget.LinearSeries;
import com.michaelpardo.chartview.widget.LinearSeries.LinearPoint;

import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.stats.StatsEntry;

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

    @Override
    protected void plot(Cursor stats, ChartView chart) {
        // create a new series list
        ArrayList<LinearSeries> series = new ArrayList<LinearSeries>(mChartMap.length);
        
        for (int i = 0; i < mChartMap.length; i++) {
            LinearSeries s = new LinearSeries();
            s.setLineColor(getResources().getColor(mChartColors[i]));
            s.setLineWidth(4);
            series.add(s);
        }
        
        StatsEntry.ColumnsMap cmap = new StatsEntry.ColumnsMap();
        
        int points = 0;
        
        // move before the first position
        stats.moveToPosition(-1);
        
        long last_timestamp = 0L;

        while (stats.moveToNext()) {
            StatsEntry e = new StatsEntry(getActivity(), stats, cmap);
            Long timestamp = e.getTimestamp().getTime() / 1000;
            
            for (int i = 0; i < mChartMap.length; i++) {
                Double value = StatsListAdapter.getRowDouble(mChartMap[i], e);
                LinearPoint p = new LinearPoint(Double.valueOf(timestamp), value);
                series.get(i).addPoint(p);
            }
            
            points++;
            last_timestamp = timestamp;
        }

        if (points > 0) {
            // Add chart view data
            for (LinearSeries s : series) {
                chart.addSeries(s);
            }
        }
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
