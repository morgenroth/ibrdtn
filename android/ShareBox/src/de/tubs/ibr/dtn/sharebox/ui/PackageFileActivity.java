package de.tubs.ibr.dtn.sharebox.ui;

import android.os.Bundle;
import android.support.v4.app.FragmentActivity;

public class PackageFileActivity extends FragmentActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        if (savedInstanceState == null) {
            // During initial setup, plug in the messages fragment.
            PackageFileListFragment files = new PackageFileListFragment();
            files.setArguments(getIntent().getExtras());
            getSupportFragmentManager().beginTransaction().add(android.R.id.content, files).commit();
        }
    }

}
