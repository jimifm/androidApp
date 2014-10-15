package com.imput.i2cinputsystem;



import android.view.Menu;

//import java.io.UnsupportedEncodingException;

//import android.annotation.SuppressLint;
import android.app.Activity;
import android.os.Bundle;
//import android.view.View;
//import android.view.View.OnClickListener;
//import android.widget.Button;
//import android.widget.EditText;
import android.widget.TextView;

public class I2CInput extends Activity {
	static TextView readTV;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_i2c_input);
		readTV = (TextView) findViewById(R.id.readTV);
		start_i2c_prc();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.i2_cinput, menu);
		return true;
	}	

	//public native int NativeFileOpen(String filename, int flags);
	private static void JNICBProc(/*byte[]*/String buf, int sizes)
	//private static void JNICBProc()
	{
		System.out.println("poll"+ buf+sizes);
		//readTV.setText( new String(buf) + "");
		
		//if(sizes == 1)
		//	readTV.setText(buf);		
	}
	public native static void native_init();
	public native static void start_i2c_prc();
	static {
		System.loadLibrary("i2c_input");
		native_init();
	}
}
