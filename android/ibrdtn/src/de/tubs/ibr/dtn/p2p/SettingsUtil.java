package de.tubs.ibr.dtn.p2p;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.preference.PreferenceManager;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.p2p.scheduler.GridStrategy;
import de.tubs.ibr.dtn.p2p.scheduler.ProbalisticStrategy;
import de.tubs.ibr.dtn.p2p.scheduler.StategyAlwaysOn;
import de.tubs.ibr.dtn.p2p.scheduler.Strategy;
import de.tubs.ibr.dtn.p2p.scheduler.TwentyOnTenOfStrategy;

public final class SettingsUtil {

    private SettingsUtil() {
    }

    private static final String KEY_NEXT_SCHEDULED_CHECK = "nextScheduledCheck";
    private static final String KEY_SCHEDULER_INFO = "schedulerInfo";
    private static final String KEY_SCHEDULER_STOPPED = "schedulerStopped";
    
    public static final String KEY_P2P_ENABLED = "p2p_enabled";
    public static final String KEY_P2P_STRATEGY = "p2p_strategy";
    public static final String KEY_ENDPOINT_ID = "endpoint_id";

    public static Strategy getStrategy(Context context) {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(context);
        String type = prefs.getString(KEY_P2P_STRATEGY, "on");
        Strategy s = null;
        if ("on".equals(type)) {
            s = new StategyAlwaysOn();
        } else if ("tentwenty".equals(type)) {
            s = new TwentyOnTenOfStrategy();
        } else if ("grid".equals(type)) {
            s = new GridStrategy();
        } else if ("prob".equals(type)) {
            s = new ProbalisticStrategy();
        }
        return s;
    }

    public static long getNextScheduledCheck(Context context) {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(context);
        return prefs.getLong(KEY_NEXT_SCHEDULED_CHECK, -1L);
    }

    public static void setNextScheduledCheck(Context context, long value) {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(context);
        Editor edit = prefs.edit();
        edit.putLong(KEY_NEXT_SCHEDULED_CHECK, value);
        edit.apply();
    }

    public static String getSchedulerInfo(Context context) {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(context);
        return prefs.getString(KEY_SCHEDULER_INFO, "");
    }

    public static void setSchedulerInfo(Context context, String value) {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(context);
        Editor edit = prefs.edit();
        edit.putString(KEY_SCHEDULER_INFO, value);
        edit.apply();
    }

    public static boolean isSchedulerStopped(Context context) {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(context);
        return prefs.getBoolean(KEY_SCHEDULER_STOPPED, false);
    }

    public static void setSchedulerStopped(Context context, boolean value) {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(context);
        Editor edit = prefs.edit();
        edit.putBoolean(KEY_SCHEDULER_STOPPED, value);
        edit.apply();
    }

    public static void setDefaultValues(Context context) {
        PreferenceManager.setDefaultValues(context, R.xml.prefs_p2p, false);
    }

    public static boolean isPeerDiscoveryEnabled(Context context) {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(context);
        return prefs.getBoolean(KEY_P2P_ENABLED, false);
    }

    public static String getEid(Context context) {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(context);
        return prefs.getString(KEY_ENDPOINT_ID, "dtn:none");
    }

}
