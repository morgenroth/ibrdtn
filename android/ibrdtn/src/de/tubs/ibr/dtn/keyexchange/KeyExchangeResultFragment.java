
package de.tubs.ibr.dtn.keyexchange;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.PorterDuff.Mode;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.service.DaemonService;

public class KeyExchangeResultFragment extends Fragment {
	
	public final static String TAG = "KeyExchangeResultFrg.";
	
	private KeyExchangeManager mManager = null;
	private Bundle mKeyInfo = null;
	private SingletonEndpoint mEndpoint;
	
	private ImageView trustLevelImage1;
	private ImageView trustLevelImage2;
	private ImageView trustLevelImage3;
	
	private ImageView imageResult;
	private TextView textResult;
	private TextView textEndpoint;
	
	public static KeyExchangeResultFragment create(String action, SingletonEndpoint endpoint) {
		KeyExchangeResultFragment f = new KeyExchangeResultFragment();
		Bundle args = new Bundle();
		args.putParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, endpoint);
		args.putString(KeyExchangeService.EXTRA_ACTION, action);
		
		f.setArguments(args);
		return f;
	}
	
	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName name, IBinder s) {
			mManager = KeyExchangeManager.Stub.asInterface(s);
			
			refresh();
			if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service connected");
		}

		public void onServiceDisconnected(ComponentName name) {
			mManager = null;
			if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service disconnected");
		}
	};
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.key_exchange_result_fragment, container, false);
		
		trustLevelImage1 = (ImageView) v.findViewById(R.id.viewTrustMarker1);
		trustLevelImage2 = (ImageView) v.findViewById(R.id.viewTrustMarker2);
		trustLevelImage3 = (ImageView) v.findViewById(R.id.viewTrustMarker3);
		
		imageResult = (ImageView) v.findViewById(R.id.imageResult);
		textResult = (TextView) v.findViewById(R.id.textResult);
		textEndpoint = (TextView) v.findViewById(R.id.textEndpoint);
		
		Button buttonOk = (Button) v.findViewById(R.id.buttonOk);
		
		String action = getArguments().getString(KeyExchangeService.EXTRA_ACTION);
		
		LinearLayout trustGroup = (LinearLayout) v.findViewById(R.id.groupTrustLevel);
		
		if (KeyExchangeService.ACTION_COMPLETE.equals(action)) {
			imageResult.setImageResource(R.drawable.ic_hint_success);
			textResult.setText(getString(R.string.keyexchange_success));
			buttonOk.setText(getString(R.string.complete));
			trustGroup.setVisibility(View.VISIBLE);
		}
		else {
			imageResult.setImageResource(R.drawable.ic_hint_fail);
			textResult.setText(getString(R.string.keyexchange_fail));
			trustGroup.setVisibility(View.GONE);
		}

		buttonOk.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				getActivity().onBackPressed();
			}
		});
		
		// retrieve endpoint
		mEndpoint = getArguments().getParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT);

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
		mManager = null;
		
		// clear current key-info
		mKeyInfo = null;

		super.onPause();
	}

	public void refresh() {
		try {
			// get key info
			mKeyInfo = mManager.getKeyInfo(mEndpoint);
			
			int trust_level = 0;
			
			if (mKeyInfo != null)
			{
				trust_level = mKeyInfo.getInt("trustlevel");
			}
			
			textEndpoint.setText(mEndpoint.toString());
			
			if (trust_level > 67) {
				Drawable d = getResources().getDrawable(R.drawable.ic_security_closed);
				d.setColorFilter(getResources().getColor(R.color.trust_high), Mode.SRC_IN);
				trustLevelImage1.setImageDrawable(d);
				trustLevelImage2.setImageDrawable(d);
				trustLevelImage3.setImageDrawable(d);
			}
			else if (trust_level > 33) {
				Drawable d = getResources().getDrawable(R.drawable.ic_security_closed);
				d.setColorFilter(getResources().getColor(R.color.trust_medium), Mode.SRC_IN);
				trustLevelImage1.setImageDrawable(d);
				trustLevelImage2.setImageDrawable(d);
				
				d = getResources().getDrawable(R.drawable.ic_action_security_open);
				trustLevelImage3.setImageDrawable(d);
			}
			else if (trust_level > 0) {
				Drawable d = getResources().getDrawable(R.drawable.ic_security_closed);
				d.setColorFilter(getResources().getColor(R.color.trust_low), Mode.SRC_IN);
				trustLevelImage1.setImageDrawable(d);
				
				d = getResources().getDrawable(R.drawable.ic_action_security_open);
				trustLevelImage2.setImageDrawable(d);
				trustLevelImage3.setImageDrawable(d);
			}
			else {
				Drawable d = getResources().getDrawable(R.drawable.ic_action_security_open);
				trustLevelImage1.setImageDrawable(d);
				trustLevelImage2.setImageDrawable(d);
				trustLevelImage3.setImageDrawable(d);
			}
		} catch (RemoteException e) {
			Log.e(TAG, "Failed to retrieve key information", e);
		}
	}
}
