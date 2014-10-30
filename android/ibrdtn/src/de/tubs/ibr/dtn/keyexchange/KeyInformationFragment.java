package de.tubs.ibr.dtn.keyexchange;

import java.util.LinkedList;
import java.util.List;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.database.DataSetObserver;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Color;
import android.graphics.PorterDuff.Mode;
import android.graphics.drawable.Drawable;
import android.nfc.NdefMessage;
import android.nfc.NdefRecord;
import android.nfc.NfcAdapter;
import android.nfc.NfcAdapter.CreateNdefMessageCallback;
import android.nfc.NfcEvent;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Parcelable;
import android.os.RemoteException;
import android.preference.PreferenceManager;
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
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.WriterException;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.qrcode.QRCodeWriter;

import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.daemon.Preferences;
import de.tubs.ibr.dtn.service.DaemonService;

public class KeyInformationFragment extends Fragment {

	private static final String TAG = "KeyInformationFragment";
	
	private KeyExchangeManager mManager = null;
	
	private Boolean mLocal;
	private SingletonEndpoint mEndpoint;
	
	private Bundle mKeyInfo = null;
	
	private LinearLayout mProtocolList = null;
	private ProtocolListAdapter mAdapter = null;
	
	private ImageView trustLevelImage1;
	private ImageView trustLevelImage2;
	private ImageView trustLevelImage3;
	private TextView textViewFingerprint;
	
	private View groupTrustLevel = null;
	private View groupProtocols = null;
	private View groupQRCode = null;
	
	private ImageView viewQRCode;
	private MenuItem mQrCodeScanMenu = null;
	private MenuItem mDeleteMenu = null;
	//private MenuItem mKeyExchangeMenu = null;
	
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
	
