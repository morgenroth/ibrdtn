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

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import android.annotation.TargetApi;
import android.content.Intent;
import android.os.Build;
import android.os.ParcelFileDescriptor;
import android.os.ParcelFileDescriptor.AutoCloseInputStream;
import android.os.Parcelable;
import android.os.RemoteException;
import android.util.Log;
import de.tubs.ibr.dtn.api.Block;
import de.tubs.ibr.dtn.api.Bundle;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.DTNSession;
import de.tubs.ibr.dtn.api.DTNSessionCallback;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.api.Timestamp;
import de.tubs.ibr.dtn.api.TransferMode;
import de.tubs.ibr.dtn.service.db.Endpoint;
import de.tubs.ibr.dtn.service.db.Session;
import de.tubs.ibr.dtn.swig.BundleNotFoundException;
import de.tubs.ibr.dtn.swig.DtnNumber;
import de.tubs.ibr.dtn.swig.NativeSerializerCallback;
import de.tubs.ibr.dtn.swig.NativeSession;
import de.tubs.ibr.dtn.swig.NativeSession.RegisterIndex;
import de.tubs.ibr.dtn.swig.NativeSessionCallback;
import de.tubs.ibr.dtn.swig.NativeSessionException;
import de.tubs.ibr.dtn.swig.PrimaryBlock;
import de.tubs.ibr.dtn.swig.PrimaryBlockFlags;
import de.tubs.ibr.dtn.swig.StatusReportBlock;

public class ClientSession {

	private final static String TAG = "ClientSession";

	private SessionManager mManager = null;
	private Session mSession = null;

	// locks for bundle register slot 1 and 2
	private Object mRegisterLock1 = new Object();
	private Object mRegisterLock2 = new Object();

