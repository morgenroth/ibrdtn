package de.tubs.ibr.dtn.p2p;

import de.tubs.ibr.dtn.*;
import android.annotation.TargetApi;
import android.os.*;
import android.preference.*;

@TargetApi(14)
public class P2pSettingsFragment extends PreferenceFragment {
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.prefs_p2p);
    }
}
