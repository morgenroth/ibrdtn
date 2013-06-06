
package de.tubs.ibr.dtn.sharebox.ui;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.text.Html;
import android.view.Menu;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.sharebox.DtnService;
import de.tubs.ibr.dtn.sharebox.R;

public class PendingDialog extends Activity {
    
    private Button mButtonReject = null;
    private Button mButtonAccept = null;
    private Button mButtonMiddle = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_pending_dialog);

        mButtonReject = (Button)findViewById(R.id.button1);
        mButtonAccept = (Button)findViewById(R.id.button2);
        mButtonMiddle = (Button)findViewById(R.id.button3);
        
        // hide button 3
        mButtonMiddle.setVisibility(View.GONE);
        
        // add accept / reject text
        mButtonAccept.setText(getString(R.string.button_accept));
        mButtonReject.setText(getString(R.string.button_reject));
        
        // if there is no bundle id, finish this activity
        if (!getIntent().hasExtra(DtnService.PARCEL_KEY_BUNDLE_ID)) finish();
        
        // get the bundle id
        final BundleID bundleid = getIntent().getParcelableExtra(DtnService.PARCEL_KEY_BUNDLE_ID);
        final Long length = getIntent().getLongExtra(DtnService.PARCEL_KEY_LENGTH, 0L);
        
        mButtonAccept.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent acceptIntent = new Intent(PendingDialog.this, DtnService.class);
                acceptIntent.setAction(DtnService.ACCEPT_DOWNLOAD_INTENT);
                acceptIntent.putExtra(DtnService.PARCEL_KEY_BUNDLE_ID, bundleid);
                PendingDialog.this.startService(acceptIntent);
                
                // close the dialog
                finish();
            }
        });
        
        mButtonReject.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent dismissIntent = new Intent(PendingDialog.this, DtnService.class);
                dismissIntent.setAction(DtnService.REJECT_DOWNLOAD_INTENT);
                dismissIntent.putExtra(DtnService.PARCEL_KEY_BUNDLE_ID, bundleid);
                PendingDialog.this.startService(dismissIntent);
                
                // close the dialog
                finish();
            }
        });
        
        String htmlMessage = "<b>BundleID:</b> " + bundleid.getSource().toString() + "<br />" +
                "<b>Length:</b> " + length.toString();
        
        // add content to the dialog
        TextView msg = (TextView) findViewById(R.id.message);
        msg.setText(Html.fromHtml(htmlMessage));
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.pending_dialog, menu);
        return true;
    }

}
