package com.it6604.hdmirx;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;

public class HdmirxActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_hdmirx);
		hdmirx_native_grun();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.hdmirx, menu);
		return true;
	}
	public native static void hdmirx_native_init();
	public native static void hdmirx_native_grun();
	public native static void hdmirx_native_exit();
	static {
		System.loadLibrary("hdmirx");
		hdmirx_native_init();
	}
}
