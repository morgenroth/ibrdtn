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

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.Date;

import android.content.Context;
import android.content.Intent;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.util.Log;
import de.tubs.ibr.dtn.api.Block;
import de.tubs.ibr.dtn.api.Bundle;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.DTNSession;
import de.tubs.ibr.dtn.api.DTNSessionCallback;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.api.TransferMode;
import de.tubs.ibr.dtn.swig.BundleNotFoundException;
import de.tubs.ibr.dtn.swig.NativeSerializerCallback;
import de.tubs.ibr.dtn.swig.NativeSession;
import de.tubs.ibr.dtn.swig.NativeSession.RegisterIndex;
import de.tubs.ibr.dtn.swig.NativeSessionCallback;
import de.tubs.ibr.dtn.swig.NativeSessionException;
import de.tubs.ibr.dtn.swig.PrimaryBlock;
import de.tubs.ibr.dtn.swig.StatusReportBlock;

public class ClientSession {

	private final static String TAG = "ClientSession";

	private String _package_name = null;

	private Context context = null;
	private Registration _registration = null;
	
	private SerializerCallback _serializer_callback = new SerializerCallback();

	/**
	 * Implemented C++ callback using SWIG directors
	 * 
	 * see http://stackoverflow.com/questions/8168517/generating-java-interface-
	 * with-swig/8246375#8246375
	 */
	private NativeSessionCallback _session_callback = new NativeSessionCallback() {

		@Override
		public void notifyBundle(de.tubs.ibr.dtn.swig.BundleID swigId)
		{
	        // forward the notification as intent
	        // create a new intent
	        Intent notify = new Intent(de.tubs.ibr.dtn.Intent.RECEIVE);
	        notify.addCategory(_package_name);
	        notify.putExtra("bundleid", toAndroid(swigId));

	        // send notification intent
	        context.sendBroadcast(notify);

	        Log.d(TAG, "RECEIVE intent (" + swigId.toString() + ") sent to " + _package_name);
		}

		@Override
		public void notifyStatusReport(de.tubs.ibr.dtn.swig.EID source, de.tubs.ibr.dtn.swig.StatusReportBlock swigReport)
		{
		    de.tubs.ibr.dtn.swig.BundleID swigId = swigReport.get_bundleid();
		    
            // forward the notification as intent
            // create a new intent
            Intent notify = new Intent(de.tubs.ibr.dtn.Intent.STATUS_REPORT);
            notify.addCategory(_package_name);
            notify.putExtra("bundleid", toAndroid(swigId));
            notify.putExtra("source", new SingletonEndpoint(source.getString()));
            notify.putExtra("status", swigReport.get_status());
            notify.putExtra("reason", swigReport.get_reasoncode());
            
            char status = swigReport.get_status();
            
            if (0 < (status & StatusReportBlock.TYPE.RECEIPT_OF_BUNDLE.swigValue())) {
                notify.putExtra("timeof_receipt", toAndroid(swigReport.get_timeof_receipt()));
            }
            
            if (0 < (status & StatusReportBlock.TYPE.DELETION_OF_BUNDLE.swigValue())) {
                notify.putExtra("timeof_deletion", toAndroid(swigReport.get_timeof_deletion()));
            }
            
            if (0 < (status & StatusReportBlock.TYPE.DELIVERY_OF_BUNDLE.swigValue())) {
                notify.putExtra("timeof_delivery", toAndroid(swigReport.get_timeof_delivery()));
            }
            
            if (0 < (status & StatusReportBlock.TYPE.FORWARDING_OF_BUNDLE.swigValue())) {
                notify.putExtra("timeof_forwarding", toAndroid(swigReport.get_timeof_forwarding()));
            }
            
            if (0 < (status & StatusReportBlock.TYPE.CUSTODY_ACCEPTANCE_OF_BUNDLE.swigValue())) {
                notify.putExtra("timeof_custodyaccept", toAndroid(swigReport.get_timeof_custodyaccept()));
            }
            
            // send notification intent
            context.sendBroadcast(notify);

            Log.d(TAG, "STATUS_REPORT intent [" + swigId.toString() + "] sent to " + _package_name);
		}

		@Override
		public void notifyCustodySignal(de.tubs.ibr.dtn.swig.EID source, de.tubs.ibr.dtn.swig.CustodySignalBlock swigCustody)
		{
		    de.tubs.ibr.dtn.swig.BundleID swigId = swigCustody.get_bundleid();
		    
            // forward the notification as intent
            // create a new intent
            Intent notify = new Intent(de.tubs.ibr.dtn.Intent.CUSTODY_SIGNAL);
            notify.addCategory(_package_name);
            notify.putExtra("bundleid", toAndroid(swigId));
            notify.putExtra("source", new SingletonEndpoint(source.getString()));
            notify.putExtra("accepted", swigCustody.get_custody_accepted());
            notify.putExtra("timeofsignal", toAndroid(swigCustody.get_timeofsignal()));

            // send notification intent
            context.sendBroadcast(notify);

            Log.d(TAG, "CUSTODY_SIGNAL intent [" + swigId.toString() + "] sent to " + _package_name);
		}

	};
	
