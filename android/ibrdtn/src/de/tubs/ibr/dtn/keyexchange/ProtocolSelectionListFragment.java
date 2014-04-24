package de.tubs.ibr.dtn.keyexchange;

import java.util.LinkedList;
import java.util.List;

import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
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
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.service.DaemonService;

public class ProtocolSelectionListFragment extends Fragment {

	private static final String TAG = "PSelectionListFragment";
	
	private KeyExchangeManager mManager = null;
	
	private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder s) {
        	mManager = (KeyExchangeManager)s;
            
            refresh();
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service connected");
        }

        public void onServiceDisconnected(ComponentName name) {
        	mManager = null;
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service disconnected");
        }
    };
    
	private SingletonEndpoint mEndpoint;
	private ListView mListView = null;
	private ProtocolListAdapter mAdapter = null;
	private ImageView trustLevelImage1;
	private ImageView trustLevelImage2;
	private ImageView trustLevelImage3;
	private TextView textViewFingerprint;
	
	private int mTrustLevel = 0;
	
	public static ProtocolSelectionListFragment create(SingletonEndpoint endpoint) {
		ProtocolSelectionListFragment f = new ProtocolSelectionListFragment();
		Bundle args = new Bundle();
		args.putParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, endpoint);
		f.setArguments(args);
		return f;
	}
	
	@Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.protocol_selection_fragment, container, false);
		
		setHasOptionsMenu(true);
		
		mEndpoint = getArguments().getParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT);
		
		TextView textEid = (TextView) v.findViewById(R.id.textViewEID);
		textEid.setText(mEndpoint.toString());
		
		trustLevelImage1 = (ImageView) v.findViewById(R.id.imageViewTrustLevel1);
		trustLevelImage2 = (ImageView) v.findViewById(R.id.imageViewTrustLevel2);
		trustLevelImage3 = (ImageView) v.findViewById(R.id.imageViewTrustLevel3);
		
		textViewFingerprint = (TextView) v.findViewById(R.id.textViewFingerprint);

        // create a new list adapter
        mAdapter = new ProtocolListAdapter(getActivity());

        // set listview adapter
        mListView = (ListView) v.findViewById(R.id.list_view);
        mListView.setAdapter(mAdapter);
        mListView.setOnItemClickListener(new OnItemClickListener() {

			@Override
			public void onItemClick(AdapterView<?> ad, View v, int position, long id) {
				final ProtocolContainer p = (ProtocolContainer) mAdapter.getItem(position);
				
				if (p.isUsed())
				{
					AlertDialog.Builder dialog = new AlertDialog.Builder(getActivity());
					dialog.setTitle(getString(R.string.protocol_already_used));
					dialog.setMessage(getString(R.string.protocol_already_used_text));
					dialog.setPositiveButton(getString(android.R.string.yes), new OnClickListener() {
						
						@Override
						public void onClick(DialogInterface dialog, int which) {
							startProtocol(p.getProtocol());
							dialog.dismiss();
						}
					});
					dialog.setNegativeButton(getString(android.R.string.no), new OnClickListener() {
						
						@Override
						public void onClick(DialogInterface dialog, int which) {
							dialog.dismiss();
						}
					});
					dialog.show();
				}
				else {
					startProtocol(p.getProtocol());
				}
				
			}
		});
        
        return v;
    }
	
	private void startProtocol(EnumProtocol protocol) {
		switch (protocol) {
		case NONE:
		case DH:
		case HASH:
			// begin key-exchange for NONE, DH, and HASH approach
			Intent startIntent = new Intent(getActivity(), DaemonService.class);
			startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_START_KEY_EXCHANGE);
			startIntent.putExtra(KeyExchangeService.EXTRA_PROTOCOL, protocol.getValue());
			startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
			getActivity().startService(startIntent);
			break;
		default:
			break;
		}

		getActivity().getIntent().putExtra(KeyExchangeService.EXTRA_PROTOCOL, protocol.getValue());
		
        FragmentTransaction ft = getFragmentManager().beginTransaction();
        ft.setCustomAnimations(R.anim.slide_in_left, R.anim.slide_out_right, R.anim.slide_in_right, R.anim.slide_out_left);
        ft.replace(R.id.keyExchange, KeyExchangeFragment.create(getActivity().getIntent(), mEndpoint, protocol)).addToBackStack(null);
        ft.commit();
	}

	@Override
	public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
		inflater.inflate(R.menu.key_exchange_menu, menu);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.itemDeleteKey:
			new AlertDialog.Builder(getActivity())
					.setTitle(getString(R.string.key_deletion_confirm))
					.setMessage(getString(R.string.key_deletion_confirm_text))
					.setPositiveButton(getString(android.R.string.yes), new OnClickListener() {
						
						@Override
						public void onClick(DialogInterface dialog, int which) {
							Utils.deleteKey(getActivity(), mEndpoint);
							dialog.dismiss();
						}
					})
					.setNegativeButton(getString(android.R.string.no), new OnClickListener() {
						
						@Override
						public void onClick(DialogInterface dialog, int which) {
							dialog.dismiss();
						}
					})
					.show();
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
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

	public void refresh() {
		try {
			// get key info
			Bundle keyinfo = mManager.getKeyInfo(mEndpoint);
			
	        List<ProtocolContainer> protocols = new LinkedList<ProtocolContainer>();
	
	        mTrustLevel = 0;
	        if (keyinfo == null)
	        {
	            protocols.add(new ProtocolContainer(EnumProtocol.NFC, false));
	            protocols.add(new ProtocolContainer(EnumProtocol.QR, false));
	            protocols.add(new ProtocolContainer(EnumProtocol.HASH, false));
	            protocols.add(new ProtocolContainer(EnumProtocol.JPAKE, false));
	            protocols.add(new ProtocolContainer(EnumProtocol.DH, false));
	            protocols.add(new ProtocolContainer(EnumProtocol.NONE, false));
	        }
	        else
	        {
        		long flags = keyinfo.getLong("flags");
        		
        		boolean pNone = (flags & 0x01) > 0;
        		boolean pDh = (flags & 0x02) > 0;
        		boolean pJpake = (flags & 0x04) > 0;
        		boolean pHash = (flags & 0x08) > 0;
        		boolean pQrCode = (flags & 0x10) > 0;
        		boolean pNfc = (flags & 0x20) > 0;
        		
	            protocols.add(new ProtocolContainer(EnumProtocol.NFC, pNfc));
	            protocols.add(new ProtocolContainer(EnumProtocol.QR, pQrCode));
	            protocols.add(new ProtocolContainer(EnumProtocol.HASH, pHash));
	            protocols.add(new ProtocolContainer(EnumProtocol.JPAKE, pJpake));
	            protocols.add(new ProtocolContainer(EnumProtocol.DH, pDh));
	            protocols.add(new ProtocolContainer(EnumProtocol.NONE, pNone));
	            
	            mTrustLevel = keyinfo.getInt("trustlevel");
	        }
	        
	        getActivity().getIntent().putExtra("trustLevel", mTrustLevel);
	        
	        synchronized (mAdapter) {
	        	mAdapter.swapList(protocols);
	        }
			
	        if(mTrustLevel > 67) {
	    		Drawable d = getResources().getDrawable(R.drawable.ic_security_closed);
	    		d.setColorFilter(getResources().getColor(R.color.light_green), Mode.MULTIPLY);
	    		trustLevelImage1.setImageDrawable(d);
	    		trustLevelImage2.setImageDrawable(d);
	    		trustLevelImage3.setImageDrawable(d);
	        }
	        else if(mTrustLevel > 33) {
	    		Drawable d = getResources().getDrawable(R.drawable.ic_security_closed);
	    		d.setColorFilter(getResources().getColor(R.color.light_yellow), Mode.MULTIPLY);
	    		trustLevelImage1.setImageDrawable(d);
	    		trustLevelImage2.setImageDrawable(d);
	    		
	    		d = getResources().getDrawable(R.drawable.ic_action_security_open);
	    		trustLevelImage3.setImageDrawable(d);
	        }
	        else if(mTrustLevel > 0) {
	    		Drawable d = getResources().getDrawable(R.drawable.ic_security_closed);
	    		d.setColorFilter(getResources().getColor(R.color.light_red), Mode.MULTIPLY);
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

	        if (keyinfo == null) {
	        	textViewFingerprint.setText(getString(R.string.no_key_available));
	        } else {
	        	String fingerprint = keyinfo.getString("fingerprint");
	        	textViewFingerprint.setText(Utils.hashToReadableString(fingerprint));
	        }
        } catch (RemoteException e) {
        	Log.e(TAG, "", e);
        }
	}
}
