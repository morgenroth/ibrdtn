package de.tubs.ibr.dtn.chat;

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
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import de.tubs.ibr.dtn.chat.core.Buddy;
import de.tubs.ibr.dtn.chat.core.Roster.RefreshCallback;
import de.tubs.ibr.dtn.chat.core.Roster.ViewHolder;
import de.tubs.ibr.dtn.chat.service.ChatService;

public class MainActivity extends ListActivity implements RefreshCallback {
	
	private final String TAG = "MainActivity";
	private ChatService service = null;
	
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			MainActivity.this.service = ((ChatService.LocalBinder)service).getService();
			
			if (!MainActivity.this.service.isServiceAvailable()) {
				showInstallServiceDialog();
			}
			
			Log.i(TAG, "service connected");
			
			// load buddies
			refresh();
			
			// register myself for refresh callback
			MainActivity.this.service.getRoster().setRefreshCallback(MainActivity.this);
			
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
	
	private void refresh()
	{
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
		
		// check if the screen is active
		PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
	    Boolean screenOn = pm.isScreenOn();
		
	    String presence_tag = preferences.getString("presencetag", "auto");
	    String presence_nick = preferences.getString("editNickname", "Nobody");
	    String presence_text = preferences.getString("statustext", "");
	    
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
	    
	    ImageView icon = (ImageView)findViewById(R.id.iconTitleBar);
	    
		if (presence_tag != null)
		{
			if (presence_tag.equalsIgnoreCase("unavailable"))
			{
				icon.setImageResource(R.drawable.offline);
			}
			else if (presence_tag.equalsIgnoreCase("xa"))
			{
				icon.setImageResource(R.drawable.xa);
			}
			else if (presence_tag.equalsIgnoreCase("away"))
			{
				icon.setImageResource(R.drawable.away);
			}
			else if (presence_tag.equalsIgnoreCase("dnd"))
			{
				icon.setImageResource(R.drawable.busy);
			}
			else if (presence_tag.equalsIgnoreCase("chat"))
			{
				icon.setImageResource(R.drawable.online);
			}
		}
		
		TextView nicknameLabel = (TextView)findViewById(R.id.labelTitleBar);
		TextView statusLabel = (TextView)findViewById(R.id.bottomtextTitleBar);
		nicknameLabel.setText(presence_nick);
		
		if (presence_text.length() > 0) {
			statusLabel.setText(presence_text);
		} else {
			statusLabel.setText("<" + getResources().getString(R.string.no_status_message) + ">");
		}
		
		if (this.service != null)
		{
			setListAdapter( service.getRoster().getListAdapter() );
		}
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.buddy_menu, menu);
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
			this.service.getRoster().removeBuddy(buddy);
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
		i.putExtra("endpointid", holder.buddy.getEndpoint());
		startActivity(i);
	}
}
