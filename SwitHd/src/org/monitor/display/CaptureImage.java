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
	// 定义系统所用的照相机
	static Camera camera;
	 // 是否在预览中
	static boolean isPreview = false;

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		videoSize videosize = switvideo_native_getsize();//first get video size;
		screenWidth = videosize.videoWidth;
		screenHeight = videosize.videoHeight;
		
		switvideo_native_fieldset();// set field
		
		super.onCreate(savedInstanceState);
		//设置全屏
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
			WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(R.layout.main);
		//landscape
		// 获取窗口管理器
		//WindowManager wm = getWindowManager();
		//Display display = wm.getDefaultDisplay();
		//DisplayMetrics metrics = new DisplayMetrics();
		// 获取屏幕的宽和高
		//display.getMetrics(metrics);
		//screenWidth = metrics.widthPixels;
		//screenHeight = metrics.heightPixels;
	
		// 获取界面中SurfaceView组件
		sView = (SurfaceView) findViewById(R.id.sView);
		// 设置该Surface不需要自己维护缓冲区
		//sView.getHolder().setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);//已经弃用
		// 获得SurfaceView的SurfaceHolder
		surfaceHolder = sView.getHolder();
		//if jnicallback but getHolder didn't exec; screen will black
		switvideo_native_grun();//second switvideo starting
		// 为surfaceHolder添加一个回调监听器
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
				// 打开摄像头
				//surfaceHolder = holder;
				initCamera();
			}

			@Override
			public void surfaceDestroyed(SurfaceHolder holder)
			{
				// 如果camera不为null ,释放摄像头
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
			// 此处默认打开后置摄像头。
			// 通过传入参数可以打开前置摄像头
			camera = Camera.open(0);  //①
			//camera.setDisplayOrientation(90);
			camera.setDisplayOrientation(0);//设置为横屏
		}
		if (camera != null && !isPreview)
		{
			try
			{
				Camera.Parameters parameters = camera.getParameters();
				// 设置预览照片的大小
				parameters.setPreviewSize(screenWidth, screenHeight);
				//parameters.setPreviewSize(1280,720);
				//parameters.set("video-size", "" + "1280x720");
				// 设置预览照片时每秒显示多少帧的最小值和最大值
				parameters.setPreviewFpsRange(4, 10);
				// 设置图片格式
				parameters.setPictureFormat(ImageFormat.JPEG);
				// 设置JPG照片的质量
				parameters.set("jpeg-quality", 85);
				// 设置照片的大小
				parameters.setPictureSize(screenWidth, screenHeight);
				//parameters.setPictureSize(1280,720);
				//CameraScreenNail d;
				camera.setParameters(parameters);//add by zhanghong 2014.11.4
				// 通过SurfaceView显示取景画面
				camera.setPreviewDisplay(surfaceHolder);  //②
				// 开始预览
				camera.startPreview();  //③
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
			// 控制摄像头自动对焦后才拍照
			camera.autoFocus(autoFocusCallback);  //④
		}
	}

	AutoFocusCallback autoFocusCallback = new AutoFocusCallback()
	{
		// 当自动对焦时激发该方法
		@Override
		public void onAutoFocus(boolean success, Camera camera)
		{
			if (success)
			{
				// takePicture()方法需要传入3个监听器参数
				// 第1个监听器：当用户按下快门时激发该监听器
				// 第2个监听器：当相机获取原始照片时激发该监听器
				// 第3个监听器：当相机获取JPG照片时激发该监听器
				camera.takePicture(new ShutterCallback()
				{
					public void onShutter()
					{
						// 按下快门瞬间会执行此处代码
					}
				}, new PictureCallback()
				{
					public void onPictureTaken(byte[] data, Camera c)
					{
						// 此处代码可以决定是否需要保存原始照片信息
					}
				}, myJpegCallback);  //⑤
			}
		}
	};

	PictureCallback myJpegCallback = new PictureCallback()
	{
		@Override
		public void onPictureTaken(byte[] data, Camera camera)
		{
			// 根据拍照所得的数据创建位图
			final Bitmap bm = BitmapFactory.decodeByteArray(data, 0,
				data.length);
			// 加载/layout/save.xml文件对应的布局资源
			View saveDialog = getLayoutInflater().inflate(R.layout.save,
				null);
			final EditText photoName = (EditText) saveDialog
				.findViewById(R.id.phone_name);
			// 获取saveDialog对话框上的ImageView组件
			ImageView show = (ImageView) saveDialog
				.findViewById(R.id.show);
			// 显示刚刚拍得的照片
			show.setImageBitmap(bm);
			// 使用对话框显示saveDialog组件
			new AlertDialog.Builder(CaptureImage.this).setView(saveDialog)
				.setPositiveButton("保存", new OnClickListener()
				{
					@Override
					public void onClick(DialogInterface dialog, int which)
					{
						// 创建一个位于SD卡上的文件
						File file = new File(Environment
							.getExternalStorageDirectory(), photoName
							.getText().toString() + ".jpg");
						FileOutputStream outStream = null;
						Toast toast=Toast.makeText(CaptureImage.this,photoName
								.getText().toString(),Toast.LENGTH_SHORT);
						toast.show();
						try
						{
							// 打开指定文件对应的输出流
							outStream = new FileOutputStream(file);
							// 把位图输出到指定文件中
							bm.compress(CompressFormat.JPEG, 100,
								outStream);
							outStream.close();
						}
						catch (IOException e)
						{
							e.printStackTrace();
						}
					}
				}).setNegativeButton("取消", null).show();
			// 重新浏览
			camera.stopPreview();
			camera.startPreview();
			isPreview = true;
		}
	};
//	@Override
	//protected void onResume() {
	 /**
	  * 设置为横屏
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

