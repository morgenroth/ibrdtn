package de.tubs.ibr.dtn.sharebox.ui;

import android.os.Bundle;
import android.support.v4.app.FragmentActivity;

public class ShareWithActivity extends FragmentActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        if (savedInstanceState == null) {
            // During initial setup, plug in the messages fragment.
            ShareWithFragment share = new ShareWithFragment();
            share.setArguments(getIntent().getExtras());
            getSupportFragmentManager().beginTransaction().add(android.R.id.content, share).commit();
        }
    }
    
}
