package com.yichou.test.jni;

import android.os.Bundle;
import android.os.Handler;
import android.app.Activity;
import android.app.ListActivity;
import android.view.Menu;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;

public class MainActivity extends ListActivity {
	static final String[] TITLES = {
		"消息1",
		"消息2",
		"消息3",
		"消息4",
		"测试零时 handler",
		
		"退出"
	};
	
	static {
		System.loadLibrary("TestJNI");
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		init();
		
		Handler mHandler;
		
//		setContentView(R.layout.activity_main);
		setListAdapter(new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, TITLES));
	}
	
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		switch (position) {
		case 0:
			for (int i = 0; i < 10; i++) {
				sendMsg(1, 1023+i, 2934);
			}
			break;
			
		case 1:
			sendMsg(2, 1023, 2934);
			break;

		case 2:
			sendMsg(3, 1023, 2934);
			break;

		case 3:
			sendMsg(4, 1023, 2934);
			break;

		case 4:
			new Thread(new Runnable() {
				
				@Override
				public void run() {
					testHandler();
				}
			}).start();
			break;

		case 5:
			sendMsg(-100, 1023, 2934);
			exit();
			break;
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	private static native void init();
	private static native void exit();
	private static native void testHandler();
	private static native void sendMsg(int what, int arg0, int arg1);
}
