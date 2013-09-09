package de.tubs.ibr.dtn.chat;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Build;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import de.tubs.ibr.dtn.chat.core.Buddy;

public class RosterItem extends LinearLayout {
	
	@SuppressWarnings("unused")
	private static final String TAG = "RosterItem";
	
	private Buddy mBuddy = null;
	
	ImageView mImage = null;
	TextView mLabel = null;
	TextView mBottomtext = null;
	ImageView mHint = null;

    public RosterItem(Context context) {
        super(context);
    }

    public RosterItem(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
    
    public Long getBuddyId() {
    	return mBuddy.getId();
    }
    
    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

		mImage = (ImageView)findViewById(R.id.icon);
		mLabel = (TextView)findViewById(R.id.label);
		mBottomtext = (TextView)findViewById(R.id.bottomtext);
		mHint = (ImageView)findViewById(R.id.hinticon);
    }
	
	public void bind(Buddy b, int position) {
		mBuddy = b;
		onDataChanged();
	}
	
	public void unbind() {
        // Clear all references to the message item, which can contain attachments and other
        // memory-intensive objects
	}
	
	@SuppressLint("NewApi")
	private void onDataChanged() {
		mLabel.setText(mBuddy.getNickname());
		mImage.setImageResource(R.drawable.online);
		
		String presence = mBuddy.getPresence();
		
		if (presence != null)
		{
			if (presence.equalsIgnoreCase("unavailable"))
			{
				mImage.setImageResource(R.drawable.offline);
			}
			else if (presence.equalsIgnoreCase("xa"))
			{
				mImage.setImageResource(R.drawable.xa);
			}
			else if (presence.equalsIgnoreCase("away"))
			{
				mImage.setImageResource(R.drawable.away);
			}
			else if (presence.equalsIgnoreCase("dnd"))
			{
				mImage.setImageResource(R.drawable.busy);
			}
			else if (presence.equalsIgnoreCase("chat"))
			{
				mImage.setImageResource(R.drawable.online);
			}
		}
		
		// if the presence is older than 60 minutes then mark the buddy as offline
		if (!mBuddy.isOnline())
		{
			mImage.setImageResource(R.drawable.offline);
		}
		
		if (mBuddy.getStatus() != null)
		{
			if (mBuddy.getStatus().length() > 0) { 
				mBottomtext.setText(mBuddy.getStatus());
			} else {
				mBottomtext.setText(mBuddy.getEndpoint());
			}
		}
		else
		{
			mBottomtext.setText(mBuddy.getEndpoint());
		}
		
		if (mBuddy.hasDraft()) {
			mHint.setVisibility(View.VISIBLE);
			mHint.setImageResource(R.drawable.ic_draft);
		} else {
			mHint.setVisibility(View.GONE);
		}
		
		if (mBuddy.getCountry() != null) {
		    String resourceName = "ic_flag_" + mBuddy.getCountry().toLowerCase();
		    
		    Log.d(TAG, "Search for " + resourceName);
		    int flagsId = getResources().getIdentifier(resourceName, "drawable", getContext().getPackageName());
		    
		    if (flagsId == 0) {
		        mLabel.setCompoundDrawables(null, null, null, null);
		    } else {
		        mLabel.setCompoundDrawablesWithIntrinsicBounds(null, null, getResources().getDrawable(flagsId), null);
		    }
		} else {
		    mLabel.setCompoundDrawables(null, null, null, null);
		}
		
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB) {
			if (this.isActivated()) {
				mHint.setVisibility(View.VISIBLE);
				mHint.setImageResource(R.drawable.ic_selected);
			}
		}
	}
}
