package ibrdtn.api;

import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

public class Timestamp {
	
	public final static Long TIMEVAL_CONVERSION = 946684800L;
	
	private Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
	
	public Timestamp(Date d)
	{
		calendar.setTime(d);
	}
	
	public Timestamp(Long dtntimestamp)
	{
		Long utc_timestamp = (dtntimestamp + Timestamp.TIMEVAL_CONVERSION) * 1000;
		calendar.setTimeInMillis(utc_timestamp);
	}
	
	public Date getDate()
	{
		return calendar.getTime();
	}
	
	public Long getValue()
	{
		return (getDate().getTime() / 1000) - Timestamp.TIMEVAL_CONVERSION;
	}
}
