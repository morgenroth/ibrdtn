package de.tubs.ibr.dtn.daemon;

import java.util.ArrayList;

import android.annotation.SuppressLint;
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
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.ListView;

import com.jjoe64.graphview.CustomLabelFormatter;
import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.GraphView.GraphViewData;
import com.jjoe64.graphview.GraphViewSeries;
import com.jjoe64.graphview.GraphViewSeries.GraphViewSeriesStyle;
import com.jjoe64.graphview.LineGraphView;

import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.daemon.data.CurrentStatsLoader;
import de.tubs.ibr.dtn.daemon.data.StatsListAdapter;
import de.tubs.ibr.dtn.daemon.data.StatsLoader;
import de.tubs.ibr.dtn.service.ControlService;
import de.tubs.ibr.dtn.service.DaemonService;
import de.tubs.ibr.dtn.stats.StatsEntry;
import de.tubs.ibr.dtn.stats.StatsUtils;

public abstract class StatsChartFragment extends Fragment implements CustomLabelFormatter {
    
    // The loader's unique id. Loader ids are specific to the Activity or
    // Fragment in which they reside.
    private static final int GRAPH_LOADER_ID = 1;
    private static final int STATS_LOADER_ID = 2;

    private static final String TAG = "StatsChartFragment";
    
    private ControlService mService = null;
    
    private LinearLayout mChartView = null;
    private LineGraphView mGraphView = null;
    private ListView mListView = null;
    
    private SparseArray<GraphViewSeries> mData = null;
    
    private StatsListAdapter mAdapter = null;
    
    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder s) {
            mService = ControlService.Stub.asInterface(s);
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service connected");

            // initialize the loader
            getLoaderManager().initLoader(GRAPH_LOADER_ID, null, mGraphLoader);
            getLoaderManager().initLoader(STATS_LOADER_ID, null, mStatsLoader);
        }

        public void onServiceDisconnected(ComponentName name) {
            getLoaderManager().destroyLoader(GRAPH_LOADER_ID);
            getLoaderManager().destroyLoader(STATS_LOADER_ID);

            mService = null;
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service disconnected");
        }
    };
    
    private LoaderManager.LoaderCallbacks<Cursor> mGraphLoader = new LoaderManager.LoaderCallbacks<Cursor>() {
        @Override
        public Loader<Cursor> onCreateLoader(int id, Bundle args) {
            return new StatsLoader(getActivity());
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
        if (stats == null) return;
        int charts_count = mAdapter.getDataRows();
        
        ArrayList<ArrayList<GraphViewData>> data = new ArrayList<ArrayList<GraphViewData>>(charts_count);
        
        // convert data into an structured array
        StatsUtils.convertData(getActivity(), stats, data, mAdapter);
        
        // get line width in pixels
        Float lineWidth = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 2, getActivity().getResources().getDisplayMetrics());
        
        // add one series for each data-set
        for (int i = 0; i < charts_count; i++) {
            ArrayList<GraphViewData> dataset = data.get(i);
            
            GraphViewSeries gs = mData.get(i);
            
            if (gs == null) {
                GraphViewSeriesStyle style = new GraphViewSeriesStyle(getResources().getColor(mAdapter.getDataColor(i)), lineWidth.intValue());
                
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
    }
    
    @SuppressLint("DefaultLocale")
    @Override
    public String formatLabel(double value, boolean isValueX) {
        if (isValueX) {
            return StatsUtils.formatTimeStampString(getActivity(), Double.valueOf(value * 1000.0).longValue());
        }
        return String.format("%.2f", value);
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
        mGraphView.getGraphViewStyle().setTextSize(getResources().getDimension(R.dimen.stats_axis_text));
        mGraphView.setCustomLabelFormatter(this);
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

        Intent bindIntent = DaemonService.createControlServiceIntent(getActivity());
        getActivity().bindService(bindIntent, mConnection, Context.BIND_AUTO_CREATE);
    }
}
