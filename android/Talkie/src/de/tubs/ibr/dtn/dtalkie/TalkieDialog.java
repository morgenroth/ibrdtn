
package de.tubs.ibr.dtn.dtalkie;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.LinearInterpolator;
import android.view.animation.ScaleAnimation;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import de.tubs.ibr.dtn.api.EID;
import de.tubs.ibr.dtn.dtalkie.service.RecorderService;
import de.tubs.ibr.dtn.dtalkie.service.Utils;

public class TalkieDialog extends Activity {
    
    @SuppressWarnings("unused")
    private static final String TAG = "TalkieDialog";
    
    private ImageButton mRecButton = null;
    private FrameLayout mRecIndicator = null;
    private Button mCancelButton = null;
    
    private Boolean mRecordingCompleted = false;
    
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
    	
    	// start voice recording
    	startRecording();
    }
    
    private void startRecording() {
        Bundle extras = getIntent().getExtras();
        EID destination = Utils.getEndpoint(extras, "destination", "singleton", RecorderService.TALKIE_GROUP_EID);
        
        // start recording
        RecorderService.startRecording(this, destination, true, true);
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
			        // set indicator level to zero
			        setIndicator(0.0f);
				}
				else if (RecorderService.ACTION_STOP_RECORDING.equals(action)) {
			        // set indicator level to zero
			        setIndicator(0.0f);
			        
			        // close the dialog
			        TalkieDialog.this.finish();
				}
				else if (RecorderService.ACTION_ABORT_RECORDING.equals(action)) {
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
