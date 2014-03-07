package de.tubs.ibr.dtn.chat;

import de.tubs.ibr.dtn.chat.service.ChatService;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.Spinner;
import android.widget.TextView;

public class MeDialog extends DialogFragment {
	
//	private String TAG = "MeDialog";
	private String presence = null;
	private String status = null;
	
	public interface OnChangeListener {
		void onStateChanged(String presence, String message);
	}
	
	public OnChangeListener change_lister = null;
	
	public void setOnChangeListener(OnChangeListener val) {
		this.change_lister = val;
	}
	
    public MeDialog() {
        // Empty constructor required for DialogFragment
    }
	
    @Override
	public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
    	this.presence = getArguments().getString(ChatService.EXTRA_PRESENCE);
    	this.status = getArguments().getString(ChatService.EXTRA_STATUS);
	}

	static MeDialog newInstance(String presence, String status) {
    	MeDialog m = new MeDialog();
        Bundle args = new Bundle();
        args.putString(ChatService.EXTRA_PRESENCE, presence);
        args.putString(ChatService.EXTRA_STATUS, status);
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
        final String[] spin_names = getResources().getStringArray(R.array.presence_tag_names);
        final Integer[] images = { R.drawable.online, R.drawable.online, R.drawable.online, R.drawable.away, R.drawable.xa, R.drawable.busy, R.drawable.offline };
        spin.setAdapter(new PresenceSpinnerAdapter(getActivity(), R.layout.spinner_presence, spin_names, images));
        
        int i = 0; for (String val : spin_values) {
        	if (val.equals(this.presence)) {
        		spin.setSelection(i);
        		break;
        	}
        	i++;
        }

        textedit.setText(this.status);
    	
    	AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
    	builder.setTitle(R.string.dialog_me_title);
    	builder.setView(v);
    	builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
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
    
    public class PresenceSpinnerAdapter extends ArrayAdapter<String> {

    	private String[] objects = null;
    	private Integer[] images = null;
    	
        public PresenceSpinnerAdapter(Context context, int textViewResourceId, String[] objects, Integer[] images) {
            super(context, textViewResourceId, objects);
            this.objects = objects;
            this.images = images;
        }

        @Override
        public View getDropDownView(int position, View convertView, ViewGroup parent) {
            return getCustomView(position, convertView, parent);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            return getCustomView(position, convertView, parent);
        }

        public View getCustomView(int position, View convertView, ViewGroup parent) {
            LayoutInflater inflater = getActivity().getLayoutInflater();
            View row = inflater.inflate(R.layout.spinner_presence, parent, false);
            TextView label = (TextView)row.findViewById(R.id.presence_text);
            label.setText(objects[position]);
            
            ImageView icon = (ImageView)row.findViewById(R.id.presence_icon);
            icon.setImageResource(images[position]);
            
            return row;
        }    
    }
}
