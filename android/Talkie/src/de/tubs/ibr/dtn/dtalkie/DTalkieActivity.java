/*
 * DTalkieActivity.java
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
package de.tubs.ibr.dtn.dtalkie;

import java.io.IOException;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.ActivityInfo;
import android.content.pm.ResolveInfo;
import android.content.res.Configuration;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.support.v4.view.MenuItemCompat;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnCreateContextMenuListener;
import android.view.View.OnTouchListener;
import android.widget.AdapterView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.ListView;
import de.tubs.ibr.dtn.dtalkie.db.Message;
import de.tubs.ibr.dtn.dtalkie.db.MessageDatabase;
import de.tubs.ibr.dtn.dtalkie.db.MessageDatabase.Folder;
import de.tubs.ibr.dtn.dtalkie.db.MessageDatabase.OnUpdateListener;
import de.tubs.ibr.dtn.dtalkie.db.MessageDatabase.ViewHolder;
import de.tubs.ibr.dtn.dtalkie.service.DTalkieService;

public class DTalkieActivity extends Activity {
	
	private static final String TAG = "DTalkieActivity";
	private DTalkieService service = null;
	
	private SensorManager manager = null;
	private Sensor sensor = null;
	
	private Object recording_state_mutex = new Object();
	private RecState recording_state = RecState.IDLE;
	
	private enum RecState {
		IDLE,
		SENSOR_RECORDING,
		BUTTON_RECORDING
	};
	
	private SensorEventListener listener = new SensorEventListener() {
		@Override
		public void onSensorChanged(SensorEvent event) {
			if (event.values.length > 0) {
				float current = event.values[0];
				float maxRange = event.sensor.getMaximumRange();
				boolean far = (current == maxRange);
				
				if (service != null) service.setOnEar(!far);
				
				if (far) {
					stopRecording(RecState.SENSOR_RECORDING);
				} else {
					SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(DTalkieActivity.this);
					if (prefs.getBoolean("sensor", false)) startRecording(RecState.SENSOR_RECORDING);
				}
			}
		}

		@Override
		public void onAccuracyChanged(Sensor sensor, int accuracy) {
		}
	};
	

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        manager = (SensorManager) getSystemService(SENSOR_SERVICE);

        // set volume control to MUSIC
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        	
		Intent checkService = new Intent("de.tubs.ibr.dtn.DTNService");
		List<ResolveInfo> list = getPackageManager().queryIntentServices(checkService, 0);    
		if (list.size() > 0)
		{
			// Establish a connection with the service.  We use an explicit
			// class name because we want a specific service implementation that
			// we know will be running in our own process (and thus won't be
			// supporting component replacement by other applications).
			bindService(new Intent(DTalkieActivity.this, DTalkieService.class), mConnection, Context.BIND_AUTO_CREATE);
		}
		else
		{
			mConnection = null;
			
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
			        DTalkieActivity.this.finish();
			    }
			};

			AlertDialog.Builder builder = new AlertDialog.Builder(this);
			builder.setMessage(getResources().getString(R.string.alert_missing_daemon));
			builder.setPositiveButton(getResources().getString(R.string.alert_yes), dialogClickListener);
			builder.setNegativeButton(getResources().getString(R.string.alert_no), dialogClickListener);
			builder.show();
		}
    }
    
    @Override
	public boolean onContextItemSelected(MenuItem item) {
    	AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
    	
    	MessageDatabase db = this.service.getDatabase();
    	
    	switch (item.getItemId())
    	{
    	case R.id.itemDelete:
    		Message msg = db.get(Folder.INBOX, info.position);
    		db.remove(Folder.INBOX, msg);
    		return true;
    	case R.id.itemClearList:
    		db.clear(Folder.INBOX);
    		return true;
    	}
	
		return super.onContextItemSelected(item);
	}

	private OnItemClickListener _messages_click = new OnItemClickListener() {
		@Override
		public void onItemClick(AdapterView<?> arg0, View v, int position, long id) {
			de.tubs.ibr.dtn.dtalkie.db.MessageDatabase.ViewHolder holder = (ViewHolder)v.getTag();
			DTalkieActivity.this.service.play(Folder.INBOX, holder.msg);
		}
    };
    
    private OnCreateContextMenuListener _messages_menu = new OnCreateContextMenuListener() {
		@Override
		public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
			MenuInflater inflater = getMenuInflater();
			inflater.inflate(R.menu.message_menu, menu);
		}
    };
    
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.main_menu, menu);
	    
	    MenuItem autorec = menu.findItem(R.id.itemAutoRec);
	    MenuItem autoplay = menu.findItem(R.id.itemAutoPlay);
	    
        MenuItemCompat.setShowAsAction(autoplay, MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
	    MenuItemCompat.setShowAsAction(autorec, MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
	    MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemClearList), MenuItemCompat.SHOW_AS_ACTION_NEVER | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
	    return true;
	}
	
	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
	    MenuItem autorec = menu.findItem(R.id.itemAutoRec);
	    MenuItem autoplay = menu.findItem(R.id.itemAutoPlay);
	    
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(DTalkieActivity.this);

        // restore autoplay option
        autoplay.setIcon(prefs.getBoolean("autoplay", false) ? R.drawable.ic_autoplay_pause : R.drawable.ic_autoplay);
        autoplay.setChecked(prefs.getBoolean("autoplay", false));
        
        autorec.setIcon(prefs.getBoolean("sensor", false) ? R.drawable.ic_autorec_on : R.drawable.ic_autorec_off);
        autorec.setChecked(prefs.getBoolean("sensor", false));
        
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(DTalkieActivity.this);
		
	    switch (item.getItemId()) {
		    case R.id.itemAutoPlay:
		    {
				Editor edit = prefs.edit();
				Boolean newvalue = (!prefs.getBoolean("autoplay", false));
				edit.putBoolean("autoplay", newvalue);
				DTalkieActivity.this.service.setAutoPlay(newvalue);
				item.setChecked(newvalue);
				item.setIcon(newvalue ? R.drawable.ic_autoplay_pause : R.drawable.ic_autoplay);
				edit.commit();
				return true;
		    }
				
		    case R.id.itemAutoRec:
		    {
				Editor edit = prefs.edit();
				Boolean newvalue = (!prefs.getBoolean("sensor", false));
				edit.putBoolean("sensor", newvalue);
				item.setChecked(newvalue);
				item.setIcon(newvalue ? R.drawable.ic_autorec_on : R.drawable.ic_autorec_off);
				edit.commit();
				return true;
		    }
	    		
	    	case R.id.itemClearList:
	        	MessageDatabase db = this.service.getDatabase();
	    		db.clear(Folder.INBOX);
	    		return true;
	    	
	    	default:
	    		return super.onOptionsItemSelected(item);
	    }
	}
    
	@Override
	protected void onPause() {
		manager.unregisterListener(listener);
		sensor = null;
		
		// stop sensor recording if active
		stopRecording(RecState.SENSOR_RECORDING);
		stopRecording(RecState.BUTTON_RECORDING);
		
		unlockScreenOrientation();
		
		// we deactivated the sensor, so we reset to default settings
		if (service != null) service.setOnEar(false);

		super.onPause();
	}

	@Override
	protected void onResume() {
        sensor = null;
        List<Sensor> sensors = manager.getSensorList(Sensor.TYPE_PROXIMITY);
        if (sensors.size() > 0) {
            sensor = sensors.get(0);
            manager.registerListener(listener, sensor, SensorManager.SENSOR_DELAY_NORMAL);
        }
        
        lockCurrentScreenOrientation();
        
		super.onResume();
	}
	
	private OnUpdateListener _update_listener = new OnUpdateListener() {
		@Override
		public void update(Folder folder) {
			DTalkieActivity.this.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					ListView listMessages = (ListView) findViewById(R.id.listMessage);
					((BaseAdapter)listMessages.getAdapter()).notifyDataSetChanged();
				}
			});
		}
	};
	  
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			DTalkieActivity.this.service = ((DTalkieService.LocalBinder)service).getService();
			
			if (!DTalkieActivity.this.service.isServiceAvailable()) {
				showInstallServiceDialog();
			}
			
			Log.i(TAG, "service connected");
			
			// set send handler
			ImageButton buttonTalk = (ImageButton) findViewById(R.id.buttonTalk);
			buttonTalk.setOnTouchListener(new OnTouchListener() {
				@Override
				public boolean onTouch( View yourButton , MotionEvent theMotion ) {
					ImageButton btn = (ImageButton)yourButton;
					
				    switch ( theMotion.getAction() ) {
				    	case MotionEvent.ACTION_DOWN:
				    		btn.setPressed(true);
				    		startRecording(RecState.BUTTON_RECORDING);
				    		break;
				    	case MotionEvent.ACTION_UP:
				    		btn.setPressed(false);
				    		stopRecording(RecState.BUTTON_RECORDING);
				    		break;
				    }
				    return true;
				}
			});
				
			// assign update listener
			DTalkieActivity.this.service.getDatabase().setOnUpdateListener(_update_listener);
			
			// assign listviews
			ListView listMessages = (ListView) findViewById(R.id.listMessage);
			listMessages.setAdapter( DTalkieActivity.this.service.getDatabase().getListAdapter(Folder.INBOX) );
			listMessages.setOnCreateContextMenuListener(_messages_menu);
			listMessages.setOnItemClickListener(_messages_click);
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.i(TAG, "service disconnected");
			
			ImageButton buttonTalk = (ImageButton) findViewById(R.id.buttonTalk);
			buttonTalk.setOnTouchListener(null);
			
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
	
    public void startRecording(RecState type)
    {
    	synchronized(recording_state_mutex) {
    		if (!recording_state.equals(RecState.IDLE)) return;
    		recording_state = type;
    		
//    		lockCurrentScreenOrientation();
    	}
    	
    	try {
			if (service != null) service.startRecording();
    	} catch (IOException ex) {
    		Log.e(TAG, "start recording failed", ex);
    	};
    }
    
    private void lockCurrentScreenOrientation() {
    	switch (this.getResources().getConfiguration().orientation) {
    	case Configuration.ORIENTATION_LANDSCAPE:
    		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    		break;
    		
    	case Configuration.ORIENTATION_PORTRAIT:
    		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
    		break;
    		
    	default:
    		setRequestedOrientation(getRequestedOrientation());
    		break;
    	}
    }
    
    private void unlockScreenOrientation() {
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_USER);
    }
    
    public void stopRecording(RecState type)
    {
    	synchronized(recording_state_mutex) {
    		if (!recording_state.equals(type)) return;
    		recording_state = RecState.IDLE;
    		
//    		unlockScreenOrientation();
    	}
    	
    	try {
    		if (service != null) service.stopRecording();
		} catch (IOException ex) {
			Log.e(TAG, "stop recording failed", ex);
		};
    }
	
	@Override
	protected void onDestroy() {
		// assign update listener
		if (service != null)
		{
			if (service.getDatabase() != null)
			{
				service.getDatabase().setOnUpdateListener(null);
			}
		}
		
	    super.onDestroy();
	    
	    if (mConnection != null) {
	    	// Detach our existing connection.
	    	unbindService(mConnection);
	    }
	}
}