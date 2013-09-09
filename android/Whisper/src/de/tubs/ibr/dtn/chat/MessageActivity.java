package de.tubs.ibr.dtn.chat;

import android.content.res.Configuration;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;

public class MessageActivity extends FragmentActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        if (savedInstanceState == null) {
            // During initial setup, plug in the messages fragment.
            MessageFragment messages = new MessageFragment();
            messages.setArguments(getIntent().getExtras());
            getSupportFragmentManager().beginTransaction().add(android.R.id.content, messages).commit();
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        int screenSize = (newConfig.screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK);
        int orientation = newConfig.orientation;

        if (
                ((orientation == Configuration.ORIENTATION_LANDSCAPE) &&
                (screenSize == Configuration.SCREENLAYOUT_SIZE_LARGE) || (screenSize == Configuration.SCREENLAYOUT_SIZE_XLARGE)) &&
                (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB)
            )
        {
            // If the screen is now in landscape mode, we can show the
            // dialog in-line with the list so we don't need this activity.
            finish();
            return;
        }
    }
}
