package de.tubs.ibr.dtn.sharebox.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
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
    private ImageView mSideIcon = null;
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
        mSideIcon = (ImageView) findViewById(R.id.sideicon);
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
        
        String title = getContext().getString(R.string.item_download_title);
        String text = getContext().getString(R.string.item_download_text);
        String side = getContext().getString(R.string.item_download_side);
        
        mLabel.setText(String.format(title, mDownload.getId(), Utils.humanReadableByteCount(mDownload.getLength(), true)));
        mBottomText.setText(String.format(text, mDownload.getBundleId().getSource().toString()));
        mSideText.setText(String.format(side, timestamp));
        
        switch (mDownload.getState()) {
        case PENDING:
        	mSideIcon.setImageResource(R.drawable.ic_state_pending);
        	mSideIcon.setVisibility(View.VISIBLE);
        	break;
        	
        case ACCEPTED:
        	mSideIcon.setImageResource(R.drawable.ic_state_accepted);
        	mSideIcon.setVisibility(View.VISIBLE);
        	break;
        	
        case DOWNLOADING:
        	mSideIcon.setVisibility(View.GONE);
        	break;
        	
        case COMPLETED:
        	mSideIcon.setImageResource(R.drawable.ic_state_completed);
        	mSideIcon.setVisibility(View.VISIBLE);
        	break;
        	
        case ABORTED:
        	mSideIcon.setImageResource(R.drawable.ic_state_aborted);
        	mSideIcon.setVisibility(View.VISIBLE);
            break;
        }
    }
}
