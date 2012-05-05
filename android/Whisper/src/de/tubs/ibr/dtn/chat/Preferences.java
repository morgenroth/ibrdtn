package de.tubs.ibr.dtn.chat;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import de.tubs.ibr.dtn.chat.service.EventReceiver;

public class Preferences extends PreferenceActivity {
	@Override
	public void onCreate(Bundle savedInstanceState) {
	    super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.preferences);

		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		prefs.registerOnSharedPreferenceChangeListener(pref_listener);
	}
	
	@Override
	protected void onDestroy() {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		prefs.unregisterOnSharedPreferenceChangeListener(pref_listener);
		super.onDestroy();
	}

	SharedPreferences.OnSharedPreferenceChangeListener pref_listener = new SharedPreferences.OnSharedPreferenceChangeListener() {
		@Override
		public void onSharedPreferenceChanged(SharedPreferences prefs, String text) {
			if (text.equals("checkBroadcastPresence")) {
				if (prefs.getBoolean(text, false)) {
					EventReceiver.activatePresenceGenerator(Preferences.this);
				} else {
					EventReceiver.deactivatePresenceGenerator(Preferences.this);
				}
			}
		}
	};
}
