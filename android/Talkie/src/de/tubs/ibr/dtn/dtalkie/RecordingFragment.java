package de.tubs.ibr.dtn.dtalkie;

import java.io.Serializable;
import java.util.List;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioManager;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v4.app.Fragment;
import android.support.v4.view.MenuItemCompat;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.ImageButton;
import de.tubs.ibr.dtn.dtalkie.service.RecorderService;
import de.tubs.ibr.dtn.dtalkie.service.Utils;

public class RecordingFragment extends Fragment {

    @SuppressWarnings("unused")
    private static final String TAG = "RecordingFragment";
    
    private ImageButton mRecordButton = null;
    
    private SensorEventListener mSensorListener = new SensorEventListener() {
        public void onSensorChanged(SensorEvent event) {
            if (event.values.length > 0) {
                float current = event.values[0];
                float maxRange = event.sensor.getMaximumRange();
                boolean far = (current == maxRange);
                
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
                if (prefs.getBoolean("sensor", false)) 
                {
                    if (far) {
                        stopRecording();
                    } else {
                        startRecording();
                    }
                }
            }
        }

        public void onAccuracyChanged(Sensor sensor, int accuracy) {
        }
    };
    
    private OnTouchListener mTouchListener = new OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            switch ( event.getAction() ) {
                case MotionEvent.ACTION_DOWN:
                    startRecording();
                    break;
                    
                case MotionEvent.ACTION_UP:
                    stopRecording();
                    break;
            }
            
            return true;
        }
    };
    
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.recording_fragment, container, false);
        
        mRecordButton = (ImageButton) v.findViewById(R.id.button_record);
        mRecordButton.setOnTouchListener(mTouchListener);
        
        return v;
    }
    
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        
        // enable options menu
        setHasOptionsMenu(true);
        
        // set volume control to MUSIC
        getActivity().setVolumeControlStream(AudioManager.STREAM_MUSIC);
    }
    
    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.record_menu, menu);
        MenuItem autorec = menu.findItem(R.id.itemAutoRec);
        MenuItemCompat.setShowAsAction(autorec, MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
    }
    
    @Override
    public void onPrepareOptionsMenu(Menu menu) {
        MenuItem autorec = menu.findItem(R.id.itemAutoRec);
        
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());

        autorec.setIcon(prefs.getBoolean("sensor", false) ? R.drawable.ic_autorec_on : R.drawable.ic_autorec_off);
        autorec.setChecked(prefs.getBoolean("sensor", false));
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
        
        switch (item.getItemId()) {
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
            
            default:
                return super.onOptionsItemSelected(item);
        }
    }
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onPause() {
        SensorManager sm = (SensorManager) getActivity().getSystemService(Context.SENSOR_SERVICE);
        sm.unregisterListener(mSensorListener);

        // we are going out of scope - stop recording
        stopRecording();
        
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        
        SensorManager sm = (SensorManager) getActivity().getSystemService(Context.SENSOR_SERVICE);
        List<Sensor> sensors = sm.getSensorList(Sensor.TYPE_PROXIMITY);
        if (sensors.size() > 0) {
            Sensor sensor = sensors.get(0);
            sm.registerListener(mSensorListener, sensor, SensorManager.SENSOR_DELAY_NORMAL);
        }
    }
    
    private void startRecording() {
        // lock screen orientation
        Utils.lockScreenOrientation(getActivity());
        
        mRecordButton.setPressed(true);
        
        Intent rec_i = new Intent(getActivity(), RecorderService.class);
        rec_i.setAction(RecorderService.ACTION_RECORD);
        rec_i.putExtra("destination", (Serializable)RecorderService.TALKIE_GROUP_EID);
        getActivity().startService(rec_i);
    }
    
    private void stopRecording() {
        // unlock screen orientation
        Utils.unlockScreenOrientation(getActivity());
        
        mRecordButton.setPressed(false);
        
        Intent rec_i = new Intent(getActivity(), RecorderService.class);
        rec_i.setAction(RecorderService.ACTION_STOP);
        getActivity().startService(rec_i);
    }
}
