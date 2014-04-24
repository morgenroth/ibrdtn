package de.tubs.ibr.dtn.keyexchange;

import android.content.Context;
import de.tubs.ibr.dtn.R;

public enum EnumProtocol {
	NONE(R.string.protocol_none_title, 0),
	DH(R.string.protocol_dh_title, 1),
	JPAKE(R.string.protocol_jpake_title, 2),
	HASH(R.string.protocol_hash_title, 3),
	QR(R.string.protocol_qr_title, 4),
	NFC(R.string.protocol_nfc_title, 5),
	JPAKE_PROMPT(R.string.protocol_jpake_title, 102);
	
	private int mTitle;
	private int mValue;
	
	EnumProtocol(int title, int value) {
		this.mTitle = title;
		this.mValue = value;
	}
	
	public String getTitle(Context context) {
		return context.getString(mTitle);
	}
	
	public int getValue() {
		return mValue;
	}
	
	static public EnumProtocol valueOf(int i) {
		for (EnumProtocol p : EnumProtocol.values()) {
			if (p.getValue() == i) return p;
		}
		return EnumProtocol.NONE;
	}
}
