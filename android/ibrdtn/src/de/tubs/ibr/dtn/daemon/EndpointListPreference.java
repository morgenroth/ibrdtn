package de.tubs.ibr.dtn.daemon;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.DialogInterface;
import android.preference.ListPreference;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.AttributeSet;
import android.widget.Button;
import android.widget.EditText;
import de.tubs.ibr.dtn.R;

public class EndpointListPreference extends ListPreference {
	
	private String mEndpointValue = null;

	public EndpointListPreference(Context context) {
		super(context);
	}
	
	public EndpointListPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	@Override
	protected void onPrepareDialogBuilder(Builder builder) {
		if (getEntries() == null || getEntries() == null) {
			throw new IllegalStateException(
					"ListPreference requires an entries array and an entryValues array.");
		}
		
		int selectedItem = findIndexOfValue(getValue());
		if (selectedItem == -1) selectedItem = findIndexOfValue("custom");

		builder.setSingleChoiceItems(getEntries(), selectedItem,
			new DialogInterface.OnClickListener() {
				public void onClick(final DialogInterface dialog, final int which) {
					CharSequence[] entries = getEntryValues();
					
					if ("custom".equals( entries[which] )) {
						// show input dialog for custom value
						showCustomInputDialog(dialog);
					}
					else {
						// Save preference and close dialog
						mEndpointValue = entries[which].toString();

						EndpointListPreference.this.onClick(dialog, DialogInterface.BUTTON_POSITIVE);
						dialog.dismiss();
					}
				}
			}
		);

		builder.setPositiveButton(null, null);
	}
	
	private void showCustomInputDialog(final DialogInterface parentDialog) {
		AlertDialog.Builder alert = new AlertDialog.Builder(getContext());
		alert.setTitle(R.string.endpoint_id);
		
		final EditText input = new EditText(getContext());
		alert.setView(input);
		
		// assign current endpoint ID
		input.setText(Preferences.getEndpoint(getContext()));
		
		alert.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				// store selected value
				mEndpointValue = input.getText().toString();
				
				// fake click on "Ok"
				EndpointListPreference.this.onClick(parentDialog, DialogInterface.BUTTON_POSITIVE);
				parentDialog.dismiss();
			}
		});
		
		alert.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				// fake click on "Cancel"
				EndpointListPreference.this.onClick(parentDialog, DialogInterface.BUTTON_NEGATIVE);
				parentDialog.dismiss();
			}
		});
		
		final AlertDialog alertDialog = alert.show();
		
		// get ok button
		final Button okButton = alertDialog.getButton(AlertDialog.BUTTON_POSITIVE);
		
		// add validator for text
		input.addTextChangedListener(new TextWatcher() {

			@Override
			public void afterTextChanged(Editable s) {
			}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {
			}

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				String customEndpoint = s.toString();
				boolean valid = true;
				
				// check if EID is valid
				if (!customEndpoint.contains(":")) valid = false;
				if (customEndpoint.indexOf(":") == 0) valid = false;
				if (customEndpoint.lastIndexOf(":") == (customEndpoint.length() - 1)) valid = false;
				if (customEndpoint.startsWith("dtn:") && !customEndpoint.startsWith("dtn://")) valid = false;
				
				// enable / disable ok button
				okButton.setEnabled(valid);
			}
			
		});
	}

	@Override
	protected void onDialogClosed(boolean positiveResult) {
		super.onDialogClosed(positiveResult);
		
		if (positiveResult && mEndpointValue != null) {
			if (callChangeListener(mEndpointValue)) {
				setValue(mEndpointValue);
			}
		}
	}
}