	/**
	 * Implemented C++ callback using SWIG directors
	 * 
	 * see http://stackoverflow.com/questions/8168517/generating-java-interface-
	 * with-swig/8246375#8246375
	 */
	private final NativeSessionCallback mSessionCallback = new NativeSessionCallback() {

		@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
        @Override
		public void notifyBundle(de.tubs.ibr.dtn.swig.BundleID swigId)
		{
	        // forward the notification as intent
	        // create a new intent
	        Intent notify = new Intent(de.tubs.ibr.dtn.Intent.RECEIVE);
	        notify.addCategory(mSession.getPackageName());
	        notify.setPackage(mSession.getPackageName());
	        notify.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
	        notify.putExtra("bundleid", toAndroid(swigId));

	        // send notification intent
	        mManager.getContext().sendBroadcast(notify, de.tubs.ibr.dtn.Intent.PERMISSION_COMMUNICATION);

	        Log.d(TAG, "RECEIVE intent (" + swigId.toString() + ") sent to " + mSession.getPackageName());
		}

		@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
        @Override
		public void notifyStatusReport(de.tubs.ibr.dtn.swig.EID source, de.tubs.ibr.dtn.swig.StatusReportBlock swigReport)
		{
		    de.tubs.ibr.dtn.swig.BundleID swigId = swigReport.getBundleid();
		    
            // forward the notification as intent
            // create a new intent
            Intent notify = new Intent(de.tubs.ibr.dtn.Intent.STATUS_REPORT);
            notify.addCategory(mSession.getPackageName());
            notify.setPackage(mSession.getPackageName());
            notify.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
            notify.putExtra("bundleid", toAndroid(swigId));
            notify.putExtra("source", (Parcelable)new SingletonEndpoint(source.getString()));
            notify.putExtra("status", swigReport.getStatus());
            notify.putExtra("reason", swigReport.getReasoncode());
            
            char status = swigReport.getStatus();
            
            if (0 < (status & StatusReportBlock.TYPE.RECEIPT_OF_BUNDLE.swigValue())) {
                notify.putExtra("timeof_receipt", toAndroid(swigReport.getTimeof_receipt()));
            }
            
            if (0 < (status & StatusReportBlock.TYPE.DELETION_OF_BUNDLE.swigValue())) {
                notify.putExtra("timeof_deletion", toAndroid(swigReport.getTimeof_deletion()));
            }
            
            if (0 < (status & StatusReportBlock.TYPE.DELIVERY_OF_BUNDLE.swigValue())) {
                notify.putExtra("timeof_delivery", toAndroid(swigReport.getTimeof_delivery()));
            }
            
            if (0 < (status & StatusReportBlock.TYPE.FORWARDING_OF_BUNDLE.swigValue())) {
                notify.putExtra("timeof_forwarding", toAndroid(swigReport.getTimeof_forwarding()));
            }
            
            if (0 < (status & StatusReportBlock.TYPE.CUSTODY_ACCEPTANCE_OF_BUNDLE.swigValue())) {
                notify.putExtra("timeof_custodyaccept", toAndroid(swigReport.getTimeof_custodyaccept()));
            }
            
            // send notification intent
            mManager.getContext().sendBroadcast(notify, de.tubs.ibr.dtn.Intent.PERMISSION_COMMUNICATION);

            Log.d(TAG, "STATUS_REPORT intent [" + swigId.toString() + "] sent to " + mSession.getPackageName());
		}

		@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
        @Override
		public void notifyCustodySignal(de.tubs.ibr.dtn.swig.EID source, de.tubs.ibr.dtn.swig.CustodySignalBlock swigCustody)
		{
		    de.tubs.ibr.dtn.swig.BundleID swigId = swigCustody.getBundleid();
		    
            // forward the notification as intent
            // create a new intent
            Intent notify = new Intent(de.tubs.ibr.dtn.Intent.CUSTODY_SIGNAL);
            notify.addCategory(mSession.getPackageName());
            notify.setPackage(mSession.getPackageName());
            notify.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
            notify.putExtra("bundleid", toAndroid(swigId));
            notify.putExtra("source", (Parcelable)new SingletonEndpoint(source.getString()));
            notify.putExtra("accepted", swigCustody.getCustody_accepted());
            notify.putExtra("timeofsignal", toAndroid(swigCustody.getTimeofsignal()));

            // send notification intent
            mManager.getContext().sendBroadcast(notify, de.tubs.ibr.dtn.Intent.PERMISSION_COMMUNICATION);

            Log.d(TAG, "CUSTODY_SIGNAL intent [" + swigId.toString() + "] sent to " + mSession.getPackageName());
		}

	};
	
    private final static class SerializerCallback extends NativeSerializerCallback {
        
        private DTNSessionCallback _cb = null;
        private TransferMode _mode = TransferMode.NULL;
        private OutputStream _output = null;
        private long _current = 0L;
        private long _length = 0L;
                
        public SerializerCallback() {
        }
        
        public void setCallback(DTNSessionCallback cb) {
            this._cb = cb;
        }

        @Override
        public void beginBundle(PrimaryBlock block) {
            try {
                _cb.startBundle(toAndroid(block));
            } catch (RemoteException e) {
                Log.e(TAG, "Remote call startBundle() failed", e);
            }
        }

        @Override
        public void endBundle() {
            try {
                _cb.endBundle();
            } catch (RemoteException e) {
                Log.e(TAG, "Remote call endBundle() failed", e);
            }
        }

        @Override
        public void beginBlock(de.tubs.ibr.dtn.swig.Block block, long payload_length) {
            try {
                _mode = _cb.startBlock(toAndroid(block));
            } catch (RemoteException e) {
                Log.e(TAG, "Remote call startBlock() failed", e);
                return;
            }
            
            if (TransferMode.FILEDESCRIPTOR.equals(_mode)) {
                try {
                    // ask for a filedescriptor
                    _output = new ParcelFileDescriptor.AutoCloseOutputStream(_cb.fd());
                    _length = payload_length;
                } catch (RemoteException e) {
                    Log.e(TAG, "Remote call fd() failed", e);
                }
            }
        }

