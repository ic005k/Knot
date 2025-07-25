package com.x;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlarmManager;
import android.app.Application;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.UriMatcher;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.ResolveInfo;
import android.content.pm.ShortcutInfo;
import android.content.pm.ShortcutManager;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.Rect;
import android.graphics.drawable.Icon;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaPlayer;
import android.media.MediaRecorder;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.FileObserver;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Parcelable;
import android.os.PersistableBundle;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.Process;
import android.os.StrictMode;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.view.ActionMode;
import android.view.ContextMenu;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodSession.EventCallback;
import android.webkit.JavascriptInterface;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import androidx.core.app.ActivityCompat;
import androidx.core.content.FileProvider;

import com.x.FilePicker;
import com.x.MyService;
import com.x.ShareReceiveActivity;
import com.x.TTSUtils;
import com.x.AlarmReceiver;

import com.xhh.pdfui.PDFActivity;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.lang.reflect.Field;
import java.net.URLDecoder;
import java.nio.ByteBuffer;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.text.DecimalFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Objects;
import java.util.Properties;
import java.util.Random;
import java.util.Stack;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.location.LocationListenerCompat;
import androidx.core.location.LocationManagerCompat;
import androidx.core.location.LocationRequestCompat;
import android.location.LocationRequest;
import android.location.LocationProvider;
import android.location.GpsSatellite;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.provider.Settings;
import androidx.appcompat.app.AlertDialog;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import android.os.SystemClock;
import android.os.Vibrator;
import android.content.SharedPreferences;
import android.Manifest;
import android.view.WindowInsetsController;
import android.view.inputmethod.InputMethodManager;

//Qt
import org.qtproject.qt.android.bindings.QtActivity;

