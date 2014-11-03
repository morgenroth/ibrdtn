package de.tubs.ibr.dtn.dtalkie.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.view.KeyEvent;

public class MediaButtonReceiver extends BroadcastReceiver {
	
	@Override
	public void onReceive(Context context, Intent intent) {
		String intentAction = intent.getAction();
		
		if (AudioManager.ACTION_AUDIO_BECOMING_NOISY.equals(intentAction)) {
			Intent i = new Intent(context, HeadsetService.class);
			i.setAction(HeadsetService.LEAVE_HEADSET_MODE);
			context.startService(i);
		}
		else if (Intent.ACTION_MEDIA_BUTTON.equals(intentAction)) {
			KeyEvent event = (KeyEvent)intent.getParcelableExtra(Intent.EXTRA_KEY_EVENT);
			
			if (event == null) {
				return;
			}
			
			int keycode = event.getKeyCode();
			int action = event.getAction();
			
			// single quick press: record/stop
			
			String command = null;
			switch (keycode) {
				case KeyEvent.KEYCODE_MEDIA_STOP:
					command = HeadsetService.ACTION_STOP_RECORD;
					break;
				case KeyEvent.KEYCODE_MEDIA_PLAY:
					command = HeadsetService.ACTION_START_RECORD;
					break;
				case KeyEvent.KEYCODE_MEDIA_PAUSE:
					command = HeadsetService.ACTION_STOP_RECORD;
					break;
				case KeyEvent.KEYCODE_HEADSETHOOK:
				case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
					command = HeadsetService.ACTION_TOGGLE_RECORD;
					break;
				case KeyEvent.KEYCODE_MEDIA_NEXT:
					command = HeadsetService.ACTION_PLAY_ALL;
				default:
					break;
			}
			
			if (command != null) {
				if (action == KeyEvent.ACTION_DOWN) {
					if (event.getRepeatCount() == 0) {
						// only consider the first event in a sequence, not the repeat events,
						// so that we don't trigger in cases where the first event went to
						// a different app (e.g. when the user ends a phone call by
						// long pressing the headset button)
						// The service may or may not be running, but we need to send it
						// a command.
						Intent i = new Intent(context, HeadsetService.class);
						i.setAction(command);
						context.startService(i);
					}
				}
				if (isOrderedBroadcast()) {
					abortBroadcast();
				}
			}
		}
	}

}
