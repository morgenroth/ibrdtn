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

import java.util.LinkedList;
import java.util.List;

import de.tubs.ibr.dtn.R;
import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Color;
import android.os.Build;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import android.support.v4.app.ListFragment;
import android.support.v4.app.LoaderManager;
import android.support.v4.content.Loader;

public class LogListFragment extends ListFragment implements LoaderManager.LoaderCallbacks<LogMessage> {
	private LogListAdapter mAdapter = null;
	private boolean mPlayLog = true;
	private MenuItem mPlayItem;

	private static final int MAX_LINES = 300;

	@Override
	public void onActivityCreated(Bundle savedInstanceState)
	{
		super.onActivityCreated(savedInstanceState);

		// We have a menu item to show in action bar.
		setHasOptionsMenu(true);

		getListView().setTranscriptMode(ListView.TRANSCRIPT_MODE_NORMAL);
		getListView().setStackFromBottom(true);

		getListView().setOnScrollListener(new AbsListView.OnScrollListener() {

			@Override
			public void onScrollStateChanged(AbsListView view, int scrollState)
			{
				pauseLog();
			}

			@Override
			public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount, int totalItemCount)
			{
			}
		});

		setEmptyText("no log");

		// create a new list adapter
		mAdapter = new LogListAdapter(getActivity());

		// set listview adapter
		setListAdapter(mAdapter);
	}

	@Override
	public void onCreateOptionsMenu(Menu menu, MenuInflater inflater)
	{
		super.onCreateOptionsMenu(menu, inflater);
		inflater.inflate(R.menu.log_menu, menu);
		mPlayItem = menu.findItem(R.id.itemPlayPauseLog);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		switch (item.getItemId())
		{
		case R.id.itemPlayPauseLog:
		{
			if (mPlayLog)
			{
				pauseLog();
			} else
			{
				playLog();
			}
		}

		default:
			return super.onOptionsItemSelected(item);
		}
	}

	/**
	 * On Android < 3 this is called every time the menu is opened
	 * 
	 * On Android >= 3 invalidateOptionsMenu() has to be called to execute this
	 */
	@Override
	public void onPrepareOptionsMenu(Menu menu)
	{
		if (mPlayLog)
		{
			mPlayItem.setTitle(R.string.log_pause);
			mPlayItem.setIcon(android.R.drawable.ic_media_pause);
		} else
		{
			mPlayItem.setTitle(R.string.log_play);
			mPlayItem.setIcon(android.R.drawable.ic_media_play);
		}
		super.onPrepareOptionsMenu(menu);
	}

	@SuppressLint("NewApi")
	private void pauseLog()
	{
		mPlayLog = false;

		getLoaderManager().destroyLoader(0);

		getActivity().getWindow().setTitle(getResources().getString(R.string.list_logs_paused));

		// change menu item to indicate pause
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB)
		{
			getActivity().invalidateOptionsMenu();
		}
	}

	@SuppressLint("NewApi")
	private void playLog()
	{
		mPlayLog = true;

		getLoaderManager().initLoader(0, null, this);

		getActivity().getWindow().setTitle(getResources().getString(R.string.list_logs));

		// change menu item to indicate play
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB)
		{
			getActivity().invalidateOptionsMenu();
		}
	}

	private void scrollToBottom()
	{
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
	public void onResume()
	{
		super.onResume();

		if (mPlayLog)
		{
			playLog();
		} else
		{
			pauseLog();
		}
	}

	@Override
	public Loader<LogMessage> onCreateLoader(int id, Bundle args)
	{
		return new LogLoader(getActivity());
	}

	@Override
	public void onLoadFinished(Loader<LogMessage> loader, LogMessage data)
	{
		scrollToBottom();

		mAdapter.add(data);
		if (mAdapter.getCount() > MAX_LINES)
		{
			mAdapter.remove(0);
		}
		mAdapter.notifyDataSetChanged();

		// The list should now be shown.
		if (isResumed())
		{
			setListShown(true);
		} else
		{
			setListShownNoAnimation(true);
		}
	}

	@Override
	public void onLoaderReset(Loader<LogMessage> loader)
	{

	}

	private class LogListAdapter extends BaseAdapter {
		private LayoutInflater inflater = null;
		private List<LogMessage> list = new LinkedList<LogMessage>();
		private Context context;

		private class ViewHolder {
			public ImageView imageMark;
			public TextView textDate;
			public TextView textTag;
			public TextView textLog;
			public LogMessage msg;
		}

		public LogListAdapter(Context context) {
			this.context = context;
			this.inflater = LayoutInflater.from(context);
		}

		public int getCount()
		{
			return list.size();
		}

		public void add(LogMessage msg)
		{
			list.add(msg);
		}

		public void remove(int position)
		{
			list.remove(position);
		}

		public Object getItem(int position)
		{
			return list.get(position);
		}

		public long getItemId(int position)
		{
			return position;
		}

		public View getView(int position, View convertView, ViewGroup parent)
		{
			ViewHolder holder;

			if (convertView == null)
			{
				convertView = this.inflater.inflate(R.layout.log_item, null, true);
				holder = new ViewHolder();
				holder.imageMark = (ImageView) convertView.findViewById(R.id.imageMark);
				holder.textDate = (TextView) convertView.findViewById(R.id.textDate);
				holder.textTag = (TextView) convertView.findViewById(R.id.textTag);
				holder.textLog = (TextView) convertView.findViewById(R.id.textLog);
				convertView.setTag(holder);
			} else
			{
				holder = (ViewHolder) convertView.getTag();
			}

			holder.msg = list.get(position);

			if (holder.msg.tag.equals("E"))
			{
				holder.imageMark.setImageLevel(1);
				holder.textTag.setTextColor(context.getResources().getColor(R.color.light_red));
			} else if (holder.msg.tag.equals("W"))
			{
				holder.imageMark.setImageLevel(3);
				holder.textTag.setTextColor(context.getResources().getColor(R.color.light_violett));
			} else if (holder.msg.tag.equals("I"))
			{
				holder.imageMark.setImageLevel(4);
				holder.textTag.setTextColor(context.getResources().getColor(R.color.light_green));
			} else if (holder.msg.tag.equals("D"))
			{
				holder.imageMark.setImageLevel(5);
				holder.textTag.setTextColor(context.getResources().getColor(R.color.light_yellow));
			} else
			{
				holder.imageMark.setImageLevel(0);
				holder.textTag.setTextColor(Color.WHITE);
			}

			holder.textDate.setText(holder.msg.date);
			holder.textLog.setText(holder.msg.msg);

			if (LogMessage.TAG_LABELS.containsKey(holder.msg.tag))
			{
				holder.textTag.setText(LogMessage.TAG_LABELS.get(holder.msg.tag));
			} else
			{
				holder.textTag.setText(holder.msg.tag);
			}

			return convertView;
		}

	};

}
