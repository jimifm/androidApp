package org.monitor.display;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import org.monitor.display.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.ShutterCallback;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.Toast;

/**
 * Description:
 * <br/>site: <a href="http://www.crazyit.org">crazyit.org</a>
 * <br/>Copyright (C), 2001-2014, Yeeku.H.Lee
 * <br/>This program is protected by copyright laws.
 * <br/>Program Name:
 * <br/>Date:
 * @author  Yeeku.H.Lee kongyeeku@163.com
 * @version  1.0
 */
class videoSize
{
	public int videoWidth;
	public int videoHeight;
	videoSize(int width, int height)
	{
		videoWidth = width;
		videoHeight = height;
	}
	public void videSizeInfo()
	{
		System.out.println("videoWidth = "+ videoWidth + "videoHeight = " + videoHeight);
	}
}

public class CaptureImage extends Activity
{
	SurfaceView sView;
	static SurfaceHolder surfaceHolder;
	static int screenWidth;
	static int screenHeight;
	// ����ϵͳ���õ������
	static Camera camera;
	 // �Ƿ���Ԥ����
	static boolean isPreview = false;

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		videoSize videosize = switvideo_native_getsize();//first get video size;
		screenWidth = videosize.videoWidth;
		screenHeight = videosize.videoHeight;
		
		switvideo_native_fieldset();// set field
		
		super.onCreate(savedInstanceState);
		//����ȫ��
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
			WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(R.layout.main);
		//landscape
		// ��ȡ���ڹ�����
		//WindowManager wm = getWindowManager();
		//Display display = wm.getDefaultDisplay();
		//DisplayMetrics metrics = new DisplayMetrics();
		// ��ȡ��Ļ�Ŀ�͸�
		//display.getMetrics(metrics);
		//screenWidth = metrics.widthPixels;
		//screenHeight = metrics.heightPixels;
	
