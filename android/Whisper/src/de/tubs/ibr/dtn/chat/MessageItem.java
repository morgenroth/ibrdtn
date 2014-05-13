package de.tubs.ibr.dtn.chat;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
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
	ImageView mSigned = null;
	ImageView mEncrypted = null;
	LinearLayout mMessageBlock = null;

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
		mSigned = (ImageView) findViewById(R.id.signed_indicator);
		mDetails = (ImageView) findViewById(R.id.details_indicator);
		mEncrypted = (ImageView) findViewById(R.id.encrypted_indicator);
		mMessageBlock = (LinearLayout) findViewById(R.id.chat_block);
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
			if (mMessage.hasFlag(Message.FLAG_DELIVERED)) {
				mDelivered.setVisibility(View.VISIBLE);
			} else {
				mDelivered.setVisibility(View.GONE);
			}
			
			if (mMessage.hasFlag(Message.FLAG_SENT)) {
				mPending.setVisibility(View.GONE);
			} else {
				mPending.setVisibility(View.VISIBLE);
			}
		}
		else
		{
			if (mMessage.hasFlag(Message.FLAG_SIGNED)) {
				mSigned.setVisibility(View.VISIBLE);
			} else {
				mSigned.setVisibility(View.GONE);
			}
			
			if (mMessage.hasFlag(Message.FLAG_ENCRYPTED)) {
				mEncrypted.setVisibility(View.VISIBLE);
			} else {
				mEncrypted.setVisibility(View.GONE);
			}
		}
		
		mLabel.setText(date);
		mText.setText(mMessage.getPayload());
	}
}
