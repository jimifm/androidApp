package com.comm.commi2c;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;

public class I2cActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_i2c);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.i2c, menu);
		return true;
	}

	public native int open(String nodeName);

	public native int read(int fileHander, int i2c_adr, byte buf[], int Length);


	public native int write(int fileHander, int i2c_adr, int sub_adr, int buf[], int Length);
	public native void close(int fileHander);
	static {
	      System.loadLibrary("comm_i2c");

	}
}
