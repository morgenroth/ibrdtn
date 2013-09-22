/*
 * SoundFXManager.java
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
package de.tubs.ibr.dtn.dtalkie.service;

import java.io.IOException;
import java.util.LinkedList;

import android.content.Context;
import android.media.AudioManager;
import android.media.SoundPool;
import android.util.Log;

public class SoundFXManager {
	
	private final static String TAG = "SoundFXManager";
	
	private int mStreamType = 0;
	private SoundPool mPool = null;
	private LinkedList<Sound> mSounds = new LinkedList<Sound>();
	
	private SoundPool.OnLoadCompleteListener mCompleteListener = new SoundPool.OnLoadCompleteListener() {
		@Override
		public void onLoadComplete(SoundPool soundPool, int sampleId, int status) {
			synchronized(mSounds) {
				for (Sound s : mSounds) {
					if (s.getSoundId() == sampleId) {
						s.setReady(true);
						mSounds.notifyAll();
						return;
					}
				}
			}
		}
	};
	
    public SoundFXManager(int streamType, int maxStreams) {
    	mStreamType = streamType;
    	mPool = new SoundPool(maxStreams, streamType, 0);
    	mPool.setOnLoadCompleteListener(mCompleteListener);
    }
    
    public void release() {
    	mPool.release();
    	mSounds.clear();
    }
	
	public void load(Context context, Sound s) {
		try {
			synchronized(mSounds) {
				int id = mPool.load(context.getAssets().openFd(s.getFilename()), 1);
				Sound local_sound = new Sound(s);
				local_sound.setSoundId(id);
				mSounds.add(local_sound);
			}
		} catch (IOException e) {
			Log.e(TAG, "sound loading failed.", e);
		}
	}
	
    public void play(Context context, Sound s) {
        /* Updated: The next 4 lines calculate the current volume in a scale of 0.0 to 1.0 */
        AudioManager mgr = (AudioManager)context.getSystemService(Context.AUDIO_SERVICE);
        float streamVolumeCurrent = mgr.getStreamVolume(mStreamType);
        float streamVolumeMax = mgr.getStreamMaxVolume(mStreamType);
        
        float volume = streamVolumeCurrent / streamVolumeMax;
        
        // get right sound id
        for (Sound ls : mSounds) {
        	if (ls.equals(s)) {
                /* Play the sound with the correct volume */
                mPool.play(ls.getSoundId(), volume, volume, 1, 0, 1f);
                return;
        	}
        }
    }
    
    public boolean isLoadCompleted() throws InterruptedException {
    	boolean ready = false;
    	
    	synchronized(mSounds) {
			ready = true;
    		for (Sound s : mSounds) {
    			if (!s.isReady()) {
    				ready = false;
    				break;
    			}
    		}
    	}
    	
    	return ready;
    }
}
