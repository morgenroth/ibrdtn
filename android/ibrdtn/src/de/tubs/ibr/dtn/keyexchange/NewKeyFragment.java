package de.tubs.ibr.dtn.keyexchange;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
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
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.service.DaemonService;

public class NewKeyFragment extends Fragment {
	
	private static final String TAG = "NewKeyFragment";
	
	private KeyExchangeManager mManager = null;
	
	private SingletonEndpoint mEndpoint;
	private int mSession;
	private int mProtocol;
	private String mNewKeyFingerprint;
	
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
	
	public static NewKeyFragment create(SingletonEndpoint endpoint, int protocol, int session, String newKey) {
		NewKeyFragment f = new NewKeyFragment();
		Bundle args = new Bundle();
		args.putParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, endpoint);
		args.putInt("session", session);
		args.putInt("protocol", protocol);
		args.putString(KeyExchangeService.EXTRA_DATA, newKey);
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
		mNewKeyFingerprint = getArguments().getString(KeyExchangeService.EXTRA_DATA);
		
		// BUTTON
		Button abortButton = (Button) v.findViewById(R.id.buttonOk);
		abortButton.setText(getString(android.R.string.cancel));
		abortButton.setOnClickListener(mAbortClick);
		
		// OLD KEY VIEW
		RelativeLayout viewOldKey = (RelativeLayout) v.findViewById(R.id.viewOldKey);
		viewOldKey.setClickable(true);
		viewOldKey.setOnClickListener(mAbortClick);
		
		// NEW KEY VIEW
		RelativeLayout viewNewKey = (RelativeLayout) v.findViewById(R.id.viewNewKey);
		viewNewKey.setClickable(true);
		viewNewKey.setOnClickListener(mAcceptClick);
		
