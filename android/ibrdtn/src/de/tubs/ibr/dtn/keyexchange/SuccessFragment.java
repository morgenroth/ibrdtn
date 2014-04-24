package de.tubs.ibr.dtn.keyexchange;

import android.graphics.PorterDuff.Mode;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import de.tubs.ibr.dtn.R;

public class SuccessFragment extends Fragment {

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.success_fragment, container, false);
		
		ImageView trustLevelImage1 = (ImageView) v.findViewById(R.id.imageViewTrustLevel1);
		ImageView trustLevelImage2 = (ImageView) v.findViewById(R.id.imageViewTrustLevel2);
		ImageView trustLevelImage3 = (ImageView) v.findViewById(R.id.imageViewTrustLevel3);
		
		int protocol = getActivity().getIntent().getIntExtra("protocol", -1);

		if (protocol == 5 || protocol == 4) {
    		Drawable d = getResources().getDrawable(R.drawable.ic_security_closed);
    		d.setColorFilter(getResources().getColor(R.color.light_green), Mode.MULTIPLY);
    		trustLevelImage1.setImageDrawable(d);
    		trustLevelImage2.setImageDrawable(d);
    		trustLevelImage3.setImageDrawable(d);
        }
        else if (protocol == 3 || protocol == 2) {
    		Drawable d = getResources().getDrawable(R.drawable.ic_security_closed);
    		d.setColorFilter(getResources().getColor(R.color.light_yellow), Mode.MULTIPLY);
    		trustLevelImage1.setImageDrawable(d);
    		trustLevelImage2.setImageDrawable(d);
    		
    		d = getResources().getDrawable(R.drawable.ic_action_security_open);
    		trustLevelImage3.setImageDrawable(d);
        }
        else if (protocol == 1 || protocol == 0) {
    		Drawable d = getResources().getDrawable(R.drawable.ic_security_closed);
    		d.setColorFilter(getResources().getColor(R.color.light_red), Mode.MULTIPLY);
    		trustLevelImage1.setImageDrawable(d);
    		
    		d = getResources().getDrawable(R.drawable.ic_action_security_open);
    		trustLevelImage2.setImageDrawable(d);
    		trustLevelImage3.setImageDrawable(d);
        }
        else {
    		Drawable d = getResources().getDrawable(R.drawable.ic_action_security_open);
    		trustLevelImage1.setImageDrawable(d);
    		trustLevelImage2.setImageDrawable(d);
    		trustLevelImage3.setImageDrawable(d);
        }

		Button complete = (Button) v.findViewById(R.id.buttonReady);
		complete.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				getActivity().finish();
			}
		});
		
		return v;
	}

}
