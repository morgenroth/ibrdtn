
package de.tubs.ibr.dtn.sharebox.ui;

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.view.Menu;
import android.view.MenuItem;
import de.tubs.ibr.dtn.sharebox.R;

public class TransferListActivity extends FragmentActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_transfer_list);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.transfer_list, menu);
        return true;
    }

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_settings:
            {
                // Launch Preference activity
                Intent i = new Intent(this, SettingsActivity.class);
                startActivity(i);
                return true;
            }
        }
        return super.onMenuItemSelected(featureId, item);
    }
}
