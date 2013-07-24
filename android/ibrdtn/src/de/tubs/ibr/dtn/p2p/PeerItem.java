package de.tubs.ibr.dtn.p2p;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.RelativeLayout;
import android.widget.TextView;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.p2p.db.Peer;
import de.tubs.ibr.dtn.stats.StatsUtils;

public class PeerItem extends RelativeLayout {

    @SuppressWarnings("unused")
    private static final String TAG = "PeerItem";
    
    private Peer mPeer = null;
    
    TextView mMacAddress = null;
    TextView mEid = null;
    TextView mLastContact = null;
    TextView mState = null;

    public PeerItem(Context context) {
        super(context);
    }

    public PeerItem(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
    
    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        mMacAddress = (TextView) findViewById(R.id.mac);
        mEid = (TextView) findViewById(R.id.eid);
        mLastContact = (TextView) findViewById(R.id.contact);
        mState = (TextView) findViewById(R.id.state);
    }
    
    public void bind(Peer p, int position) {
        mPeer = p;
        onDataChanged();
    }
    
    public void unbind() {
        // Clear all references to the message item, which can contain attachments and other
        // memory-intensive objects
    }
    
    private void onDataChanged() {
        String date = StatsUtils.formatTimeStampString(getContext(), mPeer.getLastSeen().getTime());

        mMacAddress.setText(mPeer.getP2pAddress());
        mEid.setText(mPeer.getEndpoint());
        mLastContact.setText(date);
        mState.setText(String.valueOf(mPeer.getConnectState()));
    }
    
}
