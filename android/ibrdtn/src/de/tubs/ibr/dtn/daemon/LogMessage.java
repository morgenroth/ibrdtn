package de.tubs.ibr.dtn.daemon;

public class LogMessage {
	
	public String date;
	public String tag;
	public String msg;

	public LogMessage(String raw) {
		date = raw.substring(0, 24).trim();
		tag = raw.substring(24, raw.indexOf(':', 24)).trim();
		msg = raw.substring(raw.indexOf(':', 24) + 2, raw.length());
	}
}
