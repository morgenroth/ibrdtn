
package de.tubs.ibr.dtn.dtalkie;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.os.Bundle;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.LinearInterpolator;
import android.view.animation.ScaleAnimation;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import de.tubs.ibr.dtn.api.EID;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.dtalkie.service.RecorderService;
import de.tubs.ibr.dtn.dtalkie.service.Sound;
import de.tubs.ibr.dtn.dtalkie.service.SoundFXManager;
import de.tubs.ibr.dtn.dtalkie.service.Utils;

public class TalkieDialog extends Activity {
    
    @SuppressWarnings("unused")
    private static final String TAG = "TalkieDialog";
    
    private ImageButton mRecButton = null;
    private FrameLayout mRecIndicator = null;
    private Button mCancelButton = null;
    
    private Boolean mRecordingCompleted = false;
    
    private SoundFXManager mSoundManager = null;
    
    private float mAnimScaleX = 1.0f;
    private float mAnimScaleY = 1.0f;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.recording_dialog);
        
        mRecButton = (ImageButton)findViewById(R.id.button_record);
        mRecButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // stop recording
                mRecordingCompleted = true;
                stopRecording();
            }
        });
        
        mRecIndicator = (FrameLayout)findViewById(R.id.record_indicator);
        
        mCancelButton = (Button)findViewById(R.id.cancel);
        mCancelButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // abort recording
                stopRecording();
            }
        });
        
        // init sound pool
        mSoundManager = new SoundFXManager(AudioManager.STREAM_VOICE_CALL, 2);
        
        mSoundManager.load(this, Sound.BEEP);
        mSoundManager.load(this, Sound.QUIT);
        mSoundManager.load(this, Sound.SQUELSH_LONG);
        mSoundManager.load(this, Sound.SQUELSH_SHORT);
    }
    
    @Override
	protected void onDestroy() {
	    // free sound manager resources
	    mSoundManager.release();
	    
		super.onDestroy();
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

	@Override
    protected void onPause() {
        // we are going out of scope - stop recording
        stopRecording();
        
        // unregister from recorder events
        unregisterReceiver(mRecorderEventReceiver);
        
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        
    	IntentFilter filter = new IntentFilter();
    	filter.addAction(RecorderService.EVENT_RECORDING_EVENT);
    	filter.addAction(RecorderService.EVENT_RECORDING_INDICATOR);
    	registerReceiver(mRecorderEventReceiver, filter);
    	
        // set output to speaker
		setAudioOutput();
    	
    	// start voice recording
    	startRecording();
    }
    
    private void startRecording() {
    	EID destination = null;
    	Bundle extras = getIntent().getExtras();
    	
    	if ( de.tubs.ibr.dtn.Intent.ACTION_NEIGHBOR.equals( getIntent().getAction() ) ) {
    		// check if an endpoint exists
    		if (extras.containsKey(de.tubs.ibr.dtn.Intent.EXTRA_KEY_ENDPOINT)) {
    			// extract endpoint
    			String endpoint = extras.getString(de.tubs.ibr.dtn.Intent.EXTRA_KEY_ENDPOINT);
    			
    			// add application endpoint to different EID schemes
    			if (endpoint.startsWith("dtn:")) {
    				// add dtn application path for 'dtalkie'
    				destination = new SingletonEndpoint(endpoint + "/dtalkie");
    			}
    			else if (endpoint.startsWith("ipn:")) {
    				// add ipn application number for 'dtalkie'
    				destination = new SingletonEndpoint(endpoint + ".3066158454");
    			}
    		}
    	}
    	else {
    		destination = Utils.getEndpoint(extras, "destination", "singleton", RecorderService.TALKIE_GROUP_EID);
    	}
    	
    	// abort if not destination is set
    	if (destination == null) {
    		// stop recording
    		stopRecording();
    	} else {
			// start recording
			RecorderService.startRecording(this, destination, true, true);
    	}
    }
    
    private void stopRecording() {
        // stop recording
        if (mRecordingCompleted)
        	RecorderService.stopRecording(this);
        else
        	RecorderService.abortRecording(this);
    }
    
    private BroadcastReceiver mRecorderEventReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			if (RecorderService.EVENT_RECORDING_EVENT.equals(intent.getAction())) {
				String action = intent.getStringExtra(RecorderService.EXTRA_RECORDING_ACTION);
				
				if (RecorderService.ACTION_START_RECORDING.equals(action)) {
			        // make a noise
					mSoundManager.play(TalkieDialog.this, Sound.BEEP);

			        // set indicator level to zero
			        setIndicator(0.0f);
				}
				else if (RecorderService.ACTION_STOP_RECORDING.equals(action)) {
					// make a noise
					mSoundManager.play(TalkieDialog.this, Sound.QUIT);

			        // set indicator level to zero
			        setIndicator(0.0f);
			        
			        // close the dialog
			        TalkieDialog.this.finish();
				}
				else if (RecorderService.ACTION_ABORT_RECORDING.equals(action)) {
					// make a noise
					mSoundManager.play(TalkieDialog.this, Sound.SQUELSH_SHORT);

			        // set indicator level to zero
			        setIndicator(0.0f);
			        
			        // close the dialog
			        TalkieDialog.this.finish();
				}
			}
			else if (RecorderService.EVENT_RECORDING_INDICATOR.equals(intent.getAction())) {
				// read indicator value
				float level = intent.getFloatExtra(RecorderService.EXTRA_LEVEL_NORMALIZED, 0.0f);
				
		        // set indicator level
		        setIndicator(level);
			}
		}
    	
    };
    
    private void setIndicator(Float level) {
    	float newScaleX = 1.0f + level;
    	float newScaleY = 1.0f + level;
        
        Animation a = new ScaleAnimation(mAnimScaleX, newScaleX, mAnimScaleY, newScaleY, Animation.RELATIVE_TO_SELF, 0.5f, Animation.RELATIVE_TO_SELF, 0.5f);
        
        mAnimScaleX = newScaleX;
        mAnimScaleY = newScaleY;

        a.setDuration(100);
        a.setInterpolator(new LinearInterpolator());
        a.startNow();
        mRecIndicator.startAnimation(a);
    }
}
