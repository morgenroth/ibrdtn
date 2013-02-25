/*
 * NativeLibraryWrapper.java
 * 
 * Copyright (C) 2013 IBR, TU Braunschweig
 *
 * Written-by: Dominik Schürmann <dominik@dominikschuermann.de>
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

import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.EID;
import ibrdtn.api.object.InputStreamBlockData;
import ibrdtn.api.object.PayloadBlock;
import ibrdtn.api.object.PlainSerializer;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.util.Log;

/**
 * Native methods are implemented in
 * jni/native-library-wrapper/NativeLibraryWrapper.cpp
 * 
 * C/C++ header file (jni/native-library-wrapper/NativeLibraryWrapper.h) can be
 * generated using javah:
 * 
 * <pre>
 * 1. Compile Android project
 * 2. javah -classpath bin/classes de.tubs.ibr.dtn.service.NativeDaemonWrapper
 * </pre>
 */
public class NativeDaemonWrapper {
	private final static String TAG = "IBR-DTN NativeLibraryWrapper Java";

	private final static String GNUSTL_NAME = "gnustl_shared";
	private final static String CRYPTO_NAME = "crypto";
	private final static String SSL_NAME = "ssl";
	private final static String IBRCOMMON_NAME = "ibrcommon";
	private final static String IBRDTN_NAME = "ibrdtn";
	private final static String DTND_NAME = "dtnd";
	private final static String NATIVE_LIBRARY_WRAPPER_NAME = "native-library-wrapper";

	/**
	 * Loads all shared libraries in the right order with System.loadLibrary()
	 */
	private static void loadLibraries()
	{
		try
		{
			System.loadLibrary(GNUSTL_NAME);

			// System.loadLibrary(CRYPTO_NAME);
			// System.loadLibrary(SSL_NAME);

			System.loadLibrary(IBRCOMMON_NAME);
			System.loadLibrary(IBRDTN_NAME);
			System.loadLibrary(DTND_NAME);

			System.loadLibrary(NATIVE_LIBRARY_WRAPPER_NAME);
		} catch (UnsatisfiedLinkError e)
		{
			Log.e(TAG, "UnsatisfiedLinkError! Are you running special hardware?", e);
		} catch (Exception e)
		{
			Log.e(TAG, "Loading the libraries failed!", e);
		}
	}

	static
	{
		// load libraries on first use of this class
		loadLibraries();
	}

	// can not be initialized
	private NativeDaemonWrapper() {
	}

	public static native void daemonInitialize(String configPath, boolean enableDebug);

	public static native void daemonMainLoop();

	public static native void daemonShutdown();

	public static native String[] getNeighbors();

	public static native void addConnection(String eid, String protocol, String address, String port);

	public static native void removeConnection(String eid, String protocol, String address, String port);

	public static native void setEndpoint(String id);

	public static native void addRegistration(String eid);

	public static native void loadBundle(String id);

	public static native void getBundle();

	public static native void loadAndGetBundle();

	public static native void markDelivered(String bundleId);

	public static native void send(byte[] output);

	/**
	 * Send a bundle directly to the daemon.
	 * 
	 * @param bundle
	 *            The bundle to send.
	 * @throws APIException
	 *             If the transmission fails.
	 */
	public static synchronized void send(Bundle bundle)
	{
		// TODO: implement native

		ByteArrayOutputStream out = new ByteArrayOutputStream();

		// // clear the previous bundle first
		// if (query("bundle clear") != 200) throw new
		// APIException("bundle clear failed");
		//
		// // announce a proceeding plain bundle
		// if (query("bundle put plain") != 100) throw new
		// APIException("bundle put failed");

		PlainSerializer serializer = new PlainSerializer(out);

		try
		{
			serializer.serialize(bundle);
		} catch (IOException e)
		{
			// throw new APIException("serialization of bundle failed.");
		}

		send(out.toByteArray());

		// if (_receiver.getResponse().getCode() != 200)
		// {
		// throw new APIException("bundle rejected or put failed");
		// }

		// send the bundle away
		// if (query("bundle send") != 200) throw new
		// APIException("bundle send failed");
	}

	/**
	 * Send a bundle directly to the daemon.
	 * 
	 * @param destination
	 *            The destination of the bundle.
	 * @param lifetime
	 *            The lifetime of the bundle.
	 * @param data
	 *            The payload as byte-array.
	 * @throws APIException
	 *             If the transmission fails.
	 */
	public static void send(EID destination, Integer lifetime, byte[] data)
	{
		send(destination, lifetime, data, Bundle.Priority.NORMAL);
	}

	/**
	 * Send a bundle directly to the daemon.
	 * 
	 * @param destination
	 *            The destination of the bundle.
	 * @param lifetime
	 *            The lifetime of the bundle.
	 * @param data
	 *            The payload as byte-array.
	 * @param priority
	 *            The priority of the bundle
	 * @throws APIException
	 *             If the transmission fails.
	 */
	public static void send(EID destination, Integer lifetime, byte[] data, Bundle.Priority priority)
	{
		// wrapper to the send(Bundle) function
		Bundle bundle = new Bundle(destination, lifetime);
		bundle.appendBlock(new PayloadBlock(data));
		bundle.setPriority(priority);
		send(bundle);
	}

	/**
	 * Send a bundle directly to the daemon. The given stream is used as payload
	 * of the bundle.
	 * 
	 * @param destination
	 *            The destination of the bundle.
	 * @param lifetime
	 *            The lifetime of the bundle.
	 * @param stream
	 *            The stream containing the payload data.
	 * @param length
	 *            The length of the payload.
	 * @throws APIException
	 *             If the transmission fails.
	 */
	public static void send(EID destination, Integer lifetime, InputStream stream, Long length)
	{
		send(destination, lifetime, stream, length, Bundle.Priority.NORMAL);
	}

	/**
	 * Send a bundle directly to the daemon. The given stream is used as payload
	 * of the bundle.
	 * 
	 * @param destination
	 *            The destination of the bundle.
	 * @param lifetime
	 *            The lifetime of the bundle.
	 * @param stream
	 *            The stream containing the payload data.
	 * @param length
	 *            The length of the payload.
	 * @param priority
	 *            The priority of the bundle
	 * @throws APIException
	 *             If the transmission fails.
	 */
	public static void send(EID destination, Integer lifetime, InputStream stream, Long length, Bundle.Priority priority)
	{
		Bundle bundle = new Bundle(destination, lifetime);
		bundle.appendBlock(new PayloadBlock(new InputStreamBlockData(stream, length)));
		bundle.setPriority(priority);
		send(bundle);
	}

	@Override
	protected void finalize() throws Throwable
	{
		Log.d(TAG, "NativeDaemonWrapper was finalized! GC!");
		super.finalize();
	}

}
