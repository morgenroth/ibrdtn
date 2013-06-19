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

        mTabHost.addTab(mTabHost.newTabSpec("info").setIndicator("Info"),
                InfoChartFragment.class, null);
        
        mTabHost.addTab(mTabHost.newTabSpec("bundles").setIndicator("Bundles"),
                BundleChartFragment.class, null);
        
        mTabHost.addTab(mTabHost.newTabSpec("transfer").setIndicator("Transfer"),
                TransferChartFragment.class, null);
        
        mTabHost.addTab(mTabHost.newTabSpec("clock").setIndicator("Clock"),
                ClockChartFragment.class, null);
    }
}
