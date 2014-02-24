/*
 * LogListFragment.java
 * 
 * Copyright (C) 2013 IBR, TU Braunschweig
 * 
 * Written-by: Dominik Schürmann <dominik@dominikschuermann.de>
 * 	           Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

package de.tubs.ibr.dtn.daemon;

import android.annotation.SuppressLint;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.ListFragment;
import android.support.v4.app.LoaderManager;
import android.support.v4.content.Loader;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.AbsListView;
import android.widget.ListView;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.daemon.data.LogListAdapter;
import de.tubs.ibr.dtn.daemon.data.LogLoader;

public class LogListFragment extends ListFragment implements
        LoaderManager.LoaderCallbacks<LogMessage> {
	
	// The loader's unique id. Loader ids are specific to the Activity or
	// Fragment in which they reside.
	private static final int LOADER_ID = 1;
	
    private LogListAdapter mAdapter = null;
    private boolean mPlayLog = true;
    private MenuItem mPlayItem;

    @SuppressLint("NewApi")
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        // We have a menu item to show in action bar.
        setHasOptionsMenu(true);

        getListView().setTranscriptMode(AbsListView.TRANSCRIPT_MODE_NORMAL);
        getListView().setStackFromBottom(true);

        // add scrollbar handle
        getListView().setFastScrollEnabled(true);

        getListView().setOnScrollListener(new AbsListView.OnScrollListener() {

            @Override
            public void onScrollStateChanged(AbsListView view, int scrollState) {
                pauseLog();
            }

            @Override
            public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount,
                    int totalItemCount) {
            }
        });

        setEmptyText(getActivity().getResources().getString(R.string.list_no_log));

        // create a new list adapter
        mAdapter = new LogListAdapter(getActivity());

        // set listview adapter
        setListAdapter(mAdapter);
        
        // Start out with a progress indicator.
        setListShown(false);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
        inflater.inflate(R.menu.log_menu, menu);
        mPlayItem = menu.findItem(R.id.itemPlayPauseLog);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.itemPlayPauseLog: {
                if (mPlayLog) {
                    pauseLog();
                } else {
                    playLog();
                }
            }

            default:
                return super.onOptionsItemSelected(item);
        }
    }

    /**
     * On Android < 3 this is called every time the menu is opened On Android >=
     * 3 invalidateOptionsMenu() has to be called to execute this
     */
    @Override
    public void onPrepareOptionsMenu(Menu menu) {
        if (mPlayLog) {
            mPlayItem.setTitle(R.string.log_pause);
            mPlayItem.setIcon(android.R.drawable.ic_media_pause);
        } else {
            mPlayItem.setTitle(R.string.log_play);
            mPlayItem.setIcon(android.R.drawable.ic_media_play);
        }
        super.onPrepareOptionsMenu(menu);
    }

    @SuppressLint("NewApi")
    private void pauseLog() {
        mPlayLog = false;

        // pause the loader
        getLoaderManager().destroyLoader(LOADER_ID);

        getActivity().getWindow().setTitle(getResources().getString(R.string.list_logs_paused));

        // change menu item to indicate pause
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB) {
            getActivity().invalidateOptionsMenu();
        }
    }

    @SuppressLint("NewApi")
    private void playLog() {
        mPlayLog = true;

        // initialize the loader
        getLoaderManager().initLoader(LOADER_ID, null, this);

        getActivity().getWindow().setTitle(getResources().getString(R.string.list_logs));

        // change menu item to indicate play
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB) {
            getActivity().invalidateOptionsMenu();
        }
    }

    private void scrollToBottom() {
        final ListView list = getListView();
        list.post(new Runnable() {
            @Override
            public void run()
            {
                list.setSelection(mAdapter.getCount() - 1);
            }
        });
    }

    @Override
    public void onResume() {
        super.onResume();

        if (mPlayLog) {
            playLog();
        } else {
            pauseLog();
        }
    }

    @Override
    public Loader<LogMessage> onCreateLoader(int id, Bundle args) {
        return new LogLoader(getActivity());
    }

    @Override
    public void onLoadFinished(Loader<LogMessage> loader, LogMessage data) {
    	synchronized (mAdapter) {
    		mAdapter.add(data);
    	}
    	
    	scrollToBottom();

        // The list should now be shown.
        if (isResumed()) {
            setListShown(true);
        } else {
            setListShownNoAnimation(true);
        }
    }

    @Override
    public void onLoaderReset(Loader<LogMessage> loader) {
    }
}
