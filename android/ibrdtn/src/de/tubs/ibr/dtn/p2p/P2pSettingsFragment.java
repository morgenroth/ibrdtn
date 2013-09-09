package de.tubs.ibr.dtn.p2p;

import android.annotation.TargetApi;
import android.os.Bundle;
import android.preference.PreferenceFragment;
import de.tubs.ibr.dtn.R;

@TargetApi(16)
public class P2pSettingsFragment extends PreferenceFragment {
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.prefs_p2p);
    }
}
