package com.x;

import android.Manifest;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlarmManager;
import android.app.Application;
import android.app.KeyguardManager;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
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
import android.content.SharedPreferences;
import android.content.UriMatcher;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.ResolveInfo;
import android.content.pm.ShortcutInfo;
import android.content.pm.ShortcutManager;
import android.content.res.Configuration;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.drawable.Icon;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.location.GpsSatellite;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.location.LocationRequest;
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
import android.os.Looper;
import android.os.Message;
import android.os.Parcelable;
import android.os.PersistableBundle;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.Process;
import android.os.StrictMode;
import android.os.SystemClock;
import android.os.Vibrator;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.provider.Settings;
import android.speech.tts.TextToSpeech;
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
import android.view.WindowInsetsController;
import android.view.WindowManager;
import android.view.WindowManager;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.view.inputmethod.InputMethodSession.EventCallback;
import android.webkit.JavascriptInterface;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.app.NotificationCompat; // 兼容库（必须）
import androidx.core.content.ContextCompat;
import androidx.core.content.FileProvider;
import androidx.core.location.LocationListenerCompat;
import androidx.core.location.LocationManagerCompat;
import androidx.core.location.LocationRequestCompat;
import com.x.FilePicker;
import com.x.MyService;
import com.x.ShareReceiveActivity;
import com.x.TTSUtils;
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
import java.lang.reflect.Method;
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
import java.util.Queue; // 导入Queue接口（属于java.util包）
import java.util.Random;
import java.util.Stack;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentLinkedQueue; // 导入线程安全的队列实现（属于java.util.concurrent包）
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService; // 新增导入，解决ExecutorService找不到问题
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit; // 导入时间单位枚举类
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;
import org.osmdroid.util.GeoPoint;
//Qt
import org.qtproject.qt.android.bindings.QtActivity;

