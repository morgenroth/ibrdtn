package de.tubs.ibr.dtn.chat;

import android.os.Bundle;
import android.preference.PreferenceActivity;

public class Preferences extends PreferenceActivity {
	@Override
	public void onCreate(Bundle savedInstanceState) {
	    super.onCreate(savedInstanceState);
	       
		addPreferencesFromResource(R.xml.preferences);
	}
}
