package de.tubs.ibr.dtn.p2p;

import android.annotation.*;
import android.app.*;
import android.os.*;

@TargetApi(14)
public class P2pSettingsActivity extends Activity {
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, new P2pSettingsFragment())
                .commit();
    }

}
