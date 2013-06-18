package de.tubs.ibr.dtn.daemon;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.ListFragment;
import android.support.v4.app.LoaderManager;
import android.support.v4.content.Loader;
import android.util.Log;
import android.view.View;
import android.widget.ListView;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.service.DaemonService;
import de.tubs.ibr.dtn.service.DaemonService.LocalDTNService;
import de.tubs.ibr.dtn.swig.NativeStats;

public class BundleStatsFragment extends ListFragment implements
    LoaderManager.LoaderCallbacks<NativeStats> {

    // The loader's unique id. Loader ids are specific to the Activity or
    // Fragment in which they reside.
    private static final int LOADER_ID = 1;

    private static final String TAG = "BundleStatsFragment";

    private Object mServiceLock = new Object();
    private DaemonService mService = null;
    private BundleStatsListAdapter mAdapter = null;

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder s) {
            synchronized(mServiceLock) {
                mService = ((LocalDTNService)s).getLocal();
            }
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service connected");

            // initialize the loader
            getLoaderManager().initLoader(LOADER_ID,  null, BundleStatsFragment.this);
        }

        public void onServiceDisconnected(ComponentName name) {
            getLoaderManager().destroyLoader(LOADER_ID);

            synchronized(mServiceLock) {
                mService = null;
            }
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service disconnected");
        }
    };

    @Override
    public void onListItemClick(ListView l, View v, int position, long id) {
        BundleStatsListAdapter sla = (BundleStatsListAdapter)this.getListAdapter();
        BundleStatsListAdapter.StatsEntry e = (BundleStatsListAdapter.StatsEntry)sla.getItem(position);

        // call super-method
        super.onListItemClick(l, v, position, id);
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

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        setEmptyText(getActivity().getResources().getString(R.string.list_no_neighbors));

        // create a new list adapter
        mAdapter = new BundleStatsListAdapter(getActivity());

        // set listview adapter
        setListAdapter(mAdapter);

        // Start out with a progress indicator.
        setListShown(false);
    }

    @Override
    public Loader<NativeStats> onCreateLoader(int id, Bundle args) {
        return new StatsLoader(getActivity(), mService);
    }

    @Override
    public void onLoadFinished(Loader<NativeStats> loader, NativeStats stats) {
        synchronized (mAdapter) {
            mAdapter.swapList(stats);
        }

        // The list should now be shown.
        if (isResumed()) {
            setListShown(true);
        } else {
            setListShownNoAnimation(true);
        }
    }

    @Override
    public void onLoaderReset(Loader<NativeStats> loader) {
    }
}
