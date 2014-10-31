package de.tubs.ibr.dtn.daemon;

import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.PagerTitleStrip;
import android.support.v4.view.ViewPager;
import de.tubs.ibr.dtn.R;

public class StatsActivity extends FragmentActivity {
    
    private Pages mAdapter = null;
    private ViewPager mViewPager = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        setContentView(R.layout.stats_activity);
        
        PagerTitleStrip pts = (PagerTitleStrip)findViewById(R.id.pager_title_strip);
        pts.setNonPrimaryAlpha(0.5f);

        Boolean show_clock = PreferenceManager.getDefaultSharedPreferences(this).getString(Preferences.KEY_TIMESYNC_MODE, "disabled").equals("slave");
        mAdapter = new Pages(getSupportFragmentManager(), show_clock);

        mViewPager = (ViewPager)findViewById(R.id.pager);
        mViewPager.setAdapter(mAdapter);
    }
    
    private class Pages extends FragmentPagerAdapter {
        
        private Boolean mShowClock = false;

        public Pages(FragmentManager fm, Boolean show_clock) {
            super(fm);
            mShowClock = show_clock;
        }
        
        @Override
        public CharSequence getPageTitle(int position) {
            switch (position) {
                case 0:
                    return StatsActivity.this.getString(R.string.stats_tab_storage);

                case 1:
                    return StatsActivity.this.getString(R.string.stats_tab_bundles);

                case 2:
                    return StatsActivity.this.getString(R.string.stats_tab_transfer);

                case 3:
                    return StatsActivity.this.getString(R.string.stats_tab_cl);

                case 4:
                    return StatsActivity.this.getString(R.string.stats_tab_clock);
            }
            
            return super.getPageTitle(position);
        }



        @Override
        public Fragment getItem(int index) {
            switch (index) {
                case 0:
                    return new StorageChartFragment();

                case 1:
                    return new BundleChartFragment();

                case 2:
                    return new TransferChartFragment();

                case 3:
                    return new ConvergenceLayerStatsChartFragment();

                case 4:
                    return new ClockChartFragment();
            }
            
            return null;
        }

        @Override
        public int getCount() {
            return mShowClock ? 5 : 4;
        }
    }
}
