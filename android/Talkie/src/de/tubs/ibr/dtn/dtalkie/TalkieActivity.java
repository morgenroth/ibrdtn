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

import java.util.List;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioManager;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v4.app.FragmentActivity;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import de.tubs.ibr.dtn.dtalkie.service.HeadsetService;
import de.tubs.ibr.dtn.dtalkie.service.RecorderService;

public class TalkieActivity extends FragmentActivity {
	
	@SuppressWarnings("unused")
    private static final String TAG = "TalkieActivity";
	
	/**
	 * Listen to sensor events for automatic recording
	 */
    private SensorEventListener mSensorListener = new SensorEventListener() {
        public void onSensorChanged(SensorEvent event) {
            if (event.values.length > 0) {
                float current = event.values[0];
                float maxRange = event.sensor.getMaximumRange();
                boolean far = (current == maxRange);
                
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(TalkieActivity.this);
                if (prefs.getBoolean("sensor", false))
                {
                    if (far) {
                        // organize audio output
                        setAudioOutput();
                        
                    	// stop recording
                        RecorderService.stopRecording(TalkieActivity.this);
                    } else {
                    	// disable speaker phone when on ear
                    	AudioManager am = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
                    	am.setSpeakerphoneOn(false);
                    	
                    	// start recording
                    	RecorderService.startRecording(TalkieActivity.this, RecorderService.TALKIE_GROUP_EID, true, false);
                    }
                }
            }
        }

        public void onAccuracyChanged(Sensor sensor, int accuracy) {
        }
    };
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        // disable background service
        Intent i = new Intent(this, HeadsetService.class);
        i.setAction(HeadsetService.LEAVE_HEADSET_MODE);
        startService(i);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.pref_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.itemPreferences:
            {
                // Launch Preference activity
                Intent i = new Intent(this, Preferences.class);
                startActivity(i);
                return true;
            }
            
            case R.id.itemHeadsetMode:
            {
                // start background service
                Intent i = new Intent(this, HeadsetService.class);
                i.setAction(HeadsetService.ENTER_HEADSET_MODE);
                startService(i);
                
                // finish the activity
                finish();
                return true;
            }
        }
        return super.onOptionsItemSelected(item);
    }
    
    @Override
	protected void onPause() {
        SensorManager sm = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        sm.unregisterListener(mSensorListener);
        
        super.onPause();
	}

	@Override
	public void onResume() {
		super.onResume();
		
        SensorManager sm = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        List<Sensor> sensors = sm.getSensorList(Sensor.TYPE_PROXIMITY);
        if (sensors.size() > 0) {
            Sensor sensor = sensors.get(0);
            sm.registerListener(mSensorListener, sensor, SensorManager.SENSOR_DELAY_NORMAL);
        }
		
		// set output to speaker
		setAudioOutput();
	}

	@SuppressWarnings("deprecation")
	private void setAudioOutput() {
        AudioManager am = (AudioManager)getSystemService(Context.AUDIO_SERVICE);

        if (am.isBluetoothA2dpOn()) {
        	// play without speaker
        	am.setSpeakerphoneOn(false);
        }
        else if (am.isWiredHeadsetOn()) {
        	// play without speaker
        	am.setSpeakerphoneOn(false);
        }
        else {
        	// without headset, enable speaker
        	am.setSpeakerphoneOn(true);
        }
    }
}