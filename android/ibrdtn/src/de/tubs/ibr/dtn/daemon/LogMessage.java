/*
 * LogMessage.java
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
package de.tubs.ibr.dtn.daemon;

import android.util.Log;

public class LogMessage {
	
	public String date;
	public String tag;
	public String msg;

	public LogMessage(String raw) {
		try {
			date = raw.substring(0, 24).trim();
			tag = raw.substring(24, raw.indexOf(':', 24)).trim();
			msg = raw.substring(raw.indexOf(':', 24) + 2, raw.length());
		} catch (java.lang.StringIndexOutOfBoundsException e) {
			Log.e("LogMessage", "Error while parsing the log messages", e);
			if (date == null) date = "";
			tag = "";
			msg = raw;
		}
	}
}
