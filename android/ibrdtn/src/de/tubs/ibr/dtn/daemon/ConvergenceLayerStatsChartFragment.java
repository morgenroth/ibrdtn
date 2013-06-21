package de.tubs.ibr.dtn.daemon;

import java.util.HashMap;
import java.util.List;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.Fragment;
import android.support.v4.app.LoaderManager;
import android.support.v4.content.Loader;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

import com.michaelpardo.chartview.widget.ChartView;
import com.michaelpardo.chartview.widget.LinearSeries;
import com.michaelpardo.chartview.widget.LinearSeries.LinearPoint;

import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.service.DaemonService;
import de.tubs.ibr.dtn.service.DaemonService.LocalDTNService;
import de.tubs.ibr.dtn.stats.ConvergenceLayerStatsEntry;
import de.tubs.ibr.dtn.stats.ConvergenceLayerStatsLoader;

public class ConvergenceLayerStatsChartFragment extends Fragment {
    
    // The loader's unique id. Loader ids are specific to the Activity or
    // Fragment in which they reside.
    private static final int DB_LOADER_ID = 1;
    private static final int STATS_LOADER_ID = 2;

    private static final String TAG = "ClStatsChartFragment";
    
    private Object mServiceLock = new Object();
    private DaemonService mService = null;
    
    private ChartView mChartView = null;
    private ListView mListView = null;
    
    private ConvergenceLayerStatsListAdapter mAdapter = null;
    
    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder s) {
            synchronized(mServiceLock) {
                mService = ((LocalDTNService)s).getLocal();
            }
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service connected");

            // initialize the loader
            getLoaderManager().initLoader(DB_LOADER_ID, null, mDbLoader);
            getLoaderManager().initLoader(STATS_LOADER_ID, null, mStatsLoader);
        }

        public void onServiceDisconnected(ComponentName name) {
            getLoaderManager().destroyLoader(DB_LOADER_ID);
            getLoaderManager().destroyLoader(STATS_LOADER_ID);

            synchronized(mServiceLock) {
                mService = null;
            }
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service disconnected");
        }
    };
    
    private LoaderManager.LoaderCallbacks<Cursor> mDbLoader = new LoaderManager.LoaderCallbacks<Cursor>() {
        @Override
        public Loader<Cursor> onCreateLoader(int id, Bundle args) {
            return new ConvergenceLayerStatsLoader(getActivity(), mService, null);
        }

        @Override
        public void onLoadFinished(Loader<Cursor> loader, Cursor stats) {
            // clear the series
            mChartView.clearSeries();
            
            // plot the charts
            plotChart(stats, mChartView);
        }

        @Override
        public void onLoaderReset(Loader<Cursor> loader) {
        }
    };
    
    private void plotChart(Cursor stats, ChartView chart) {
        
        HashMap<String, LinearSeries> series = new HashMap<String, LinearSeries>();

        ConvergenceLayerStatsEntry.ColumnsMap cmap = new ConvergenceLayerStatsEntry.ColumnsMap();
        
        // move before the first position
        stats.moveToPosition(-1);
        
        HashMap<String, ConvergenceLayerStatsEntry> last_map = new HashMap<String, ConvergenceLayerStatsEntry>();

        while (stats.moveToNext()) {
            // create an entry object
            ConvergenceLayerStatsEntry e = new ConvergenceLayerStatsEntry(getActivity(), stats, cmap);
            
            // generate a data key for this series
            String key = e.getConvergenceLayer() + "|" + e.getDataTag();
            
            // add a new series is there is none
            if (!series.containsKey(key)) {
                LinearSeries s = new LinearSeries();
                s.setLineColor(getResources().getColor(mColorProvider.getColor(key)));
                s.setLineWidth(4);
                series.put(key, s);
            }
            
            if (last_map.containsKey(key)) {
                ConvergenceLayerStatsEntry last_entry = last_map.get(key);
                
                Double timestamp = Double.valueOf(e.getTimestamp().getTime()) / 1000.0;
                
                Double last_timestamp = Double.valueOf(last_entry.getTimestamp().getTime()) / 1000.0;
                Double timestamp_diff = timestamp - last_timestamp;

                Double last_value = last_entry.getDataValue();

                LinearPoint p = null;

                if (e.getDataValue() < last_value) {
                    p = new LinearPoint(timestamp, 0);
                } else {
                    p = new LinearPoint(timestamp, (e.getDataValue() - last_value) / timestamp_diff);
                }

                series.get(key).addPoint(p);
            }
            
            // store the last entry for the next round
            last_map.put( key, e );
        }

        // Add chart view data
        for (LinearSeries s : series.values()) {
            chart.addSeries(s);
        }
    }
    
    private LoaderManager.LoaderCallbacks<List<ConvergenceLayerStatsEntry>> mStatsLoader = new LoaderManager.LoaderCallbacks<List<ConvergenceLayerStatsEntry>>() {
        @Override
        public Loader<List<ConvergenceLayerStatsEntry>> onCreateLoader(int id, Bundle args) {
            return new CurrentConvergenceLayerStatsLoader(getActivity(), mService, null);
        }

        @Override
        public void onLoadFinished(Loader<List<ConvergenceLayerStatsEntry>> loader, List<ConvergenceLayerStatsEntry> stats) {
            synchronized (mAdapter) {
                mAdapter.swapList(stats);
            }
        }

        @Override
        public void onLoaderReset(Loader<List<ConvergenceLayerStatsEntry>> loader) {
        }
    };

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.stats_chart_fragment, container, false);
        
        // Find the chart and list view
        mChartView = (ChartView) v.findViewById(R.id.chart_view);
        mListView = (ListView) v.findViewById(R.id.list_view);
        
        return v;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        // add label adapter
        mChartView.setLabelAdapter(new StatsLabelAdapter(getActivity(), StatsLabelAdapter.Orientation.VERTICAL), ChartView.POSITION_LEFT);
        mChartView.setLabelAdapter(new StatsLabelAdapter(getActivity(), StatsLabelAdapter.Orientation.HORIZONTAL), ChartView.POSITION_BOTTOM);
        
        // create a new data list adapter
        mAdapter = new ConvergenceLayerStatsListAdapter(getActivity(), mColorProvider);
        mListView.setAdapter(mAdapter);
    }

    @Override
    public void onPause() {
        // Detach our existing connection.
        getActivity().unbindService(mConnection);
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();

        // Establish a connection with the service. We use an explicit
        // class name because we want a specific service implementation that
        // we know will be running in our own process (and thus won't be
        // supporting component replacement by other applications).
        getActivity().bindService(new Intent(DTNService.class.getName()), mConnection,
                Context.BIND_AUTO_CREATE);
    }
    
    private ConvergenceLayerStatsListAdapter.ColorProvider mColorProvider = new ConvergenceLayerStatsListAdapter.ColorProvider() {

        final int[] mColors = {
                R.color.blue,
                R.color.yellow,
                R.color.green,
                R.color.red,
                R.color.violett,
                R.color.gray
        };
        
        int assignedColors = 0;
        HashMap<String, Integer> mColorMap = new HashMap<String, Integer>();
        
        @Override
        public int getColor(String tag) {
            if (mColorMap.containsKey(tag))
                return mColorMap.get(tag);
            
            if (assignedColors >= mColors.length)
                return R.color.gray;
            
            int color = mColors[assignedColors];
            mColorMap.put(tag, color);
            assignedColors++;
            
            return color;
        }
        
    };
}
