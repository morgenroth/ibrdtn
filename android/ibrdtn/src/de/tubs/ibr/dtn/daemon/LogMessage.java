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
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class LogMessage {
    public static HashMap<String, String> LEVEL_LABELS;

    // try to match something like this:
    // 04-04 22:36:25.865 I/IBR-DTN/RequeueBundleEvent( 1336): Bundle...
    private static final String LOGCAT_LINE_REGEX = "^(\\S+\\s\\S+)\\s(\\w)/IBR-DTN/(.*)\\(\\s*\\d+\\)\\:\\s(.*)$";
    private static Pattern mLinePattern;
    private static Matcher mLineMatcher;

    static {
        LEVEL_LABELS = new HashMap<String, String>();
        LEVEL_LABELS.put("E", "Exception");
        LEVEL_LABELS.put("W", "Warning");
        LEVEL_LABELS.put("I", "Information");
        LEVEL_LABELS.put("D", "Debug");
        LEVEL_LABELS.put("V", "Verbose");

        mLinePattern = Pattern.compile(LOGCAT_LINE_REGEX, Pattern.CASE_INSENSITIVE);
    }

    public String date;
    public String level;
    public String tag;
    public String msg;

    public LogMessage(String raw) throws IllegalStateException {
        mLineMatcher = mLinePattern.matcher(raw);

        if (mLineMatcher.matches()) {
            date = mLineMatcher.group(1);
            level = mLineMatcher.group(2);
            tag = mLineMatcher.group(3);
            msg = mLineMatcher.group(4);
        } else {
            throw new IllegalStateException("Does not match: " + raw);
        }
    }

}