		// ��ȡ������SurfaceView���
		sView = (SurfaceView) findViewById(R.id.sView);
		// ���ø�Surface����Ҫ�Լ�ά��������
		//sView.getHolder().setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);//�Ѿ�����
		// ���SurfaceView��SurfaceHolder
		surfaceHolder = sView.getHolder();
		//if jnicallback but getHolder didn't exec; screen will black
		switvideo_native_grun();//second switvideo starting
		// ΪsurfaceHolder���һ���ص�������
		surfaceHolder.addCallback(new Callback()
		{
			@Override
			public void surfaceChanged(SurfaceHolder holder, int format,
				int width, int height)
			{
				System.out.println("surfaceChanged: " + width +" x " + height);
			}

			@Override
			public void surfaceCreated(SurfaceHolder holder)
			{
				// ������ͷ
				//surfaceHolder = holder;
				initCamera();
			}

			@Override
			public void surfaceDestroyed(SurfaceHolder holder)
			{
				// ���camera��Ϊnull ,�ͷ�����ͷ
				if (camera != null)
				{
					if (isPreview) camera.stopPreview();
					camera.release();
					camera = null;
					switvideo_native_exit();//need test!?
				}
			}
		});
	}

	private static void initCamera()
	{

		if (!isPreview)
		{
			// �˴�Ĭ�ϴ򿪺�������ͷ��
			// ͨ������������Դ�ǰ������ͷ
			camera = Camera.open(0);  //��
			//camera.setDisplayOrientation(90);
			camera.setDisplayOrientation(0);//����Ϊ����
		}
		if (camera != null && !isPreview)
		{
			try
			{
				Camera.Parameters parameters = camera.getParameters();
				// ����Ԥ����Ƭ�Ĵ�С
				parameters.setPreviewSize(screenWidth, screenHeight);
				//parameters.setPreviewSize(1280,720);
				//parameters.set("video-size", "" + "1280x720");
				// ����Ԥ����Ƭʱÿ����ʾ����֡����Сֵ�����ֵ
				parameters.setPreviewFpsRange(4, 10);
				// ����ͼƬ��ʽ
				parameters.setPictureFormat(ImageFormat.JPEG);
				// ����JPG��Ƭ������
				parameters.set("jpeg-quality", 85);
				// ������Ƭ�Ĵ�С
				parameters.setPictureSize(screenWidth, screenHeight);
				//parameters.setPictureSize(1280,720);
				//CameraScreenNail d;
				camera.setParameters(parameters);//add by zhanghong 2014.11.4
				// ͨ��SurfaceView��ʾȡ������
				camera.setPreviewDisplay(surfaceHolder);  //��
				// ��ʼԤ��
				camera.startPreview();  //��
			}
			catch (Exception e)
			{
				e.printStackTrace();
			}
			isPreview = true;
		}
	}

	public void capture(View source)
	{
		if (camera != null)
		{
			// ��������ͷ�Զ��Խ��������
			camera.autoFocus(autoFocusCallback);  //��
		}
	}

	AutoFocusCallback autoFocusCallback = new AutoFocusCallback()
	{
		// ���Զ��Խ�ʱ�����÷���
		@Override
		public void onAutoFocus(boolean success, Camera camera)
		{
			if (success)
			{
				// takePicture()������Ҫ����3������������
				// ��1�������������û����¿���ʱ�����ü�����
				// ��2�����������������ȡԭʼ��Ƭʱ�����ü�����
				// ��3�����������������ȡJPG��Ƭʱ�����ü�����
				camera.takePicture(new ShutterCallback()
				{
					public void onShutter()
					{
						// ���¿���˲���ִ�д˴�����
					}
				}, new PictureCallback()
				{
					public void onPictureTaken(byte[] data, Camera c)
					{
						// �˴�������Ծ����Ƿ���Ҫ����ԭʼ��Ƭ��Ϣ
					}
				}, myJpegCallback);  //��
			}
		}
	};

	PictureCallback myJpegCallback = new PictureCallback()
	{
		@Override
		public void onPictureTaken(byte[] data, Camera camera)
		{
			// �����������õ����ݴ���λͼ
			final Bitmap bm = BitmapFactory.decodeByteArray(data, 0,
				data.length);
			// ����/layout/save.xml�ļ���Ӧ�Ĳ�����Դ
			View saveDialog = getLayoutInflater().inflate(R.layout.save,
				null);
			final EditText photoName = (EditText) saveDialog
				.findViewById(R.id.phone_name);
			// ��ȡsaveDialog�Ի����ϵ�ImageView���
			ImageView show = (ImageView) saveDialog
				.findViewById(R.id.show);
			// ��ʾ�ո��ĵõ���Ƭ
			show.setImageBitmap(bm);
			// ʹ�öԻ�����ʾsaveDialog���
			new AlertDialog.Builder(CaptureImage.this).setView(saveDialog)
				.setPositiveButton("����", new OnClickListener()
				{
					@Override
					public void onClick(DialogInterface dialog, int which)
					{
						// ����һ��λ��SD���ϵ��ļ�
						File file = new File(Environment
							.getExternalStorageDirectory(), photoName
							.getText().toString() + ".jpg");
						FileOutputStream outStream = null;
						Toast toast=Toast.makeText(CaptureImage.this,photoName
								.getText().toString(),Toast.LENGTH_SHORT);
						toast.show();
						try
						{
							// ��ָ���ļ���Ӧ�������
							outStream = new FileOutputStream(file);
							// ��λͼ�����ָ���ļ���
							bm.compress(CompressFormat.JPEG, 100,
								outStream);
							outStream.close();
						}
						catch (IOException e)
						{
							e.printStackTrace();
						}
					}
				}).setNegativeButton("ȡ��", null).show();
			// �������
			camera.stopPreview();
			camera.startPreview();
			isPreview = true;
		}
	};
//	@Override
	//protected void onResume() {
	 /**
	  * ����Ϊ����
	  */
//	 if(getRequestedOrientation()!=ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE){
	//  setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
	 //}
	 //super.onResume();
	//}

	private static void switVideoResetCb(int videoWidth, int videoHeight)
	//private static void JNICBProc()
	{
		System.out.println("exec switvideo callback running");
		if(camera != null)
		{
			//camera.setPreviewCallback(null);
			camera.stopPreview();
			
			camera.release();
			//Camera.Parameters parameters = camera.getParameters();
			//parameters.setPreviewSize(screenWidth, screenHeight);
			camera = null;			
		}
		
		isPreview = false;
		switvideo_native_fieldset();
		screenWidth = videoWidth;
		screenHeight = videoHeight;
		System.out.println("switvideo callback " + screenWidth +" x " + screenHeight);
		initCamera();
	} 
	public native static void switvideo_native_init();
	public native static void switvideo_native_grun();
	public native static void switvideo_native_exit();
	public native static void switvideo_native_fieldset();
	public native static videoSize switvideo_native_getsize();
	public native static int switvideo_native_setinterface(byte videoInterface);//0x01 SDI1;0x02 SDI2;0x03 HDMI;0x04 CVBS
	
	static {
		System.loadLibrary("switvideo");
		switvideo_native_init();
	}
}

