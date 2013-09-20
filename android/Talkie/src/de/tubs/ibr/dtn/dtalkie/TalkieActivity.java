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

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import de.tubs.ibr.dtn.dtalkie.service.HeadsetService;

public class TalkieActivity extends FragmentActivity {
	
	@SuppressWarnings("unused")
    private static final String TAG = "TalkieActivity";
	
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
}