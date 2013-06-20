package de.tubs.ibr.dtn.daemon;

import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentTabHost;
import de.tubs.ibr.dtn.R;

public class StatsActivity extends FragmentActivity {
    
    private FragmentTabHost mTabHost;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        setContentView(R.layout.fragment_tabs);
        
        mTabHost = (FragmentTabHost)findViewById(android.R.id.tabhost);
        mTabHost.setup(this, getSupportFragmentManager(), R.id.realtabcontent);

        mTabHost.addTab(mTabHost.newTabSpec("info").setIndicator(getString(R.string.stats_tab_info)),
                InfoChartFragment.class, null);
        
        mTabHost.addTab(mTabHost.newTabSpec("bundles").setIndicator(getString(R.string.stats_tab_bundles)),
                BundleChartFragment.class, null);
        
        mTabHost.addTab(mTabHost.newTabSpec("transfer").setIndicator(getString(R.string.stats_tab_transfer)),
                TransferChartFragment.class, null);
        
        mTabHost.addTab(mTabHost.newTabSpec("clock").setIndicator(getString(R.string.stats_tab_clock)),
                ClockChartFragment.class, null);
    }
}
