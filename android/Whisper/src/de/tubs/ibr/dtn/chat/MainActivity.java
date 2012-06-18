/*
 * MainActivity.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
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
package de.tubs.ibr.dtn.chat;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.os.PowerManager;
import android.preference.PreferenceManager;
import android.support.v4.view.MenuItemCompat;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import de.tubs.ibr.dtn.chat.RosterView.ViewHolder;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.service.ChatService;

public class MainActivity extends ListActivity {
	private final String TAG = "MainActivity";
	private RosterView view = null;
	private ChatService service = null;
	
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			MainActivity.this.service = ((ChatService.LocalBinder)service).getService();
			
			if (!MainActivity.this.service.isServiceAvailable()) {
				showInstallServiceDialog();
			}
			
			Log.i(TAG, "service connected");
			
			// activate roster view
			MainActivity.this.view = new RosterView(MainActivity.this, MainActivity.this.service.getRoster());
			MainActivity.this.setListAdapter(MainActivity.this.view);
						
			// set process bar to invisible
			setProgressBarIndeterminateVisibility(false);
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.i(TAG, "service disconnected");
			service = null;
		}
	};
	
	private void showInstallServiceDialog() {
		DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
		    @Override
		    public void onClick(DialogInterface dialog, int which) {
		        switch (which){
		        case DialogInterface.BUTTON_POSITIVE:
					final Intent marketIntent = new Intent(Intent.ACTION_VIEW);
					marketIntent.setData(Uri.parse("market://details?id=de.tubs.ibr.dtn"));
					startActivity(marketIntent);
		            break;

		        case DialogInterface.BUTTON_NEGATIVE:
		            break;
		        }
		        finish();
		    }
		};

		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setMessage(getResources().getString(R.string.alert_missing_daemon));
		builder.setPositiveButton(getResources().getString(R.string.alert_yes), dialogClickListener);
		builder.setNegativeButton(getResources().getString(R.string.alert_no), dialogClickListener);
		builder.show();
	}
	
	@Override
	protected void onDestroy() {
		MainActivity.this.view.onDestroy(this);
		MainActivity.this.view = null;
	    super.onDestroy();
	    
	    if (mConnection != null) {
	    	// Detach our existing connection.
	    	unbindService(mConnection);
	    }
	}
	
	public void refreshCallback()
	{
		MainActivity.this.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				((BaseAdapter)getListView().getAdapter()).notifyDataSetChanged();
			}
		});
	}
	
	@Override
	protected void onPause() {
		super.onPause();
	}

	@Override
	protected void onResume() {
		refresh();
		super.onResume();
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		if (android.os.Build.VERSION.SDK_INT < 11) {
			requestWindowFeature(Window.FEATURE_NO_TITLE);
		}
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.roster_main);
		
		// Establish a connection with the service.  We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		bindService(new Intent(MainActivity.this, ChatService.class), mConnection, Context.BIND_AUTO_CREATE);
		
		ListView lv = getListView();
		registerForContextMenu(lv);
	}
	
	@Override
	protected void onPostCreate(Bundle savedInstanceState) {
		
	    if (android.os.Build.VERSION.SDK_INT >= 11) {
	    	LinearLayout layout = (LinearLayout)findViewById(R.id.roster_layoutTitleBar);
	    	View view = (View)findViewById(R.id.roster_titlebar_separator);
	    	view.setVisibility(View.GONE);
	    	layout.setVisibility(View.GONE);
	    }
	    
	    super.onPostCreate(savedInstanceState);
	}
	
	private void refresh()
	{
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
		
		// check if the screen is active
		PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
	    Boolean screenOn = pm.isScreenOn();
		
	    String presence_tag = preferences.getString("presencetag", "auto");
	    String presence_nick = preferences.getString("editNickname", "Nobody");
	    String presence_text = preferences.getString("statustext", "");
	    int presence_icon = R.drawable.online;
	    
	    if (presence_text.length() == 0) {
	    	presence_text = "<" + getResources().getString(R.string.no_status_message) + ">";
	    }
	    
	    if (presence_tag.equals("auto"))
	    {
		    if (screenOn)
		    {
		    	presence_tag = "chat";
		    }
		    else
		    {
		    	presence_tag = "away";
		    }
	    }
	    
		if (presence_tag != null)
		{
			if (presence_tag.equalsIgnoreCase("unavailable"))
			{
				presence_icon = R.drawable.offline;
			}
			else if (presence_tag.equalsIgnoreCase("xa"))
			{
				presence_icon = R.drawable.xa;
			}
			else if (presence_tag.equalsIgnoreCase("away"))
			{
				presence_icon = R.drawable.away;
			}
			else if (presence_tag.equalsIgnoreCase("dnd"))
			{
				presence_icon = R.drawable.busy;
			}
			else if (presence_tag.equalsIgnoreCase("chat"))
			{
				presence_icon = R.drawable.online;
			}
		}
	    
	    if (android.os.Build.VERSION.SDK_INT >= 11) {
	    	ActionBar actionbar = getActionBar();
	    	actionbar.setTitle(presence_nick);
	    	actionbar.setSubtitle(presence_text);
	    	if (android.os.Build.VERSION.SDK_INT >= 14) {
	    		actionbar.setIcon(presence_icon);
	    	}
	    } else {
		    ImageView icon = (ImageView)findViewById(R.id.iconTitleBar);
			TextView nicknameLabel = (TextView)findViewById(R.id.labelTitleBar);
			TextView statusLabel = (TextView)findViewById(R.id.bottomtextTitleBar);
			
			icon.setImageResource(presence_icon);
			nicknameLabel.setText(presence_nick);
			statusLabel.setText(presence_text);
	    }
		
		if (this.view != null)
		{
			view.refresh();
		}
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.buddy_menu, menu);
	    MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemPreferences), MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
	    return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    // Handle item selection
	    switch (item.getItemId()) {
	    case R.id.itemPreferences:
	    {
			// Launch Preference activity
			Intent i = new Intent(MainActivity.this, Preferences.class);
			startActivity(i);
	        return true;
	    }
	    
	    default:
	        return super.onOptionsItemSelected(item);
	    }
	}
	
	@Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
		super.onCreateContextMenu(menu, v, menuInfo);
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.buddycontext_menu, menu);
	}
	
	@Override
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();

		Buddy buddy = this.service.getRoster().get(info.position);
		if (buddy == null) return false;

		switch (item.getItemId())
		{
		case R.id.itemDelete:
			this.service.getRoster().remove(buddy);
			return true;
		default:
			return super.onContextItemSelected(item);
		}
	}
	
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		super.onListItemClick(l, v, position, id);
		
		ViewHolder holder = (ViewHolder)v.getTag();
		
		Intent i = new Intent(MainActivity.this, MessageActivity.class);
		i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//		i.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_CLEAR_TOP);
		i.putExtra("buddy", holder.buddy.getEndpoint());
		startActivity(i);
	}
}
