package com.jpf.myjni;

import android.os.Bundle;
import android.app.Activity;
//import android.view.Menu;
import android.widget.TextView;

public class MainActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		//setContentView(R.layout.activity_main);
        TextView  tv = new TextView(this);

        tv.setText( stringFromJNI() );

        System.out.println("Here we go ...");

        setContentView(tv);

        System.out.println("Done!");
	}

	public native String  stringFromJNI();

    static {

           System.loadLibrary("myjni");

    }

}
