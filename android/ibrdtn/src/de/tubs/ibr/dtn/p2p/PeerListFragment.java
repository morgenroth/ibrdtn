package de.tubs.ibr.dtn.p2p;

import android.database.Cursor;
import android.os.Bundle;
import android.support.v4.app.ListFragment;
import android.support.v4.app.LoaderManager;
import android.support.v4.content.Loader;
import de.tubs.ibr.dtn.p2p.db.Database;
import de.tubs.ibr.dtn.p2p.db.PeerAdapter;

public class PeerListFragment extends ListFragment implements LoaderManager.LoaderCallbacks<Cursor> {
    
    private final static int LOADER_ID = 1;
    
    @SuppressWarnings("unused")
    private final String TAG = "PeerListFragment";
    
    private PeerAdapter mAdapter = null;
    private Database mDatabase = null;
    
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        
        // Create an empty adapter we will use to display the loaded data.
        mAdapter = new PeerAdapter(getActivity(), null, new PeerAdapter.ColumnsMap());
        setListAdapter(mAdapter);
        
        // Start out with a progress indicator.
        setListShown(false);
        
        // open the database
        mDatabase = Database.getInstance(getActivity());
        mDatabase.open();
        
        // load roster
        getLoaderManager().initLoader(LOADER_ID, null, PeerListFragment.this);
    }

    @Override
    public Loader<Cursor> onCreateLoader(int ID, Bundle ARGS) {
        // Now create and return a CursorLoader that will take care of
        // creating a Cursor for the data being displayed.
        return new PeerListLoader(getActivity(), mDatabase);
    }

    @Override
    public void onLoadFinished(Loader<Cursor> loader, Cursor data) {
        // Swap the new cursor in.  (The framework will take care of closing the
        // old cursor once we return.)
        mAdapter.swapCursor(data);
        
        // The list should now be shown.
        if (isResumed()) {
            setListShown(true);
        } else {
            setListShownNoAnimation(true);
        }
    }

    @Override
    public void onLoaderReset(Loader<Cursor> loader) {
        // This is called when the last Cursor provided to onLoadFinished()
        // above is about to be closed.  We need to make sure we are no
        // longer using it.
        mAdapter.swapCursor(null);
    }
}
