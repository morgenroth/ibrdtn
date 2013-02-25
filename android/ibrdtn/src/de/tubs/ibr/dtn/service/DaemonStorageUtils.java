/*
 * StorageUtils.java
 * 
 * Copyright (C) 2013 IBR, TU Braunschweig
 *
 * Written-by: Dominik Schürmann <dominik@dominikschuermann.de>
 * 	           Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
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

import java.io.File;

import android.os.Environment;

public class DaemonStorageUtils {
	public static synchronized void clearStorage()
	{
		// check if the daemon is running
		// if (isRunning())
		// return;

		// clear the storage directory
		File bundles = getStoragePath("bundles");
		if (bundles == null) return;

		// flush storage path
		File[] files = bundles.listFiles();
		if (files != null)
		{
			for (File f : files)
			{
				f.delete();
			}
		}
	}

	public static File getStoragePath(String subdir)
	{
		if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED))
		{
			File externalStorage = Environment.getExternalStorageDirectory();
			return new File(externalStorage.getPath() + File.separatorChar + "ibrdtn" + File.separatorChar + subdir);
		}

		return null;
	}
}