public class MyActivity
    extends QtActivity
    implements Application.ActivityLifecycleCallbacks {

  public static final String ACTION_TODO_ALARM = "com.x.Knot.TODO_ALARM";

  // 安卓版本>=11时存储授权
  private static final String PREFS_NAME = "app_prefs";
  private static final String KEY_SHOULD_REQUEST = "should_request_storage";

  public static boolean isOpenSearchResult = false;
  public static String strSearchText = "";
  public static boolean isEdit = false;
  public static String strMDTitle = "MarkDown";
  public static String strMDFile = "";
  public static String strImageFile;
  public static int myFontSize;
  public static boolean isBackMainUI = false;

  public static boolean isDark = false;
  public static MyActivity m_instance = null;
  private static SensorManager mSensorManager;

  private static final int DELAY = SensorManager.SENSOR_DELAY_NORMAL;

  public static boolean isScreenOff = false;
  public static int keyBoardHeight;

  private static final String TAG = "QtKnot";

  public static Context getMyAppContext() {
    return m_instance != null ? m_instance.getApplicationContext() : null;
  }

  private ShortcutManager shortcutManager;
  public static TTSUtils mytts;

  private AudioRecord audioRecord = null;
  private int recordBufsize = 0;
  private boolean isRecording = false;
  private Thread recordingThread;

  private MediaRecorder recorder;
  private MediaPlayer player;
  public static ArrayList<Activity> alarmWindows = new ArrayList<Activity>();

  private AlarmReceiver mAlarmReceiver;

  public static native void CallJavaNotify_0();

  public static native void CallJavaNotify_1();

  public static native void CallJavaNotify_2();

  public static native void CallJavaNotify_3();

  public static native void CallJavaNotify_4();

  public static native void CallJavaNotify_5();

  public static native void CallJavaNotify_6();

  public static native void CallJavaNotify_7();

  public static native void CallJavaNotify_8();

  public static native void CallJavaNotify_9();

  public static native void CallJavaNotify_10();

  public static native void CallJavaNotify_11();

  public static native void CallJavaNotify_12();

  public static native void CallJavaNotify_13();

  public native static void CallJavaNotify_14();

  public native static void CallJavaNotify_15();

  // 添加服务绑定状态标记
  private boolean mServiceBound = false;

  private InternalConfigure internalConfigure;
  public static boolean isReadShareData = false;
  public static boolean zh_cn;

  private LocationManager locationManager;
  private double latitude = 0;
  private double longitude = 0;
  private Executor executor;

  private boolean isTracking = false;
  private long startTime = 0L;
  private long movingTime;
  private float totalDistance = 0f;
  private float maxSpeed = 0f;
  private float mySpeed = 0f;
  private float totalClimb = 0f;
  private Location previousLocation;
  private double previousAltitude;

  private String strGpsStatus = "GPS Status";
  private String strRunTime = "00:00:00";
  private String strAltitude = "Altitude";
  private String strTotalDistance = "0 km";
  private String strMaxSpeed = "Max Speed";
  private String strTotalClimb = "Total Climb";
  private String strAverageSpeed = "0 km/h";

  public class VibrateUtils {
    // 产生震动
    public static void vibrate(Context context, long milliseconds) {
      // 获取Vibrator实例
      Vibrator vibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
      // 检查设备是否支持震动
      if (vibrator != null && vibrator.hasVibrator()) {
        // 震动
        vibrator.vibrate(milliseconds);
      }
    }
  }

  public void setVibrate() {

    VibrateUtils.vibrate(this, 50);
  }

  // ------------------------------------------------------------------------

  public void setDark(boolean dark) {
    isDark = dark;
    updateStatusBarColor();

  }

  // ------------------------------------------------------------------------
  public static void setMini() {
    m_instance.moveTaskToBack(true);
  }

  public static void bringToFront() {
    Intent intent = new Intent(getMyAppContext(), MyActivity.class);
    // 关键标志组合
    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
    getMyAppContext().startActivity(intent);
  }

  // ------------------------------------------------------------------------

  // 全透状态栏
  private void setStatusBarFullTransparent() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
      // 透明状态栏
      getWindow().addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
      // 状态栏字体设置为深色，SYSTEM_UI_FLAG_LIGHT_STATUS_BAR 为SDK23增加
      getWindow()
          .getDecorView()
          .setSystemUiVisibility(
              View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                  View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
      // 部分机型的statusbar会有半透明的黑色背景
      getWindow()
          .addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
      getWindow()
          .clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
      getWindow().setStatusBarColor(Color.TRANSPARENT); // SDK21
    }
  }

  // 非全透,带颜色的状态栏,需要指定颜色（目前采用）
  private void setStatusBarColor(String color) {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
      // 需要安卓版本大于5.0以上
      getWindow()
          .addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
      getWindow().setStatusBarColor(Color.parseColor(color));
    }
  }

  // ----------------------------------------------------------------------

  private void registAlarmReceiver() {
    // 动态注册 AlarmReceiver
    mAlarmReceiver = new AlarmReceiver();
    IntentFilter filter = new IntentFilter();
    filter.addAction(ACTION_TODO_ALARM); // 定义要监听的广播 Action
    registerReceiver(mAlarmReceiver, filter); // 注册接收器

  }

  ///////////////// 屏幕唤醒相关 ////////////////////////////
  private ScreenStatusReceiver mScreenStatusReceiver = null;
  private Handler mHandler = new Handler(Looper.getMainLooper());

  private void registSreenStatusReceiver() {
    mScreenStatusReceiver = new ScreenStatusReceiver();
    IntentFilter screenStatusIF = new IntentFilter();
    screenStatusIF.addAction(Intent.ACTION_SCREEN_ON);
    screenStatusIF.addAction(Intent.ACTION_SCREEN_OFF);
    registerReceiver(mScreenStatusReceiver, screenStatusIF);
  }

  class ScreenStatusReceiver extends BroadcastReceiver {
    String SCREEN_ON = "android.intent.action.SCREEN_ON";
    String SCREEN_OFF = "android.intent.action.SCREEN_OFF";

    @Override
    public void onReceive(Context context, Intent intent) {
      if (SCREEN_ON.equals(intent.getAction())) {
        isScreenOff = false;

        // 延迟调用确保服务已初始化
        mHandler.postDelayed(() -> {
          if (MyService.isReady) {
            CallJavaNotify_2();
            Log.w("Knot", "成功获取步数");
          } else {
            Log.e("Knot", "服务未准备好，无法获取步数");
          }
        }, 500); // 延迟 500 毫秒

        Log.w("Knot", "屏幕亮了");
      } else if (SCREEN_OFF.equals(intent.getAction())) {

        isScreenOff = true;

        Log.w("Knot", "屏幕熄了");
      }
    }
  }

  public static boolean getLockScreenStatus() {
    return isScreenOff;
  }

  // --------------------------------------------------------------------------------------------------
  // 目前正在使用，经过改良，目前能解压所有的epub文件
  public static void Unzip(String zipFile, String targetDir) {
    Log.i(TAG, zipFile);
    Log.i(TAG, targetDir);

    int BUFFER = 1024 * 1024; // 这里缓冲区我们使用4KB，
    String strEntry; // 保存每个zip的条目名称
    try {
      BufferedOutputStream dest = null; // 缓冲输出流
      InputStream fis = new FileInputStream(zipFile);
      ZipInputStream zis = new ZipInputStream(new BufferedInputStream(fis));
      ZipEntry entry; // 每个zip条目的实例
      while ((entry = zis.getNextEntry()) != null) {
        try {
          // 先创建目录，否则有些文件没法解压，比如根目录里面的文件
          if (entry.isDirectory()) {
            File fmd = new File(targetDir + entry.getName());
            fmd.mkdirs();
            continue;
          }
          int count;
          byte data[] = new byte[BUFFER];
          strEntry = entry.getName();
          File entryFile = new File(targetDir + strEntry);
          Log.i(TAG, targetDir + strEntry);
          File entryDir = new File(entryFile.getParent());
          Log.i(TAG, " entryDir: " + entryFile.getParent());
          if (!entryDir.exists()) {
            entryDir.mkdirs();
          }
          FileOutputStream fos = new FileOutputStream(entryFile);
          dest = new BufferedOutputStream(fos, BUFFER);
          while ((count = zis.read(data, 0, BUFFER)) != -1) {
            dest.write(data, 0, count);
          }
          dest.flush();
          dest.close();
        } catch (Exception ex) {
          ex.printStackTrace();
        }
      }
      zis.close();
      Log.i(TAG, "unzip end...");
    } catch (Exception cwj) {
      cwj.printStackTrace();
    }
  }

  /**
   * 读取文件内容并压缩，既支持文件也支持文件夹
   *
   * @param filePath 文件路径
   */
  private static void compressFileToZip(String filePath, String zipFilePath) {
    try (
        ZipOutputStream zos = new ZipOutputStream(
            new FileOutputStream(zipFilePath))) {
      // 递归的压缩文件夹和文件
      doCompress("", filePath, zos);
      // 必须
      zos.finish();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  private static void doCompress(
      String parentFilePath,
      String filePath,
      ZipOutputStream zos) {
    File sourceFile = new File(filePath);
    if (!sourceFile.exists()) {
      return;
    }
    String zipEntryName = parentFilePath + "/" + sourceFile.getName();
    if (parentFilePath.isEmpty()) {
      zipEntryName = sourceFile.getName();
    }
    if (sourceFile.isDirectory()) {
      File[] childFiles = sourceFile.listFiles();
      if (Objects.isNull(childFiles)) {
        return;
      }
      for (File childFile : childFiles) {
        doCompress(zipEntryName, childFile.getAbsolutePath(), zos);
      }
    } else {
      int len = -1;
      byte[] buf = new byte[1024];
      try (
          InputStream input = new BufferedInputStream(
              new FileInputStream(sourceFile))) {
        zos.putNextEntry(new ZipEntry(zipEntryName));
        while ((len = input.read(buf)) != -1) {
          zos.write(buf, 0, len);
        }
      } catch (Exception e) {
        e.printStackTrace();
      }
    }
  }

  private String processViewIntent(Intent intent) {
    return intent.getDataString();
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    requestPermission();
    requestSensorPermission();

    if (m_instance != null) {
      Log.d(TAG, "App is already running... this won't work");
      Intent intent = getIntent();
      if (intent != null) {
        Log.d(TAG, "There's an intent waiting...");
        String sharedData = processViewIntent(intent);
        if (sharedData != null) {
          Log.d(TAG, "It's a view intent");
          Intent viewIntent = new Intent(
              getApplicationContext(),
              MyActivity.class);
          viewIntent.setAction(Intent.ACTION_VIEW);
          viewIntent.putExtra("sharedData", sharedData);
          startActivity(viewIntent);
        }
      }
      finish();
      return;
    }
    m_instance = this;
    Log.d(TAG, "Android activity created");

    isZh(m_instance);

    registSreenStatusReceiver();
    // registAlarmReceiver();

    Application application = this.getApplication();
    application.registerActivityLifecycleCallbacks(this);

    // 服务
    Intent bindIntent = new Intent(MyActivity.this, MyService.class);
    if (Build.VERSION.SDK_INT >= 26) {
      startForegroundService(bindIntent);
      mServiceBound = false; // 注意：这是启动服务而非绑定
    } else {
      bindService(bindIntent, mCon, Context.BIND_AUTO_CREATE);
      startService(new Intent(bindIntent));
      mServiceBound = true;
    }

    addDeskShortcuts();

    mytts = TTSUtils.getInstance(this);
    // 初始化TTS
    mytts.initialize(new TTSUtils.InitCallback() {
      @Override
      public void onSuccess() {
        // 使用系统默认语言
        // mytts.speak("TTS initialized successfully");

        // 使用特定语言
        // mytts.speak("Bonjour", Locale.FRENCH);

        Log.w("TTS", "TTS initialized successfully");
      }

      @Override
      public void onError(String error) {
        Log.w("TTS", "TTS init failed: " + error);
      }
    });

  }

  // 使用LocationListenerCompat定义位置监听器
  private final LocationListenerCompat locationListener1 = new LocationListenerCompat() {
    @Override
    public void onLocationChanged(@NonNull Location location) {
      // 位置更新时触发
      latitude = location.getLatitude();
      longitude = location.getLongitude();
      updateTrackingData(location);

    }

    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {
      switch (status) {
        case LocationProvider.AVAILABLE:
          strGpsStatus = "GPS: Available";
          break;
        case LocationProvider.OUT_OF_SERVICE:
          strGpsStatus = "GPS: Out of Service";
          break;
        case LocationProvider.TEMPORARILY_UNAVAILABLE:
          strGpsStatus = "GPS: Temporarily Unavailable";
          break;
      }
      Log.i(TAG, "GPS Status: " + strGpsStatus);
    }

    @Override
    public void onProviderEnabled(@NonNull String provider) {
      Log.d(TAG, "Provider enabled: " + provider);
    }

    @Override
    public void onProviderDisabled(@NonNull String provider) {
      Log.d(TAG, "Provider disabled: " + provider);
    }
  };

  private final GpsStatus.Listener gpsStatusListener = new GpsStatus.Listener() {
    @Override
    public void onGpsStatusChanged(int event) {
      if (locationManager != null) {
        GpsStatus gpsStatus = locationManager.getGpsStatus(null);
        switch (event) {
          case GpsStatus.GPS_EVENT_SATELLITE_STATUS:
            Iterable<GpsSatellite> satellites = gpsStatus.getSatellites();
            Iterator<GpsSatellite> it = satellites.iterator();
            int satelliteCount = 0;
            StringBuilder statusText = new StringBuilder();
            while (it.hasNext()) {
              GpsSatellite satellite = it.next();
              satelliteCount++;
              statusText.append("卫星 ").append(satelliteCount).append(" 强度: ").append(satellite.getSnr()).append("\n");
            }
            statusText.insert(0, "可见卫星数量: ").append(satelliteCount).append("\n");
            strGpsStatus = statusText.toString();
            break;
          case GpsStatus.GPS_EVENT_FIRST_FIX:
            // 首次定位成功
            break;
          case GpsStatus.GPS_EVENT_STARTED:
            // GPS启动
            break;
          case GpsStatus.GPS_EVENT_STOPPED:
            // GPS停止
            break;
        }
      }
    }
  };

  public String getGpsStatus() {
    return strTotalDistance + "\n" + strRunTime + "\n" + strAverageSpeed + "\n" + strMaxSpeed + "\n" + strAltitude
        + "\n" + strTotalClimb + "\n" + strGpsStatus;
  }

  public double startGpsUpdates() {
    setVibrate();
    latitude = 0;
    longitude = 0;
    startTime = System.currentTimeMillis();
    totalDistance = 0f;
    maxSpeed = 0f;
    mySpeed = 0f;
    totalClimb = 0f;
    previousLocation = null;
    movingTime = 0;
    previousAltitude = 0f;

    // 初始化LocationManager
    locationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
    executor = Executors.newSingleThreadExecutor();

    // 检查位置服务是否开启
    boolean isGpsEnabled = locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
    boolean isNetworkEnabled = locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
    if (!isGpsEnabled && !isNetworkEnabled) {
      new AlertDialog.Builder(this)
          .setMessage("位置服务未开启，请开启位置服务以获取位置信息。")
          .setPositiveButton("去开启", (dialog, which) -> {
            Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
            startActivity(intent);
          })
          .setNegativeButton("取消", null)
          .show();
    }

    if (locationManager != null) {
      // 检测是否有定位权限
      int permission = ActivityCompat.checkSelfPermission(
          this,
          "android.permission.ACCESS_FINE_LOCATION");
      if (permission != PackageManager.PERMISSION_GRANTED) {
        // 没有定位权限，去申请定位权限，会弹出对话框
        ActivityCompat.requestPermissions(this,
            new String[] { "android.permission.ACCESS_FINE_LOCATION" },
            1);
      }

      if (ActivityCompat.checkSelfPermission(this,
          "android.permission.ACCESS_FINE_LOCATION") == PackageManager.PERMISSION_GRANTED) {

        // 创建 LocationRequestCompat 对象
        LocationRequestCompat locationRequest = new LocationRequestCompat.Builder(2000L) // 最小时间间隔
            .setMinUpdateDistanceMeters(1.0f) // 最小距离间隔
            .build();
        // 使用 LocationManagerCompat 请求位置更新（兼容 Android 6.0+）
        LocationManagerCompat.requestLocationUpdates(
            locationManager,
            LocationManager.GPS_PROVIDER, // 使用 GPS 提供者
            locationRequest,
            executor,
            locationListener1);

        // 添加GPS状态侦听
        if (locationManager != null) {
          // locationManager.addGpsStatusListener(gpsStatusListener);
        }

        return 1;
      }
    }
    return 0;
  }

  public double getTotalDistance() {

    return totalDistance;

  }

  public double getMySpeed() {

    return mySpeed;

  }

  public double getMaxSpeed() {

    return maxSpeed;

  }

  public double getLatitude() {

    return latitude;

  }

  public double getLongitude() {

    return longitude;

  }

  // 停止 GPS 更新
  public double stopGpsUpdates() {
    setVibrate();
    if (locationManager != null && locationListener1 != null) {
      try {
        LocationManagerCompat.removeUpdates(locationManager, locationListener1);
      } catch (SecurityException e) {
        e.printStackTrace();
      }
    }
    return totalDistance;
  }

  // 更新运动数据
  private void updateTrackingData(Location currentLocation) {
    if (previousLocation != null) {
      if (currentLocation.getSpeed() > 0) {
        // 运动状态
        long currentTime = System.currentTimeMillis();
        movingTime += currentTime - startTime;
        startTime = currentTime;

        // 计算距离
        totalDistance += previousLocation.distanceTo(currentLocation) / 1000; // 转换为公里

        // 计算最大速度
        mySpeed = currentLocation.getSpeed() * 3.6f;// 转换为 km/h
        if (mySpeed > maxSpeed) {
          maxSpeed = mySpeed;
        }

        // 计算爬升
        double currentAltitude = currentLocation.getAltitude();
        if (currentAltitude > previousAltitude) {
          totalClimb += currentAltitude - previousAltitude;
        }
        previousAltitude = currentAltitude;
      } else {
        // 静止状态，更新开始时间
        startTime = System.currentTimeMillis();
      }
    } else {
      previousAltitude = currentLocation.getAltitude();
    }

    // 更新 UI
    updateUI(currentLocation);

    previousLocation = currentLocation;
  }

  private void updateUI(Location location) {
    // 运动距离
    strTotalDistance = String.format("%.2f km", totalDistance);

    // 运动时间
    long seconds = movingTime / 1000;
    strRunTime = String.format("%02d:%02d:%02d", seconds / 3600, (seconds % 3600) / 60, seconds % 60);

    // 平均速度
    double avgSpeed = totalDistance / (movingTime / 3600000f);
    strAverageSpeed = String.format("%.2f km/h", avgSpeed);

    // 最大速度
    if (zh_cn)
      strMaxSpeed = String.format("最大速度: %.2f km/h", maxSpeed);
    else
      strMaxSpeed = String.format("Max Speed: %.2f km/h", maxSpeed);

    // 海拔
    if (zh_cn)
      strAltitude = String.format("海拔: %.2f 米", location.getAltitude());
    else
      strAltitude = String.format("Altitude: %.2f 米", location.getAltitude());

    // 爬升
    if (zh_cn)
      strTotalClimb = String.format("累计爬升: %.2f 米", totalClimb);
    else
      strTotalClimb = String.format("Total Climb: %.2f 米", totalClimb);
  }

  private static ServiceConnection mCon = new ServiceConnection() {

    @Override
    public void onServiceConnected(ComponentName arg0, IBinder arg1) {
      // Auto-generated method stub
    }

    @Override
    public void onServiceDisconnected(ComponentName arg0) {
      // Auto-generated method stub
    }
  };

  @Override
  public boolean onKeyDown(int keyCode, KeyEvent event) {
    if (keyCode == KeyEvent.KEYCODE_BACK) {
      // 直接处理返回键
      CallJavaNotify_15();
      return true; // 事件已处理
    }
    return super.onKeyDown(keyCode, event);
  }

  @Override
  public void onBackPressed() {

    super.onBackPressed();
  }

  @Override
  public void onPause() {
    System.out.println("onPause...");
    if (MyService.isReady)
      CallJavaNotify_1();
    super.onPause();

  }

  @Override
  protected void onResume() {
    System.out.println("onResume...");
    super.onResume();
    updateStatusBarColor();
    if (MyService.isReady)
      CallJavaNotify_0();
  }

  @Override
  public void onStop() {
    System.out.println("onStop...");
    super.onStop();
  }

  @Override
  protected void onDestroy() {
    Log.i(TAG, "Main onDestroy...");

    if (ReOpen) {
      openAppFromPackageName("com.x");
      Log.i(TAG, "reopen = done...");
    }

    getApplication().unregisterActivityLifecycleCallbacks(this); // 注销回调

    if (mScreenStatusReceiver != null) {
      unregisterReceiver(mScreenStatusReceiver);
      mScreenStatusReceiver = null;
    }

    if (mAlarmReceiver != null) {
      unregisterReceiver(mAlarmReceiver);
      mAlarmReceiver = null;
    }

    if (mServiceBound && mCon != null) {
      unbindService(mCon); // 解绑服务
      mServiceBound = false;
      mCon = null;
    }

    super.onDestroy();
    m_instance = null;
    mytts.shutdown();
    alarmWindows.remove(this); // 防止 Activity 泄漏
  }

  @Override
  public void onActivityCreated(Activity activity, Bundle savedInstanceState) {
  }

  @Override
  public void onActivityStarted(Activity activity) {
  }

  @Override
  public void onActivityResumed(Activity activity) {

  }

  @Override
  public void onActivityPaused(Activity activity) {

  }

  @Override
  public void onActivityStopped(Activity activity) {
    // 转至后台
    System.out.println("MyActivity onActivityStopped...");
  }

  @Override
  public void onActivitySaveInstanceState(Activity activity, Bundle outState) {
  }

  @Override
  public void onActivityDestroyed(Activity activity) {
  }

  // ----------------------------------------------------------------------------------------------
  /*
   * This method can parse out the real local file path from a file URI.
   */
  public String getUriPath(String uripath) {
    String URL = uripath;
    String str = "None";
    try {
      // if (Build.VERSION.SDK_INT >= 26) {
      str = URLDecoder.decode(URL, "UTF-8");
      // }
    } catch (Exception e) {
      System.err.println("Error : URLDecoder.decode");
      e.printStackTrace();
    }

    Log.i(TAG, "UriString  " + uripath);
    Log.i(TAG, "RealPath  " + str);

    return str;
  }

  // ---------------------------------------------------------------------------------------------
  public static int copyFile(String srcPath, String FileName) {
    Log.i(TAG, "src  " + srcPath);
    Log.i(TAG, "dest  " + FileName);

    int result = 0;
    if ((srcPath == null) || (FileName == null)) {
      return result;
    }
    File src = new File(srcPath);
    // File dest = new File("/storage/emulated/0/Download/ " + FileName);
    File dest = new File(FileName);
    if (dest != null && dest.exists()) {
      dest.delete(); // delete file
    }
    try {
      dest.createNewFile();
    } catch (IOException e) {
      e.printStackTrace();
    }

    FileChannel srcChannel = null;
    FileChannel dstChannel = null;

    try {
      srcChannel = new FileInputStream(src).getChannel();
      dstChannel = new FileOutputStream(dest).getChannel();
      srcChannel.transferTo(0, srcChannel.size(), dstChannel);
      result = 1;
    } catch (FileNotFoundException e) {
      e.printStackTrace();
      return result;
    } catch (IOException e) {
      e.printStackTrace();
      return result;
    }
    try {
      srcChannel.close();
      dstChannel.close();
    } catch (IOException e) {
      e.printStackTrace();
    }
    return result;
  }

  // ----------------------------------------------------------------------------------------------
  public void openKnotBakDir() {
    Uri dir = Uri.parse("/storage/emulated/0/KnotBak/");
    int PICKFILE_RESULT_CODE = 1;
    Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
    intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, dir);
    intent.setType("*/*");
    Intent i = Intent.createChooser(intent, "View Default File Manager");
    startActivityForResult(i, PICKFILE_RESULT_CODE);
  }

  // ----------------------------------------------------------------------------------------------
  public static int getAndroidVer() {
    int a = Build.VERSION.SDK_INT;

    Log.i(TAG, "os ver=" + a);
    return a;
  }

  private static String souAPK;

  public static void setAPKFile(String apkfile) {
    souAPK = apkfile;
  }

  public void installApk(String sou) {
    File newApkFile = new File(sou);
    Intent intent = new Intent(Intent.ACTION_VIEW);
    intent.addCategory(Intent.CATEGORY_DEFAULT);
    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    String type = "application/vnd.android.package-archive";
    Uri uri;
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
      uri = FileProvider.getUriForFile(
          getMyAppContext(),
          getMyAppContext().getPackageName(),
          newApkFile);
      intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
    } else {
      uri = Uri.fromFile(newApkFile);
    }
    intent.setDataAndType(uri, type);
    getMyAppContext().startActivity(intent);
  }

  private boolean ReOpen = false;

  public int setReOpen() {
    ReOpen = true;

    return 1;
  }

  public void openAppFromPackageName(String pname) {
    PackageManager packageManager = getPackageManager();
    Intent it = packageManager.getLaunchIntentForPackage(pname);
    startActivity(it);
  }

  private void doStartApplicationWithPackageName(String packagename) {
    Log.i(TAG, "自启动开始...");
    // 通过包名获取此APP详细信息，包括Activities、services、versioncode、name等等
    PackageInfo packageinfo = null;
    try {
      packageinfo = getPackageManager().getPackageInfo(packagename, 0);
    } catch (NameNotFoundException e) {
      e.printStackTrace();
    }
    if (packageinfo == null) {
      return;
    }

    // 创建一个类别为CATEGORY_LAUNCHER的该包名的Intent
    Intent resolveIntent = new Intent(Intent.ACTION_MAIN, null);
    resolveIntent.addCategory(Intent.CATEGORY_LAUNCHER);
    resolveIntent.setPackage(packageinfo.packageName);

    // 通过getPackageManager()的queryIntentActivities方法遍历
    List<ResolveInfo> resolveinfoList = getPackageManager()
        .queryIntentActivities(resolveIntent, 0);

    ResolveInfo resolveinfo = resolveinfoList.iterator().next();
    if (resolveinfo != null) {
      // packagename = 参数packname
      String packageName = resolveinfo.activityInfo.packageName;
      // 这个就是我们要找的该APP的LAUNCHER的Activity[组织形式：packagename.mainActivityname]
      String className = resolveinfo.activityInfo.name;
      // LAUNCHER Intent
      Intent intent = new Intent(Intent.ACTION_MAIN);
      intent.addCategory(Intent.CATEGORY_LAUNCHER);

      // 设置ComponentName参数1:packagename参数2:MainActivity路径
      ComponentName cn = new ComponentName(packageName, className);

      intent.setComponent(cn);
      startActivity(intent);

      Log.i(TAG, "启动自己已完成...");
    }

    Log.i(TAG, "过程完成...");
  }

  // ==============================================================================================
  private void checkStoragePermission() {
    SharedPreferences prefs = getSharedPreferences(PREFS_NAME, MODE_PRIVATE);
    boolean shouldRequest = prefs.getBoolean(KEY_SHOULD_REQUEST, true);

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
      if (!Environment.isExternalStorageManager() && shouldRequest) {
        // 更新标记，避免重复跳转
        prefs.edit().putBoolean(KEY_SHOULD_REQUEST, false).apply();

        Intent intent = new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
        startActivity(intent);
      }
    }
  }

  // 动态获取权限需要添加的常量
  private static final int REQUEST_EXTERNAL_STORAGE = 1;
  private static String[] PERMISSIONS_STORAGE = {
      "android.permission.READ_EXTERNAL_STORAGE",
      "android.permission.WRITE_EXTERNAL_STORAGE",
  };

  // 被调用的方法
  public static void verifyStoragePermissions(Activity activity) {
    try {
      // 检测是否有写的权限
      int permission = ActivityCompat.checkSelfPermission(
          activity,
          "android.permission.WRITE_EXTERNAL_STORAGE");
      if (permission != PackageManager.PERMISSION_GRANTED) {
        // 没有写的权限，去申请写的权限，会弹出对话框
        ActivityCompat.requestPermissions(
            activity,
            PERMISSIONS_STORAGE,
            REQUEST_EXTERNAL_STORAGE);
      }

      // 申请记录音频的权限，会弹出对话框
      int permissionRecordAudio = ActivityCompat.checkSelfPermission(
          activity,
          "android.permission.RECORD_AUDIO");
      if (permissionRecordAudio != PackageManager.PERMISSION_GRANTED) {
        ActivityCompat.requestPermissions(
            activity,
            new String[] { "android.permission.RECORD_AUDIO" },
            2000);
      }

      // 检查摄像头权限
      int permissionRecordCamera = ActivityCompat.checkSelfPermission(
          activity,
          "android.permission.CAMERA");
      if (permissionRecordCamera != PackageManager.PERMISSION_GRANTED) {
        ActivityCompat.requestPermissions(
            activity,
            new String[] { "android.permission.CAMERA" },
            2000);
      } else {

      }

    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  public static boolean checkCamera() {
    int permissionRecordCamera = ActivityCompat.checkSelfPermission(
        m_instance,
        "android.permission.CAMERA");
    if (permissionRecordCamera != PackageManager.PERMISSION_GRANTED) {
      return false;
    } else {
      return true;
    }
  }

  public static int checkRecordAudio() {
    int permissionRecordCamera = ActivityCompat.checkSelfPermission(
        m_instance,
        "android.permission.RECORD_AUDIO");
    if (permissionRecordCamera != PackageManager.PERMISSION_GRANTED) {
      return 0;
    } else {
      return 1;
    }
  }

  // ==============================================================================================
  public class InternalConfigure {
    private final Context context;
    private Properties properties;

    public InternalConfigure(Context context) {
      super();
      this.context = context;
    }

    /**
     * 保存文件filename为文件名，filecontent为存入的文件内容
     * 例:configureActivity.saveFiletoSD("text.ini","");
     */
    public void saveFile(String filename, Properties properties)
        throws Exception {
      // 设置Context.MODE_PRIVATE表示每次调用该方法会覆盖原来的文件数据
      FileOutputStream fileOutputStream; // = context.openFileOutput(filename, Context.MODE_PRIVATE);
      File file = new File(filename);
      fileOutputStream = new FileOutputStream(file);
      // 通过properties.stringPropertyNames()获得所有key的集合Set，里面是String对象
      for (String key : properties.stringPropertyNames()) {
        String s = key + " = " + properties.getProperty(key) + "\n";
        System.out.println(s);
        fileOutputStream.write(s.getBytes());
      }
      fileOutputStream.close();
    }

    /**
     * 读取文件
     */
    public void readFrom(String filename) throws Exception {
      properties = new Properties();

      FileInputStream fileInputStream; // = context.openFileInput(filename);

      File file = new File(filename);
      fileInputStream = new FileInputStream(file);

      InputStreamReader reader = new InputStreamReader(
          fileInputStream,
          "UTF-8");
      BufferedReader br = new BufferedReader(reader);

      properties.load(br);

      br.close();
      reader.close();
      fileInputStream.close();
    }

    /**
     * 返回指定key对应的value
     */
    public String getIniKey(String key) {
      if (properties.containsKey(key) == false) {
        return null;
      }
      return String.valueOf(properties.get(key));
    }
  }

  // ----------------------------------------------------------------------------------------------

  public void shareString(String title, String content, QtActivity activity) {
    Intent share = new Intent(Intent.ACTION_SEND);
    share.setType("text/plain"); // 分享字符串
    share.putExtra(Intent.EXTRA_TEXT, content);
    activity.startActivity(Intent.createChooser(share, title));
  }

  /**
   * 分享功能
   */
  // 分享单张图片
  public void shareImage(
      String title,
      String path,
      String fileType,
      QtActivity activity) {
    Intent share = new Intent(Intent.ACTION_SEND);
    share.setType(fileType); // "image/png"

    Uri photoUri;
    if (Build.VERSION.SDK_INT >= 24) {
      photoUri = FileProvider.getUriForFile(
          getMyAppContext(),
          getMyAppContext().getPackageName(),
          new File(path));
    } else {
      photoUri = Uri.fromFile(new File(path));
    }
    System.out.println("path=" + path + "  pathUri=" + photoUri);
    share.putExtra(Intent.EXTRA_STREAM, photoUri);
    activity.startActivity(Intent.createChooser(share, title));
  }

  // 分享多张图片
  public static void shareImages(
      String title,
      String imagesPath,
      QtActivity activity) {
    String[] pathList = imagesPath.split("\\|"); // 由于"|"是转义字符，所以不能直接写 "|"做分割
    ArrayList<Uri> imagesUriList = new ArrayList<Uri>();
    for (int i = 0; i < pathList.length; ++i) {
      File file = new File(pathList[i]);
      if (file.isFile()) {
        imagesUriList.add(Uri.fromFile(file));
      }
    }

    Intent intent = new Intent(Intent.ACTION_SEND_MULTIPLE);
    intent.setType("image/*");
    intent.putParcelableArrayListExtra(Intent.EXTRA_STREAM, imagesUriList);
    intent.putExtra(Intent.EXTRA_SUBJECT, title);
    activity.startActivity(Intent.createChooser(intent, title));
  }

  public void openNoteEditor() {
    Intent i = new Intent(getMyAppContext(), NoteEditor.class);
    i.putExtra("MD_FILE_PATH", strMDFile);
    i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    getMyAppContext().startActivity(i);
  }

  public static void openMDWindow() {
    Intent i = new Intent(getMyAppContext(), MDActivity.class);
    i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    getMyAppContext().startActivity(i);
  }

  public void openDateTimePicker() {
    Intent i = new Intent(getMyAppContext(), DateTimePicker.class);
    i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    getMyAppContext().startActivity(i);
  }

  public void openMyPDF(String path) {
    Uri fileUri;
    if (Build.VERSION.SDK_INT >= 24) {
      fileUri = FileProvider.getUriForFile(
          getMyAppContext(),
          getMyAppContext().getPackageName(),
          new File(path));
    } else {
      fileUri = Uri.fromFile(new File(path));
    }

    Intent i = new Intent(getMyAppContext(), PDFActivity.class);
    i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    i.setData(fileUri);
    getMyAppContext().startActivity(i);
  }

  public void closeMyPDF() {
    if (PDFActivity.mPdfActivity != null)
      PDFActivity.mPdfActivity.finish();
  }

  public void openFilePicker() {
    Intent i = new Intent(getMyAppContext(), FilePicker.class);
    i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    getMyAppContext().startActivity(i);
  }

  public void closeFilePicker() {
    if (FilePicker.MyFilepicker != null)
      FilePicker.MyFilepicker.finish();
  }

  private void addDeskShortcuts() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N_MR1) {
      SharedPreferences prefs = getSharedPreferences("app_prefs", MODE_PRIVATE);
      if (prefs.getBoolean("shortcuts_initialized", false)) {
        return; // 已初始化则跳过
      }

      // 获取ShortcutManager对象
      shortcutManager = getSystemService(ShortcutManager.class);

      String lblAddRecord = null;
      String lblNewTodo = null;
      String lblNewNote = null;
      String lblContinueReading = null;
      String lblExercise = null;
      if (zh_cn) {
        lblAddRecord = getString(R.string.addRecord_shortcut_short_label_zh);
        lblNewTodo = getString(R.string.newTodo_shortcut_short_label_zh);
        lblNewNote = getString(R.string.newNote_shortcut_short_label_zh);
        lblContinueReading = getString(R.string.continueReading_shortcut_short_label_zh);
        lblExercise = getString(R.string.exercise_shortcut_short_label_zh);
      } else {
        lblAddRecord = getString(R.string.addRecord_shortcut_short_label);
        lblNewTodo = getString(R.string.newTodo_shortcut_short_label);
        lblNewNote = getString(R.string.newNote_shortcut_short_label);
        lblContinueReading = getString(R.string.continueReading_shortcut_short_label);
        lblExercise = getString(R.string.exercise_shortcut_short_label);
      }

      ShortcutInfo shortcut0 = new ShortcutInfo.Builder(this, "Add_Record")
          .setShortLabel(lblAddRecord)
          .setIcon(Icon.createWithResource(this, R.drawable.addrecord))
          .setIntent(new Intent(Intent.ACTION_MAIN, null, this, AddRecord.class))
          .build();

      ShortcutInfo shortcut1 = new ShortcutInfo.Builder(this, "New_Todo")
          .setShortLabel(lblNewTodo)
          .setIcon(Icon.createWithResource(this, R.drawable.newtodo))
          .setIntent(new Intent("com.x.action.NEW_TODO", null, this, NewTodo.class))
          .build();

      ShortcutInfo shortcut2 = new ShortcutInfo.Builder(this, "New_Note")
          .setShortLabel(lblNewNote)
          .setIcon(Icon.createWithResource(this, R.drawable.newnote))
          .setIntent(new Intent(Intent.ACTION_MAIN, null, this, NewNote.class))
          .build();

      ShortcutInfo shortcut3 = new ShortcutInfo.Builder(this, "New_Exercise")
          .setShortLabel(lblExercise)
          .setIcon(Icon.createWithResource(this, R.drawable.exercise))
          .setIntent(new Intent(Intent.ACTION_MAIN, null, this, Desk_Exercise.class))
          .build();

      ShortcutInfo shortcut4 = new ShortcutInfo.Builder(
          this,
          "Continue_Reading")
          .setShortLabel(lblContinueReading)
          .setIcon(Icon.createWithResource(this, R.drawable.continuereading))
          .setIntent(
              new Intent(Intent.ACTION_MAIN, null, this, ContinueReading.class))
          .build();

      // 设置动态快捷方式（仅首次）
      shortcutManager.setDynamicShortcuts(Arrays.asList(shortcut0, shortcut1,
          shortcut2, shortcut3));
      prefs.edit().putBoolean("shortcuts_initialized", true).apply();

    }
  }

  private static Handler m_handler = new Handler() {

    @Override
    public void handleMessage(Message msg) {
      switch (msg.what) {
        case 1:
          Toast toast = Toast.makeText(m_instance, (String) msg.obj, Toast.LENGTH_LONG);
          toast.show();
          break;
      }
    }
  };

  public void showToastMessage(String msg) {
    m_handler.sendMessage(m_handler.obtainMessage(1, msg));
  }

  public static void showAndroidProgressBar() {
    Intent i = new Intent(getMyAppContext(), MyProgBar.class);
    i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    getMyAppContext().startActivity(i);
  }

  public static void closeAndroidProgressBar() {
    if (MyProgBar.m_MyProgBar != null)
      MyProgBar.m_MyProgBar.finish();
  }

  public void showTempActivity() {
    Intent i = new Intent(getMyAppContext(), TempActivity.class);
    i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    getMyAppContext().startActivity(i);
  }

  public static void playMyText(String text) {
    mytts.speak(text);

  }

  public static void stopPlayMyText() {
    mytts.stop();

  }

  public void speakText(String text) {
    // 简短播报直接使用
    // TTSUtils.getInstance(this).speak(text);

    // 长时间播报使用前台服务
    Intent serviceIntent = new Intent(this, TTSForegroundService.class);
    serviceIntent.putExtra("tts_text", text);

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      startForegroundService(serviceIntent);
    } else {
      startService(serviceIntent);
    }
  }

  private void createAudioRecord() {
    recordBufsize = AudioRecord.getMinBufferSize(
        44100,
        AudioFormat.CHANNEL_IN_MONO,
        AudioFormat.ENCODING_PCM_16BIT);
    Log.i("audioRecordTest", "size->" + recordBufsize);
    audioRecord = new AudioRecord(
        MediaRecorder.AudioSource.MIC,
        44100,
        AudioFormat.CHANNEL_IN_MONO,
        AudioFormat.ENCODING_PCM_16BIT,
        recordBufsize);
  }

  public void startRecord_pcm(String FILE_NAME) {
    createAudioRecord();
    if (isRecording) {
      return;
    }
    isRecording = true;
    audioRecord.startRecording();
    Log.i("audioRecordTest", "开始录音");
    recordingThread = new Thread(
        () -> {
          byte data[] = new byte[recordBufsize];
          File file = new File(FILE_NAME);
          FileOutputStream os = null;
          try {
            if (!file.exists()) {
              file.createNewFile();
              Log.i("audioRecordTest", "创建录音文件->" + FILE_NAME);
            }
            os = new FileOutputStream(file);
          } catch (Exception e) {
            e.printStackTrace();
          }
          int read;
          if (os != null) {
            while (isRecording) {
              read = audioRecord.read(data, 0, recordBufsize);
              if (AudioRecord.ERROR_INVALID_OPERATION != read) {
                try {
                  os.write(data);
                  Log.i("audioRecordTest", "写录音数据->" + read);
                } catch (IOException e) {
                  e.printStackTrace();
                }
              }
            }
          }
          try {
            os.close();
          } catch (IOException e) {
            e.printStackTrace();
          }
        });
    recordingThread.start();
  }

  public void stopRecord_pcm() {
    isRecording = false;
    if (audioRecord != null) {
      audioRecord.stop();
      Log.i("audioRecordTest", "停止录音");
      audioRecord.release();
      audioRecord = null;
      recordingThread = null;
    }
  }

  public void startRecord(String outputFile) {
    try {
      recorder = new MediaRecorder();
      recorder.setAudioSource(MediaRecorder.AudioSource.MIC);
      recorder.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);
      recorder.setAudioEncoder(MediaRecorder.AudioEncoder.AAC);
      recorder.setOutputFile(outputFile);
      recorder.prepare();
      recorder.start();

      updateMicStatus();
    } catch (Exception ex) {
      ex.printStackTrace();
    }

    Log.i("audioRecord", "开始录音");
  }

  public double updateMicStatus() {
    double db = 0;// 分贝
    if (recorder != null) {
      double ratio = (double) recorder.getMaxAmplitude() / 1; // 参考振幅为 1
      if (ratio > 1) {
        db = 20 * Math.log10(ratio);
      }
      // Log.d(TAG, "计算分贝值 = " + db + "dB");
    }
    return db;
  }

  public void stopRecord() {
    if (recorder != null) {
      try {
        recorder.stop();
        recorder.release(); // 释放资源
      } catch (Exception e) {
        Log.e(TAG, "stopRecord error", e);
      } finally {
        recorder = null;
      }
    }
  }

  public void playRecord(String outputFile) {
    if (player != null) {
      player.stop();
    }

    try {
      player = new MediaPlayer();
      player.setDataSource(outputFile);
      player.prepare();
      player.start();
    } catch (Exception ex) {
      ex.printStackTrace();
    }
  }

  public void startPlay() {
    if (player != null) {
      player.start();
    }
  }

  public void pausePlay() {
    if (player != null) {
      player.pause();
    }
  }

  public void stopPlayRecord() {
    if (player != null) {
      try {
        player.stop();
        player.release(); // 释放资源
      } catch (Exception e) {
        Log.e(TAG, "stopPlayRecord error", e);
      } finally {
        player = null;
      }
    }
  }

  public int getPlayDuration() {
    int nDuration = 0;
    if (player != null) {
      nDuration = player.getDuration();
    }
    return nDuration;
  }

  public int getPlayPosition() {
    int nPosition = 0;
    if (player != null) {
      nPosition = player.getCurrentPosition();
    }
    return nPosition;
  }

  public int isPlaying() {
    int a = 0;
    if (player.isPlaying())
      a = 1;
    else
      a = 0;
    return a;
  }

  public void seekTo(String strPos) {
    if (player != null) {
      int position = Integer.parseInt(strPos);
      player.seekTo(position);
    }
  }

  public static void closeAllAlarmWindows() {
    int count = alarmWindows.size();
    for (int i = 0; i < count; i++) {
      Activity mActivity = alarmWindows.get(count - 1 - i);
      mActivity.finish();
    }
    alarmWindows.clear();
  }

  public static boolean isZh(Context context) {
    Locale locale = context.getResources().getConfiguration().locale;
    String language = locale.getLanguage();
    if (language.endsWith("zh"))
      zh_cn = true;
    else
      zh_cn = false;

    return zh_cn;
  }

  public double getEditStatus() {
    if (isEdit == true) {
      return 1;
    }

    return 0;
  }

  public void setMDTitle(String strTitle) {
    strMDTitle = strTitle;
  }

  public void setMDFile(String file) {
    strMDFile = file;
  }

  public void setFontSize(int nSize) {
    myFontSize = nSize - 3;
  }

  public void setOpenSearchResult(boolean value) {
    isOpenSearchResult = value;
  }

  public void setSearchText(String text) {
    strSearchText = text;
  }

  public boolean getIsBackMainUI() {
    return isBackMainUI;
  }

  public void setIsBackMainUI(boolean value) {
    isBackMainUI = value;
  }

  private void requestPermission() {

    // 安卓11及以上的存储权限
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
      checkStoragePermission();
    } else {
      // 安卓11以下的存储授权
      verifyStoragePermissions(this);
    }

    // 请求通知权限 Android 12+
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
      // 定义权限请求的 Request Code（需唯一）
      int REQUEST_CODE_POST_NOTIFICATIONS = 100;
      if (ContextCompat.checkSelfPermission(this,
          Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
        // 请求权限
        requestPermissions(
            new String[] { Manifest.permission.POST_NOTIFICATIONS },
            REQUEST_CODE_POST_NOTIFICATIONS);
      }
    }

  }

  private void requestSensorPermission() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
      if (ContextCompat.checkSelfPermission(this,
          Manifest.permission.ACTIVITY_RECOGNITION) != PackageManager.PERMISSION_GRANTED) {

        ActivityCompat.requestPermissions(
            this,
            new String[] { Manifest.permission.ACTIVITY_RECOGNITION },
            1997);
      }
    }
  }

  private void updateStatusBarColor() {
    Window window = getWindow();
    WindowInsetsController insetsController = window.getInsetsController();

    // 设置状态栏颜色
    if (isDark) {
      window.setStatusBarColor(Color.parseColor("#19232D")); // 深色背景

      // 设置状态栏文本和图标为白色（亮色）
      if (insetsController != null) {
        insetsController.setSystemBarsAppearance(0, WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS);
      } else {
        window.getDecorView().setSystemUiVisibility(
            window.getDecorView().getSystemUiVisibility() &
                ~View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
      }
    } else {
      window.setStatusBarColor(Color.parseColor("#F3F3F3")); // 浅色背景

      // 设置状态栏文本和图标为黑色（暗色）
      if (insetsController != null) {
        insetsController.setSystemBarsAppearance(
            WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS,
            WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS);
      } else {
        window.getDecorView().setSystemUiVisibility(
            window.getDecorView().getSystemUiVisibility() |
                View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
      }
    }
  }

  public static void hideSoftInput() {
    Log.d(TAG, "===== 开始执行hideSoftInput =====");

    // 1. 检查m_instance是否有效
    if (m_instance == null) {
      Log.e(TAG, "hideSoftInput失败：m_instance（当前Activity）为空！");
      return;
    }
    Log.d(TAG, "当前Activity有效：" + m_instance.getClass().getSimpleName());

    // 2. 获取InputMethodManager（安卓输入法管理器）
    InputMethodManager imm = (InputMethodManager) m_instance.getSystemService(Context.INPUT_METHOD_SERVICE);
    if (imm == null) {
      Log.e(TAG, "hideSoftInput失败：无法获取InputMethodManager！");
      return;
    }
    Log.d(TAG, "成功获取InputMethodManager");

    // 3. 获取当前聚焦的View（输入法依附的输入框）
    View currentFocus = m_instance.getCurrentFocus();
    if (currentFocus == null) {
      Log.w(TAG, "当前没有聚焦的View（可能输入法已关闭或无输入框聚焦）");
      return;
    }
    Log.d(TAG, "当前聚焦的View：" + currentFocus.getClass().getSimpleName() + "（窗口令牌：" + currentFocus.getWindowToken() + "）");

    // 4. 调用原生方法关闭输入法
    boolean result = imm.hideSoftInputFromWindow(
        currentFocus.getWindowToken(),
        InputMethodManager.HIDE_NOT_ALWAYS);
    if (result) {
      Log.d(TAG, "hideSoftInput成功：输入法已关闭");
    } else {
      Log.e(TAG, "hideSoftInput失败：调用系统方法返回false（可能输入法未打开或窗口令牌无效）");
    }

    Log.d(TAG, "===== 结束执行hideSoftInput =====");
  }

  // 在InputMethodUtils类中添加
  public static void forceDisconnectInputMethod() {
    Log.d(TAG, "强制断开输入法连接...");
    if (m_instance == null)
      return;

    m_instance.runOnUiThread(() -> {
      try {
        InputMethodManager imm = (InputMethodManager) m_instance.getSystemService(Context.INPUT_METHOD_SERVICE);

        // 1. 关闭当前输入法
        View currentFocus = m_instance.getCurrentFocus();
        if (currentFocus != null) {
          imm.hideSoftInputFromWindow(currentFocus.getWindowToken(), 0);
        }

        // 2. 根据API级别使用不同方法断开连接
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
          // Android 11+ 推荐方法
          try {
            java.lang.reflect.Method method = imm.getClass().getMethod(
                "dispose", boolean.class);
            method.invoke(imm, true);
            Log.d(TAG, "已通过dispose方法断开输入法连接");
          } catch (Exception e) {
            Log.w(TAG, "无法使用dispose方法: " + e.getMessage());
          }
        } else {
          // Android 11以下尝试finishInputLocked
          try {
            java.lang.reflect.Method method = imm.getClass().getMethod(
                "finishInputLocked", android.view.inputmethod.InputConnection.class);
            method.invoke(imm, (Object) null);
            Log.d(TAG, "已通过finishInputLocked方法断开输入法连接");
          } catch (Exception e) {
            Log.w(TAG, "无法使用finishInputLocked方法: " + e.getMessage());
          }
        }

        // 3. 最后清除焦点（双重保险）
        View decorView = m_instance.getWindow().getDecorView();
        if (decorView != null) {
          decorView.clearFocus();
        }

        Log.d(TAG, "输入法连接已完全断开");
      } catch (Exception e) {
        Log.e(TAG, "断开输入法连接异常: " + e.getMessage());
      }
    });
  }

}