    private static class SerializerCallback extends NativeSerializerCallback {
        
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
                    ParcelFileDescriptor target_fd = _cb.fd();
                    _output = new FileOutputStream(target_fd.getFileDescriptor());
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

	/*
	 * This is the actual native implementation of this session. See
	 * daemon/src/api/NativeSession.cpp. Java parts are generated by SWIG
	 */
	private NativeSession nativeSession = null;
	
	private Thread _receiver_thread = new Thread() {

        @Override
        public void run() {
            try {
                while (!this.isInterrupted()) {
                    ClientSession.this.nativeSession.receive();
                }
            } catch (NativeSessionException e) {
                Log.e(TAG, "Receiver thread terminated.", e);
            }
        }
	    
	};

	public ClientSession(Context context, Registration reg, String packageName) {
	    this.nativeSession = new NativeSession(_session_callback, _serializer_callback);
		this.context = context;
		this._package_name = packageName;
		this._registration = reg;
		
        // Register application
        register(_registration);
        
        // start-up receiver thread
        _receiver_thread.start();
	}
	
	/**
	 * destroy the session
	 */
	public void destroy() {
	    this.nativeSession.destroy();
	    try {
	        this._receiver_thread.join();
        } catch (InterruptedException e) {
            Log.e(TAG, "Join on receiver thread failed.", e);
        }
	    
	    this.nativeSession.delete();
	    this.nativeSession = null;
	    
	    this._session_callback.delete();
	    this._session_callback = null;
	    
	    this._serializer_callback.delete();
	    this._serializer_callback = null;
	}
	
	public void update(Registration reg) {
	    this._registration = reg;
	    
	    // remove all registrations
	    nativeSession.clearRegistration();
	    
        // register again
        register(_registration);
	}

	public void register(Registration reg)
	{
		try {
		    // set local endpoint
            nativeSession.setEndpoint(reg.getEndpoint());
            
            if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "endpoint registered: " + reg.getEndpoint());

            for (GroupEndpoint group : reg.getGroups()) {
                de.tubs.ibr.dtn.swig.EID swigEid = new de.tubs.ibr.dtn.swig.EID(group.toString());
                nativeSession.addRegistration(swigEid);
                
                if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "registration added: " + group.toString());
            }

            // send out registration intent to the application
            Intent broadcastIntent = new Intent(de.tubs.ibr.dtn.Intent.REGISTRATION);
            broadcastIntent.addCategory(_package_name);
            broadcastIntent.putExtra("key", _package_name);

            // send notification intent
            context.sendBroadcast(broadcastIntent);

