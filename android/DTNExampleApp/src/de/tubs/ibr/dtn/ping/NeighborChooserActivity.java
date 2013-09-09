package de.tubs.ibr.dtn.ping;

import android.os.Bundle;
import android.support.v4.app.FragmentActivity;

public class NeighborChooserActivity extends FragmentActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        if (savedInstanceState == null) {
            // During initial setup, plug in the messages fragment.
            NeighborChooserFragment chooser = new NeighborChooserFragment();
            chooser.setArguments(getIntent().getExtras());
            getSupportFragmentManager().beginTransaction().add(android.R.id.content, chooser).commit();
        }
    }
    
}