        @Override
        public void endBlock() {
            try {
                _cb.endBlock();
            } catch (RemoteException e) {
                Log.e(TAG, "Remote call endBlock() failed", e);
            }
            
            if (_output != null) {
                try {
                    _output.close();
                } catch (IOException e) {
                    Log.e(TAG, "Error while closing output stream", e);
                }
            }
            
            _output = null;
            _mode = TransferMode.NULL;
            _length = 0L;
            _current = 0L;
        }
        
        @Override
        public void payload(byte data[]) {
            // skip this if the transfer mode is set to null
            if (TransferMode.NULL.equals(_mode)) {
                return;
            }
            
            if (_output != null) {
                try {
                    // put content into the output stream
                    _output.write(data);
                    
                    // increment current position
                    _current += data.length;
                    
                    // signal progress of copying
                    _cb.progress(_current, _length);
                } catch (IOException e) {
                    Log.e(TAG, "Failed to put payload into the output stream", e);
                    
                    try {
                        _output.close();
                    } catch (IOException ex) {
                        Log.e(TAG, "Error while closing output stream", ex);
                    }

                    _output = null;
                } catch (RemoteException e) {
                    Log.e(TAG, "Remote call progress() failed", e);
                }
            } else {
                try {
                    // passthrough the data
                    _cb.payload(data);
                } catch (RemoteException e) {
                    Log.e(TAG, "Remote call payload() failed", e);
                }
            }
        }
    }