            Log.d(TAG, "REGISTRATION intent sent to " + _package_name);
        } catch (NativeSessionException e) {
            Log.e(TAG, "registration failed", e);
        }
	}

	/**
	 * This is the actual implementation of the DTNSession API
	 */
	private final DTNSession.Stub mBinder = new DTNSession.Stub() {
        public boolean query(DTNSessionCallback cb, BundleID id) throws RemoteException
        {
            synchronized(_serializer_callback) {
                // set serializer for this query
                _serializer_callback.setCallback(cb);
                
                try {
                    // load the bundle into the register
                    nativeSession.load(NativeSession.RegisterIndex.REG1, toSwig(id));
    
                    // get the bundle
                    nativeSession.get(NativeSession.RegisterIndex.REG1);
                    
                    // set serializer back to null
                    _serializer_callback.setCallback(null);
    
                    // bundle loaded - return true
                    return true;
                } catch (BundleNotFoundException e) {
                    // set serializer back to null
                    _serializer_callback.setCallback(null);
                    
                    // bundle not found - return false
                    return false;
                }
            }
        }

        public boolean queryNext(DTNSessionCallback cb) throws RemoteException
        {
            synchronized(_serializer_callback) {
                // set serializer for this query
                _serializer_callback.setCallback(cb);
                
                try {
                    // load the next bundle into the register
                    nativeSession.next(NativeSession.RegisterIndex.REG1);
    
                    // get the bundle
                    nativeSession.get(NativeSession.RegisterIndex.REG1);
                    
                    // set serializer back to null
                    _serializer_callback.setCallback(null);
    
                    // bundle loaded - return true
                    return false;
                } catch (BundleNotFoundException e) {
                    // set serializer back to null
                    _serializer_callback.setCallback(null);
                    
                    // bundle not found - return false
                    return false;
                }
            }
        }

		public boolean delivered(BundleID id) throws RemoteException
		{
			try {
                nativeSession.delivered(toSwig(id));
                return true;
            } catch (BundleNotFoundException e) {
                return false;
            }
		}

		@Override
		public BundleID send(DTNSessionCallback cb, Bundle bundle, byte[] data) throws RemoteException {
			try {
				PrimaryBlock b = new PrimaryBlock();

				b.set(PrimaryBlock.FLAGS.DESTINATION_IS_SINGLETON, bundle.get(Bundle.ProcFlags.DESTINATION_IS_SINGLETON));
				b.set_destination(new de.tubs.ibr.dtn.swig.EID(bundle.getDestination().toString()));
				
				// set lifetime
				if (bundle.getLifetime() != null)
					b.set_lifetime(bundle.getLifetime());
				
				if (bundle.getReportto() != null)
					b.set_reportto(new de.tubs.ibr.dtn.swig.EID(bundle.getReportto().toString()));
				
				if (bundle.getCustodian() != null)
					b.set_custodian(new de.tubs.ibr.dtn.swig.EID(bundle.getCustodian().toString()));
				
				// add flags from procflags
				b.set_procflags( b.get_procflags() | bundle.getProcflags() );

				// put the primary block into the register
				nativeSession.put(RegisterIndex.REG2, b);
				
				if (cb != null) {
					cb.progress(0, data.length);
				}
				
				// add data
				nativeSession.write(RegisterIndex.REG2, data);
				
				if (cb != null) {
					cb.progress(data.length, data.length);
				}
				
				// send the bundle
				de.tubs.ibr.dtn.swig.BundleID ret = nativeSession.send(RegisterIndex.REG2);

				return toAndroid(ret);
			} catch (Exception e) {
				Log.e(TAG, "send failed", e);
				return null;
			}
		}

		@Override
		public BundleID sendFileDescriptor(DTNSessionCallback cb, Bundle bundle, ParcelFileDescriptor fd, long length) throws RemoteException {
			try {
				if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "Received file descriptor as bundle payload.");
				
				PrimaryBlock b = new PrimaryBlock();
				
				b.set(PrimaryBlock.FLAGS.DESTINATION_IS_SINGLETON, bundle.get(Bundle.ProcFlags.DESTINATION_IS_SINGLETON));
				b.set_destination(new de.tubs.ibr.dtn.swig.EID(bundle.getDestination().toString()));
				
				// set lifetime
				if (bundle.getLifetime() != null)
					b.set_lifetime(bundle.getLifetime());
				
				if (bundle.getReportto() != null)
					b.set_reportto(new de.tubs.ibr.dtn.swig.EID(bundle.getReportto().toString()));
				
				if (bundle.getCustodian() != null)
					b.set_custodian(new de.tubs.ibr.dtn.swig.EID(bundle.getCustodian().toString()));
				
				// add flags from procflags
				b.set_procflags( b.get_procflags() | bundle.getProcflags() );

				// put the primary block into the register
				nativeSession.put(RegisterIndex.REG2, b);
				
				if (cb != null) {
					cb.progress(0, length);
				}
				
				FileInputStream stream = new FileInputStream(fd.getFileDescriptor());
                FileChannel inChannel = stream.getChannel();
                try {
                    int offset = 0;
                    int count = 0;
                    ByteBuffer buffer = ByteBuffer.allocateDirect(8192);
                    while ((count = inChannel.read(buffer)) > 0) {
                        // add data
                        byte data[] = new byte[count];
                        buffer.flip();
                        buffer.get(data);
                        nativeSession.write(RegisterIndex.REG2, data, offset);
                        offset += count;
                        buffer.clear();
                        
        				if (cb != null) {
        					cb.progress(offset, length);
        				}
                    }

                    // send the bundle
                    de.tubs.ibr.dtn.swig.BundleID ret = nativeSession.send(RegisterIndex.REG2);

                    return toAndroid(ret);
                } finally {
                    try {
                        inChannel.close();
                        stream.close();
                        fd.close();
                    } catch (IOException e) {
                    }
                }
			} catch (Exception e) {
				Log.e(TAG, "sendFileDescriptor failed", e);
				return null;
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
	
	private static de.tubs.ibr.dtn.swig.BundleID toSwig(BundleID id)
	{
		de.tubs.ibr.dtn.swig.BundleID swigId = new de.tubs.ibr.dtn.swig.BundleID();
		swigId.setSource(new de.tubs.ibr.dtn.swig.EID(id.getSource().toString()));
		swigId.setSequencenumber(BigInteger.valueOf(id.getSequencenumber()));

		Timestamp ts = new Timestamp(id.getTimestamp());
		swigId.setTimestamp(BigInteger.valueOf(ts.getValue()));
		
		return swigId;
	}
	
	@SuppressWarnings("unused")
    private static de.tubs.ibr.dtn.swig.PrimaryBlock toSwig(Bundle bundle) {
		/*
		 * Convert API Bundle to SWIG bundle
		 */
		de.tubs.ibr.dtn.swig.PrimaryBlock ret = new de.tubs.ibr.dtn.swig.PrimaryBlock();
		ret.set_custodian(new de.tubs.ibr.dtn.swig.EID(bundle.getCustodian().toString()));
		ret.set_destination(new de.tubs.ibr.dtn.swig.EID(bundle.getDestination().toString()));
		ret.set_fragmentoffset(bundle.getFragmentOffset());
		ret.set_lifetime(bundle.getLifetime());
		ret.set_procflags(bundle.getProcflags());
		ret.set_reportto(new de.tubs.ibr.dtn.swig.EID(bundle.getReportto().toString()));
		ret.set_sequencenumber(bundle.getSequencenumber());
		ret.set_source(new de.tubs.ibr.dtn.swig.EID(bundle.getSource().toString()));

		Timestamp ts = new Timestamp(bundle.getTimestamp());
		ret.set_timestamp(ts.getValue());
		
		return ret;
	}
	
	private static Bundle toAndroid(PrimaryBlock block) {
		Bundle ret = new Bundle( block.get_procflags() );
		
		if (block.get(PrimaryBlock.FLAGS.DESTINATION_IS_SINGLETON)) {
			ret.setDestination( new SingletonEndpoint(block.get_destination().getString()) );
		} else {
			ret.setDestination( new GroupEndpoint(block.get_destination().getString()) );
		}

		ret.setSource( new SingletonEndpoint(block.get_source().getString()) );
		ret.setReportto( new SingletonEndpoint(block.get_reportto().getString()) );
		ret.setCustodian( new SingletonEndpoint(block.get_custodian().getString()) );
		
		ret.setLifetime( block.get_lifetime());
		
		Timestamp ts = new Timestamp(block.get_timestamp());
		ret.setTimestamp( ts.getDate() );
		
		ret.setSequencenumber( block.get_sequencenumber() );

		if (block.get(PrimaryBlock.FLAGS.FRAGMENT)) {
			ret.setAppDataLength( block.get_appdatalength() );
			ret.setFragmentOffset( block.get_fragmentoffset() );
		}
		
		return ret;
	}
	
	private static Block toAndroid(de.tubs.ibr.dtn.swig.Block block) {
		Block ret = new Block();
		ret.type = Integer.valueOf(block.getType());
		ret.length = block.getLength();
		ret.procflags = block.getProcessingFlags();
		return ret;
	}
	
	private static BundleID toAndroid(de.tubs.ibr.dtn.swig.BundleID swigId) {
		// convert from swig BundleID to api BundleID
		BundleID id = new BundleID();
		id.setSequencenumber(swigId.getSequencenumber().longValue());
		id.setSource(new SingletonEndpoint(swigId.getSource().getString()));

		long swigTime = swigId.getTimestamp().longValue();
		Timestamp ts = new Timestamp(swigTime);
		id.setTimestamp(ts.getDate());
		
		return id;
	}
	
    private static Date toAndroid(de.tubs.ibr.dtn.swig.DTNTime time) {
        long seconds = time.getTimestamp().getValue().longValue();
        long nanoseconds = time.getNanoseconds().getValue().longValue();
        long milliseconds = nanoseconds / 1000000;

        // convert to UNIX time (starting at 1970)
        Timestamp timestamp = new Timestamp( seconds );
        
        // add sub-seconds (millisecond resolution)
        Date d = new Date();
        d.setTime( timestamp.getValue() + milliseconds ); 
        
        return d;
    }
}
