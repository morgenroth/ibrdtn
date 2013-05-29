package de.tubs.ibr.dtn.chat;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;
import de.tubs.ibr.dtn.chat.service.ChatService;

public class MessageComposer extends LinearLayout {
	
	EditText mTextMessage = null;
	ImageButton mButton = null;
	Long mBuddyId = null;
	
	public MessageComposer(Context context) {
		super(context);
	}
	
    public MessageComposer(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
    
    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

		// set "enter" handler
        mTextMessage = (EditText) findViewById(R.id.textMessage);
        mTextMessage.setEnabled(false);
		
        mTextMessage.setOnKeyListener(new OnKeyListener() {
			public boolean onKey(View v, int keycode, KeyEvent event) {
				if ((KeyEvent.KEYCODE_ENTER == keycode) && (event.getAction() == KeyEvent.ACTION_DOWN))
				{
					flushTextBox();
					return true;
				}
				return false;
			}
		});
		
        mTextMessage.setOnEditorActionListener(new OnEditorActionListener() {
		    public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
		        if (actionId == EditorInfo.IME_ACTION_SEND) {
					flushTextBox();
					return true;
				}
				return false;
		    }
		});
		
		// set send handler
		mButton = (ImageButton) findViewById(R.id.buttonSend);
		mButton.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				flushTextBox();
			}
		});
    }
    
    public void setBuddyId(Long buddyId) {
    	mBuddyId = buddyId;
    }
    
	public void setMessage(String text) {
		mTextMessage.setText(text);
	}
	
	public String getMessage() {
		return mTextMessage.getText().toString();
	}
	
	@Override
	public void setEnabled(boolean enabled) {
		mTextMessage.setEnabled(enabled);
		mButton.setEnabled(enabled);
		super.setEnabled(enabled);
	}
	
	private void flushTextBox()
	{
		final String text = getMessage();
		if (text.length() > 0) {
	        final Intent intent = new Intent(getContext(), ChatService.class);
	        intent.setAction(ChatService.ACTION_SEND_MESSAGE);
	        intent.putExtra("buddyId", mBuddyId);
	        intent.putExtra("text", text);

			getContext().startService(intent);
					
			// clear the text field
			mTextMessage.setText("");
		}
	}
}
