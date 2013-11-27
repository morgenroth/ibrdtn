package de.tubs.ibr.dtn.daemon;

import java.util.ArrayList;

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
import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.ListView;

import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.GraphView.GraphViewData;
import com.jjoe64.graphview.GraphViewSeries;
import com.jjoe64.graphview.GraphViewSeries.GraphViewSeriesStyle;
import com.jjoe64.graphview.LineGraphView;

import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.service.DaemonService;
import de.tubs.ibr.dtn.service.DaemonService.LocalDTNService;
import de.tubs.ibr.dtn.stats.StatsEntry;
import de.tubs.ibr.dtn.stats.StatsLoader;
import de.tubs.ibr.dtn.stats.StatsUtils;

public abstract class StatsChartFragment extends Fragment {
    
    // The loader's unique id. Loader ids are specific to the Activity or
    // Fragment in which they reside.
    private static final int DB_LOADER_ID = 1;
    private static final int STATS_LOADER_ID = 2;

    private static final String TAG = "StatsChartFragment";
    
    private Object mServiceLock = new Object();
    private DaemonService mService = null;
    
    private LinearLayout mChartView = null;
    private LineGraphView mGraphView = null;
    private ListView mListView = null;
    
    private SparseArray<GraphViewSeries> mData = null;
    
    private StatsListAdapter mAdapter = null;
    
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
            return new StatsLoader(getActivity(), mService);
        }

        @Override
        public void onLoadFinished(Loader<Cursor> loader, Cursor stats) {
            // plot the charts
            plotChart(stats, mGraphView);
        }

        @Override
        public void onLoaderReset(Loader<Cursor> loader) {
        }
    };
    
    private void plotChart(Cursor stats, GraphView chart) {
        int charts_count = mAdapter.getDataRows();
        
        ArrayList<ArrayList<GraphViewData>> data = new ArrayList<ArrayList<GraphViewData>>(charts_count);
        
        // convert data into an structured array
        StatsUtils.convertData(getActivity(), stats, data, mAdapter);
        
        // get min/max timestamp of data-set
        Double min_timestamp = 0.0;
        Double max_timestamp = 0.0;
        
        for (ArrayList<GraphViewData> dataset : data) {
            for (GraphViewData d : dataset) {
                if ((min_timestamp == 0) || (min_timestamp > d.valueX)) min_timestamp = d.valueX;
                if (max_timestamp < d.valueX) max_timestamp = d.valueX;
            }
        }
        
        // generate timestamp labels
        ArrayList<String> labels = new ArrayList<String>();
        StatsUtils.generateTimestampLabels(getActivity(), min_timestamp, max_timestamp, labels);
        
        // add one series for each data-set
        for (int i = 0; i < charts_count; i++) {
            ArrayList<GraphViewData> dataset = data.get(i);
            
            GraphViewSeries gs = mData.get(i);
            
            if (gs == null) {
                GraphViewSeriesStyle style = new GraphViewSeriesStyle(getResources().getColor(mAdapter.getDataColor(i)), 5);
                String text = StatsListAdapter.getRowTitle(getActivity(), i);
                gs = new GraphViewSeries(text, style, dataset.toArray(new GraphViewData[dataset.size()]));
                mGraphView.addSeries(gs);
                mData.setValueAt(i, gs);
                
                // redraw the graph
                mGraphView.redrawAll();
            } else {
                gs.resetData(dataset.toArray(new GraphViewData[dataset.size()]));
            }
        }
        
        // change the horizontal labels
        mGraphView.setHorizontalLabels(labels.toArray(new String[labels.size()]));
    }
    
    private LoaderManager.LoaderCallbacks<StatsEntry> mStatsLoader = new LoaderManager.LoaderCallbacks<StatsEntry>() {
        @Override
        public Loader<StatsEntry> onCreateLoader(int id, Bundle args) {
            return new CurrentStatsLoader(getActivity(), mService);
        }

        @Override
        public void onLoadFinished(Loader<StatsEntry> loader, StatsEntry stats) {
            synchronized (mAdapter) {
                mAdapter.swapList(stats);
            }
        }

        @Override
        public void onLoaderReset(Loader<StatsEntry> loader) {
        }
    };

    @Override
    public void onDestroyView() {
        mGraphView = null;
        mData = null;
        super.onDestroyView();
    }
    
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.stats_chart_fragment, container, false);
        
        // create a new holder for plot data
        mData = new SparseArray<GraphViewSeries>();
        
        // Find the chart and list view
        mListView = (ListView) v.findViewById(R.id.list_view);
        mChartView = (LinearLayout) v.findViewById(R.id.chart_view);
        
        // create a new LineGraphView
        mGraphView = new LineGraphView(getActivity(), "");
        mChartView.addView(mGraphView);
        
        return v;
    }
    
    protected void setStatsListAdapter(StatsListAdapter adapter) {
        mAdapter = adapter;
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
        getActivity().bindService(new Intent(getActivity(), DTNService.class), mConnection,
                Context.BIND_AUTO_CREATE);
    }
}
