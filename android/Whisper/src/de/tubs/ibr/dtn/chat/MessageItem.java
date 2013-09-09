package de.tubs.ibr.dtn.chat;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import de.tubs.ibr.dtn.chat.core.Message;

public class MessageItem extends RelativeLayout {
	
	@SuppressWarnings("unused")
	private static final String TAG = "MessageItem";
	
	private Message mMessage = null;
	
	TextView mLabel = null;
	TextView mText = null;
	ImageView mPending = null;
	ImageView mDelivered = null;
	ImageView mDetails = null;

    public MessageItem(Context context) {
        super(context);
    }

    public MessageItem(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
    
    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

		mLabel = (TextView) findViewById(R.id.label);
		mText = (TextView) findViewById(R.id.text);
		mPending = (ImageView) findViewById(R.id.pending_indicator);
		mDelivered = (ImageView) findViewById(R.id.delivered_indicator);
		mDetails = (ImageView) findViewById(R.id.details_indicator);
    }
    
	public void bind(Message m, int position) {
		mMessage = m;
		onDataChanged();
	}
	
	public void unbind() {
        // Clear all references to the message item, which can contain attachments and other
        // memory-intensive objects
	}
	
	private void onDataChanged() {

		String date = MessageAdapter.formatTimeStampString(getContext(), mMessage.getCreated().getTime());

		// set details invisible
		mDetails.setVisibility(View.GONE);
		
		if (!mMessage.isIncoming())
		{
			if (mMessage.getFlags() == 0) {
				mPending.setVisibility(View.VISIBLE);
				mDelivered.setVisibility(View.GONE);
			} else if ((mMessage.getFlags() & 2) > 0) {
				mPending.setVisibility(View.GONE);
				mDelivered.setVisibility(View.VISIBLE);
			} else {
				mPending.setVisibility(View.GONE);
				mDelivered.setVisibility(View.GONE);
			}
		}
		
		mLabel.setText(date);
		mText.setText(mMessage.getPayload());
	}
}
