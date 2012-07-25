package de.tubs.ibr.dtn.chat;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;
import android.widget.Spinner;

public class MeDialog extends DialogFragment {
	
	private String TAG = "MeDialog";
	private String presence = null;
	private String status = null;
	private Activity activity = null;
	
	public interface OnChangeListener {
		void onStateChanged(String presence, String message);
	}
	
	public OnChangeListener change_lister = null;
	
	public void setOnChangeListener(OnChangeListener val) {
		this.change_lister = val;
	}
	
    public MeDialog(Activity activity) {
        // Empty constructor required for DialogFragment
    	this.activity = activity;
    }
	
    @Override
	public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
    	this.presence = getArguments().getString("presence");
    	this.status = getArguments().getString("status");
	}

	static MeDialog newInstance(Activity activity, String presence, String status) {
    	MeDialog m = new MeDialog(activity);
        Bundle args = new Bundle();
        args.putString("presence", presence);
        args.putString("status", status);
        m.setArguments(args);
        return m;
    }

    @Override
	public Dialog onCreateDialog(Bundle savedInstanceState) {
    	LayoutInflater inflater = LayoutInflater.from(getActivity());
        final View v = inflater.inflate(R.layout.me_dialog, null);
        
        final Spinner spin = (Spinner)v.findViewById(R.id.spinnerPresence);
        final EditText textedit = (EditText)v.findViewById(R.id.editTextStatus);
        
        final String[] spin_values = getResources().getStringArray(R.array.presence_tag_values);
        int i = 0; for (String val : spin_values) {
        	if (val.equals(this.presence)) {
        		spin.setSelection(i);
        		break;
        	}
        	i++;
        }

        textedit.setText(this.status);
    	
    	AlertDialog.Builder builder = new AlertDialog.Builder(activity);
    	builder.setTitle(R.string.dialog_me_title);
    	builder.setView(v);
    	builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				if (change_lister != null) {
					String new_presence = spin_values[spin.getSelectedItemPosition()];
					String new_message = textedit.getText().toString();
					
					if (!new_message.equals(status) || !new_presence.equals(presence)) {
						change_lister.onStateChanged(new_presence, new_message);
					}
				}
			}
		});
    	builder.setNegativeButton(android.R.string.cancel, null);
    	
    	return builder.create();
	}    
}
