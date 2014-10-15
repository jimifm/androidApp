package com.FileSystem.fsoperator;


import java.io.UnsupportedEncodingException;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class FsActivity extends Activity {
	@SuppressLint("SdCardPath")
	String filename = "/sdcard/test.txt";
	//String filename = "/dev/input/event0";
	EditText writestrET;
	Button writeBT;
	Button readBT;
	Button seekBT;
	TextView readTV;
	String writeStr;
	byte[] buf_write;
	byte[] buf_read;
	int fd;
	
	int O_ACCMODE  =    0003;
	int O_RDONLY   =      00;
	int O_WRONLY   =      01;
	int O_RDWR     =      02;
	int O_CREAT    =    0100; /* not fcntl */
	int O_EXCL     =    0200; /* not fcntl */
	int O_NOCTTY   =   0400; /* not fcntl */
	int O_TRUNC    =   01000; /* not fcntl */
	int O_APPEND   =   02000;
	int O_NONBLOCK =   04000;
	int O_NDELAY   = O_NONBLOCK;
	int O_SYNC     =  010000;
	int O_FSYNC    =  O_SYNC;
	int O_ASYNC    =  020000;
	
	int SEEK_SET   =		0;//将读写位置指向文件头后再增加offset个位移量。
	int SEEK_CUR   =		1;//以目前的读写位置往后增加offset个位移量。
	int EEK_END    =		2;//将读写位置指向文件尾后再增加offset个位移量。 
	
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_fs);
		writestrET = (EditText) findViewById(R.id.writeET);
		writeBT = (Button) findViewById(R.id.writeBT);
		readBT = (Button) findViewById(R.id.readBT);
		seekBT = (Button) findViewById(R.id.seekBT);
		readTV = (TextView) findViewById(R.id.readTV);
		writeBT.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				fd = NativeFileOpen(filename, O_CREAT  | O_RDWR);
				System.out.println("fd_write---->" + fd);
				writeStr = writestrET.getText().toString();
				buf_write = writeStr.getBytes();
				int ret_write = NativeFileWrite(fd, buf_write, buf_write.length);
				System.out.println("写入返回结果" + ret_write);
				NativeFileClose(fd);

			}
		});
		readBT.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				fd = NativeFileOpen(filename, O_CREAT  | O_RDWR);
				System.out.println("fd_read---->" + fd);
				buf_read = new byte[buf_write.length];
				int ret_read = NativeFileRead(fd, buf_read, buf_write.length);
				
				System.out.println("读出返回结果" + ret_read);
				try {
					readTV.setText( new String(buf_read, "GB2312") + "");
				} catch (UnsupportedEncodingException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				NativeFileClose(fd);
			}
		});
		seekBT.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				fd = NativeFileOpen(filename, O_CREAT  | O_RDWR);
				long Offset=20;
				long ret_seek =NativeFileSeek(fd, Offset, SEEK_CUR);
			
				System.out.println("seek返回结果" + ret_seek);
				
				NativeFileClose(fd);
				/*　　  1) 欲将读写位置移到文件开头时:
　						　lseek（int fildes,0,SEEK_SET）；
　					　2) 欲将读写位置移到文件尾时:
　						　lseek（int fildes，0,SEEK_END）；
　					　3) 想要取得目前文件位置时:
　						　lseek（int fildes，0,SEEK_CUR）；
				返回值：当调用成功时则返回目前的读写位置，也就是距离文件开头多少个字节。若有错误则返回-1，errno 会存放错误代码。
				 * */
			}
		});

	}

	public native int NativeFileOpen(String filename, int flags);

	public native int NativeFileRead(int fd, byte[] buf, int sizes);

	public native int NativeFileWrite(int fd, byte[] buf, int sizes);

	public native long NativeFileSeek(int fd, long Offset, int whence);
	//Offset：偏移量，每一读写操作所需要移动的距离，单位是字节的数量，可正可负（向前移，向后移）。

	public native int NativeFileClose(int fd);

	static {
		System.loadLibrary("fs");
	}
}