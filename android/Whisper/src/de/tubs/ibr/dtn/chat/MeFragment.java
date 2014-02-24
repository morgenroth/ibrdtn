package de.tubs.ibr.dtn.chat;

import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class MeFragment extends Fragment implements View.OnClickListener {
	
	private final static String TAG = "MeFragment";
	
	TextView mNickname = null;
	TextView mStatusMsg = null;
	ImageView mState = null;

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.me_fragment, container, false);
		
		RelativeLayout me_layout = (RelativeLayout)v.findViewById(R.id.roster_me_display);
		me_layout.setOnClickListener(this);
		
		mNickname = (TextView)v.findViewById(R.id.me_nickname);
		mStatusMsg = (TextView)v.findViewById(R.id.me_statusmessage);
		mState = (ImageView)v.findViewById(R.id.me_icon);
		
		return v;
	}
	
	@Override
	public void onPause() {
		super.onPause();
	}

	@Override
	public void onResume() {
		super.onResume();
		onPreferencesChanged();
	}

	@Override
	public void onClick(View v) {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
	    String presence_tag = prefs.getString("presencetag", "auto");
	    String presence_text = prefs.getString("statustext", "");
	    
		MeDialog dialog = MeDialog.newInstance(presence_tag, presence_text);

		dialog.setOnChangeListener(new MeDialog.OnChangeListener() {
			public void onStateChanged(String presence, String message) {
				Log.d(TAG, "state changed: " + presence + ", " + message);
				SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
				Editor edit = prefs.edit();
				edit.putString("presencetag", presence);
				edit.putString("statustext", message);
				edit.commit();
				onPreferencesChanged();
			}
		});
		
		dialog.show(getActivity().getSupportFragmentManager(), "me");
	}

	private void onPreferencesChanged() {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
	    String presence_tag = prefs.getString("presencetag", "auto");
	    String presence_nick = prefs.getString("editNickname", "Nobody");
	    String presence_text = prefs.getString("statustext", "");
	    
	    int presence_icon = R.drawable.online;
		if (presence_tag != null)
		{
			if (presence_tag.equalsIgnoreCase("unavailable"))
			{
				presence_icon = R.drawable.offline;
			}
			else if (presence_tag.equalsIgnoreCase("xa"))
			{
				presence_icon = R.drawable.xa;
			}
			else if (presence_tag.equalsIgnoreCase("away"))
			{
				presence_icon = R.drawable.away;
			}
			else if (presence_tag.equalsIgnoreCase("dnd"))
			{
				presence_icon = R.drawable.busy;
			}
			else if (presence_tag.equalsIgnoreCase("chat"))
			{
				presence_icon = R.drawable.online;
			}
		}
		mState.setImageResource(presence_icon);
		mNickname.setText(presence_nick);
		
		if (presence_text.length() > 0) {
			mStatusMsg.setText(presence_text);
		} else {
			mStatusMsg.setText(this.getResources().getText(R.string.no_status_message));
		}
	}
}