	private final Thread mReceiverThread = new Thread() {

        @Override
        public void run() {
        	// lower the thread priority
        	android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);
        	
            try {
                while (!this.isInterrupted()) {
                    ClientSession.this.mNativeSession.receive();
                }
            } catch (NativeSessionException e) {
                if (!e.getMessage().startsWith("loop aborted")) {
                    Log.e(TAG, "Receiver thread terminated.", e);
                }
            }
        }
	    
	};

	private final SerializerCallback mSerializerCallback = new SerializerCallback();
    private NativeSession mNativeSession = null;

	public ClientSession(SessionManager manager, Session session) {
		mManager = manager;
		mSession = session;

		// initialize native session class
		mNativeSession = new NativeSession(mSessionCallback, mSerializerCallback, mSession.getSessionKey());
		
		try {
			// set default endpoint
			if (session.getDefaultEndpoint() != null) mNativeSession.setEndpoint(session.getDefaultEndpoint());
		} catch (NativeSessionException e) {
			Log.e(TAG, "can not set default endpoint", e);
		}

        // start-up receiver thread
        mReceiverThread.start();
	}
	
	/**
	 * destroy the session
	 */
	public void destroy() {
	    mNativeSession.destroy();

	    try {
	        mReceiverThread.join();
        } catch (InterruptedException e) {
            Log.e(TAG, "Join on receiver thread failed.", e);
        }
	    
	    mNativeSession.delete();
	    
	    synchronized(mSerializerCallback) {
	        mSessionCallback.delete();
	    }
	    
	    synchronized(mSerializerCallback) {
	        mSerializerCallback.delete();
	    }
	}
	
	public void setDefaultEndpoint(String endpoint) throws NativeSessionException {
	    mNativeSession.setEndpoint(endpoint);
	}
	
    public void addEndpoint(GroupEndpoint group) throws NativeSessionException {
        de.tubs.ibr.dtn.swig.EID eid = new de.tubs.ibr.dtn.swig.EID(group.toString());
        mNativeSession.addRegistration(eid);
    }

	public void addEndpoint(Endpoint e) throws NativeSessionException {
		if (e.isFqeid()) {
			de.tubs.ibr.dtn.swig.EID eid = new de.tubs.ibr.dtn.swig.EID(e.getEndpoint());
			mNativeSession.addRegistration(eid);
		} else {
			mNativeSession.addEndpoint(e.getEndpoint());
		}
	}
	
	public void removeEndpoint(Endpoint e) throws NativeSessionException {
		if (e.isFqeid()) {
			de.tubs.ibr.dtn.swig.EID eid = new de.tubs.ibr.dtn.swig.EID(e.getEndpoint());
			mNativeSession.removeRegistration(eid);
		} else {
			mNativeSession.removeEndpoint(e.getEndpoint());
		}
	}

    /**
     * This is the actual implementation of the DTNSession API
     */
    private final DTNSession.Stub mBinder = new DTNSession.Stub() {
        public boolean queryInfo(DTNSessionCallback cb, BundleID id) throws RemoteException
        {
            synchronized (mSerializerCallback) {
                // set serializer for this query
                mSerializerCallback.setCallback(cb);

                try {
                    synchronized (mRegisterLock1) {
                        // load the bundle into the register
                        mNativeSession.load(NativeSession.RegisterIndex.REG1, toSwig(id));

                        // get the bundle
                        mNativeSession.getInfo(NativeSession.RegisterIndex.REG1);
                    }

                    // set serializer back to null
                    mSerializerCallback.setCallback(null);

                    // bundle loaded - return true
                    return true;
                } catch (BundleNotFoundException e) {
                    // set serializer back to null
                    mSerializerCallback.setCallback(null);

                    // bundle not found - return false
                    return false;
                }
            }
        }
        
        public boolean query(DTNSessionCallback cb, BundleID id) throws RemoteException
        {
            synchronized (mSerializerCallback) {
                // set serializer for this query
                mSerializerCallback.setCallback(cb);

                try {
                    synchronized (mRegisterLock1) {
                        // load the bundle into the register
                        mNativeSession.load(NativeSession.RegisterIndex.REG1, toSwig(id));

                        // get the bundle
                        mNativeSession.get(NativeSession.RegisterIndex.REG1);
                    }

                    // set serializer back to null
                    mSerializerCallback.setCallback(null);

                    // bundle loaded - return true
                    return true;
                } catch (BundleNotFoundException e) {
                    // set serializer back to null
                    mSerializerCallback.setCallback(null);

                    // bundle not found - return false
                    return false;
                }
            }
        }
        
        public boolean queryInfoNext(DTNSessionCallback cb) throws RemoteException
        {
            synchronized (mSerializerCallback) {
                // set serializer for this query
                mSerializerCallback.setCallback(cb);

                try {
                    synchronized (mRegisterLock1) {
                        // load the next bundle into the register
                        mNativeSession.next(NativeSession.RegisterIndex.REG1);

                        // get the bundle
                        mNativeSession.getInfo(NativeSession.RegisterIndex.REG1);
                    }

                    // set serializer back to null
                    mSerializerCallback.setCallback(null);

                    // bundle loaded - return true
                    return true;
                } catch (BundleNotFoundException e) {
                    // set serializer back to null
                    mSerializerCallback.setCallback(null);

                    // bundle not found - return false
                    return false;
                }
            }
        }

        public boolean queryNext(DTNSessionCallback cb) throws RemoteException
        {
            synchronized (mSerializerCallback) {
                // set serializer for this query
                mSerializerCallback.setCallback(cb);

                try {
                    synchronized (mRegisterLock1) {
                        // load the next bundle into the register
                        mNativeSession.next(NativeSession.RegisterIndex.REG1);

                        // get the bundle
                        mNativeSession.get(NativeSession.RegisterIndex.REG1);
                    }

                    // set serializer back to null
                    mSerializerCallback.setCallback(null);

                    // bundle loaded - return true
                    return true;
                } catch (BundleNotFoundException e) {
                    // set serializer back to null
                    mSerializerCallback.setCallback(null);

                    // bundle not found - return false
                    return false;
                }
            }
        }

        public boolean delivered(BundleID id) throws RemoteException
        {
            try {
                mNativeSession.delivered(toSwig(id));
                return true;
            } catch (BundleNotFoundException e) {
                return false;
            }
        }

        @Override
        public BundleID sendByteArray(DTNSessionCallback cb, Bundle bundle, byte[] data)
                throws RemoteException {
            try {
                PrimaryBlock b = new PrimaryBlock();

                b.set(PrimaryBlock.FLAGS.DESTINATION_IS_SINGLETON,
                        bundle.get(Bundle.ProcFlags.DESTINATION_IS_SINGLETON));
                b.setDestination(new de.tubs.ibr.dtn.swig.EID(bundle.getDestination().toString()));

                // set lifetime
                if (bundle.getLifetime() != null)
                    b.setLifetime(new DtnNumber(bundle.getLifetime()));

                if (bundle.getReportto() != null)
                    b.setReportto(new de.tubs.ibr.dtn.swig.EID(bundle.getReportto().toString()));

                if (bundle.getCustodian() != null)
                    b.setCustodian(new de.tubs.ibr.dtn.swig.EID(bundle.getCustodian().toString()));

                // add flags from procflags
                b.setProcflags( new PrimaryBlockFlags( b.getProcflags().get() | bundle.getProcflags() ) );

                // bundle id return value
                de.tubs.ibr.dtn.swig.BundleID ret = null;

                synchronized (mRegisterLock2) {
                    // put the primary block into the register
                    mNativeSession.put(RegisterIndex.REG2, b);

                    if (cb != null) {
                        cb.progress(0, data.length);
                    }

                    // add data
                    mNativeSession.write(RegisterIndex.REG2, data);

                    if (cb != null) {
                        cb.progress(data.length, data.length);
                    }

                    // send the bundle
                    ret = mNativeSession.send(RegisterIndex.REG2);
                }

                return toAndroid(ret);
            } catch (Exception e) {
                Log.e(TAG, "send failed", e);
                return null;
            }
        }

        @Override
        public BundleID sendFileDescriptor(DTNSessionCallback cb, Bundle bundle,
                ParcelFileDescriptor fd) throws RemoteException {
            try {
                if (Log.isLoggable(TAG, Log.DEBUG))
                    Log.d(TAG, "Received file descriptor as bundle payload.");

                PrimaryBlock b = new PrimaryBlock();

                b.set(PrimaryBlock.FLAGS.DESTINATION_IS_SINGLETON,
                        bundle.get(Bundle.ProcFlags.DESTINATION_IS_SINGLETON));
                b.setDestination(new de.tubs.ibr.dtn.swig.EID(bundle.getDestination().toString()));

                // set lifetime
                if (bundle.getLifetime() != null)
                    b.setLifetime( new DtnNumber( bundle.getLifetime() ) );

                if (bundle.getReportto() != null)
                    b.setReportto(new de.tubs.ibr.dtn.swig.EID(bundle.getReportto().toString()));

                if (bundle.getCustodian() != null)
                    b.setCustodian(new de.tubs.ibr.dtn.swig.EID(bundle.getCustodian().toString()));

                // add flags from procflags
                b.setProcflags( new PrimaryBlockFlags( b.getProcflags().get() | bundle.getProcflags() ) );

                // open the file descriptor
                AutoCloseInputStream stream = new AutoCloseInputStream(fd);
                FileChannel inChannel = stream.getChannel();

                try {
                    de.tubs.ibr.dtn.swig.BundleID ret = null;

                    synchronized (mRegisterLock2) {
                        // put the primary block into the register
                        mNativeSession.put(RegisterIndex.REG2, b);

                        if (cb != null) {
                            cb.progress(0, -1);
                        }

                        int offset = 0;
                        int count = 0;

                        // allocate a buffer for data copying
                        ByteBuffer buffer = ByteBuffer.allocateDirect(8192);

                        // copy the data of the inChannel to the register 2
                        while ((count = inChannel.read(buffer)) > 0) {
                            // create a new data array
                            byte data[] = new byte[count];

                            // put the data into a byte array
                            buffer.flip();
                            buffer.get(data);

                            // write buffer to register 2
                            mNativeSession.write(RegisterIndex.REG2, data, offset);

                            // increment written data
                            offset += count;

                            // clear the buffer
                            buffer.clear();

                            // report progress
                            if (cb != null) {
                                cb.progress(offset, -1);
                            }
                        }

                        // send the bundle
                        ret = mNativeSession.send(RegisterIndex.REG2);
                    }

                    return toAndroid(ret);
                } finally {
                    try {
                        inChannel.close();
                        stream.close();
                    } catch (IOException e) {
                        Log.e(TAG, "channel close failed", e);
                    }
                }
            } catch (Exception e) {
                Log.e(TAG, "sendFileDescriptor failed", e);
                return null;
            }
        }

        @Override
        public boolean join(GroupEndpoint group) throws RemoteException {
            try {
                mManager.join(ClientSession.this, mSession, group);
                return true;
            } catch (NativeSessionException e) {
                // TODO remove stack-trace
                e.printStackTrace();
                return false;
            }
        }

        @Override
        public boolean leave(GroupEndpoint group) throws RemoteException {
            try {
                mManager.leave(ClientSession.this, mSession, group);
                return true;
            } catch (NativeSessionException e) {
                // TODO remove stack-trace
                e.printStackTrace();
                return false;
            }
        }

        @Override
        public List<GroupEndpoint> getGroups() throws RemoteException {
            ArrayList<GroupEndpoint> ret = new ArrayList<GroupEndpoint>();
            
            try {
                List<Endpoint> eids = mManager.getEndpoints(mSession);
                for (Endpoint e : eids) {
                    if (!e.isSingleton()) ret.add(e.asGroup());
                }
            } catch (NativeSessionException e) {
                // Error
            }
            
            return ret;
        }
    };

	public DTNSession getBinder()
	{
		return mBinder;
	}
	
	private static de.tubs.ibr.dtn.swig.BundleID toSwig(BundleID id)
	{
		de.tubs.ibr.dtn.swig.BundleID swigId = new de.tubs.ibr.dtn.swig.BundleID();
		swigId.setSource(new de.tubs.ibr.dtn.swig.EID(id.getSource().toString()));
		swigId.setSequencenumber(new DtnNumber(id.getSequencenumber()));
		swigId.setTimestamp(new DtnNumber(id.getTimestamp().getValue()));
		
		if (id.isFragment()) {
            swigId.setFragment(true);
            swigId.setFragmentoffset(new DtnNumber(id.getFragmentOffset()));
            swigId.setPayloadLength(id.getFragmentPayload());
		} else {
            swigId.setFragment(false);
            swigId.setFragmentoffset(new DtnNumber(0));
            swigId.setPayloadLength(0);
		}
		
		return swigId;
	}
	
	@SuppressWarnings("unused")
    private static de.tubs.ibr.dtn.swig.PrimaryBlock toSwig(Bundle bundle) {
		/*
		 * Convert API Bundle to SWIG bundle
		 */
		de.tubs.ibr.dtn.swig.PrimaryBlock ret = new de.tubs.ibr.dtn.swig.PrimaryBlock();
		ret.setCustodian(new de.tubs.ibr.dtn.swig.EID(bundle.getCustodian().toString()));
		ret.setDestination(new de.tubs.ibr.dtn.swig.EID(bundle.getDestination().toString()));
		
		ret.setLifetime( new DtnNumber( bundle.getLifetime().longValue() ) );
		ret.setProcflags( new PrimaryBlockFlags( bundle.getProcflags().longValue() ) );
		ret.setReportto(new de.tubs.ibr.dtn.swig.EID(bundle.getReportto().toString()));
		ret.setSequencenumber( new DtnNumber( bundle.getSequencenumber() ) );
		ret.setSource(new de.tubs.ibr.dtn.swig.EID(bundle.getSource().toString()));

		ret.setTimestamp( new DtnNumber( bundle.getTimestamp().getValue() ) );
		
        if (bundle.get(Bundle.ProcFlags.FRAGMENT)) {
            ret.setFragment(true);
            ret.setFragmentoffset(new DtnNumber(bundle.getFragmentOffset()));
            ret.setPayloadLength(bundle.getFragmentPayload());
            ret.setAppdatalength(new DtnNumber(bundle.getAppDataLength()));
        } else {
            ret.setFragment(false);
            ret.setFragmentoffset(new DtnNumber(0));
            ret.setPayloadLength(0);
            ret.setAppdatalength(new DtnNumber(0));
        }
		
		return ret;
	}
	
	private static Bundle toAndroid(PrimaryBlock block) {
		Bundle ret = new Bundle( block.getProcflags().get() );
		
		if (block.get(PrimaryBlock.FLAGS.DESTINATION_IS_SINGLETON)) {
			ret.setDestination( new SingletonEndpoint(block.getDestination().getString()) );
		} else {
			ret.setDestination( new GroupEndpoint(block.getDestination().getString()) );
		}

		ret.setSource( new SingletonEndpoint(block.getSource().getString()) );
		ret.setReportto( new SingletonEndpoint(block.getReportto().getString()) );
		ret.setCustodian( new SingletonEndpoint(block.getCustodian().getString()) );
		
		ret.setLifetime( block.getLifetime().get() );
		
		Timestamp ts = new Timestamp(block.getTimestamp().get());
		ret.setTimestamp( ts );
		
		ret.setSequencenumber( block.getSequencenumber().get() );

		if (block.get(PrimaryBlock.FLAGS.FRAGMENT)) {
			ret.setAppDataLength( block.getAppdatalength().get() );
			ret.setFragmentOffset( block.getFragmentoffset().get() );
			ret.setFragmentPayload( block.getPayloadLength() );
		} else {
            ret.setAppDataLength( 0L );
            ret.setFragmentOffset( 0L );
            ret.setFragmentPayload( 0L );
		}
		
		return ret;
	}
	
	private static Block toAndroid(de.tubs.ibr.dtn.swig.Block block) {
		Block ret = new Block();
		ret.type = Integer.valueOf(block.getType());
		ret.length = block.getLength();
		ret.procflags = block.getProcessingFlags().get();
		return ret;
	}
	
	private static BundleID toAndroid(de.tubs.ibr.dtn.swig.BundleID swigId) {
		// convert from swig BundleID to api BundleID
		BundleID id = new BundleID();
		id.setSequencenumber(swigId.getSequencenumber().get());
		id.setSource(new SingletonEndpoint(swigId.getSource().getString()));

		long swigTime = swigId.getTimestamp().get();
		Timestamp ts = new Timestamp(swigTime);
		id.setTimestamp(ts);
		
		if (swigId.isFragment()) {
		    id.setFragment(true);
		    id.setFragmentOffset(swigId.getFragmentoffset().get());
		    id.setFragmentPayload(swigId.getPayloadLength());
		} else {
            id.setFragment(false);
            id.setFragmentOffset(0L);
            id.setFragmentPayload(0L);
		}
		
		return id;
	}
	
    private static Date toAndroid(de.tubs.ibr.dtn.swig.DTNTime time) {
        long seconds = time.getTimestamp().get();
        long nanoseconds = time.getNanoseconds().get();
        long milliseconds = nanoseconds / 1000000;

        // convert to UNIX time (starting at 1970)
        Timestamp timestamp = new Timestamp( seconds );
        
        // add sub-seconds (millisecond resolution)
        Date d = new Date();
        d.setTime( timestamp.getValue() + milliseconds ); 
        
        return d;
    }
}