	public static KeyInformationFragment create(SingletonEndpoint endpoint, Boolean local) {
		KeyInformationFragment f = new KeyInformationFragment();
		Bundle args = new Bundle();
		args.putParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, endpoint);
		args.putBoolean(KeyInformationActivity.EXTRA_IS_LOCAL, local);
		f.setArguments(args);
		return f;
	}
	
	@SuppressLint("NewApi")
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.key_information_fragment, container, false);
		
		setHasOptionsMenu(true);
		
		mEndpoint = getArguments().getParcelable(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT);
		
		TextView textEid = (TextView) v.findViewById(R.id.textEndpoint);
		textEid.setText(mEndpoint.toString());
		
		trustLevelImage1 = (ImageView) v.findViewById(R.id.viewTrustMarker1);
		trustLevelImage2 = (ImageView) v.findViewById(R.id.viewTrustMarker2);
		trustLevelImage3 = (ImageView) v.findViewById(R.id.viewTrustMarker3);
		
		textViewFingerprint = (TextView) v.findViewById(R.id.textFingerprint);
		
		groupQRCode = v.findViewById(R.id.group_qrcode);
		viewQRCode = (ImageView) v.findViewById(R.id.viewQRCode);
		
		groupProtocols = v.findViewById(R.id.group_protocols);
		groupTrustLevel = v.findViewById(R.id.group_trustlevel);
		
		// protocol list
		mProtocolList = (LinearLayout)v.findViewById(R.id.listProtocols);

		// create a new list adapter
		mAdapter = new ProtocolListAdapter(getActivity());
		mAdapter.registerDataSetObserver(new DataSetObserver() {
			@Override
			public void onChanged() {
				final int adapterCount = mAdapter.getCount();

				for (int i = 0; i < adapterCount; i++) {
					View item = mProtocolList.getChildAt(i);
					
					final int index = i;

					if (item == null) {
						item = mAdapter.getView(i, null, mProtocolList);
						mProtocolList.addView(item);
						
						item.setClickable(true);
						item.setOnClickListener(new View.OnClickListener() {
							@Override
							public void onClick(View v) {
								mProtocolClickListener.onItemClick(null, v, index, 0);
							}
						});
					} else {
						// update item
						mAdapter.getView(i, item, mProtocolList);
					}
				}
			}

			@Override
			public void onInvalidated() {
				super.onInvalidated();
			}
		});
		
		// prepare view for local / non-local detail view
		mLocal = getArguments().getBoolean(KeyInformationActivity.EXTRA_IS_LOCAL);
		
		if (mLocal) {
			// hide protocol selection
			groupProtocols.setVisibility(View.GONE);
			
			// hide trust level
			groupTrustLevel.setVisibility(View.GONE);
			
			// enable NFC (only Android >= Ice Cream Sandwich)
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
				NfcAdapter nfc = NfcAdapter.getDefaultAdapter(getActivity());
				if (nfc != null) {
					nfc.setNdefPushMessageCallback(new CreateNdefMessageCallback() {
						@Override
						public NdefMessage createNdefMessage(NfcEvent event) {
							// abort if no key data is available, stop here
							if (mKeyInfo == null) return null;
							
							String text = "dtn:none";

							text = mEndpoint.toString();
							text += "_NFC_SEPERATOR_";
							text += mKeyInfo.getString(KeyExchangeService.EXTRA_DATA);
							
							NdefMessage msg = new NdefMessage(
									new NdefRecord[] { NdefRecord.createMime(
											"text/plain", text.getBytes())
							});
							return msg;
						}
					}, getActivity());
				}
			}
		} else {
			// show trust level
			groupTrustLevel.setVisibility(View.VISIBLE);
			
			// show protocol selection
			groupProtocols.setVisibility(View.VISIBLE);
		}
		
		return v;
	}
	
	private AdapterView.OnItemClickListener mProtocolClickListener = new AdapterView.OnItemClickListener() {

		@Override
		public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
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

	};
	
	private void startProtocol(EnumProtocol protocol) {
		switch (protocol)
		{
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
		ft.replace(R.id.keyExchange, KeyExchangeFragment.create(mEndpoint, protocol, -1)).addToBackStack(null);
		ft.commit();
	}

	@SuppressLint("NewApi")
	@Override
	public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
		inflater.inflate(R.menu.key_exchange_menu, menu);
		
		// show / hide delete key button
		mDeleteMenu = menu.findItem(R.id.itemDeleteKey);
		mDeleteMenu.setVisible(hasRemoteKey());
		
		// show / hide qr code scan button
		mQrCodeScanMenu = menu.findItem(R.id.itemQrCodeScan);
		mQrCodeScanMenu.setVisible(canScanQrCode());
		
		// show / hide key-exchange button
		//mKeyExchangeMenu = menu.findItem(R.id.itemUntrustedExchange);
		//mKeyExchangeMenu.setVisible(!mLocal && (mKeyInfo == null));
		
		// show / hide local key window
		MenuItem localKeyMenu = menu.findItem(R.id.itemKeyPanel);
		localKeyMenu.setVisible(!mLocal);
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
			
//		case R.id.itemUntrustedExchange:
//			startProtocol(EnumProtocol.NONE);
//			return true;
			
		case R.id.itemQrCodeScan:
			// launch QR code scanner
			startProtocol(EnumProtocol.QR);
			return true;
			
		case R.id.itemKeyPanel: {
			// open statistic activity
			Intent i = new Intent(getActivity(), KeyInformationActivity.class);
			
			// create local singleton endpoint
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
			SingletonEndpoint node = new SingletonEndpoint(Preferences.getEndpoint(getActivity(), prefs));
			
			i.putExtra(KeyInformationActivity.EXTRA_IS_LOCAL, true);
			i.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)node);
			
			startActivity(i);
			return true;
		}
			
		default:
			return super.onOptionsItemSelected(item);
		}
	}
	
	@Override
	public void onResume() {
		super.onResume();
		
		// register to key-exchange events
		IntentFilter intentFilter = new IntentFilter(KeyExchangeService.INTENT_KEY_EXCHANGE);
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
		// Detach our existing connection.
		getActivity().unbindService(mConnection);
		mManager = null;
		
		// unregister from key-exchange events
		getActivity().unregisterReceiver(mUpdateReceiver);
		
		// clear current key-info
		mKeyInfo = null;

		super.onPause();
	}
	
	private boolean canScanQrCode() {
		// check if a camera is available
		return getActivity().getPackageManager().hasSystemFeature("android.hardware.camera") && hasRemoteKey();
	}
	
	private boolean hasRemoteKey() {
		return !mLocal && (mKeyInfo != null);
	}

	@SuppressLint("NewApi")
	public void refresh() {
		try {
			// get key info
			mKeyInfo = mManager.getKeyInfo(mEndpoint);
			
			// show / hide QR code scan menu item
			if (mQrCodeScanMenu != null) {
				mQrCodeScanMenu.setVisible(canScanQrCode());
			}
			
			// show / hide delete action item
			if (mDeleteMenu != null) {
				mDeleteMenu.setVisible(hasRemoteKey());
			}
			
//			// show / hide key-exchange action item
//			if (mKeyExchangeMenu != null) {
//				mKeyExchangeMenu.setVisible(!mLocal && (mKeyInfo == null));
//			}
			
			List<ProtocolContainer> protocols = new LinkedList<ProtocolContainer>();
	
			int trust_level = 0;
			
			if (mKeyInfo == null)
			{
				//protocols.add(new ProtocolContainer(EnumProtocol.NFC, false));
				//protocols.add(new ProtocolContainer(EnumProtocol.QR, false));
				protocols.add(new ProtocolContainer(EnumProtocol.HASH, false));
				
				// enable only in development mode
				if (Preferences.isDebuggable(getActivity())) {
					protocols.add(new ProtocolContainer(EnumProtocol.JPAKE, false));
					//protocols.add(new ProtocolContainer(EnumProtocol.DH, false));
				}
				
				protocols.add(new ProtocolContainer(EnumProtocol.NONE, false));
			}
			else
			{
				long flags = mKeyInfo.getLong(KeyExchangeService.EXTRA_FLAGS);
				
				boolean pNone = (flags & 0x01) > 0;
				//boolean pDh = (flags & 0x02) > 0;
				boolean pJpake = (flags & 0x04) > 0;
				boolean pHash = (flags & 0x08) > 0;
				//boolean pQrCode = (flags & 0x10) > 0;
				//boolean pNfc = (flags & 0x20) > 0;
				
				//protocols.add(new ProtocolContainer(EnumProtocol.NFC, pNfc));
				//protocols.add(new ProtocolContainer(EnumProtocol.QR, pQrCode));
				protocols.add(new ProtocolContainer(EnumProtocol.HASH, pHash));
				
				// enable only in development mode
				if (Preferences.isDebuggable(getActivity())) {
					protocols.add(new ProtocolContainer(EnumProtocol.JPAKE, pJpake));
					//protocols.add(new ProtocolContainer(EnumProtocol.DH, pDh));
				}
				
				protocols.add(new ProtocolContainer(EnumProtocol.NONE, pNone));
				
				trust_level = mKeyInfo.getInt("trustlevel");
			}
			
			getActivity().getIntent().putExtra("trustLevel", trust_level);
			
			synchronized (mAdapter) {
				mAdapter.swapList(protocols);
			}
			
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

			if (mKeyInfo == null) {
				textViewFingerprint.setText(getString(R.string.no_key_available));
				
				// hide QR code
				groupQRCode.setVisibility(View.GONE);
			} else {
				String fingerprint = mKeyInfo.getString("fingerprint");
				textViewFingerprint.setText(Utils.hashToReadableString(fingerprint));
				
				if (mLocal) {
					try {
						// show QR code
						int pixel = getResources().getDimensionPixelSize(R.dimen.qrcode_rect_size);
						QRCodeWriter writer = new QRCodeWriter();
						BitMatrix matrix = writer.encode(fingerprint, BarcodeFormat.QR_CODE, pixel, pixel);
						Bitmap bitmap = Bitmap.createBitmap(pixel, pixel, Config.ARGB_8888);

						for (int i = 0; i < pixel; i++) {
							for (int j = 0; j < pixel; j++) {
								bitmap.setPixel(i, j, matrix.get(i, j) ? Color.BLACK : Color.WHITE);
							}
						}
		
						viewQRCode.setImageBitmap(bitmap);
						
						// show QR code
						groupQRCode.setVisibility(View.VISIBLE);
					} catch (WriterException e) {
						Log.e(TAG, "", e);
						
						// hide QR code
						groupQRCode.setVisibility(View.GONE);
					}
				} else {
					// hide QR code
					groupQRCode.setVisibility(View.GONE);
				}
			}
		} catch (RemoteException e) {
			Log.e(TAG, "Failed to retrieve key information", e);
		}
	}
	
	private BroadcastReceiver mUpdateReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			// get the endpoint
			SingletonEndpoint endpoint = new SingletonEndpoint(intent.getStringExtra(KeyExchangeService.EXTRA_ENDPOINT));
			
			// stop here if the currently viewed endpoint does not match the event
			if (!mEndpoint.equals(endpoint)) return;
			
			// refresh current fragment if the ProtocolSelectionListFragment is shown
			refresh();
		}
	};
}
