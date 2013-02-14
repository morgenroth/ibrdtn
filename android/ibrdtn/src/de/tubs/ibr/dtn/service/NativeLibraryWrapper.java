package de.tubs.ibr.dtn.service;

import android.os.Handler;
import android.util.Log;

/**
 * Generate JNI header file:
 * 
 * <pre>
 * 1. Compile project
 * 2. javah -classpath bin/classes de.tubs.ibr.dtn.service.NativeLibraryWrapper
 * </pre>
 * 
 */
public class NativeLibraryWrapper {
	static Handler h;

	public NativeLibraryWrapper() {
	}

	public NativeLibraryWrapper(Handler h) {
		this.h = h;
	}

	public static native void daemonInitialize();

	public static native void daemonMainLoop();

	public static native void daemonShutdown();

	// public static native void ibrdtn_daemon_runtime_debug(boolean val);

	// public static native void ibrdtn_daemon_reload();

	private final static String TAG = "DaemonLibrary";

	private final static String STD_LIB_NAME = "gnustl_shared";
	private final static String CRYPTO_NAME = "crypto";
	private final static String SSL_NAME = "ssl";

	private final static String IBRCOMMON_NAME = "ibrcommon";
	private final static String IBRDTN_NAME = "ibrdtn";
	private final static String DTND_NAME = "dtnd";

	private final static String NATIVE_LIBRARY_WRAPPER_NAME = "native-library-wrapper";

	private boolean tryToLoadStack() {
		try {
			// Try to load the stack
			// System.loadLibrary(CRYPTO_NAME);
			// System.loadLibrary(SSL_NAME);
			System.loadLibrary(STD_LIB_NAME);

			System.loadLibrary(IBRCOMMON_NAME);
			System.loadLibrary(IBRDTN_NAME);
			System.loadLibrary(DTND_NAME);

			System.loadLibrary(NATIVE_LIBRARY_WRAPPER_NAME);

			return true;
		} catch (UnsatisfiedLinkError e) {
			Log.e(TAG,
					"UnsatisfiedLinkError! Are you running special hardware?",
					e);
			return false;
		} catch (Exception e) {
			Log.e(TAG, "Loading the stack failed!", e);
		}
		return false;
	}

	public void start() throws Exception {
		if (!tryToLoadStack()) {
			throw new Exception("Native libraries were not loaded correctly!");
		}

		daemonInitialize();
	}

}
