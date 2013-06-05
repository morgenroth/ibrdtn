package de.tubs.ibr.dtn.sharebox.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import de.tubs.ibr.dtn.sharebox.R;
import de.tubs.ibr.dtn.sharebox.data.Download;
import de.tubs.ibr.dtn.sharebox.data.Utils;

public class DownloadItem extends RelativeLayout {
    
    private Download mDownload = null;
    
    private TextView mLabel = null;
    private TextView mBottomText = null;
    private ImageView mIcon = null;
    private TextView mSideText = null;

    public DownloadItem(Context context) {
        super(context);
    }

    public DownloadItem(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
    
    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        mLabel = (TextView) findViewById(R.id.label);
        mBottomText = (TextView) findViewById(R.id.bottomtext);
        mIcon = (ImageView) findViewById(R.id.icon);
        mSideText = (TextView) findViewById(R.id.sidetext);
    }
    
    public Download getObject() {
        return mDownload;
    }
    
    public void bind(Download d, int position) {
        mDownload = d;
        onDataChanged();
    }
    
    public void unbind() {
        // Clear all references to the message item, which can contain attachments and other
        // memory-intensive objects
    }
    
    private void onDataChanged() {
        String timestamp = Utils.formatTimeStampString(getContext(), mDownload.getTimestamp().getTime());
        
        mLabel.setText(mDownload.getSource());
        mBottomText.setText(timestamp);
        
        mSideText.setText(String.valueOf(mDownload.getLength()));
    }
}
