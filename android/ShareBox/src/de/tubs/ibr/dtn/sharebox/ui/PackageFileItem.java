package de.tubs.ibr.dtn.sharebox.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.RelativeLayout;
import de.tubs.ibr.dtn.sharebox.data.PackageFile;

public class PackageFileItem extends RelativeLayout {
    
    @SuppressWarnings("unused")
    private PackageFile mFile = null;

    public PackageFileItem(Context context) {
        super(context);
    }

    public PackageFileItem(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
    
    public void bind(PackageFile f, int position) {
        mFile = f;
        onDataChanged();
    }
    
    public void unbind() {
        // Clear all references to the message item, which can contain attachments and other
        // memory-intensive objects
    }
    
    private void onDataChanged() {
        
    }
}