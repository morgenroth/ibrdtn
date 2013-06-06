package de.tubs.ibr.dtn.sharebox.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import de.tubs.ibr.dtn.sharebox.R;
import de.tubs.ibr.dtn.sharebox.data.PackageFile;
import de.tubs.ibr.dtn.sharebox.data.Utils;

public class PackageFileItem extends RelativeLayout {
    
    @SuppressWarnings("unused")
    private PackageFile mFile = null;
    
    private TextView mLabel = null;
    private TextView mBottomText = null;
    private ImageView mIcon = null;
    private TextView mSideText = null;

    public PackageFileItem(Context context) {
        super(context);
    }

    public PackageFileItem(Context context, AttributeSet attrs) {
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
    
    public PackageFile getObject() {
    	return mFile;
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
        mLabel.setText(mFile.getFile().getName());
        mBottomText.setText(Utils.humanReadableByteCount(mFile.getLength(), true));
    }
}