		return v;
	}

	public void refresh() {
		if (getView() == null) return;
		
		TextView textInfo = (TextView) getView().findViewById(R.id.textViewInfo);
		textInfo.setText(getString(R.string.key_selection_request));
		
		// ENDPOINT
		TextView textEndpoint = (TextView) getView().findViewById(R.id.textEndpoint);
		textEndpoint.setText(mEndpoint.toString());
		
		// OLD KEY
		ImageView trustLevelImageOldKey1 = (ImageView) getView().findViewById(R.id.imageViewTrustLevelOldKey1);
		ImageView trustLevelImageOldKey2 = (ImageView) getView().findViewById(R.id.imageViewTrustLevelOldKey2);
		ImageView trustLevelImageOldKey3 = (ImageView) getView().findViewById(R.id.imageViewTrustLevelOldKey3);
		
		int trustLevel = 0;
		
		TextView textOldKey = (TextView) getView().findViewById(R.id.textViewOldKey);
		
		try {
			Bundle keyinfo = mManager.getKeyInfo(mEndpoint);
			trustLevel = keyinfo.getInt("trustlevel");
			textOldKey.setText(Utils.hashToReadableString(keyinfo.getString("fingerprint")));
		} catch (RemoteException e) {
			Log.e(TAG, "", e);
		}
		
		if (trustLevel > 67) {
			Drawable d = getResources().getDrawable(R.drawable.ic_security_closed).mutate();
			d.setColorFilter(getResources().getColor(R.color.trust_high), Mode.SRC_IN);
			trustLevelImageOldKey1.setImageDrawable(d);
			trustLevelImageOldKey2.setImageDrawable(d);
			trustLevelImageOldKey3.setImageDrawable(d);
		}
		else if (trustLevel > 33) {
			Drawable d = getResources().getDrawable(R.drawable.ic_security_closed).mutate();
			d.setColorFilter(getResources().getColor(R.color.trust_medium), Mode.SRC_IN);
			trustLevelImageOldKey1.setImageDrawable(d);
			trustLevelImageOldKey2.setImageDrawable(d);
			
			d = getResources().getDrawable(R.drawable.ic_action_security_open);
			trustLevelImageOldKey3.setImageDrawable(d);
		}
		else if (trustLevel > 0) {
			Drawable d = getResources().getDrawable(R.drawable.ic_security_closed).mutate();
			d.setColorFilter(getResources().getColor(R.color.trust_low), Mode.SRC_IN);
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
		TextView textNewKey = (TextView) getView().findViewById(R.id.textViewNewKey);
		textNewKey.setText(Utils.hashToReadableString(mNewKeyFingerprint));
		
		ImageView trustLevelImageNewKey1 = (ImageView) getView().findViewById(R.id.imageViewTrustLevelNewKey1);
		ImageView trustLevelImageNewKey2 = (ImageView) getView().findViewById(R.id.imageViewTrustLevelNewKey2);
		ImageView trustLevelImageNewKey3 = (ImageView) getView().findViewById(R.id.imageViewTrustLevelNewKey3);
		
		if (mProtocol == 5 || mProtocol == 4) {
			Drawable d = getResources().getDrawable(R.drawable.ic_security_closed).mutate();
			d.setColorFilter(getResources().getColor(R.color.trust_high), Mode.SRC_IN);
			trustLevelImageNewKey1.setImageDrawable(d);
			trustLevelImageNewKey2.setImageDrawable(d);
			trustLevelImageNewKey3.setImageDrawable(d);
		}
		else if (mProtocol == 3 || mProtocol == 2) {
			Drawable d = getResources().getDrawable(R.drawable.ic_security_closed).mutate();
			d.setColorFilter(getResources().getColor(R.color.trust_medium), Mode.SRC_IN);
			trustLevelImageNewKey1.setImageDrawable(d);
			trustLevelImageNewKey2.setImageDrawable(d);
			
			d = getResources().getDrawable(R.drawable.ic_action_security_open);
			trustLevelImageNewKey3.setImageDrawable(d);
		}
		else if (mProtocol == 1 || mProtocol == 0) {
			Drawable d = getResources().getDrawable(R.drawable.ic_security_closed).mutate();
			d.setColorFilter(getResources().getColor(R.color.trust_low), Mode.SRC_IN);
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
	}
	
	@Override
	public void onResume() {
		super.onResume();
		
		IntentFilter intentFilter = new IntentFilter(KeyExchangeService.INTENT_KEY_EXCHANGE);
		intentFilter.setPriority(1000);
		intentFilter.addCategory(Intent.CATEGORY_DEFAULT);
		getActivity().registerReceiver(mUpdateReceiver, intentFilter);
		
		// Establish a connection with the service. We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		Intent bindIntent = DaemonService.createKeyExchangeManagerIntent(getActivity());
		getActivity().bindService(bindIntent, mConnection, Context.BIND_AUTO_CREATE);
	}
	
	@Override
	public void onPause() {
		getActivity().unregisterReceiver(mUpdateReceiver);
		
		// Detach our existing connection.
		getActivity().unbindService(mConnection);
		mManager = null;
		
		super.onPause();
	}

	private View.OnClickListener mAbortClick = new OnClickListener() {
		@Override
		public void onClick(View v) {
			Intent startIntent = new Intent(getActivity(), DaemonService.class);
			startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_GIVE_NEW_KEY_RESPONSE);
			startIntent.putExtra("session", mSession);
			startIntent.putExtra("newKey", 0);
			startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
			
			getActivity().startService(startIntent);
			
			Toast.makeText(getActivity(), getString(R.string.key_dropped), Toast.LENGTH_SHORT).show();
			getActivity().onBackPressed();
		}
	};
	
	private View.OnClickListener mAcceptClick = new OnClickListener() {
		@Override
		public void onClick(View v) {
			Intent startIntent = new Intent(getActivity(), DaemonService.class);
			startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_GIVE_NEW_KEY_RESPONSE);
			startIntent.putExtra("session", mSession);
			startIntent.putExtra("newKey", 1);
			startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
			
			getActivity().startService(startIntent);
		}
	};
	
	private BroadcastReceiver mUpdateReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			// get the action
			String action = intent.getStringExtra(KeyExchangeService.EXTRA_ACTION);
			
			// get the endpoint
			SingletonEndpoint endpoint = new SingletonEndpoint(intent.getStringExtra(KeyExchangeService.EXTRA_ENDPOINT));
			
			// stop here if the currently viewed endpoint does not match the event
			if (!mEndpoint.equals(endpoint)) return;
			
			// refresh current fragment if the ProtocolSelectionListFragment is shown
			Fragment f = getActivity().getSupportFragmentManager().findFragmentById(R.id.keyExchange);
			if (f instanceof KeyInformationFragment) {
				((KeyInformationFragment) f).refresh();
			}
			
			// get the session ID
			int session = Integer.valueOf(intent.hasExtra(KeyExchangeService.EXTRA_SESSION) ? intent.getStringExtra(KeyExchangeService.EXTRA_SESSION) : "-1");
			
			// get the protocol ID
			int protocol = Integer.valueOf(intent.hasExtra(KeyExchangeService.EXTRA_PROTOCOL) ? intent.getStringExtra(KeyExchangeService.EXTRA_PROTOCOL) : "-1");
	
			// stop here if the currently viewed protocol does not match the event
			if (protocol != mProtocol) return;
			
			// stop here if the currently viewed session does not match the event
			if (session != mSession) return;
			
			// abort any further broadcasts of this intent
			abortBroadcast();
			
			if (KeyExchangeService.ACTION_ERROR.equals(action)) {
				FragmentTransaction ft = getActivity().getSupportFragmentManager().beginTransaction();
				ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
				ft.replace(R.id.keyExchange, KeyExchangeResultFragment.create(action, mEndpoint)).addToBackStack(null);
				ft.commit();
			}
			else if (KeyExchangeService.ACTION_COMPLETE.equals(action)) {
				FragmentTransaction ft = getActivity().getSupportFragmentManager().beginTransaction();
				ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
				ft.replace(R.id.keyExchange, KeyExchangeResultFragment.create(action, mEndpoint)).addToBackStack(null);
				ft.commit();
			}
		}
	};
}
