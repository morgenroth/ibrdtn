package de.tubs.ibr.dtn.daemon;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

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
import de.tubs.ibr.dtn.daemon.data.ConvergenceLayerStatsListAdapter;
import de.tubs.ibr.dtn.daemon.data.ConvergenceLayerStatsLoader;
import de.tubs.ibr.dtn.daemon.data.CurrentConvergenceLayerStatsLoader;
import de.tubs.ibr.dtn.service.ControlService;
import de.tubs.ibr.dtn.service.DaemonService;
import de.tubs.ibr.dtn.stats.ConvergenceLayerStatsEntry;
import de.tubs.ibr.dtn.stats.StatsUtils;

public class ConvergenceLayerStatsChartFragment extends Fragment implements CustomLabelFormatter {
    
    // The loader's unique id. Loader ids are specific to the Activity or
    // Fragment in which they reside.
    private static final int GRAPH_LOADER_ID = 1;
    private static final int STATS_LOADER_ID = 2;

    private static final String TAG = "ClStatsChartFragment";
    
    private ControlService mService = null;
    
    private LinearLayout mChartView = null;
    private GraphView mGraphView = null;
    private ListView mListView = null;
    
    private HashMap<String, GraphViewSeries> mData = null;
    
    private ConvergenceLayerStatsListAdapter mAdapter = null;
    
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
            return new ConvergenceLayerStatsLoader(getActivity(), null);
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
        
        HashMap<String, ArrayList<GraphViewData>> series = new HashMap<String, ArrayList<GraphViewData>>();

        // convert data into an structured array
        StatsUtils.convertData(getActivity(), stats, series);
        
        // get line width in pixels
        Float lineWidth = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 2, getActivity().getResources().getDisplayMetrics());
        
        // add one series for each data-set
        for (Map.Entry<String, ArrayList<GraphViewData>> entry : series.entrySet()) {
            ArrayList<GraphViewData> dataset = entry.getValue();
            
            GraphViewSeries gs = mData.get(entry.getKey());
            
            if (gs == null) {
                int color = getResources().getColor(mColorProvider.getColor(entry.getKey()));
                GraphViewSeriesStyle style = new GraphViewSeriesStyle(color, lineWidth.intValue());
                gs = new GraphViewSeries(entry.getKey(), style, dataset.toArray(new GraphViewData[dataset.size()]));
                mGraphView.addSeries(gs);
                mData.put(entry.getKey(), gs);
                
                // redraw the graph
                mGraphView.redrawAll();
            } else {
                gs.resetData(dataset.toArray(new GraphViewData[dataset.size()]));
            }
        }
    }
    
    @Override
    public String formatLabel(double value, boolean isValueX) {
        if (isValueX) {
            return StatsUtils.formatTimeStampString(getActivity(), Double.valueOf(value * 1000.0).longValue());
        } else {
            return StatsUtils.formatByteString(Double.valueOf(value).intValue(), true);
        }
    }
    
	private LoaderManager.LoaderCallbacks<List<ConvergenceLayerStatsEntry>> mStatsLoader = new LoaderManager.LoaderCallbacks<List<ConvergenceLayerStatsEntry>>() {
		@Override
		public Loader<List<ConvergenceLayerStatsEntry>> onCreateLoader(int id, Bundle args) {
			return new CurrentConvergenceLayerStatsLoader(getActivity(), mService, null);
		}

		@Override
		public void onLoadFinished(Loader<List<ConvergenceLayerStatsEntry>> loader,
				List<ConvergenceLayerStatsEntry> stats) {
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
        
        // create a new holder for plot data
        mData = new HashMap<String, GraphViewSeries>();
        
        // Find the chart and list view
        mChartView = (LinearLayout) v.findViewById(R.id.chart_view);
        mListView = (ListView) v.findViewById(R.id.list_view);
        
        // create a new LineGraphView
        mGraphView = new LineGraphView(getActivity(), "");
        mGraphView.getGraphViewStyle().setTextSize(getResources().getDimension(R.dimen.stats_axis_text));
        mGraphView.setCustomLabelFormatter(this);
        mChartView.addView(mGraphView);
        
        return v;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

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

        Intent bindIntent = DaemonService.createControlServiceIntent(getActivity());
        getActivity().bindService(bindIntent, mConnection, Context.BIND_AUTO_CREATE);
    }
    
    private ConvergenceLayerStatsListAdapter.ColorProvider mColorProvider = new ConvergenceLayerStatsListAdapter.ColorProvider() {

        final int[] mColors = {
                R.color.stats_first,
                R.color.stats_second,
                R.color.stats_fourth,
                R.color.stats_third,
                R.color.stats_fifth,
                R.color.stats_sixth
        };
        
        int assignedColors = 0;
        HashMap<String, Integer> mColorMap = new HashMap<String, Integer>();
        
        @Override
        public int getColor(String tag) {
            if (mColorMap.containsKey(tag))
                return mColorMap.get(tag);
            
            if (assignedColors >= mColors.length)
                return R.color.stats_default;
            
            int color = mColors[assignedColors];
            mColorMap.put(tag, color);
            assignedColors++;
            
            return color;
        }
        
    };
}
