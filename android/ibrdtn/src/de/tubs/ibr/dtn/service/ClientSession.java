/*
 * ClientSession.java
 * 
 * Copyright (C) 2011-2013 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *             Dominik Schürmann <dominik@dominikschuermann.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
package de.tubs.ibr.dtn.service;

import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.util.Log;
import de.tubs.ibr.dtn.api.Block;
import de.tubs.ibr.dtn.api.Bundle;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.DTNSession;
import de.tubs.ibr.dtn.api.DTNSessionCallback;
import de.tubs.ibr.dtn.api.EID;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.api.TransferMode;

import de.tubs.ibr.dtn.swig.NativeSession;
import de.tubs.ibr.dtn.swig.NativeSessionCallback;

public class ClientSession {

	private final static String TAG = "ClientSession";

	private String _package_name = null;

	private Context context = null;
	private Registration _registration = null;

	/*
	 * TODO: is this used in the original code?
	 */
	// private Boolean _daemon_online = false;

	// callback to the service-client
	private Object _callback_mutex = new Object();

	private DTNSessionCallback _callback_real = null;
	private DTNSessionCallback _callback_wrapper = new DTNSessionCallback() {

		public IBinder asBinder()
		{
			return null;
		}

		public void startBundle(Bundle bundle) throws RemoteException
		{
			synchronized (_callback_mutex) {
				if (_callback_real == null) return;
				try {
					_callback_real.startBundle(bundle);
				} catch (Exception e) {
					// remove the callback on error
					_callback_real = null;
					Log.e(TAG, "Error on callback [startBundle]", e);
				}
			}
		}

		public void endBundle() throws RemoteException
		{
			synchronized (_callback_mutex) {
				if (_callback_real == null) return;
				try {
					_callback_real.endBundle();
				} catch (Exception e) {
					// remove the callback on error
					_callback_real = null;
					Log.e(TAG, "Error on callback [endBundle]", e);
				}
			}
		}

		public TransferMode startBlock(Block block) throws RemoteException
		{
			synchronized (_callback_mutex) {
				if (_callback_real == null) return TransferMode.NULL;
				try {
					return _callback_real.startBlock(block);
				} catch (Exception e) {
					// remove the callback on error
					_callback_real = null;
					Log.e(TAG, "Error on callback [startBlock]", e);
					return TransferMode.NULL;
				}
			}
		}

		public void endBlock() throws RemoteException
		{
			synchronized (_callback_mutex) {
				if (_callback_real == null) return;
				try {
					_callback_real.endBlock();
				} catch (Exception e) {
					// remove the callback on error
					_callback_real = null;
					Log.e(TAG, "Error on callback [endBlock]", e);
				}
			}
		}

		public ParcelFileDescriptor fd() throws RemoteException
		{
			synchronized (_callback_mutex) {
				if (_callback_real == null) return null;
				try {
					return _callback_real.fd();
				} catch (Exception e) {
					// remove the callback on error
					_callback_real = null;
					Log.e(TAG, "Error on callback [fd]", e);
				}

				return null;
			}
		}

		public void progress(long current, long length) throws RemoteException
		{
			synchronized (_callback_mutex) {
				if (_callback_real == null) return;
				try {
					_callback_real.progress(current, length);
				} catch (Exception e) {
					// remove the callback on error
					_callback_real = null;
					Log.e(TAG, "Error on callback [progress]", e);
				}
			}
		}

		public void payload(byte[] data) throws RemoteException
		{
			synchronized (_callback_mutex) {
				if (_callback_real == null) return;
				try {
					_callback_real.payload(data);
				} catch (Exception e) {
					// remove the callback on error
					_callback_real = null;
					Log.e(TAG, "Error on callback [payload]", e);
				}
			}
		}

		public void characters(String data) throws RemoteException
		{
			synchronized (_callback_mutex) {
				if (_callback_real == null) return;
				try {
					_callback_real.characters(data);
				} catch (Exception e) {
					// remove the callback on error
					_callback_real = null;
					Log.e(TAG, "Error on callback [characters]", e);
				}
			}
		}
	};

	/**
	 * Implemented C++ callback using SWIG directors
	 * 
	 * see http://stackoverflow.com/questions/8168517/generating-java-interface-
	 * with-swig/8246375#8246375
	 */
	private class NativeSessionCallbackImpl extends NativeSessionCallback {

		@Override
		public void notifyBundle(de.tubs.ibr.dtn.swig.BundleID swigId)
		{
			// convert from swig BundleID to api BundleID
			BundleID id = new BundleID();
			id.setSequencenumber(swigId.getSequencenumber());
			id.setSource(swigId.getSource().getString());

			long swigTime = swigId.getTimestamp();
			Timestamp ts = new Timestamp(swigTime);
			id.setTimestamp(ts.getDate());

			invoke_receive_intent(id);

			// original invokation from SABHandler
			// invoke_receive_intent(new BundleID());
		}

		@Override
		public void notifyStatusReport(de.tubs.ibr.dtn.swig.StatusReportBlock swigReport)
		{
			// TODO
		}

		@Override
		public void notifyCustodySignal(de.tubs.ibr.dtn.swig.CustodySignalBlock swigCustody)
		{
			// TODO
		}

	};

	/*
	 * This is the actual native implementation of this session. See
	 * daemon/src/api/NativeSession.cpp. Java parts are generated by SWIG
	 */
	private NativeSession nativeSession = new NativeSession(new NativeSessionCallbackImpl());

	public ClientSession(Context context, Registration reg, String packageName) {
		this.context = context;
		this._package_name = packageName;
		this._registration = reg;
	}

	/*
	 * The following methods are Helper methods to interact with NativeSession
	 * based on our API calls
	 */

	public void setEndpoint(String id)
	{
		nativeSession.setEndpoint(id);
	}

	public void addRegistration(GroupEndpoint eid)
	{
		de.tubs.ibr.dtn.swig.EID swigEid = new de.tubs.ibr.dtn.swig.EID(eid.toString());
		nativeSession.addRegistration(swigEid);
	}

	public void getBundle()
	{
		nativeSession.get(de.tubs.ibr.dtn.swig.NativeSession.RegisterIndex.REG2);

		// TODO: read! (get, read, load)
		
//		byte[] outputBuffer = new byte[];
//		ByteArrayOutputStream boas =   new ByteArrayOutputStream();
//		boas.w
//		nativeSession.read(de.tubs.ibr.dtn.swig.NativeSession.RegisterIndex.REG2, buf, len)

	}

	public void loadBundle(BundleID id)
	{
		de.tubs.ibr.dtn.swig.BundleID swigId = new de.tubs.ibr.dtn.swig.BundleID();
		swigId.setSource(new de.tubs.ibr.dtn.swig.EID(id.getSource()));
		swigId.setSequencenumber(id.getSequencenumber());

		Timestamp ts = new Timestamp(id.getTimestamp());
		swigId.setTimestamp(ts.getValue());

		// nativeSession.load(de.tubs.ibr.dtn.swig.NativeSession.RegisterIndex.REG2,
		// swigId);
	}

	public void loadAndGetBundle()
	{
		// get next bundle in the queue
		// nativeSession.next(de.tubs.ibr.dtn.swig.NativeSession.RegisterIndex.REG1);
	}

	public void markDelivered(BundleID id)
	{
		de.tubs.ibr.dtn.swig.BundleID swigId = new de.tubs.ibr.dtn.swig.BundleID();
		swigId.setSource(new de.tubs.ibr.dtn.swig.EID(id.getSource()));
		swigId.setSequencenumber(id.getSequencenumber());

		Timestamp ts = new Timestamp(id.getTimestamp());
		swigId.setTimestamp(ts.getValue());

		nativeSession.delivered(swigId);
	}

	/**
	 * Put Bundle into NativeSession
	 */
	public synchronized void put(Bundle bundle)
	{
		/*
		 * Convert API Bundle to SWIG bundle
		 */
		de.tubs.ibr.dtn.swig.PrimaryBlock swigPrimaryBlock = new de.tubs.ibr.dtn.swig.PrimaryBlock();
		swigPrimaryBlock.set_custodian(new de.tubs.ibr.dtn.swig.EID(bundle.custodian));
		swigPrimaryBlock.set_destination(new de.tubs.ibr.dtn.swig.EID(bundle.destination));
		swigPrimaryBlock.set_fragmentoffset(bundle.fragment_offset);
		swigPrimaryBlock.set_lifetime(bundle.lifetime);
		swigPrimaryBlock.set_procflags(bundle.procflags);
		swigPrimaryBlock.set_reportto(new de.tubs.ibr.dtn.swig.EID(bundle.reportto));
		swigPrimaryBlock.set_sequencenumber(bundle.sequencenumber);
		swigPrimaryBlock.set_source(new de.tubs.ibr.dtn.swig.EID(bundle.source));

		Timestamp ts = new Timestamp(bundle.timestamp);
		swigPrimaryBlock.set_timestamp(ts.getValue());

		// TODO: priority
		swigPrimaryBlock.setPriority(de.tubs.ibr.dtn.swig.PrimaryBlock.PRIORITY.PRIO_MEDIUM);

		// put it into native session
		nativeSession.put(de.tubs.ibr.dtn.swig.NativeSession.RegisterIndex.REG1, swigPrimaryBlock);
	}

	public void writeAndSend(byte[] data)
	{
		nativeSession.write(de.tubs.ibr.dtn.swig.NativeSession.RegisterIndex.REG1, data);

		nativeSession.send(de.tubs.ibr.dtn.swig.NativeSession.RegisterIndex.REG1);
	}

	public void writeAndSend(InputStream stream, Long length)
	{
		// read input as buffered stream and write into NativeSession Bundle
		byte[] buf = new byte[8192];
		int len;
		int total = 0;
		try {
			while ((len = stream.read(buf, 0, Math.min(buf.length, (int) Math.min(Integer.MAX_VALUE, length - total)))) > 0) {
				total += len;

				nativeSession.write(de.tubs.ibr.dtn.swig.NativeSession.RegisterIndex.REG1, buf, Long.valueOf(0));
				// out.write(buf, 0, len);
			}
		} catch (IOException e) {
			Log.e(TAG, "Writing into NativeSession bundle failed!", e);
		}

		nativeSession.send(de.tubs.ibr.dtn.swig.NativeSession.RegisterIndex.REG1);
	}

	/**
	 * Send a bundle using the daemon.
	 * 
	 * @param destination
	 *            The destination of the bundle.
	 * @param lifetime
	 *            The lifetime of the bundle.
	 * @param data
	 *            The payload as byte-array.
	 */
	public void send(EID destination, int lifetime, byte[] data)
	{
		Bundle bundle = new Bundle();
		if (destination instanceof GroupEndpoint) {
			bundle.procflags = (long) 16;
		} else {
			bundle.procflags = (long) 0;
		}
		bundle.destination = destination.toString();
		bundle.lifetime = Long.valueOf(lifetime);

		// put it into native session
		put(bundle);

		writeAndSend(data);
	}

	/**
	 * Send a bundle using the daemon. The given stream is used as payload of
	 * the bundle.
	 * 
	 * @param destination
	 *            The destination of the bundle.
	 * @param lifetime
	 *            The lifetime of the bundle.
	 * @param stream
	 *            The stream containing the payload data.
	 * @param length
	 *            The length of the payload.
	 */
	public void send(EID destination, int lifetime, InputStream stream, Long length)
	{
		Bundle bundle = new Bundle();
		if (destination instanceof GroupEndpoint) {
			bundle.procflags = (long) 16;
		} else {
			bundle.procflags = (long) 0;
		}
		bundle.destination = destination.toString();
		bundle.lifetime = Long.valueOf(lifetime);

		// put it into native session
		put(bundle);

		writeAndSend(stream, length);
	}

	public synchronized void initialize()
	{
		// _daemon_online = true;

		// Register application
		register(_registration);
	}

	// public synchronized void terminate()
	// {
	// _daemon_online = false;
	// }

	public void register(Registration reg)
	{
		setEndpoint(reg.getEndpoint());

		if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "endpoint registered: " + reg.getEndpoint());

		for (GroupEndpoint group : reg.getGroups()) {

			addRegistration(group);
			if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "registration added: " + group.toString());

		}

		invoke_registration_intent();
	}

	/**
	 * This is the actual implementation of the DTNSession API
	 */
	private final DTNSession.Stub mBinder = new DTNSession.Stub() {
		public boolean query(DTNSessionCallback cb, BundleID id) throws RemoteException
		{
			try {
				// load the bundle
				if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "load bundle: " + id.toString());
				ClientSession.this.loadBundle(id);

				// set callback and mode
				synchronized (_callback_mutex) {
					ClientSession.this._callback_real = cb;
				}

				if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "get bundle: " + id.toString());
				ClientSession.this.getBundle();
				if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "load successful");

				return true;
			} catch (Exception e) {
				Log.e(TAG, "query failed", e);
				return false;
			}
		}

		public boolean queryNext(DTNSessionCallback cb) throws RemoteException
		{
			try {
				// load the next bundle
				if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "load and get bundle");

				// set callback and mode
				synchronized (_callback_mutex) {
					ClientSession.this._callback_real = cb;
				}

				// get next bundle
				ClientSession.this.loadAndGetBundle();
				if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "load successful");

				return true;
			} catch (Exception e) {
				Log.e(TAG, "queryNext failed", e);
				return false;
			}
		}

		public boolean delivered(BundleID id) throws RemoteException
		{
			try {
				// check for bundle to ack
				if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "ack bundle: " + id.toString());

				ClientSession.this.markDelivered(id);
				return true;
			} catch (Exception e) {
				Log.e(TAG, "delivered failed", e);
				return false;
			}
		}

		public boolean send(SingletonEndpoint destination, int lifetime, byte[] data) throws RemoteException
		{
			try {
				// send the message to the daemon
				ClientSession.this.send(destination, lifetime, data);

				// debug
				Log.i(TAG, "Message sent: " + data);

				return true;
			} catch (Exception e) {
				Log.e(TAG, "send failed", e);
				return false;
			}
		}

		public boolean sendGroup(GroupEndpoint destination, int lifetime, byte[] data) throws RemoteException
		{
			try {
				// send the message to the daemon
				ClientSession.this.send(destination, lifetime, data);

				// debug
				Log.i(TAG, "Message sent: " + data);

				return true;
			} catch (Exception e) {
				Log.e(TAG, "sendGroup failed", e);
				return false;
			}
		}

		public boolean sendFileDescriptor(SingletonEndpoint destination, int lifetime, ParcelFileDescriptor fd, long length) throws RemoteException
		{
			try {
				if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "Received file descriptor as bundle payload.");
				FileInputStream stream = new FileInputStream(fd.getFileDescriptor());
				try {
					ClientSession.this.send(destination, lifetime, stream, length);
					return true;
				} finally {
					try {
						stream.close();
						fd.close();
					} catch (IOException e) {
					}
				}
			} catch (Exception e) {
				Log.e(TAG, "sendFileDescriptor failed", e);
				return false;
			}
		}

		public boolean sendGroupFileDescriptor(GroupEndpoint destination, int lifetime, ParcelFileDescriptor fd, long length) throws RemoteException
		{
			try {
				if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "Received file descriptor as bundle payload.");
				FileInputStream stream = new FileInputStream(fd.getFileDescriptor());
				try {
					ClientSession.this.send(destination, lifetime, stream, length);
					return true;
				} finally {
					try {
						stream.close();
						fd.close();
					} catch (IOException e) {
					}
				}
			} catch (Exception e) {
				Log.e(TAG, "sendGroupFileDescriptor failed", e);
				return false;
			}
		}
	};

	public DTNSession getBinder()
	{
		return mBinder;
	}

	public String getPackageName()
	{
		return _package_name;
	}

	public void invoke_receive_intent(BundleID id)
	{
		// forward the notification as intent
		// create a new intent
		Intent notify = new Intent(de.tubs.ibr.dtn.Intent.RECEIVE);
		notify.addCategory(_package_name);
		notify.putExtra("type", "bundle");
		notify.putExtra("data", id);

		// send notification intent
		context.sendBroadcast(notify);

		Log.d(TAG, "RECEIVE intent sent to " + _package_name);
	}

	private void invoke_registration_intent()
	{
		// send out registration intent to the application
		Intent broadcastIntent = new Intent(de.tubs.ibr.dtn.Intent.REGISTRATION);
		broadcastIntent.addCategory(_package_name);
		broadcastIntent.putExtra("key", _package_name);

		// send notification intent
		context.sendBroadcast(broadcastIntent);

		Log.d(TAG, "REGISTRATION intent sent to " + _package_name);
	}
}
