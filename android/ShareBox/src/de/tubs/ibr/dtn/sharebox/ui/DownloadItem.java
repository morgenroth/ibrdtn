package de.tubs.ibr.dtn.sharebox.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.RelativeLayout;
import de.tubs.ibr.dtn.sharebox.data.Download;

public class DownloadItem extends RelativeLayout {
    
    private Download mDownload = null;

    public DownloadItem(Context context) {
        super(context);
    }

    public DownloadItem(Context context, AttributeSet attrs) {
        super(context, attrs);
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
        
    }
}
