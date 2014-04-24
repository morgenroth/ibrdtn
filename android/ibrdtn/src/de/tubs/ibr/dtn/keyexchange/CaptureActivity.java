package de.tubs.ibr.dtn.keyexchange;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Parcelable;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.google.zxing.Result;
import com.jwetherell.quick_response_code.DecoderActivity;
import com.jwetherell.quick_response_code.result.ResultHandler;
import com.jwetherell.quick_response_code.result.ResultHandlerFactory;

import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.service.DaemonService;

public class CaptureActivity extends DecoderActivity {

    private TextView statusView = null;
    private View resultView = null;
    private boolean inScanMode = false;
    private CharSequence displayContents = "";
    private SingletonEndpoint mEndpoint = null;
    
    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.capture_activity);

        resultView = findViewById(R.id.result_view);
        statusView = (TextView) findViewById(R.id.status_view);

        inScanMode = false;
        
        mEndpoint = getIntent().getParcelableExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (inScanMode) {
            	Toast.makeText(this, getString(R.string.qr_scan_aborted), Toast.LENGTH_SHORT).show();
                finish();
            }
            else {
				Intent startIntent = new Intent(this, DaemonService.class);
				startIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_GIVE_QR_RESPONSE);
				startIntent.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)mEndpoint);
				startIntent.putExtra("data", displayContents.toString());
				
				startService(startIntent);
				
                finish();
            }
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public void handleDecode(Result rawResult, Bitmap barcode) {
        drawResultPoints(barcode, rawResult);

        ResultHandler resultHandler = ResultHandlerFactory.makeResultHandler(this, rawResult);
        handleDecodeInternally(rawResult, resultHandler, barcode);
    }

    protected void showScanner() {
        inScanMode = true;
        resultView.setVisibility(View.GONE);
        statusView.setText(getString(R.string.qr_hint));
        statusView.setVisibility(View.VISIBLE);
        viewfinderView.setVisibility(View.VISIBLE);
    }

    protected void showResults() {
        inScanMode = false;
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
    }
}