public class MyActivity
    extends QtActivity
    implements Application.ActivityLifecycleCallbacks
{

    // 新增：标记是否是配置变更导致的重构 =====
    private boolean isConfigChangeRecreate = false;

    // 新增：标记Service是否已启动（静态，跨Activity实例共享）
    private static boolean isServiceStarted = false;

    // 新增：持久化重启标记的Key
    private static final String PREFS_RESTART = "restart_prefs";
    private static final String KEY_NEED_RESTART = "need_restart";

    // ========== 新增：保存ClockActivity内容的静态变量 ==========
    public static String savedClockContent = ""; // 存储ClockActivity的原始内容
    public static final Object clockLock = new Object(); // 同步锁，保证线程安全
    public static boolean isNeedRestoreClock = false;

    private static final int REQ_LOCATION = 1;
    private static final int REQ_RECORD_AUDIO = 2;
    private static final int REQ_CAMERA = 3;
    private static final int REQ_NOTIFICATION = 4;
    private static final int REQ_ACTIVITY_RECOGNITION = 5;
    private static final int REQ_STORAGE = 6;
    private static final int REQ_BACKGROUND_LOCATION = 7;

    public static String MY_TENCENT_MAP_KEY = "error";
    public static int MapType = 1;

    public static MapActivity mapActivityInstance = null;
    public static List<GeoPoint> osmTrackPoints = new ArrayList<>();
    public static String lblDate = "Date";
    public static String lblInfo = "Speed";

    private static boolean isQtMainEnd = false;

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
    private static Application sAppContext;

    public static boolean isScreenOff = false;

    private static final String TAG = "QtKnot";

    private ShortcutManager shortcutManager;

    private AudioRecord audioRecord = null;
    private int recordBufsize = 0;
    private boolean isRecording = false;
    private Thread recordingThread;

    private MediaRecorder recorder;
    private MediaPlayer player;

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

    public static native void CallJavaNotify_14();

    public static native void CallJavaNotify_15();

    public static native void CallJavaNotify_18();

    public static native void CallJavaNotify_19();

    public static boolean isReadShareData = false;
    public static boolean zh_cn;

    public class VibrateUtils {

        // 产生震动
        public static void vibrate(Context context, long milliseconds) {
            // 获取Vibrator实例
            Vibrator vibrator = (Vibrator) context.getSystemService(
                Context.VIBRATOR_SERVICE
            );
            // 检查设备是否支持震动
            if (vibrator != null && vibrator.hasVibrator()) {
                // 震动
                vibrator.vibrate(milliseconds);
            }
        }
    }

    public static void setVibrate() {
        VibrateUtils.vibrate(m_instance, 50);
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

    // ------------------------------------------------------------------------

    // 非全透,带颜色的状态栏,需要指定颜色（目前采用）
    private void setStatusBarColor(String color) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            // 需要安卓版本大于5.0以上
            getWindow().addFlags(
                WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS
            );
            getWindow().setStatusBarColor(Color.parseColor(color));
        }
    }

    // ----------------------------------------------------------------------

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
                mHandler.postDelayed(
                    () -> {
                        if (MyService.isReady) {
                            CallJavaNotify_2();
                            Log.w("Knot", "成功获取步数");
                        } else {
                            Log.e("Knot", "服务未准备好，无法获取步数");
                        }
                    },
                    500
                ); // 延迟 500 毫秒

                Log.w("Knot", "屏幕亮了");
            } else if (SCREEN_OFF.equals(intent.getAction())) {
                isScreenOff = true;
                CallJavaNotify_18();

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
            ZipInputStream zis = new ZipInputStream(
                new BufferedInputStream(fis)
            );
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

    private String processViewIntent(Intent intent) {
        return intent.getDataString();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // 原有配置变更标记逻辑保留（完全不动你的原始逻辑）
        if (savedInstanceState != null) {
            isConfigChangeRecreate = true;
            Log.d(TAG, "检测到配置变更导致的Activity重构，跳过非必要初始化");
        }

        // 1. 轻量初始化（必须同步执行，确保JNI环境基础就绪）
        isZh(this);
        m_instance = this;
        sAppContext = getApplication();
        Log.d(
            TAG,
            "Android activity created (config change: " +
                isConfigChangeRecreate +
                ")"
        );

        // 2. 静态实例校验（保留原始逻辑）
        if (checkDuplicateInstance()) {
            return;
        }

        // 3. 关键修复：放弃HandlerThread异步，改用延迟post（避免JNI环境未就绪）
        // 核心：等待Qt层初始化完成（延迟500ms，适配你的mw_one创建时机）
        new Handler(Looper.getMainLooper()).postDelayed(
            () -> {
                // 仅在主线程执行初始化（避免JNI跨线程问题）
                try {
                    // 权限请求（仅首次创建执行）
                    if (!isConfigChangeRecreate) {
                        requestPermission();
                        requestSensorPermission();
                    }

                    // 注册广播接收器（配置变更重构时重新注册）
                    if (mScreenStatusReceiver != null) {
                        try {
                            unregisterReceiver(mScreenStatusReceiver);
                        } catch (IllegalArgumentException e) {
                            // 未注册时的异常直接忽略，避免日志告警
                            Log.d(TAG, "ScreenStatusReceiver未注册，无需注销");
                        } catch (Exception e) {
                            Log.w(TAG, "注销ScreenStatusReceiver异常", e);
                        }
                        mScreenStatusReceiver = null;
                    }
                    registSreenStatusReceiver();

                    // 注册Activity生命周期回调
                    getApplication().registerActivityLifecycleCallbacks(
                        MyActivity.this
                    );

                    // 启动服务（仅首次创建执行）
                    if (!isConfigChangeRecreate && !isServiceStarted) {
                        startMyService();
                    }

                    // 创建快捷方式（仅首次创建执行）
                    if (!isConfigChangeRecreate) {
                        addDeskShortcuts();
                    }

                    if (isConfigChangeRecreate && isQtMainEnd) {
                        // 确保mw_one已创建完成后再调用

                        Log.d(TAG, "配置变更初始化完成，Qt已就绪，可以调用C++");

                        //hideQtSplashScreen(); // 主动隐藏闪屏
                    }
                } catch (Exception e) {
                    Log.e(TAG, "初始化异常", e);
                }
            },
            500
        ); // 延迟500ms：适配你的Qt层mw_one实例创建时间（可根据实际调整）

        // 页面刚打开 / 被唤醒 / 从后台回来时，不自动弹出输入法
        getWindow().setSoftInputMode(
            WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN
        );
    }

    // ===== 新增：简化的实例重复校验方法 =====
    private boolean checkDuplicateInstance() {
        if (m_instance != null && m_instance != this) {
            Log.d(TAG, "App is already running... this won't work");
            // 处理Intent（简化逻辑）
            Intent intent = getIntent();
            if (intent != null && intent.getDataString() != null) {
                handleSharedIntent(intent);
            }
            finish();
            return true;
        }
        return false;
    }

    // ===== 新增：处理共享Intent的简化方法 =====
    private void handleSharedIntent(Intent intent) {
        try {
            String sharedData = processViewIntent(intent);
            if (sharedData != null) {
                Log.d(TAG, "It's a view intent: " + sharedData);
                Intent viewIntent = new Intent(
                    getApplicationContext(),
                    MyActivity.class
                );
                viewIntent.setAction(Intent.ACTION_VIEW);
                viewIntent.putExtra("sharedData", sharedData);
                viewIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(viewIntent);
            }
        } catch (Exception e) {
            Log.e(TAG, "处理共享Intent异常", e);
        }
    }

    // ===== 新增：启动服务的封装方法 =====
    private void startMyService() {
        try {
            Intent serviceIntent = new Intent(this, MyService.class);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                startForegroundService(serviceIntent);
            } else {
                startService(serviceIntent);
            }
            isServiceStarted = true;
            Log.d(TAG, "MyService启动成功");
        } catch (Exception e) {
            Log.e(TAG, "启动MyService异常", e);
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        // 只做一件事：暗黑模式切换时，锁定Qt交互
        int nightMode = newConfig.uiMode & Configuration.UI_MODE_NIGHT_MASK;
        if (
            nightMode == Configuration.UI_MODE_NIGHT_YES ||
            nightMode == Configuration.UI_MODE_NIGHT_NO
        ) {
            QtStateManager.getInstance().lockQtInteraction();
        }
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
            // 检查当前是否有对话框正在显示
            if (isDialogShowing()) {
                // 有对话框时，让系统默认处理（关闭对话框）
                return super.onKeyDown(keyCode, event);
            } else {
                // 无对话框时，执行自定义返回逻辑
                CallJavaNotify_15();
                return true; // 事件已处理
            }
        }
        return super.onKeyDown(keyCode, event);
    }

    // 辅助方法：判断当前是否有对话框显示
    private boolean isDialogShowing() {
        // 获取当前Activity的顶层窗口
        Activity activity = this;
        if (activity == null) return false;

        // 检查是否有模态对话框正在显示
        WindowManager.LayoutParams params = activity
            .getWindow()
            .getAttributes();
        return (
            params.type ==
            WindowManager.LayoutParams.TYPE_APPLICATION_ATTACHED_DIALOG
        );
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }

    @Override
    public void onPause() {
        System.out.println("onPause...");
        if (MyService.isReady) CallJavaNotify_1();
        super.onPause();
    }

    @Override
    protected void onResume() {
        System.out.println("onResume...");
        super.onResume();

        updateStatusBarColor();
    }

    @Override
    public void onStop() {
        System.out.println("onStop...");
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        Log.i(
            TAG,
            "Main onDestroy (config change: " + isConfigChangeRecreate + ")"
        );

        // ========== 无论是否配置变更，都移除Handler回调 ==========
        // 1. 处理屏幕状态的mHandler
        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null); // 移除所有未执行任务（关键）
            // 注意：这里先不置null，后续根据是否配置变更决定
        }

        // 2. 处理静态的m_handler（Toast用）
        if (m_handler != null) {
            m_handler.removeCallbacksAndMessages(null); // 仅清理回调，不置null
        }

        // ========== 优化点1：仅在非配置变更销毁时，注销Activity生命周期回调 ==========
        if (!isConfigChangeRecreate) {
            getApplication().unregisterActivityLifecycleCallbacks(this); // 注销回调
        }

        // ========== Qt接收器注销逻辑 ==========
        try {
            // 处理音频接收器：跳过不存在的方法，直接尝试注销
            Class<?> qtAudioDeviceManagerClass = Class.forName(
                "org.qtproject.qt.android.multimedia.QtAudioDeviceManager"
            );
            // 直接调用注销方法，不先判断是否注册（避免依赖不存在的方法）
            Method unregisterAudioMethod = null;
            try {
                unregisterAudioMethod = qtAudioDeviceManagerClass.getMethod(
                    "unregisterAudioHeadsetStateReceiver",
                    Context.class
                );
                unregisterAudioMethod.invoke(null, this);
                Log.d(TAG, "Qt音频接收器注销成功");
            } catch (NoSuchMethodException e) {
                Log.w(TAG, "Qt音频接收器注销方法不存在，跳过", e);
            } catch (Exception e) {
                Log.w(TAG, "Qt音频接收器注销失败（非致命）", e);
            }

            // 新增：处理网络代理接收器（修复第二个泄漏）
            Class<?> qtNetworkClass = Class.forName(
                "org.qtproject.qt.android.network.QtNetwork"
            );
            Method unregisterNetworkMethod = null;
            try {
                unregisterNetworkMethod = qtNetworkClass.getMethod(
                    "unregisterReceiver",
                    Context.class
                );
                unregisterNetworkMethod.invoke(null, this);
                Log.d(TAG, "Qt网络接收器注销成功");
            } catch (NoSuchMethodException e) {
                Log.w(TAG, "Qt网络接收器注销方法不存在，跳过", e);
            } catch (Exception e) {
                Log.w(TAG, "Qt网络接收器注销失败（非致命）", e);
            }
        } catch (ClassNotFoundException e) {
            Log.w(TAG, "Qt相关类未找到，跳过接收器注销", e);
        }

        // ========== 优化点2：仅在非配置变更销毁时，注销自己的广播接收器 ==========
        // 移除原有的 if (!isConfigChangeRecreate) 判断，无条件注销
        if (mScreenStatusReceiver != null) {
            try {
                unregisterReceiver(mScreenStatusReceiver);
                Log.d(TAG, "屏幕状态接收器已注销");
            } catch (Exception e) {
                Log.w(TAG, "注销屏幕状态接收器失败", e);
            } finally {
                mScreenStatusReceiver = null;
            }
        }

        // ========== 优化点3：仅在非配置变更+应用真退出时，停止Service ==========
        // 核心判断：当前Activity是任务栈根Activity（主Activity） + 是用户主动关闭（而非重建）
        boolean isAppExit = isTaskRoot() && isFinishing();

        // 仅应用真退出 + 非配置变更时，才停止Service
        if (isAppExit && !isConfigChangeRecreate) {
            // 真正退出时：清空引用，加速GC
            if (mHandler != null) {
                mHandler = null;
            }

            Intent serviceIntent = new Intent(this, MyService.class);
            stopService(serviceIntent);
            isServiceStarted = false;
            Log.d(TAG, "应用已退出，停止MyService");
        } else {
            Log.d(TAG, "仅Activity重建（配置变更），不停止MyService");
        }

        // ========== 新增：兜底释放所有资源（原有逻辑保留） ==========
        // 停止录音/播放
        stopRecord();
        stopRecord_pcm();
        stopPlayRecord();

        // ========== 优化点4：仅在非配置变更时，清空静态引用（避免重构后引用丢失） ==========
        if (!isConfigChangeRecreate) {
            mapActivityInstance = null;
            m_instance = null;
            sAppContext = null;
        }

        // 新增：重置配置变更标记（避免后续复用实例时状态异常）
        isConfigChangeRecreate = false;

        super.onDestroy();

        // ========== 优化点5：仅在非配置变更时，执行重启逻辑 ==========
        if (!isConfigChangeRecreate && checkNeedRestart()) {
            restartApp();
            Log.i(TAG, "触发应用重启...");
        }

        // ========== 补充：Activity销毁时清理资源（避免隐性问题） ==========
        Window window = getWindow();
        if (window == null) return;

        // 清理屏幕常亮Flag，避免影响其他页面
        window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        // Android 9.0+ 重置屏幕唤醒/锁屏显示状态
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            setTurnScreenOn(false);
            setShowWhenLocked(false);
        } else {
            // Android 8.0-8.1 清理废弃Flag
            window.clearFlags(WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED);
            window.clearFlags(WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD);
            window.clearFlags(WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON);
        }
    }

    @Override
    public void onActivityCreated(
        Activity activity,
        Bundle savedInstanceState
    ) {}

    @Override
    public void onActivityStarted(Activity activity) {}

    @Override
    public void onActivityResumed(Activity activity) {}

    @Override
    public void onActivityPaused(Activity activity) {}

    @Override
    public void onActivityStopped(Activity activity) {
        // 转至后台
        System.out.println("MyActivity onActivityStopped...");
    }

    @Override
    public void onActivitySaveInstanceState(
        Activity activity,
        Bundle outState
    ) {}

    @Override
    public void onActivityDestroyed(Activity activity) {}

    @Override
    public void onRequestPermissionsResult(
        int requestCode,
        @NonNull String[] permissions,
        @NonNull int[] grantResults
    ) {
        super.onRequestPermissionsResult(
            requestCode,
            permissions,
            grantResults
        );
        // 校验：权限结果数组为空直接返回（避免崩溃）
        if (grantResults == null || grantResults.length == 0) return;

        switch (requestCode) {
            // 1. 定位权限（原有逻辑保留，无需改）
            case REQ_LOCATION:
                boolean isGranted =
                    grantResults[0] == PackageManager.PERMISSION_GRANTED;
                if (isGranted) {
                    Toast.makeText(
                        this,
                        MyActivity.zh_cn
                            ? "定位权限已授予，正在启动GPS"
                            : "Location permission granted, starting GPS",
                        Toast.LENGTH_SHORT
                    ).show();

                    // Android 10+ 直接申请后台定位权限
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                        requestBackgroundLocationPermission();
                    }

                    // 直接启动GPS（服务已提前启动，无需兜底）
                    if (
                        getIntent().getBooleanExtra(
                            "START_GPS_AFTER_PERMISSION",
                            false
                        )
                    ) {
                        MyService.getInstance().startGPS();
                    }
                } else {
                    // 仅基础提示，依赖系统原生引导
                    Toast.makeText(
                        this,
                        MyActivity.zh_cn
                            ? "定位权限被拒绝，GPS功能无法使用"
                            : "Location permission denied, GPS function unavailable",
                        Toast.LENGTH_SHORT
                    ).show();
                }
                break;
            // 后台定位权限回调
            case REQ_BACKGROUND_LOCATION:
                boolean isBgGranted =
                    grantResults[0] == PackageManager.PERMISSION_GRANTED;
                if (isBgGranted) {
                    Toast.makeText(
                        this,
                        MyActivity.zh_cn
                            ? "后台定位权限已授予"
                            : "Background location permission granted",
                        Toast.LENGTH_SHORT
                    ).show();
                } else {
                    Toast.makeText(
                        this,
                        MyActivity.zh_cn
                            ? "后台定位权限被拒绝，仅前台可使用GPS"
                            : "Background location permission denied, GPS only available in foreground",
                        Toast.LENGTH_SHORT
                    ).show();
                }
                break;
            // ========== 后台定位权限回调分支结束 ==========
            // 2. 录音权限（新增处理）
            case REQ_RECORD_AUDIO:
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Toast.makeText(
                        this,
                        zh_cn
                            ? "录音权限已授予，可正常录音"
                            : "Microphone permission granted, recording available",
                        Toast.LENGTH_SHORT
                    ).show();
                    // 可选：如果用户是在点击录音按钮时申请的，这里可以自动启动录音
                    // 比如：if (isWaitingForRecord) startRecord(xxx);
                } else {
                    Toast.makeText(
                        this,
                        zh_cn
                            ? "录音权限被拒绝，无法使用录音功能"
                            : "Microphone permission denied, recording unavailable",
                        Toast.LENGTH_SHORT
                    ).show();
                }
                break;
            // 3. 相机权限（新增处理）
            case REQ_CAMERA:
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Toast.makeText(
                        this,
                        zh_cn ? "相机权限已授予" : "Camera permission granted",
                        Toast.LENGTH_SHORT
                    ).show();
                    // 若后续添加相机功能，这里可触发相机初始化
                } else {
                    Toast.makeText(
                        this,
                        zh_cn
                            ? "相机权限被拒绝，无法使用相机功能"
                            : "Camera permission denied, camera function unavailable",
                        Toast.LENGTH_SHORT
                    ).show();
                }
                break;
            // 4. 通知权限（新增处理）
            case REQ_NOTIFICATION:
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Toast.makeText(
                        this,
                        zh_cn
                            ? "通知权限已授予，可接收提醒"
                            : "Notification permission granted, alerts available",
                        Toast.LENGTH_SHORT
                    ).show();
                } else {
                    Toast.makeText(
                        this,
                        zh_cn
                            ? "通知权限被拒绝，将无法接收闹钟、提醒等通知"
                            : "Notification permission denied, unable to receive alarms, alerts, etc.",
                        Toast.LENGTH_LONG
                    ).show();
                }
                break;
            // 5. 运动传感器权限（新增处理）
            case REQ_ACTIVITY_RECOGNITION:
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Toast.makeText(
                        this,
                        zh_cn
                            ? "运动权限已授予，可统计步数"
                            : "Activity recognition permission granted, step counting available",
                        Toast.LENGTH_SHORT
                    ).show();

                    // 权限授予后初始化传感器
                    MyService service = MyService.getInstance();
                    if (service != null) {
                        //service.initStepSensor(); // 改为实例调用，而非静态
                        service.initStepSensor(getApplicationContext());
                    }
                } else {
                    Toast.makeText(
                        this,
                        zh_cn
                            ? "运动权限被拒绝，无法统计步数"
                            : "Activity recognition permission denied, step counting unavailable",
                        Toast.LENGTH_SHORT
                    ).show();
                }
                break;
            // 6. 存储权限
            case REQ_STORAGE:
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Toast.makeText(
                        this,
                        zh_cn
                            ? "存储权限已授予，可读写文件"
                            : "Storage permission granted, file read/write available",
                        Toast.LENGTH_SHORT
                    ).show();
                } else {
                    Toast.makeText(
                        this,
                        zh_cn
                            ? "存储权限被拒绝，无法读写文件"
                            : "Storage permission denied, file read/write unavailable",
                        Toast.LENGTH_SHORT
                    ).show();
                }
                break;
        }
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
                newApkFile
            );
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        } else {
            uri = Uri.fromFile(newApkFile);
        }
        intent.setDataAndType(uri, type);
        getMyAppContext().startActivity(intent);
    }

    // ==============================================================================================
    private void checkStoragePermission() {
        SharedPreferences prefs = getSharedPreferences(
            PREFS_NAME,
            MODE_PRIVATE
        );
        boolean shouldRequest = prefs.getBoolean(KEY_SHOULD_REQUEST, true);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager() && shouldRequest) {
                // 更新标记，避免重复跳转
                prefs.edit().putBoolean(KEY_SHOULD_REQUEST, false).apply();

                Intent intent = new Intent(
                    Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION
                );
                startActivity(intent);
            }
        }
    }

    // 动态获取权限需要添加的常量
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
                "android.permission.WRITE_EXTERNAL_STORAGE"
            );
            if (permission != PackageManager.PERMISSION_GRANTED) {
                // 没有写的权限，去申请写的权限，会弹出对话框
                ActivityCompat.requestPermissions(
                    activity,
                    PERMISSIONS_STORAGE,
                    REQ_STORAGE
                );
            }

            // 申请记录音频的权限，会弹出对话框
            int permissionRecordAudio = ActivityCompat.checkSelfPermission(
                activity,
                "android.permission.RECORD_AUDIO"
            );
            if (permissionRecordAudio != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(
                    activity,
                    new String[] { "android.permission.RECORD_AUDIO" },
                    REQ_RECORD_AUDIO
                );
            }

            // 检查摄像头权限
            int permissionRecordCamera = ActivityCompat.checkSelfPermission(
                activity,
                "android.permission.CAMERA"
            );
            if (permissionRecordCamera != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(
                    activity,
                    new String[] { "android.permission.CAMERA" },
                    REQ_CAMERA
                );
            } else {
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static boolean checkCamera() {
        int permissionRecordCamera = ActivityCompat.checkSelfPermission(
            m_instance,
            "android.permission.CAMERA"
        );
        if (permissionRecordCamera != PackageManager.PERMISSION_GRANTED) {
            return false;
        } else {
            return true;
        }
    }

    public static int checkRecordAudio() {
        int permissionRecordCamera = ActivityCompat.checkSelfPermission(
            m_instance,
            "android.permission.RECORD_AUDIO"
        );
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
                "UTF-8"
            );
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
        QtActivity activity
    ) {
        Intent share = new Intent(Intent.ACTION_SEND);
        share.setType(fileType); // "image/png"

        Uri photoUri;
        if (Build.VERSION.SDK_INT >= 24) {
            photoUri = FileProvider.getUriForFile(
                getMyAppContext(),
                getMyAppContext().getPackageName(),
                new File(path)
            );
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
        QtActivity activity
    ) {
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

    public void openMapWindow() {
        // 1. 根据MapType选择目标地图Activity
        Class<?> targetActivity = (MapType == 1)
            ? MapActivity.class
            : TencentMapActivity.class;

        // 2. 创建Intent，核心添加"移到前台"标志
        Intent intent = new Intent(getMyAppContext(), targetActivity);
        intent.setFlags(
            Intent.FLAG_ACTIVITY_NEW_TASK | // 非Activity上下文启动必须
                Intent.FLAG_ACTIVITY_REORDER_TO_FRONT // 关键：若Activity已存在，直接移到前台
        );

        // 3. 启动（若已存在则唤醒，不存在则新建）
        getMyAppContext().startActivity(intent);
    }

    public void forwardClearTrack() {
        // 先判断MapActivity是否已创建
        if (mapActivityInstance != null) {
            mapActivityInstance.clearTrack(); // 调用MapActivity的清除方法
        } else {
            // 可选：日志或提示地图窗口未打开
            android.util.Log.w("MyActivity", "MapActivity未启动，无法清除轨迹");
        }
    }

    public void forwardAppendTrackPoint(double latitude, double longitude) {
        if (mapActivityInstance != null) {
            // 额外判断：Activity是否未销毁且未处于 finishing 状态
            if (
                !mapActivityInstance.isDestroyed() &&
                !mapActivityInstance.isFinishing()
            ) {
                mapActivityInstance.appendTrackPoint(latitude, longitude);
            } else {
                Log.w("MyActivity", "MapActivity已销毁，跳过追加轨迹点");
            }
        } else {
            Log.w("MyActivity", "MapActivity未启动，无法追加轨迹点");
        }
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
                new File(path)
            );
        } else {
            fileUri = Uri.fromFile(new File(path));
        }

        Intent i = new Intent(getMyAppContext(), PDFActivity.class);
        i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        i.setData(fileUri);
        getMyAppContext().startActivity(i);
    }

    public void closeMyPDF() {
        if (PDFActivity.mPdfActivity != null) PDFActivity.mPdfActivity.finish();
    }

    public void openFilePicker() {
        Intent i = new Intent(getMyAppContext(), FilePicker.class);
        i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        getMyAppContext().startActivity(i);
    }

    public void closeFilePicker() {
        if (FilePicker.MyFilepicker != null) FilePicker.MyFilepicker.finish();
    }

    private void addDeskShortcuts() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N_MR1) {
            SharedPreferences prefs = getSharedPreferences(
                "app_prefs",
                MODE_PRIVATE
            );
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
                lblAddRecord = getString(
                    R.string.addRecord_shortcut_short_label_zh
                );
                lblNewTodo = getString(
                    R.string.newTodo_shortcut_short_label_zh
                );
                lblNewNote = getString(
                    R.string.newNote_shortcut_short_label_zh
                );
                lblContinueReading = getString(
                    R.string.continueReading_shortcut_short_label_zh
                );
                lblExercise = getString(
                    R.string.exercise_shortcut_short_label_zh
                );
            } else {
                lblAddRecord = getString(
                    R.string.addRecord_shortcut_short_label
                );
                lblNewTodo = getString(R.string.newTodo_shortcut_short_label);
                lblNewNote = getString(R.string.newNote_shortcut_short_label);
                lblContinueReading = getString(
                    R.string.continueReading_shortcut_short_label
                );
                lblExercise = getString(R.string.exercise_shortcut_short_label);
            }

            ShortcutInfo shortcut0 = new ShortcutInfo.Builder(
                this,
                "Add_Record"
            )
                .setShortLabel(lblAddRecord)
                .setIcon(Icon.createWithResource(this, R.drawable.addrecord))
                .setIntent(
                    new Intent(Intent.ACTION_MAIN, null, this, AddRecord.class)
                )
                .build();

            ShortcutInfo shortcut1 = new ShortcutInfo.Builder(this, "New_Todo")
                .setShortLabel(lblNewTodo)
                .setIcon(Icon.createWithResource(this, R.drawable.newtodo))
                .setIntent(
                    new Intent(
                        "com.x.action.NEW_TODO",
                        null,
                        this,
                        NewTodo.class
                    )
                )
                .build();

            ShortcutInfo shortcut2 = new ShortcutInfo.Builder(this, "New_Note")
                .setShortLabel(lblNewNote)
                .setIcon(Icon.createWithResource(this, R.drawable.newnote))
                .setIntent(
                    new Intent(Intent.ACTION_MAIN, null, this, NewNote.class)
                )
                .build();

            ShortcutInfo shortcut3 = new ShortcutInfo.Builder(
                this,
                "New_Exercise"
            )
                .setShortLabel(lblExercise)
                .setIcon(Icon.createWithResource(this, R.drawable.exercise))
                .setIntent(
                    new Intent(
                        Intent.ACTION_MAIN,
                        null,
                        this,
                        Desk_Exercise.class
                    )
                )
                .build();

            ShortcutInfo shortcut4 = new ShortcutInfo.Builder(
                this,
                "Continue_Reading"
            )
                .setShortLabel(lblContinueReading)
                .setIcon(
                    Icon.createWithResource(this, R.drawable.continuereading)
                )
                .setIntent(
                    new Intent(
                        Intent.ACTION_MAIN,
                        null,
                        this,
                        ContinueReading.class
                    )
                )
                .build();

            // 设置动态快捷方式（仅首次）
            shortcutManager.setDynamicShortcuts(
                Arrays.asList(shortcut0, shortcut1, shortcut2, shortcut3)
            );
            prefs.edit().putBoolean("shortcuts_initialized", true).apply();
        }
    }

    private static Handler m_handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case 1:
                    // 使用应用上下文，彻底避免Activity引用泄漏
                    Context appContext = getMyAppContext();
                    Toast toast = Toast.makeText(
                        appContext,
                        (String) msg.obj,
                        Toast.LENGTH_LONG
                    );
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
        if (MyProgBar.m_MyProgBar != null) MyProgBar.m_MyProgBar.finish();
    }

    public void showTempActivity() {
        Intent i = new Intent(getMyAppContext(), TempActivity.class);
        i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        getMyAppContext().startActivity(i);
    }

    private void createAudioRecord() {
        recordBufsize = AudioRecord.getMinBufferSize(
            44100,
            AudioFormat.CHANNEL_IN_MONO,
            AudioFormat.ENCODING_PCM_16BIT
        );
        Log.i("audioRecordTest", "size->" + recordBufsize);
        audioRecord = new AudioRecord(
            MediaRecorder.AudioSource.MIC,
            44100,
            AudioFormat.CHANNEL_IN_MONO,
            AudioFormat.ENCODING_PCM_16BIT,
            recordBufsize
        );
    }

    public void startRecord_pcm(String FILE_NAME) {
        createAudioRecord();
        if (isRecording) {
            return;
        }
        isRecording = true;
        audioRecord.startRecording();
        Log.i("audioRecordTest", "开始录音");
        recordingThread = new Thread(() -> {
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
        // 第一步：再次检查录音权限
        int recordPermission = ContextCompat.checkSelfPermission(
            this,
            Manifest.permission.RECORD_AUDIO
        );
        if (recordPermission != PackageManager.PERMISSION_GRANTED) {
            // 权限缺失，主动请求录音权限
            ActivityCompat.requestPermissions(
                this,
                new String[] { Manifest.permission.RECORD_AUDIO },
                REQ_RECORD_AUDIO
            );
            Toast.makeText(
                this,
                zh_cn
                    ? "需要录音权限才能开始录制"
                    : "Recording permission is required to start recording",
                Toast.LENGTH_SHORT
            ).show();
            return;
        }

        // 第二步：原有录制逻辑（保留临时变量和异常捕获）
        MediaRecorder tempRecorder = new MediaRecorder();
        try {
            tempRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
            tempRecorder.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);
            tempRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AAC);
            tempRecorder.setOutputFile(outputFile);
            tempRecorder.prepare();
            tempRecorder.start();
            recorder = tempRecorder; // 初始化成功后赋值给成员变量
            updateMicStatus();
            Log.i(TAG, "音频录制已开始：" + outputFile);
        } catch (IOException e) {
            Log.e(TAG, "录制初始化失败（IO异常）", e);
            Toast.makeText(
                this,
                zh_cn ? "录制文件创建失败" : "Failed to create recording file",
                Toast.LENGTH_SHORT
            ).show();
            tempRecorder.release();
            recorder = null;
        } catch (RuntimeException e) {
            Log.e(TAG, "录制启动失败（运行时异常）", e);
            Toast.makeText(
                this,
                zh_cn
                    ? "录音功能启动失败，请检查权限或设备"
                    : "Failed to start recording, check permission or device",
                Toast.LENGTH_SHORT
            ).show();
            tempRecorder.release();
            recorder = null;
        }
    }

    public double updateMicStatus() {
        double db = 0; // 分贝
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
        if (player == null) return 0;
        return player.isPlaying() ? 1 : 0;
    }

    public void seekTo(String strPos) {
        if (player != null) {
            int position = Integer.parseInt(strPos);
            player.seekTo(position);
        }
    }

    public static boolean isZh(Context context) {
        Locale locale;
        // 适配 API 24+ 多语言获取方式
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            locale = context
                .getResources()
                .getConfiguration()
                .getLocales()
                .get(0);
        } else {
            locale = context.getResources().getConfiguration().locale;
        }
        String language = locale.getLanguage();
        // 更严谨：兼容 "zh-CN"、"zh-TW" 等所有中文变体
        zh_cn = language.equalsIgnoreCase("zh");
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
            if (
                ContextCompat.checkSelfPermission(
                    this,
                    Manifest.permission.POST_NOTIFICATIONS
                ) !=
                PackageManager.PERMISSION_GRANTED
            ) {
                // 请求权限
                requestPermissions(
                    new String[] { Manifest.permission.POST_NOTIFICATIONS },
                    REQ_NOTIFICATION
                );
            }
        }
    }

    private void requestSensorPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            if (
                ContextCompat.checkSelfPermission(
                    this,
                    Manifest.permission.ACTIVITY_RECOGNITION
                ) !=
                PackageManager.PERMISSION_GRANTED
            ) {
                ActivityCompat.requestPermissions(
                    this,
                    new String[] { Manifest.permission.ACTIVITY_RECOGNITION },
                    REQ_ACTIVITY_RECOGNITION
                );
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
                insetsController.setSystemBarsAppearance(
                    0,
                    WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS
                );
            } else {
                window
                    .getDecorView()
                    .setSystemUiVisibility(
                        window.getDecorView().getSystemUiVisibility() &
                            ~View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR
                    );
            }
        } else {
            window.setStatusBarColor(Color.parseColor("#F3F3F3")); // 浅色背景

            // 设置状态栏文本和图标为黑色（暗色）
            if (insetsController != null) {
                insetsController.setSystemBarsAppearance(
                    WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS,
                    WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS
                );
            } else {
                window
                    .getDecorView()
                    .setSystemUiVisibility(
                        window.getDecorView().getSystemUiVisibility() |
                            View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR
                    );
            }
        }
    }

    public static void setQtMainEnd(boolean isEnd) {
        isQtMainEnd = isEnd;
    }

    public static void launchWebView() {
        if (m_instance != null) {
            WebViewActivity.openLocalHtml(m_instance);
        }
    }

    /**
     * 保持屏幕常亮（强制在 UI 线程执行）
     */
    public static void keepScreenOn(Activity activity) {
        // 关键：通过 runOnUiThread 切换到 UI 线程
        activity.runOnUiThread(
            new Runnable() {
                @Override
                public void run() {
                    activity
                        .getWindow()
                        .addFlags(
                            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
                        );
                }
            }
        );
    }

    /**
     * 取消屏幕常亮（强制在 UI 线程执行）
     */
    public static void cancelKeepScreenOn(Activity activity) {
        activity.runOnUiThread(
            new Runnable() {
                @Override
                public void run() {
                    activity
                        .getWindow()
                        .clearFlags(
                            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
                        );
                }
            }
        );
    }

    public void addTrackPoint(double latitude, double longitude) {
        // 经纬度合法性校验
        if (
            latitude < -90.0 ||
            latitude > 90.0 ||
            longitude < -180.0 ||
            longitude > 180.0
        ) {
            Log.w(
                TAG,
                "非法经纬度，跳过添加 | 纬度：" +
                    latitude +
                    "，经度：" +
                    longitude
            );
            return;
        }

        // 核心操作：向CopyOnWriteArrayList添加数据（线程安全）
        GeoPoint newPoint = new GeoPoint(latitude, longitude);
        osmTrackPoints.add(newPoint);
        // Log.d(TAG, "已添加轨迹点 | 总数量：" + osmTrackPoints.size());
    }

    public void clearTrackPoints() {
        // 核心操作：清空CopyOnWriteArrayList（线程安全）
        osmTrackPoints.clear();
        Log.d(TAG, "已清空所有轨迹数据");
    }

    /**
     * 遍历 osmTrackPoints 集合，调用 forwardAppendTrackPoint 重放所有轨迹点
     * （实际是通过 MyActivity 转发，最终调用自身的 appendTrackPoint 方法）
     */
    public void replayAllTrackPoints() {
        // 1. 校验集合非空
        if (osmTrackPoints.isEmpty()) {
            Log.d(TAG, "osmTrackPoints 集合为空，无需遍历");
            return;
        }

        // 2. 校验 MyActivity 实例有效（避免空指针）
        if (MyActivity.mapActivityInstance == null) {
            Log.e(TAG, "MyActivity 中 MapActivity 实例为空，无法重放轨迹");
            return;
        }

        Log.d(TAG, "开始遍历轨迹集合，总点数：" + osmTrackPoints.size());

        // 3. 遍历集合（CopyOnWriteArrayList 遍历线程安全）
        for (GeoPoint point : osmTrackPoints) {
            if (point == null) {
                Log.w(TAG, "跳过空的轨迹点");
                continue;
            }

            // 提取经纬度
            double latitude = point.getLatitude();
            double longitude = point.getLongitude();

            // 调用 MyActivity 的 forwardAppendTrackPoint 方法
            forwardAppendTrackPoint(latitude, longitude);

            // 可选：添加延迟，模拟轨迹实时播放效果（单位：毫秒，根据需求调整）
            try {
                Thread.sleep(100); // 每100毫秒播放一个点
            } catch (InterruptedException e) {
                Log.e(TAG, "轨迹播放延迟被中断", e);
                Thread.currentThread().interrupt(); // 恢复中断状态
                break;
            }
        }

        Log.d(TAG, "轨迹集合遍历完成");
    }

    // 调用公开接口（兼容MapActivity和TencentMapActivity）
    public void setDateTitle(String str) {
        lblDate = str;
        if (mapActivityInstance != null) {
            mapActivityInstance.setTopDate(str); // 调用父类公开接口
        }
    }

    public void setInfoTitle(String str) {
        lblInfo = str;
        if (mapActivityInstance != null) {
            mapActivityInstance.setBottomInfo(str); // 调用父类公开接口
        }
    }

    public void setMapKey(String key) {
        MY_TENCENT_MAP_KEY = key;
    }

    public void setMapType(int type) {
        MapType = type;
    }

    public boolean isMapActivityInstance() {
        if (mapActivityInstance != null) return true;
        else return false;
    }

    // 给地图提供“从lastIndex开始的新增轨迹点”
    public static List<GeoPoint> getNewTrackPoints(int lastIndex) {
        List<GeoPoint> newPoints = new ArrayList<>();
        // 避免索引越界（lastIndex超过总长度则无新增）
        if (lastIndex < 0 || lastIndex >= osmTrackPoints.size()) {
            return newPoints;
        }
        // 从lastIndex遍历到最后，获取新增点（CopyOnWriteArrayList遍历安全）
        for (int i = lastIndex; i < osmTrackPoints.size(); i++) {
            GeoPoint point = osmTrackPoints.get(i);
            if (point != null) {
                newPoints.add(point);
            }
        }
        return newPoints;
    }

    public static Context getMyAppContext() {
        if (sAppContext != null) {
            return sAppContext;
        } else if (m_instance != null) {
            return m_instance.getApplicationContext();
        }
        return null;
    }

    // 新增：标记需要重启（替代原setReOpen）
    public void markNeedRestart() {
        SharedPreferences sp = getSharedPreferences(
            PREFS_RESTART,
            Context.MODE_PRIVATE
        );
        sp.edit().putBoolean(KEY_NEED_RESTART, true).apply();
    }

    // 新增：检查并清除重启标记
    private boolean checkNeedRestart() {
        SharedPreferences sp = getSharedPreferences(
            PREFS_RESTART,
            Context.MODE_PRIVATE
        );
        boolean needRestart = sp.getBoolean(KEY_NEED_RESTART, false);
        // 清除标记，避免重复重启
        if (needRestart) {
            sp.edit().putBoolean(KEY_NEED_RESTART, false).apply();
        }
        return needRestart;
    }

    // 重构：彻底重启应用的核心方法
    private void restartApp() {
        try {
            // 1. 获取应用启动Intent（确保启动主Activity）
            PackageManager pm = getPackageManager();
            Intent launchIntent = pm.getLaunchIntentForPackage(
                getPackageName()
            );
            if (launchIntent == null) {
                Log.e(TAG, "获取启动Intent失败");
                return;
            }

            // 2. 添加关键标记：清空任务栈+新建任务+清除顶部
            launchIntent.addFlags(
                Intent.FLAG_ACTIVITY_NEW_TASK | // 新建任务栈
                    Intent.FLAG_ACTIVITY_CLEAR_TASK | // 清空原有任务栈
                    Intent.FLAG_ACTIVITY_CLEAR_TOP | // 清除顶部Activity
                    Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED // 必要时重置任务
            );

            // 3. 主线程延迟启动（避开onDestroy的销毁状态）
            new Handler(Looper.getMainLooper()).postDelayed(
                () -> {
                    startActivity(launchIntent);
                    Log.i(TAG, "启动新应用实例成功");

                    // 4. 强制终止旧进程（关键：确保旧进程退出）
                    Process.killProcess(Process.myPid());
                    System.exit(0);
                },
                300
            ); // 300ms延迟，给系统留足处理时间
        } catch (Exception e) {
            Log.e(TAG, "重启应用失败", e);
        }
    }

    // 核心：主动触发重启（无广播接收器）
    public void triggerRestart() {
        new Handler(Looper.getMainLooper()).post(() -> {
            try {
                // 1. 获取应用启动Intent（确保启动主Activity）
                PackageManager pm = getPackageManager();
                Intent launchIntent = pm.getLaunchIntentForPackage(
                    getPackageName()
                );
                if (launchIntent == null) {
                    Log.e(TAG, "获取启动Intent失败，使用备用方式");
                    // 备用方式：直接指定MyActivity
                    launchIntent = new Intent(this, MyActivity.class);
                    launchIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                }

                // 2. 关键标记：确保全新启动（兼容所有Android版本）
                launchIntent.addFlags(
                    Intent.FLAG_ACTIVITY_NEW_TASK | // 新建任务栈
                        Intent.FLAG_ACTIVITY_CLEAR_TASK | // 清空旧任务栈
                        Intent.FLAG_ACTIVITY_CLEAR_TOP | // 清除顶部Activity
                        Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED // 重置任务栈
                );

                // 3. Android 10+ 后台启动兼容（可选，增强稳定性）
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                    ActivityManager am = (ActivityManager) getSystemService(
                        Context.ACTIVITY_SERVICE
                    );
                    if (am != null) {
                        am.moveTaskToFront(
                            getTaskId(),
                            ActivityManager.MOVE_TASK_NO_USER_ACTION
                        );
                    }
                }

                // 4. 启动新实例（先启动，再终止旧进程）
                startActivity(launchIntent);
                Log.i(TAG, "已触发应用重启，等待旧进程终止");

                // 5. 延迟终止旧进程（给系统留足时间处理启动Intent）
                new Handler(Looper.getMainLooper()).postDelayed(
                    () -> {
                        finish(); // 关闭当前Activity
                        Process.killProcess(Process.myPid()); // 终止旧进程
                        System.exit(0);
                    },
                    500
                ); // 500ms延迟，确保启动Intent已生效
            } catch (Exception e) {
                Log.e(TAG, "重启失败，使用兜底方案", e);
                // 兜底：仅终止进程，依赖onDestroy中的兜底逻辑
                finish();
                Process.killProcess(Process.myPid());
            }
        });
    }

    public void openClockActivityWithContent(String customContent) {
        Intent intent = new Intent(this, ClockActivity.class);
        // 传递自定义内容（对应ClockActivity中读取的"SAVED_CLOCK_CONTENT"）
        intent.putExtra("SAVED_CLOCK_CONTENT", customContent);
        // 可选：设置窗口启动模式，避免重复创建
        intent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        setIntent(intent); // 更新 Activity 的 Intent
        handleAlarmIntent(intent);

        handleLocationPermissionRequest(intent);
    }

    private void handleAlarmIntent(Intent intent) {
        if (intent != null && intent.hasExtra("ALARM_MESSAGE")) {
            String message = intent.getStringExtra("ALARM_MESSAGE");
            // 这里可以通知 Qt 层显示提醒内容，或执行相关操作
            // CallJavaNotify_3();
        }
    }

    // 处理定位权限请求
    private void handleLocationPermissionRequest(Intent intent) {
        if (
            intent != null &&
            intent.getBooleanExtra("REQUEST_LOCATION_PERMISSION", false)
        ) {
            requestLocationPermission();
        }
    }

    public void requestLocationPermission() {
        if (
            ContextCompat.checkSelfPermission(
                this,
                Manifest.permission.ACCESS_FINE_LOCATION
            ) !=
            PackageManager.PERMISSION_GRANTED
        ) {
            // 先申请精确定位
            ActivityCompat.requestPermissions(
                this,
                new String[] { Manifest.permission.ACCESS_FINE_LOCATION },
                REQ_LOCATION
            );
        } else {
            // 已有前台定位 → 申请后台定位
            requestBackgroundLocationPermission();
        }
    }

    // 单独申请后台定位（Android 10+ 才需要）
    private void requestBackgroundLocationPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            if (
                ContextCompat.checkSelfPermission(
                    this,
                    Manifest.permission.ACCESS_BACKGROUND_LOCATION
                ) !=
                PackageManager.PERMISSION_GRANTED
            ) {
                ActivityCompat.requestPermissions(
                    this,
                    new String[] {
                        Manifest.permission.ACCESS_BACKGROUND_LOCATION,
                    },
                    REQ_BACKGROUND_LOCATION
                );
            }
        }
    }

    private void hideQtSplashScreen() {
        try {
            // 方案1：尝试调用带参数的新版方法
            Method hideMethod = getClass().getMethod(
                "hideSplashScreen",
                boolean.class
            );
            hideMethod.invoke(this, false); // false = 不保持在顶部
            Log.d("QtKnot", "新版Qt hideSplashScreen(boolean)调用成功");
        } catch (NoSuchMethodException e) {
            try {
                // 方案2：降级调用无参的旧版方法
                Method hideMethod = getClass().getMethod("hideSplashScreen");
                hideMethod.invoke(this);
                Log.d("QtKnot", "旧版Qt hideSplashScreen()调用成功");
            } catch (Exception ex) {
                // 方案3：终极备用方案 - 直接隐藏闪屏View
                View splashView = getWindow()
                    .getDecorView()
                    .findViewById(android.R.id.content)
                    .findViewWithTag("qt_splash_view");
                if (splashView != null) {
                    splashView.setVisibility(View.GONE);
                    Log.d("QtKnot", "备用方案：直接隐藏闪屏View成功");
                } else {
                    Log.e("QtKnot", "所有隐藏闪屏方案均失败");
                }
                ex.printStackTrace();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        runOnUiThread(() -> {
            // 强制整个窗口重绘
            getWindow().getDecorView().invalidate();
            // 请求布局
            getWindow().getDecorView().requestLayout();
            // 如果 Qt 使用了 SurfaceView，可以尝试显式请求
            View content = findViewById(android.R.id.content);
            if (content != null) {
                content.invalidate();
            }
        });
    }
}
