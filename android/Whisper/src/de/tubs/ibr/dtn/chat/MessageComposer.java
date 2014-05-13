package de.tubs.ibr.dtn.chat;

import android.content.Context;
import android.graphics.PorterDuff.Mode;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;
import de.tubs.ibr.dtn.chat.core.Buddy;

public class MessageComposer extends LinearLayout {
	
	EditText mTextMessage = null;
	ImageButton mButton = null;
	ImageButton mButtonSecurity = null;
	Buddy mBuddy = null;
	OnActionListener mActionListener = null;
	
	public interface OnActionListener {
		public void onSecurityClick();
		public void onSend(String message);
	}
	
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
		
		// get reference to security button
		mButtonSecurity = (ImageButton) findViewById(R.id.buttonSecurity);
		mButtonSecurity.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				if (mActionListener != null) mActionListener.onSecurityClick();
			}
		});
	}
	
	public void setSecurityHint(int icon, int color) {
		if (mButtonSecurity == null) return;
		
		Drawable d = getResources().getDrawable(icon);
		d.setColorFilter(getResources().getColor(color), Mode.SRC_IN);
		
		mButtonSecurity.setVisibility(View.VISIBLE);
		mButtonSecurity.setImageDrawable(d);
	}
	
	public void setOnActionListener(OnActionListener listener) {
		mActionListener = listener;
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
	
	private void flushTextBox() {
		final String text = getMessage();
		if (text.length() > 0) {
			// call action listener
			if (mActionListener != null) mActionListener.onSend(text);
					
			// clear the text field
			mTextMessage.setText("");
		}
	}
}
