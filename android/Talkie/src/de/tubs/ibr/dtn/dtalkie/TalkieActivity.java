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

import android.os.Bundle;
import android.support.v4.app.FragmentActivity;

public class TalkieActivity extends FragmentActivity {
	
	@SuppressWarnings("unused")
    private static final String TAG = "TalkieActivity";
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
    }
    
	@Override
	protected void onPause() {
//		unlockScreenOrientation();

		super.onPause();
	}

	@Override
	protected void onResume() {
//        lockCurrentScreenOrientation();
        
		super.onResume();
	}
    
//    private void lockCurrentScreenOrientation() {
//    	switch (this.getResources().getConfiguration().orientation) {
//    	case Configuration.ORIENTATION_LANDSCAPE:
//    		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
//    		break;
//    		
//    	case Configuration.ORIENTATION_PORTRAIT:
//    		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
//    		break;
//    		
//    	default:
//    		setRequestedOrientation(getRequestedOrientation());
//    		break;
//    	}
//    }
//    
//    private void unlockScreenOrientation() {
//        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_USER);
//    }
}