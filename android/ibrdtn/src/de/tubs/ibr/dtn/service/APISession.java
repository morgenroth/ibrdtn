/*
 * APISession.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
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

import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;

import de.tubs.ibr.dtn.api.Block;
import de.tubs.ibr.dtn.api.Bundle;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.TransferMode;
import de.tubs.ibr.dtn.api.DTNSessionCallback;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.SessionDestroyedException;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.util.Log;

public class APISession {
	
	private final String TAG = "APISession";

	
//	private DefaultSABHandler sabhandler = new DefaultSABHandler() {
//
//		Bundle current_bundle = null;
//		Block current_block = null;
//		TransferMode current_callback = null;
//		ParcelFileDescriptor fd = null;
//		ByteArrayOutputStream stream = null;
//		CountingOutputStream counter = null;
//		BufferedWriter output = null;
//		
//		Long progress_last = 0L;
//		
//		// 0 = initial
//		// 1 = receiving
//		// 2 = done
//		int progress_state = 0;
//		
//		private void updateProgress()
//		{
//			if (current_block == null) return;
//			if (current_block.length == null) return;
//			if (current_block.type != 1) return;
//			
//			switch (progress_state)
//			{
//			// initial state
//			case 0:
//				// new block, announce zero
//				try {
//					_callback_wrapper.progress(0, current_block.length);
//				} catch (RemoteException e1) {
//					raise_callback_failure(e1);
//				}
//				
//				progress_state = 1;
//				progress_last = 0L;
//				break;
//				
//			case 1:
//				if (counter != null)
//				{
//					long newcount = counter.getCount();
//					if (newcount != progress_last)
//					{
//						// only announce if the 5% has changed
//						if ( (current_block.length / 20) <= (newcount - progress_last) )
//						{
//							try {
//								_callback_wrapper.progress(newcount, current_block.length);
//							} catch (RemoteException e1) {
//								raise_callback_failure(e1);
//							}
//
//							progress_last = newcount;
//						}
//					}
//				}
//				break;
//				
//			case 2:
//				try {
//					_callback_wrapper.progress(current_block.length, current_block.length);
//				} catch (RemoteException e1) {
//					raise_callback_failure(e1);
//				}
//				progress_state = 0;
//				break;
//			}
//		}
//
//		@Override
//		public void startBundle() {
//			current_bundle = new Bundle();
//			if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "startBundle");
//		}
//
//		@Override
//		public void endBundle() {
//			try {
//				_callback_wrapper.endBundle();
//			} catch (RemoteException e1) {
//				raise_callback_failure(e1);
//			}
//			
//			current_bundle = null;
//			if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "endBundle");
//		}
//
//		@Override
//		public void startBlock(Integer type) {
//			try {
//				_callback_wrapper.startBundle(current_bundle);
//			} catch (RemoteException e1) {
//				raise_callback_failure(e1);
//			}
//
//			current_block = new Block();
//			current_block.type = type;
//			if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "startBlock: " + String.valueOf(type));
//			
//			// set callback to null. This results in a query for the right
//			// callback strategy at the beginning of the payload.
//			current_callback = null;
//		}
//
//		private void initializeBlock() {
//			// announce this block and determine the payload transfer method
//			try {
//				current_callback = _callback_wrapper.startBlock(current_block);
//			} catch (RemoteException e1) {
//				raise_callback_failure(e1);
//			}
//
//			switch (current_callback) {
//			default:
//				break;
//
//			case FILEDESCRIPTOR:
//				// request file descriptor if requested by client
//				try {
//					fd = _callback_wrapper.fd();
//				} catch (RemoteException e1) {
//					raise_callback_failure(e1);
//				}
//				
//				if (fd != null)
//				{
//					if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "FD received");
//					
//					counter = new CountingOutputStream( new FileOutputStream(fd.getFileDescriptor()) );
//					output = new BufferedWriter( 
//								new OutputStreamWriter(
//									new de.tubs.ibr.dtn.util.Base64.OutputStream( 
//										counter,
//										de.tubs.ibr.dtn.util.Base64.DECODE
//										)
//									)
//								);
//				}
//				break;
//
//			case SIMPLE:
//				// in simple mode, we just create a bytearray stream
//				stream = new ByteArrayOutputStream();
//				counter = new CountingOutputStream( stream );
//				output = new BufferedWriter( 
//						new OutputStreamWriter(
//							new de.tubs.ibr.dtn.util.Base64.OutputStream( 
//								counter,
//								de.tubs.ibr.dtn.util.Base64.DECODE
//								)
//							)
//						);
//				break;
//			}
//		}
//
//		@Override
//		public void endBlock() {
//			// close output stream of the last block
//			if (output != null)
//			{
//				try {
//					output.flush();
//					output.close();
//				} catch (IOException e) { };
//				
//				output = null;
//				counter = null;
//			}
//			
//			// close filedescriptor if there one
//			if (fd != null)
//			{
//				try {
//					fd.close();
//				} catch (IOException e) { };
//				
//				fd = null;
//			}
//			else if (stream != null)
//			{
//				try {
//					stream.close();
//				} catch (IOException e) { };
//				
//				try {
//					_callback_wrapper.payload(stream.toByteArray());
//				} catch (RemoteException e1) {
//					raise_callback_failure(e1);
//				}
//				
//				stream = null;
//			}
//			
//			try {
//				_callback_wrapper.endBlock();
//			} catch (RemoteException e1) {
//				raise_callback_failure(e1);
//			}
//			
//			// update progress stats
//			progress_state = 2;
//			updateProgress();
//			
//			current_block = null;
//			if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "endBlock");
//		}
//
//		@Override
//		public void attribute(String keyword, String value) {
//			if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "attribute " + keyword + " = " + value);
//			
//			if (current_bundle != null)
//			{
//				if (current_block == null)
//				{
//					if (keyword.equalsIgnoreCase("source")) {
//						current_bundle.source = value;
//					}
//					else if (keyword.equalsIgnoreCase("destination")) {
//						current_bundle.destination = value;
//					}
//					else if (keyword.equalsIgnoreCase("timestamp")) {
//						ibrdtn.api.Timestamp ts = new ibrdtn.api.Timestamp( Long.parseLong(value) );
//						current_bundle.timestamp = ts.getDate();
//					}
//					else if (keyword.equalsIgnoreCase("sequencenumber")) {
//						current_bundle.sequencenumber = Long.parseLong(value);
//					}
//				}
//				else
//				{
//					if (keyword.equalsIgnoreCase("length")) {
//						current_block.length = Long.parseLong(value);
//					}
//				}
//			}
//		}
//
//		@Override
//		public void characters(String data) throws SABException {
//			if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "characters: " + data);
//			
//			// if the current callback strategy is set to null
//			// then announce the block via the startBlock() callback and
//			// ask for the right callback strategy.
//			if (current_callback == null) initializeBlock();
//
//			switch (current_callback)
//			{
//			case PASSTHROUGH:
//				try {
//					_callback_wrapper.characters(data);
//				} catch (RemoteException e1) {
//					raise_callback_failure(e1);
//				}
//				break;
//
//			default:
//				if (output != null)
//				{
//					try {
//						output.append(data);
//					} catch (IOException e) {
//						if (Log.isLoggable(TAG, Log.DEBUG)) Log.e(TAG, "Can not write data to output stream.", e);
//					}
//				}
//				break;
//			}
//			
//			// update progress stats
//			updateProgress();
//		}
//
//		@Override
//		public void notify(Integer type, String data) {
//			Log.i(TAG, "new notification " + String.valueOf(type) + ": " + data);
//			if (type == 602)
//			{
//				session.invoke_receive_intent(new BundleID());
//			}
//		}
//	};
}
