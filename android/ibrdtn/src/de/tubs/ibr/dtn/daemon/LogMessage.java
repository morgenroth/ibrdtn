/*
 * LogMessage.java
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
package de.tubs.ibr.dtn.daemon;

import java.util.HashMap;

import android.util.Log;

public class LogMessage {
	private static final String TAG = "LogMessage";
	public static HashMap<String, String> TAG_LABELS;

	static
	{
		TAG_LABELS = new HashMap<String, String>();
		TAG_LABELS.put("E", "Exception");
		TAG_LABELS.put("W", "Warning");
		TAG_LABELS.put("I", "Information");
		TAG_LABELS.put("D", "Debug");
		TAG_LABELS.put("V", "Verbose");
	}

	public String date;
	public String tag;
	public String msg;

	public LogMessage(String raw) {
		try
		{
			int secondSpace = raw.indexOf(' ', raw.indexOf(' ', 0) + 1);

			date = raw.substring(0, secondSpace).trim();
			tag = raw.substring(secondSpace, raw.indexOf('/', 0)).trim();
			msg = raw.substring(raw.indexOf(':', secondSpace) + 1, raw.length()).trim();
		} catch (StringIndexOutOfBoundsException e)
		{
			Log.e(TAG, "Error while parsing the log messages", e);
			if (date == null) date = "";
			tag = "";
			msg = raw;
		}
	}

}
