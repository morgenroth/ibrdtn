package de.tubs.ibr.dtn;

import java.util.List;

import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ResolveInfo;
import android.content.pm.ServiceInfo;
import de.tubs.ibr.dtn.api.ServiceNotAvailableException;

public class Services {
	/**
	 * Version = 0 (< 0.13)
	 */
	public static final Integer VERSION_APPLICATION = 0;
	
	/**
	 * Version = 0 (< 0.13)
	 */
	public static final Integer VERSION_MANAGER = 0;
	
	/**
	 * Version = 0 (< 0.13)
	 */
	public static final Integer VERSION_SECURITY = 0;
	
	public static final Service SERVICE_APPLICATION = new Service(DTNService.class.getName(), VERSION_APPLICATION);
	public static final Service SERVICE_MANAGER = new Service(DtnManager.class.getName(), VERSION_MANAGER);
	public static final Service SERVICE_SECURITY = new Service(SecurityService.class.getName(), VERSION_SECURITY);
	
	public static final String EXTRA_VERSION = "de.tubs.ibr.dtn.Service.VERSION";
	public static final String EXTRA_NAME = "de.tubs.ibr.dtn.Service.NAME";
	
	public static class Service {
		private final String mClassName;
		private final Integer mVersion;
		
		private Service(String className, Integer version) {
			mClassName = className;
			mVersion = version;
		}
		
		public String getClassName() {
			return mClassName;
		}
		
		public Integer getVersion() {
			return mVersion;
		}
		
		public void bind(Context context, ServiceConnection conn, int flags) throws ServiceNotAvailableException {
			Intent queryIntent = new Intent(mClassName);
			List<ResolveInfo> list = context.getPackageManager().queryIntentServices(queryIntent, 0);
			if (list.size() == 0) throw new ServiceNotAvailableException();
			
			// get the first found service
			ServiceInfo serviceInfo = list.get(0).serviceInfo;
			
			// create bind intent to the first service
			Intent bindIntent = new Intent(mClassName);
			bindIntent.putExtra(EXTRA_NAME, mClassName);
			bindIntent.putExtra(EXTRA_VERSION, mVersion);
			bindIntent.setClassName(serviceInfo.packageName, serviceInfo.name);
			
			// bind to the service
			context.bindService(bindIntent, conn, flags);
		}
		
		public boolean match(Intent intent) {
			// the requested service name
			String name = intent.getStringExtra(Services.EXTRA_NAME);
			
			// check if API version matches
			if (!mClassName.equals(name)) return false;
			
			// the requested API version
			Integer version = intent.getIntExtra(Services.EXTRA_VERSION, 0);
			
			// check if API version matches
			if (version != mVersion) return false;
			
			return true;
		}
	}
}
