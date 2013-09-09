package de.tubs.ibr.dtn.p2p;

import android.annotation.TargetApi;
import android.app.Activity;
import android.os.Bundle;

@TargetApi(16)
public class P2pSettingsActivity extends Activity {
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, new P2pSettingsFragment())
                .commit();
    }

}
