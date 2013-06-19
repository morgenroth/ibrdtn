package de.tubs.ibr.dtn.daemon;

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

import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.service.DaemonService;
import de.tubs.ibr.dtn.service.DaemonService.LocalDTNService;
import de.tubs.ibr.dtn.stats.StatsEntry;
import de.tubs.ibr.dtn.stats.StatsLoader;

public abstract class StatsChartFragment extends Fragment {
    
    // The loader's unique id. Loader ids are specific to the Activity or
    // Fragment in which they reside.
    private static final int DB_LOADER_ID = 1;
    private static final int STATS_LOADER_ID = 2;

    private static final String TAG = "StatsChartFragment";
    
    private Object mServiceLock = new Object();
    private DaemonService mService = null;
    
    private ChartView mChartView = null;
    private ListView mListView = null;
    
    private StatsListAdapter mAdapter = null;
    
    protected abstract void plot(Cursor stats, ChartView chart);
    
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
            // clear the series
            mChartView.clearSeries();
            
            // plot the charts
            plot(stats, mChartView);
        }

        @Override
        public void onLoaderReset(Loader<Cursor> loader) {
        }
    };
    
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
        getActivity().bindService(new Intent(DTNService.class.getName()), mConnection,
                Context.BIND_AUTO_CREATE);
    }
}
