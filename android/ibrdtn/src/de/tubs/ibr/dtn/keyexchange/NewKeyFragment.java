package de.tubs.ibr.dtn.keyexchange;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.PorterDuff.Mode;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Parcelable;
import android.os.RemoteException;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.service.DaemonService;

public class NewKeyFragment extends Fragment {
	
	private static final String TAG = "NewKeyFragment";
	
	private KeyExchangeManager mManager = null;
	
	private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder s) {
        	mManager = (KeyExchangeManager)s;
            
            try {
            	Bundle keyinfo = mManager.getKeyInfo(mEndpoint);
            	textOldKey.setText(Utils.hashToReadableString(keyinfo.getString("fingerprint")));
            } catch (RemoteException e) {
            	// error
            	Log.e(TAG, "", e);
            }
            
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service connected");
        }

        public void onServiceDisconnected(ComponentName name) {
        	mManager = null;
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service disconnected");
        }
    };
    
	private SingletonEndpoint mEndpoint;
	private int mSession;
	private int mProtocol;
	private String mData;
	
	private TextView textOldKey;
	
	public static NewKeyFragment create(Intent intent, SingletonEndpoint endpoint) {
		NewKeyFragment f = new NewKeyFragment();
		Bundle args = new Bundle();
		args.putParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, endpoint);
		args.putInt("session", intent.getIntExtra("session", -1));
		args.putInt("protocol", intent.getIntExtra("protocol", -1));
		args.putString("data", intent.getStringExtra("data"));
		f.setArguments(args);
		return f;
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.new_key_fragment, container, false);
		
		mEndpoint = getArguments().getParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT);
		mSession = getArguments().getInt("session", -1);
		mProtocol = getArguments().getInt("protocol", -1);
		mData = getArguments().getString("data");
		
		TextView textInfo = (TextView) v.findViewById(R.id.textViewInfo);
		textInfo.setText(getString(R.string.key_selection_request));
		
		// OLD KEY
		textOldKey = (TextView) v.findViewById(R.id.textViewOldKey);
		
		ImageView trustLevelImageOldKey1 = (ImageView) v.findViewById(R.id.imageViewTrustLevelOldKey1);
		ImageView trustLevelImageOldKey2 = (ImageView) v.findViewById(R.id.imageViewTrustLevelOldKey2);
		ImageView trustLevelImageOldKey3 = (ImageView) v.findViewById(R.id.imageViewTrustLevelOldKey3);
		
		int trustLevel = 0;
		
		try {
			Bundle keyinfo = mManager.getKeyInfo(mEndpoint);
			trustLevel = keyinfo.getInt("trustlevel");
		} catch (RemoteException e) {
			Log.e(TAG, "", e);
		}
		
		if (trustLevel > 67) {
    		Drawable d = getResources().getDrawable(R.drawable.ic_security_closed).mutate();
    		d.setColorFilter(getResources().getColor(R.color.light_green), Mode.MULTIPLY);
    		trustLevelImageOldKey1.setImageDrawable(d);
    		trustLevelImageOldKey2.setImageDrawable(d);
    		trustLevelImageOldKey3.setImageDrawable(d);
        }
        else if (trustLevel > 33) {
    		Drawable d = getResources().getDrawable(R.drawable.ic_security_closed).mutate();
    		d.setColorFilter(getResources().getColor(R.color.light_yellow), Mode.MULTIPLY);
    		trustLevelImageOldKey1.setImageDrawable(d);
    		trustLevelImageOldKey2.setImageDrawable(d);
    		
    		d = getResources().getDrawable(R.drawable.ic_action_security_open);
    		trustLevelImageOldKey3.setImageDrawable(d);
        }
        else if (trustLevel > 0) {
    		Drawable d = getResources().getDrawable(R.drawable.ic_security_closed).mutate();
    		d.setColorFilter(getResources().getColor(R.color.light_red), Mode.MULTIPLY);
    		trustLevelImageOldKey1.setImageDrawable(d);
    		
    		d = getResources().getDrawable(R.drawable.ic_action_security_open);
    		trustLevelImageOldKey2.setImageDrawable(d);
    		trustLevelImageOldKey3.setImageDrawable(d);
        }
        else {
    		Drawable d = getResources().getDrawable(R.drawable.ic_action_security_open).mutate();
    		trustLevelImageOldKey1.setImageDrawable(d);
    		trustLevelImageOldKey2.setImageDrawable(d);
    		trustLevelImageOldKey3.setImageDrawable(d);
        }
		
		// NEW KEY
		TextView textNewKey = (TextView) v.findViewById(R.id.textViewNewKey);
		textNewKey.setText(Utils.hashToReadableString(mData));
		
		ImageView trustLevelImageNewKey1 = (ImageView) v.findViewById(R.id.imageViewTrustLevelNewKey1);
		ImageView trustLevelImageNewKey2 = (ImageView) v.findViewById(R.id.imageViewTrustLevelNewKey2);
		ImageView trustLevelImageNewKey3 = (ImageView) v.findViewById(R.id.imageViewTrustLevelNewKey3);
		
		if (mProtocol == 5 || mProtocol == 4) {
    		Drawable d = getResources().getDrawable(R.drawable.ic_security_closed).mutate();
    		d.setColorFilter(getResources().getColor(R.color.light_green), Mode.MULTIPLY);
    		trustLevelImageNewKey1.setImageDrawable(d);
    		trustLevelImageNewKey2.setImageDrawable(d);
    		trustLevelImageNewKey3.setImageDrawable(d);
        }
        else if (mProtocol == 3 || mProtocol == 2) {
    		Drawable d = getResources().getDrawable(R.drawable.ic_security_closed).mutate();
    		d.setColorFilter(getResources().getColor(R.color.light_yellow), Mode.MULTIPLY);
    		trustLevelImageNewKey1.setImageDrawable(d);
    		trustLevelImageNewKey2.setImageDrawable(d);
    		
    		d = getResources().getDrawable(R.drawable.ic_action_security_open);
    		trustLevelImageNewKey3.setImageDrawable(d);
        }
        else if (mProtocol == 1 || mProtocol == 0) {
    		Drawable d = getResources().getDrawable(R.drawable.ic_security_closed).mutate();
    		d.setColorFilter(getResources().getColor(R.color.light_red), Mode.MULTIPLY);
    		trustLevelImageNewKey1.setImageDrawable(d);
    		
    		d = getResources().getDrawable(R.drawable.ic_action_security_open);
    		trustLevelImageNewKey2.setImageDrawable(d);
    		trustLevelImageNewKey3.setImageDrawable(d);
        }
        else {
    		Drawable d = getResources().getDrawable(R.drawable.ic_action_security_open).mutate();
    		trustLevelImageNewKey1.setImageDrawable(d);
    		trustLevelImageNewKey2.setImageDrawable(d);
    		trustLevelImageNewKey3.setImageDrawable(d);
        }
		
		// BUTTONS
		Button buttonOldKey = (Button) v.findViewById(R.id.buttonOld);
		buttonOldKey.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				Intent startIntent = new Intent(getActivity(), DaemonService.class);
				startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_GIVE_NEW_KEY_RESPONSE);
				startIntent.putExtra("session", mSession);
				startIntent.putExtra("newKey", 0);
				startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
				
				getActivity().startService(startIntent);
				
				getFragmentManager().popBackStack();
				Toast.makeText(getActivity(), getString(R.string.key_dropped), Toast.LENGTH_SHORT).show();
			}
		});
		
		Button buttonNewKey = (Button) v.findViewById(R.id.buttonNew);
		buttonNewKey.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				Intent startIntent = new Intent(getActivity(), DaemonService.class);
				startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_GIVE_NEW_KEY_RESPONSE);
				startIntent.putExtra("session", mSession);
				startIntent.putExtra("newKey", 1);
				startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
				
				getActivity().startService(startIntent);
				
				FragmentTransaction ft = getFragmentManager().beginTransaction();
		    	ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
		        ft.replace(R.id.keyExchange, new SuccessFragment()).addToBackStack(null);
		        ft.commit();
			}
		});
		
		return v;
	}
	
	@Override
	public void onResume() {
		super.onResume();
        
        // Establish a connection with the service. We use an explicit
        // class name because we want a specific service implementation that
        // we know will be running in our own process (and thus won't be
        // supporting component replacement by other applications).
		Intent bindIntent = DaemonService.createKeyExchangeManagerIntent(getActivity());
        getActivity().bindService(bindIntent, mConnection, Context.BIND_AUTO_CREATE);
	}
	
	@Override
	public void onPause() {
        // Detach our existing connection.
        getActivity().unbindService(mConnection);
        
		super.onPause();
	}

}
