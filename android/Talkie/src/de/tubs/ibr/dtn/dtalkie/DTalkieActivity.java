package de.tubs.ibr.dtn.dtalkie;

import java.io.IOException;
import java.util.List;

import de.tubs.ibr.dtn.dtalkie.db.Message;
import de.tubs.ibr.dtn.dtalkie.db.MessageDatabase;
import de.tubs.ibr.dtn.dtalkie.db.MessageDatabase.Folder;
import de.tubs.ibr.dtn.dtalkie.db.MessageDatabase.OnUpdateListener;
import de.tubs.ibr.dtn.dtalkie.db.MessageDatabase.ViewHolder;
import de.tubs.ibr.dtn.dtalkie.service.DTalkieService;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnCreateContextMenuListener;
import android.view.View.OnTouchListener;
import android.widget.AdapterView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.ToggleButton;

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
					ToggleButton buttonSensor = (ToggleButton) findViewById(R.id.buttonSensor);
					if (buttonSensor.isChecked()) startRecording(RecState.SENSOR_RECORDING);
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
        
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(DTalkieActivity.this);
        
        // set volume control to MUSIC
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        
        // restore autoplay option
        ToggleButton buttonAutoPlay = (ToggleButton) findViewById(R.id.buttonAutoplay);
		buttonAutoPlay.setChecked(prefs.getBoolean("autoplay", false));
		
        // restore sensor option
        final ToggleButton buttonSensor = (ToggleButton) findViewById(R.id.buttonSensor);
        buttonSensor.setChecked(prefs.getBoolean("sensor", false));
        
		buttonSensor.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(DTalkieActivity.this);
				Editor edit = prefs.edit();
				edit.putBoolean("sensor", prefs.getBoolean("sensor", false));
				edit.commit();
			}
		});
		
		// Establish a connection with the service.  We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		bindService(new Intent(DTalkieActivity.this, 
				DTalkieService.class), mConnection, Context.BIND_AUTO_CREATE);
    }
    
    @Override
	public boolean onContextItemSelected(MenuItem item) {
    	AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
    	
    	MessageDatabase db = this.service.getDatabase();
    	
    	Message msg = db.get(Folder.INBOX, info.position);
    	
    	switch (item.getItemId())
    	{
    	case R.id.itemDelete:
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
			
			// set auto-play handler
			final ToggleButton buttonAutoPlay = (ToggleButton) findViewById(R.id.buttonAutoplay);
			
			buttonAutoPlay.setOnClickListener(new OnClickListener() {
				@Override
				public void onClick(View v) {
					SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(DTalkieActivity.this);
					Editor edit = prefs.edit();
					
					if (prefs.getBoolean("autoplay", false))
					{
						edit.putBoolean("autoplay", false);
						DTalkieActivity.this.service.setAutoPlay(false);
					}
					else
					{
						edit.putBoolean("autoplay", true);
						DTalkieActivity.this.service.setAutoPlay(true);
					}
					edit.commit();
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
			
			ImageButton buttonAutoPlay = (ImageButton) findViewById(R.id.buttonAutoplay);
			buttonAutoPlay.setOnClickListener(null);
			
			service = null;
		}
	};
	
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
	    
        // Detach our existing connection.
        unbindService(mConnection);
	}
}