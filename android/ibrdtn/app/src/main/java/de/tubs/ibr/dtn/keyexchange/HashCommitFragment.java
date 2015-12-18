package de.tubs.ibr.dtn.keyexchange;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Parcelable;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ListView;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.service.DaemonService;

public class HashCommitFragment extends Fragment {
	
	private SingletonEndpoint mEndpoint = null;
	private int mSession;
	private int mPosition;
	
	public static HashCommitFragment create(SingletonEndpoint endpoint, int session, String data) {
		HashCommitFragment f = new HashCommitFragment();
		
		Bundle args = new Bundle();
		args.putParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, endpoint);
		args.putInt("session", session);
		args.putString(KeyExchangeService.EXTRA_DATA, data);
		
		f.setArguments(args);
		return f;
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.hash_commit_fragment, container, false);
		
		mEndpoint = getArguments().getParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT);
		mSession = getArguments().getInt("session", -1);
		
		HashCommitListAdapter adapter = new HashCommitListAdapter(getActivity());
		
		try {
			MessageDigest md = MessageDigest.getInstance("SHA-256");
			byte[] messageDigest = md.digest(getArguments().getString(KeyExchangeService.EXTRA_DATA).getBytes());
			
			int random = (int)Math.floor(Math.random() * 2.99);
			for(int j = 0; j < 3; j++) {
				if(j == random) {
					adapter.add(messageDigest);
					mPosition = j;
				}
				else {
					adapter.add(md.digest(("" + Math.random()).getBytes()));
				}
			}
		} catch (NoSuchAlgorithmException e) {
			e.printStackTrace();
		}
		
		final ListView lw = (ListView) v.findViewById(R.id.listViewHash);
		lw.setAdapter(adapter);
		
		lw.setOnItemClickListener(new OnItemClickListener() {

			@Override
			public void onItemClick(AdapterView<?> ad, View v, int position, long id) {
				Intent startIntent = new Intent(getActivity(), DaemonService.class);
				startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_GIVE_HASH_RESPONSE);
				startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
				startIntent.putExtra("session", mSession);
				startIntent.putExtra("equals", (mPosition != position) ? 0 : 1);
				
				// forward decision to DTN service
				getActivity().startService(startIntent);
			}
		});
		
		final Button buttonNoMatch = (Button) v.findViewById(R.id.buttonOk);
		buttonNoMatch.setText(getString(R.string.no_match));
		buttonNoMatch.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				Intent startIntent = new Intent(getActivity(), DaemonService.class);
				startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_GIVE_HASH_RESPONSE);
				startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
				startIntent.putExtra("session", mSession);
				startIntent.putExtra("equals", 0);
				
				// forward decision to DTN service
				getActivity().startService(startIntent);
			}
		});
		
		return v;
	}
	
	@Override
	public void onResume() {
		super.onResume();
		
		IntentFilter intentFilter = new IntentFilter(KeyExchangeService.INTENT_KEY_EXCHANGE);
		intentFilter.setPriority(1000);
		intentFilter.addCategory(Intent.CATEGORY_DEFAULT);
		getActivity().registerReceiver(mUpdateReceiver, intentFilter);
	}
	
	@Override
	public void onPause() {
		getActivity().unregisterReceiver(mUpdateReceiver);
		super.onPause();
	}
	
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
			if (protocol != EnumProtocol.HASH.getValue()) return;
			
			// stop here if the session does not match the current session
			if (mSession != session) return;
			
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
			else if (KeyExchangeService.ACTION_NEW_KEY.equals(action)) {
				String data = intent.getStringExtra(KeyExchangeService.EXTRA_DATA);
				FragmentTransaction ft = getActivity().getSupportFragmentManager().beginTransaction();
				ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
				ft.replace(R.id.keyExchange, NewKeyFragment.create(mEndpoint, protocol, session, data)).addToBackStack(null);
				ft.commit();
			}
		}
	};
}
