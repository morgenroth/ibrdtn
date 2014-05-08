package de.tubs.ibr.dtn.keyexchange;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.zxing.Result;
import com.jwetherell.quick_response_code.DecoderActivity;
import com.jwetherell.quick_response_code.result.ResultHandler;
import com.jwetherell.quick_response_code.result.ResultHandlerFactory;

import de.tubs.ibr.dtn.R;

public class CaptureActivity extends DecoderActivity {
	
	private TextView statusView = null;
	private View resultView = null;
	private CharSequence displayContents = "";
	
	@Override
	public void onCreate(Bundle icicle) {
		super.onCreate(icicle);
		setContentView(R.layout.capture_activity);

		resultView = findViewById(R.id.result_view);
		statusView = (TextView) findViewById(R.id.status_view);
	}

	@Override
	public void handleDecode(Result rawResult, Bitmap barcode) {
		drawResultPoints(barcode, rawResult);

		ResultHandler resultHandler = ResultHandlerFactory.makeResultHandler(this, rawResult);
		handleDecodeInternally(rawResult, resultHandler, barcode);
	}

	protected void showScanner() {
		resultView.setVisibility(View.GONE);
		statusView.setText(getString(R.string.qr_hint));
		statusView.setVisibility(View.VISIBLE);
		viewfinderView.setVisibility(View.VISIBLE);
	}

	protected void showResults() {
		statusView.setVisibility(View.GONE);
		viewfinderView.setVisibility(View.GONE);
		resultView.setVisibility(View.VISIBLE);
	}

	// Put up our own UI for how to handle the decodBarcodeFormated contents.
	private void handleDecodeInternally(Result rawResult, ResultHandler resultHandler, Bitmap barcode) {
		onPause();
		showResults();

		ImageView barcodeImageView = (ImageView) findViewById(R.id.barcode_image_view);
		if (barcode == null) {
			barcodeImageView.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.drawable.icon));
		} else {
			barcodeImageView.setImageBitmap(Bitmap.createScaledBitmap(barcode, 640, 360, false));
		}
		
		TextView contentsTextView = (TextView) findViewById(R.id.contents_text_view);
		displayContents = resultHandler.getDisplayContents();
		contentsTextView.setText(Utils.hashToReadableString(displayContents.toString()));
		
		Intent result = new Intent(de.tubs.ibr.dtn.service.DaemonService.ACTION_GIVE_QR_RESPONSE);
		result.putExtra(KeyExchangeService.EXTRA_DATA, displayContents.toString());
		setResult(RESULT_OK, result);
		finish();
	}
}
