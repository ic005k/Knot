package com.x;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.AlarmManager;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.ResolveInfo;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.RingtoneManager;
import android.net.Uri;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.provider.Settings;
import android.speech.tts.TextToSpeech;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.view.inputmethod.InputMethodSession.EventCallback;
import android.widget.Toast;
import androidx.core.app.NotificationCompat;
import androidx.core.content.ContextCompat;
import com.x.MyActivity;
import java.sql.Time;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;

public class MyService extends Service {

    // ========== MyService 中新增 TTS 相关逻辑 ==========
    private boolean isTtsInitialized = false;
    private boolean isTtsInitializing = false;
    private String pendingTtsText = null;
    private final Object ttsLock = new Object();
    private TTSUtils currentPlayingTts;

    // ========== 新增GPS逻辑：核心变量 ==========
    public volatile GPSManager gpsManager; // GPS管理器实例
    private static MyService instance;
    private boolean isGpsRunning = false; // GPS运行状态标记

    private String strGpsStatus = "GPS Status";
    private String strRunTime = "00:00:00";
    private String strAltitude = "Altitude: 0.00 m";
    private String strTotalDistance = "0 km";
    private String strMaxSpeed = "Max Speed";
    private String strTotalClimb = "Total Climb";
    private String strTotalDescent = "Total Descent";
    private String strAverageSpeed = "0 km/h";
    // ==========================================

    private static final String TAG = "MyService";
    private static final String ID = "channel_1";
    private static final String NAME = "F_SERVICE";

    private static final int ALARM_REQUEST_CODE = 10086;

    public static String strTodoAlarm = "";

    public static volatile boolean isReady = false;

    private static PersistService mySensorSerivece;

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

    private static SimpleDateFormat format = new SimpleDateFormat("HH:mm:ss");

    public static String strRun;
    public static String strStatus;
    public static String strPedometer;
    public static String strTodo;

    public static Runnable runnable = new Runnable() {
        @Override
        public void run() {}
    };

    @Override
    public IBinder onBind(Intent arg0) {
        Log.i(TAG, "Service on bind"); // 服务被绑定
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "Service on create"); // 服务被创建

        instance = this; // 保存服务实例

        // 优先使用Service自身的上下文，而非依赖MyActivity
        Context serviceContext = getApplicationContext();
        if (serviceContext == null) {
            Log.e(TAG, "Service上下文为空，初始化失败");
            stopSelf();
            return;
        }

        // 初始化多语言文本（原有逻辑，仅换上下文）
        if (MyActivity.isZh(this)) {
            strRun = "运行中...";
            strStatus = "Knot";
            strTodo = "待办事项";
            strPedometer = "倒计时";
        } else {
            strRun = "Running...";
            strStatus = "Knot";
            strTodo = "Todo";
            strPedometer = "Countdown";
        }

        try {
            // 注册闹钟接收器
            IntentFilter filter = new IntentFilter(ACTION_TODO_ALARM);
            registerReceiver(myalarmReceiver, filter);

            // 计步器初始化（仅换上下文为serviceContext）
            mySensorSerivece = new PersistService(serviceContext);
            mSensorManager = (SensorManager) serviceContext.getSystemService(
                Context.SENSOR_SERVICE
            );
            if (mSensorManager != null) {
                countSensor = mSensorManager.getDefaultSensor(
                    Sensor.TYPE_STEP_COUNTER
                );
            }
            initStepSensor(serviceContext); // 传入上下文，避免依赖MyActivity

            // 先初始化AlarmManager，再检查权限（核心修复：解决NPE）
            alarmManager = (AlarmManager) serviceContext.getSystemService(
                Context.ALARM_SERVICE
            );
            // 检查并请求精确闹钟权限（alarmManager空判）
            if (
                Build.VERSION.SDK_INT >= Build.VERSION_CODES.S &&
                alarmManager != null &&
                !alarmManager.canScheduleExactAlarms()
            ) {
                requestExactAlarmPermission(serviceContext);
            }

            // GPS初始化（仅换上下文为serviceContext）
            gpsManager = GPSManager.getInstance(serviceContext);

            isReady = true;
        } catch (Exception e) {
            Log.e(TAG, "Service初始化异常", e);
        }
    }

    // 服务在每次启动的时候调用的方法 如果某些行为在服务已启动的时候就执行，可以把处理逻辑写在这个方法里面
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d("MyService", "onStartCommand()-------");

        if (intent != null) {
            String action = intent.getAction();
            if ("com.x.ACTION_START_GPS".equals(action)) {
                startGPS();
            } else if ("com.x.ACTION_STOP_GPS".equals(action)) {
                stopGPS();
            }
        }

        // 只走 Android 8.0+ 逻辑
        setForeground();

        return START_STICKY; // 仅把super.onStartCommand(...)改为START_STICKY，提升服务稳定性
    }

    // 服务销毁的时候调用的方法 可以回收部分不再使用的资源
    @Override
    public void onDestroy() {
        isReady = false;

        Log.d("MyService", "onDestroy()-------");

        // ========== TTS 资源最终兜底释放 ==========
        stopPlayMyTextInService(); // 停止并释放TTS资源
        // =======================================

        // ========== 服务销毁时停止GPS ==========
        stopGPS();
        // ==========================================
        // 取消闹钟并清空变量
        if (alarmManager != null && pendingIntentAlarm != null) {
            alarmManager.cancel(pendingIntentAlarm);
            pendingIntentAlarm.cancel();
        }
        alarmManager = null;
        pendingIntentAlarm = null;

        // 取消注册接收器
        try {
            unregisterReceiver(myalarmReceiver);
        } catch (IllegalArgumentException e) {
            Log.w(TAG, "广播已注销，无需重复操作", e);
        }

        if (isStepCounter == 1) {
            if (mySensorSerivece != null) {
                mSensorManager.unregisterListener(mySensorSerivece);
            }
        }

        instance = null; // 服务销毁时清空实例

        super.onDestroy();
    }

    @TargetApi(26)
    private void setForeground() {
        Intent notificationIntent = new Intent(this, MyActivity.class);
        notificationIntent.setFlags(
            Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_CLEAR_TOP
        );

        int pendingIntentFlags = PendingIntent.FLAG_UPDATE_CURRENT;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            pendingIntentFlags |= PendingIntent.FLAG_IMMUTABLE;
        }

        PendingIntent pendingIntent = PendingIntent.getActivity(
            this,
            0,
            notificationIntent,
            pendingIntentFlags
        );

        // 前台服务渠道：最低优先级、安静、只显示图标
        String channelId = ID;
        NotificationChannel channel = new NotificationChannel(
            channelId,
            "Knot Service",
            NotificationManager.IMPORTANCE_MIN // 所有8.0+都支持的最低优先级
        );

        // ========== 仅保留所有版本兼容的核心配置 ==========
        channel.setShowBadge(false); // 无角标（8.0+支持）
        channel.enableLights(false); // 无灯光（8.0+支持）
        channel.enableVibration(false); // 无震动（8.0+支持）
        channel.setSound(null, null); // 无声音（8.0+支持）
        channel.setBypassDnd(false); // 不绕过免打扰（8.0+支持）
        channel.setLockscreenVisibility(Notification.VISIBILITY_PRIVATE); // 8.0+支持
        // ==============================================

        NotificationManager manager = (NotificationManager) getSystemService(
            NOTIFICATION_SERVICE
        );
        if (manager != null) {
            manager.createNotificationChannel(channel);
        }

        // 极简前台通知：只留图标 + 点击跳转
        Notification notification = new Notification.Builder(this, channelId)
            .setContentTitle(strStatus)
            .setContentText(strRun)
            .setSmallIcon(R.drawable.icon)
            .setContentIntent(pendingIntent)
            .setOnlyAlertOnce(true)
            .setDefaults(0) // 无任何默认提醒（所有版本兼容）
            .build();

        startForeground(1337, notification);
    }

    // ----------------------------------------------
    // 每天行走的步数通知
    private static NotificationManager m_notificationManager;
    private static Notification.Builder m_builder;

    public static void notify(Context context, String message) {
        try {
            m_notificationManager =
                (NotificationManager) context.getSystemService(
                    Context.NOTIFICATION_SERVICE
                );

            if (
                android.os.Build.VERSION.SDK_INT >=
                android.os.Build.VERSION_CODES.O
            ) {
                int importance = NotificationManager.IMPORTANCE_LOW;
                NotificationChannel notificationChannel =
                    new NotificationChannel(
                        "Knot",
                        "Knot Notifier Steps",
                        importance
                    );
                m_notificationManager.createNotificationChannel(
                    notificationChannel
                );

                m_builder = new Notification.Builder(
                    context,
                    notificationChannel.getId()
                );
            } else {
                m_builder = new Notification.Builder(context);
            }

            m_builder
                .setContentTitle(strPedometer)
                .setContentText(message)
                .setSmallIcon(R.drawable.icon)
                .setColor(Color.GREEN)
                .setAutoCancel(true);

            m_notificationManager.notify(0, m_builder.build());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // ------------------------------------------------------------------------------

    public static void notifyTodoAlarm(Context context, String message) {
        try {
            NotificationManager m_notificationManagerAlarm =
                (NotificationManager) context.getSystemService(
                    Context.NOTIFICATION_SERVICE
                );
            Notification.Builder m_builderAlarm = null;

            String channelId = "knot_alarm_channel";
            // 通道配置（保留你原来的呼吸灯逻辑，仅静音）
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                NotificationChannel channel =
                    m_notificationManagerAlarm.getNotificationChannel(
                        channelId
                    );
                if (channel == null) {
                    channel = new NotificationChannel(
                        channelId,
                        "Knot Alarm",
                        NotificationManager.IMPORTANCE_HIGH
                    );
                    channel.enableLights(true);
                    channel.setLightColor(Color.RED);
                    channel.enableVibration(true);
                    channel.setVibrationPattern(new long[] { 0, 500, 1000 });
                    channel.setSound(null, null); // 渠道静音
                    channel.setBypassDnd(true); // 不是强需求闹钟，建议关掉
                    channel.setLockscreenVisibility(
                        Notification.VISIBILITY_PUBLIC
                    );
                    m_notificationManagerAlarm.createNotificationChannel(
                        channel
                    );
                }
                channel.enableLights(true);
                channel.setLightColor(Color.RED);
                m_notificationManagerAlarm.createNotificationChannel(channel);
            }

            // 保留原来的Intent/PendingIntent逻辑
            Intent activityIntent = new Intent(context, MyActivity.class);
            activityIntent.setFlags(
                Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP
            );
            activityIntent.putExtra("ALARM_MESSAGE", message);

            int flags = PendingIntent.FLAG_UPDATE_CURRENT;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                flags |= PendingIntent.FLAG_IMMUTABLE;
            }
            PendingIntent pendingIntent = PendingIntent.getActivity(
                context,
                (int) System.currentTimeMillis(),
                activityIntent,
                flags
            );

            // 构建通知（保留呼吸灯，静音系统声音）
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                m_builderAlarm = new Notification.Builder(context, channelId)
                    .setContentTitle(strTodo)
                    .setContentText(message)
                    .setSmallIcon(R.drawable.icon)
                    .setColor(Color.GREEN)
                    .setAutoCancel(true)
                    .setContentIntent(pendingIntent)
                    .setLights(Color.RED, 1000, 1000)
                    .setVibrate(new long[] { 0, 500, 1000 })
                    .setPriority(Notification.PRIORITY_MAX)
                    .setDefaults(
                        Notification.DEFAULT_VIBRATE |
                            Notification.DEFAULT_LIGHTS
                    );
            } else {
                m_builderAlarm = new Notification.Builder(context)
                    .setContentTitle(strTodo)
                    .setContentText(message)
                    .setSmallIcon(R.drawable.icon)
                    .setColor(Color.GREEN)
                    .setAutoCancel(true)
                    .setContentIntent(pendingIntent)
                    .setLights(Color.RED, 1000, 1000)
                    .setVibrate(new long[] { 0, 500, 1000 })
                    .setPriority(Notification.PRIORITY_MAX)
                    .setDefaults(
                        Notification.DEFAULT_VIBRATE |
                            Notification.DEFAULT_LIGHTS
                    );
            }

            // 发送通知
            m_notificationManagerAlarm.cancel("knot_alarm_tag", 10);
            int randomId = (int) (System.currentTimeMillis() % 10000);
            m_notificationManagerAlarm.notify(
                "knot_alarm_tag",
                randomId,
                m_builderAlarm.build()
            );

            // ========== 使用现成的锁屏判断方法 ==========
            boolean isMIUI = isMIUI(); // 判断是否是小米/MIUI系统
            boolean isLockScreen = MyActivity.getLockScreenStatus();

            // 播放规则：
            // 1. 非MIUI机型：无论锁屏/亮屏，都手动播放（系统通知已静音，不会双声）
            // 2. MIUI机型：锁屏时手动播放，亮屏时跳过（系统通知会自动播，避免双声）
            // 检测音频服务是否存活，异常则跳过通知音播放
            if (!isAudioServiceAlive(context)) {
                Log.w(TAG, "音频服务已崩溃，跳过通知音播放");
                // 仅显示通知，不播放声音，保证后续逻辑执行
                new Handler(Looper.getMainLooper()).post(() -> {
                    Toast.makeText(
                        context,
                        MyActivity.zh_cn
                            ? "系统音频异常，通知音暂无法播放"
                            : "System audio error, notification sound unavailable",
                        Toast.LENGTH_SHORT
                    ).show();
                });
            } else {
                if (!isMIUI || (isMIUI && isLockScreen)) {
                    playLockScreenSound(context);
                }
            }
            // ======================================================

            Log.d(
                TAG,
                "通知发送成功（ID：" +
                    randomId +
                    "），MIUI:" +
                    isMIUI +
                    "，锁屏:" +
                    isLockScreen
            );
        } catch (Exception e) {
            Log.e(TAG, "发送通知失败", e);
            e.printStackTrace();
        }
    }

    // 判断是否是MIUI系统（精准识别小米/红米）
    private static boolean isMIUI() {
        try {
            Class<?> clazz = Class.forName("android.os.SystemProperties");
            java.lang.reflect.Method get = clazz.getMethod("get", String.class);
            String miui = (String) get.invoke(null, "ro.miui.ui.version.name");
            return miui != null && !miui.isEmpty();
        } catch (Exception e) {
            return false;
        }
    }

    // 检测音频服务是否存活
    private static boolean isAudioServiceAlive(Context context) {
        try {
            AudioManager audioManager = (AudioManager) context.getSystemService(
                Context.AUDIO_SERVICE
            );
            // 尝试获取音量，能获取则服务正常
            audioManager.getStreamVolume(AudioManager.STREAM_NOTIFICATION);
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    private static void playLockScreenSound(Context context) {
        try {
            Uri soundUri = null;
            String channelId = "knot_alarm_channel"; // 和通知的通道ID保持一致

            // 1. Android 8.0+：优先读取用户为当前通知通道设置的声音
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                NotificationManager nm =
                    (NotificationManager) context.getSystemService(
                        Context.NOTIFICATION_SERVICE
                    );
                NotificationChannel channel = nm.getNotificationChannel(
                    channelId
                );
                if (channel != null) {
                    soundUri = channel.getSound(); // 读取系统设置里该通道的声音
                }
            }

            // 2. 兜底逻辑（8.0以下/通道声音为空时）：读取系统通知设置里的默认音
            if (soundUri == null || soundUri.toString().isEmpty()) {
                // 先读系统为当前App设置的通知音
                soundUri = RingtoneManager.getActualDefaultRingtoneUri(
                    context,
                    RingtoneManager.TYPE_NOTIFICATION
                );
                // 仍为空则用闹钟音兜底
                if (soundUri == null || soundUri.toString().isEmpty()) {
                    soundUri = RingtoneManager.getDefaultUri(
                        RingtoneManager.TYPE_ALARM
                    );
                }
            }

            // 3. 初始化MediaPlayer（闹钟流，锁屏也能播放）
            MediaPlayer mediaPlayer = new MediaPlayer();
            mediaPlayer.setDataSource(context, soundUri);

            // USAGE_ALARM + 通知通道配置，似乎容易和系统音频抢占冲突？
            AudioAttributes audioAttributes = new AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_ALARM)
                //.setUsage(AudioAttributes.USAGE_NOTIFICATION_EVENT)
                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                .build();
            mediaPlayer.setAudioAttributes(audioAttributes);

            // 手动设置音量（最大音量的80%，可调整）
            mediaPlayer.setVolume(1.0f, 1.0f); // 左右声道都设为80%

            mediaPlayer.setLooping(false);
            mediaPlayer.prepareAsync();

            // 4. 播放完成后释放资源
            mediaPlayer.setOnPreparedListener(mp -> mp.start());
            mediaPlayer.setOnCompletionListener(mp -> mp.release());
            mediaPlayer.setOnErrorListener((mp, what, extra) -> {
                mp.release();
                return false;
            });
        } catch (Exception e) {
            Log.e(TAG, "播放锁屏提示音失败", e);
        }
    }

    // 清除所有相关通知（包括新改造的通知）
    public static void clearNotify(Context context) {
        try {
            NotificationManager notificationManager =
                (NotificationManager) context.getSystemService(
                    Context.NOTIFICATION_SERVICE
                );
            if (notificationManager != null) {
                // 1. 清除旧通知（knot_alarm_tag + 10）
                notificationManager.cancel("knot_alarm_tag", 10);
                // 2. 清除前台服务通知（1337，复用channel_1的通知）
                notificationManager.cancel(1337);
                // 3. 兜底：清除该通道下所有通知（防止标签/ID不匹配）
                notificationManager.cancelAll(); // 可选：谨慎使用，会清除app所有通知
                Log.d(TAG, "所有相关通知已清除（包括新/旧通知）");
            }
        } catch (Exception e) {
            Log.e(TAG, "清除通知失败", e);
        }
    }

    /////////////////////// Steps Sensor /////////////////////////////////////
    class PersistService implements SensorEventListener {

        private final Context mContext; // 持有Service上下文，避免内存泄漏

        // 构造方法传入上下文
        public PersistService(Context context) {
            this.mContext = context;
        }

        public BroadcastReceiver mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if (mSensorManager != null) {
                    mSensorManager.unregisterListener(PersistService.this);
                    mSensorManager.registerListener(
                        PersistService.this,
                        mSensorManager.getDefaultSensor(
                            Sensor.TYPE_STEP_COUNTER
                        ),
                        SensorManager.SENSOR_DELAY_NORMAL
                    );
                }
            }
        };

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            Log.i(TAG, "PersistService.onAccuracyChanged().");
        }

        @Override
        public void onSensorChanged(SensorEvent sensorEvent) {
            if (sensorEvent.sensor.getType() == Sensor.TYPE_STEP_COUNTER) {
                stepCounts = (long) sensorEvent.values[0];
            }
        }
    }

    private static Sensor countSensor;
    public static int isStepCounter = -1;
    public static float stepCounts;
    private static SensorManager mSensorManager;

    // 仅新增context参数，其余逻辑不变
    public static void initStepSensor(Context context) {
        if (context == null) {
            Log.e(TAG, "initStepSensor: 上下文为空，初始化失败");
            isStepCounter = 0;
            return;
        }

        // 1. 权限校验（原有逻辑）
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            if (
                ContextCompat.checkSelfPermission(
                    context,
                    Manifest.permission.ACTIVITY_RECOGNITION
                ) !=
                PackageManager.PERMISSION_GRANTED
            ) {
                Log.w(
                    TAG,
                    "initStepSensor: 无ACTIVITY_RECOGNITION权限，跳过传感器注册"
                );
                isStepCounter = 0;
                return;
            }
        }

        // 2. 补充空判 + 重新获取传感器（原有逻辑）
        if (mSensorManager == null) {
            mSensorManager = (SensorManager) context.getSystemService(
                Context.SENSOR_SERVICE
            );
        }
        if (countSensor == null && mSensorManager != null) {
            countSensor = mSensorManager.getDefaultSensor(
                Sensor.TYPE_STEP_COUNTER
            );
        }

        // 3. 原有逻辑（静态变量引用）
        if (
            countSensor != null &&
            mSensorManager != null &&
            mySensorSerivece != null
        ) {
            mSensorManager.unregisterListener(mySensorSerivece);
            mSensorManager.registerListener(
                mySensorSerivece,
                countSensor,
                SensorManager.SENSOR_DELAY_NORMAL
            );
            isStepCounter = 1;
            Log.i(TAG, "initStepSensor: 步数传感器注册成功（权限已授予）");
        } else {
            isStepCounter = 0;
            Log.w(TAG, "initStepSensor: 传感器/管理器/服务为空，注册失败");
        }
    }

    public static int getHardStepCounter() {
        return isStepCounter;
    }

    public static float getSteps() {
        return stepCounts;
    }

    ////////////////// Todo Alarm ////// ////////////////////////////////

    public static final String ACTION_TODO_ALARM = "com.x.Knot.TODO_ALARM";
    private static AlarmManager alarmManager;

    private static PendingIntent pendingIntentAlarm = null;

    public static int startAlarm(String str) {
        String[] array = str.split("\\|");
        for (int i = 0; i < array.length; i++) System.out.println(array[i]);

        String strTime = array[0];
        String strText = array[1];
        String strTotalS = array[2];

        Calendar c = Calendar.getInstance();
        c.setTimeInMillis(System.currentTimeMillis());

        int ts = Integer.parseInt(strTotalS);
        c.add(Calendar.SECOND, ts);

        // 使用应用上下文，避免Activity引用导致的内存泄漏
        Context appContext = MyActivity.getMyAppContext();

        int flags = PendingIntent.FLAG_UPDATE_CURRENT;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            flags |= PendingIntent.FLAG_IMMUTABLE;
        }

        // 创建定时触发的 BroadcastReceiver Intent
        Intent receiverIntent = new Intent(ACTION_TODO_ALARM);
        receiverIntent.setPackage(appContext.getPackageName()); // 限定本应用，避免广播泄漏
        receiverIntent.putExtra("alarmMessage", strText);

        // 唯一请求码（不考虑 PendingIntent 复用，业务逻辑：新定时覆盖旧定时）
        pendingIntentAlarm = PendingIntent.getBroadcast(
            appContext,
            ALARM_REQUEST_CODE,
            receiverIntent,
            flags
        );

        // 设置闹钟
        alarmManager.setExactAndAllowWhileIdle(
            AlarmManager.RTC_WAKEUP,
            c.getTimeInMillis(),
            pendingIntentAlarm
        );

        Log.e("Alarm Manager", c.getTimeInMillis() + "");
        Log.e("Alarm Manager", str);

        System.out.println(ts);
        System.out.println("startAlarm+++++++++++++++++++++++");

        return 1;
    }

    public static int stopAlarm() {
        if (alarmManager != null && pendingIntentAlarm != null) {
            alarmManager.cancel(pendingIntentAlarm);
            pendingIntentAlarm.cancel();
            pendingIntentAlarm = null;
        }
        return 1;
    }

    // 请求精确闹钟权限
    private void requestExactAlarmPermission(Context context) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S) {
            return;
        }
        Intent intent = new Intent(
            Settings.ACTION_REQUEST_SCHEDULE_EXACT_ALARM
        );
        intent.setData(Uri.parse("package:" + context.getPackageName()));
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK); // Service启动Activity必须加此标志
        context.startActivity(intent);
    }

    public static int startPreciseAlarm(String str) {
        String[] array = str.split("\\|");
        for (int i = 0; i < array.length; i++) System.out.println(array[i]);

        String strTime = array[0];
        String strText = array[1];
        String strTotalS = array[2];

        Calendar c = Calendar.getInstance();
        c.setTimeInMillis(System.currentTimeMillis());

        int ts = Integer.parseInt(strTotalS);
        c.add(Calendar.SECOND, ts);

        // 使用应用上下文，避免Activity引用导致的内存泄漏
        Context appContext = MyActivity.getMyAppContext();

        // 1. 使用 setAlarmClock() 提高优先级
        Intent showIntent = new Intent(appContext, MyActivity.class);
        PendingIntent showPending = PendingIntent.getActivity(
            appContext,
            0,
            showIntent,
            PendingIntent.FLAG_IMMUTABLE
        );

        AlarmManager.AlarmClockInfo clockInfo = new AlarmManager.AlarmClockInfo(
            c.getTimeInMillis(),
            showPending
        );

        // 2. 创建精确触发的广播Intent
        Intent receiverIntent = new Intent(ACTION_TODO_ALARM);
        receiverIntent.setPackage(appContext.getPackageName()); // 限定本应用，避免广播泄漏
        receiverIntent.putExtra("alarmMessage", strText);

        pendingIntentAlarm = PendingIntent.getBroadcast(
            appContext,
            ALARM_REQUEST_CODE,
            receiverIntent,
            PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
        );

        // 3. 设置闹钟
        alarmManager.setAlarmClock(clockInfo, pendingIntentAlarm);

        return 1;
    }

    public static int startPreciseAlarmInMyService(String str) {
        strTodoAlarm = str;
        String[] array = str.split("\\|");
        String strText = array[1];
        String strTotalS = array[2];

        Calendar c = Calendar.getInstance();
        c.add(Calendar.SECOND, Integer.parseInt(strTotalS));

        Context appContext = MyActivity.getMyAppContext();

        // 1：创建广播 Intent（不是服务 Intent）
        Intent alarmIntent = new Intent(ACTION_TODO_ALARM);
        alarmIntent.setPackage(appContext.getPackageName()); // 关键：设置包名
        alarmIntent.putExtra("alarmMessage", strText);

        // 2：使用 PendingIntent.getBroadcast
        pendingIntentAlarm = PendingIntent.getBroadcast(
            appContext,
            ALARM_REQUEST_CODE,
            alarmIntent,
            PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
        );

        // 使用系统级高优先级API
        Intent showIntent = new Intent(appContext, MyActivity.class);
        PendingIntent showPending = PendingIntent.getActivity(
            appContext,
            0,
            showIntent,
            PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
        );

        AlarmManager.AlarmClockInfo clockInfo = new AlarmManager.AlarmClockInfo(
            c.getTimeInMillis(),
            showPending
        );

        alarmManager.setAlarmClock(clockInfo, pendingIntentAlarm);

        // 添加详细日志
        SimpleDateFormat sdf = new SimpleDateFormat(
            "yyyy-MM-dd HH:mm:ss",
            Locale.getDefault()
        );
        Log.d(
            "AlarmManager",
            "闹钟设置成功: " + sdf.format(c.getTime()) + " | 消息: " + strText
        );

        return 1;
    }

    // 自定义闹钟接收器（作为内部成员）
    private final BroadcastReceiver myalarmReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (ACTION_TODO_ALARM.equals(intent.getAction())) {
                String message = intent.getStringExtra("alarmMessage");
                Log.d("MyAlarmReceiver", "Alarm received: " + message);

                notifyTodoAlarm(context, message);

                // 切换到Android主线程调用C++方法（避免广播子线程）
                new Handler(Looper.getMainLooper()).post(() -> {
                    if (QtStateManager.getInstance().canInteractWithQt()) {
                        CallJavaNotify_3();
                    } else {
                        new Handler(Looper.getMainLooper()).postDelayed(
                            () -> {
                                Log.w(
                                    TAG,
                                    "Qt未就绪，延迟3秒执行CallJavaNotify_3"
                                );
                                CallJavaNotify_3();
                            },
                            3000
                        );
                    }
                });
            }
        }
    };

    public boolean startGPS() {
        MyActivity.setVibrate();

        if (isGpsRunning) {
            Log.w(TAG, "GPS已在运行，无需重复启动");
            showToast(
                MyActivity.zh_cn ? "GPS已在运行中" : "GPS is already running"
            );
            return true;
        }

        // 检查定位权限
        Context context = getApplicationContext();
        String finePerm = Manifest.permission.ACCESS_FINE_LOCATION;
        String coarsePerm = Manifest.permission.ACCESS_COARSE_LOCATION;
        // 新增：适配Android 12+后台定位权限
        String bgPerm = Manifest.permission.ACCESS_BACKGROUND_LOCATION;
        boolean hasFine =
            ContextCompat.checkSelfPermission(context, finePerm) ==
            PackageManager.PERMISSION_GRANTED;
        boolean hasCoarse =
            ContextCompat.checkSelfPermission(context, coarsePerm) ==
            PackageManager.PERMISSION_GRANTED;
        boolean hasBg =
            Build.VERSION.SDK_INT < Build.VERSION_CODES.S ||
            ContextCompat.checkSelfPermission(context, bgPerm) ==
            PackageManager.PERMISSION_GRANTED;

        // 权限判断：前台定位需要fine/coarse，后台定位需要bg（12+）
        if (!hasFine || !hasBg) {
            Log.e(TAG, "缺少定位权限，跳转到Activity申请");
            try {
                Intent intent = new Intent(context, MyActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                intent.putExtra("REQUEST_LOCATION_PERMISSION", true);
                intent.putExtra("START_GPS_AFTER_PERMISSION", true);
                // 新增：传递后台定位权限标记
                intent.putExtra(
                    "REQUEST_BACKGROUND_LOCATION",
                    Build.VERSION.SDK_INT >= Build.VERSION_CODES.S
                );
                context.startActivity(intent);
            } catch (Exception e) {
                Log.e(TAG, "跳转到Activity申请权限失败", e);
                showToast(
                    MyActivity.zh_cn
                        ? "缺少定位权限，GPS启动失败"
                        : "Missing location permission, GPS startup failed"
                );
            }
            return false;
        }

        // 有权限，正常启动GPS
        boolean success = gpsManager.startGPS(
            new GPSManager.OnLocationUpdateListener() {
                @Override
                public void onLocationUpdated(
                    double lat,
                    double lng,
                    float speed,
                    float distance
                ) {
                    Log.d(
                        TAG,
                        "GPS更新：纬度=" +
                            lat +
                            " 经度=" +
                            lng +
                            " 速度=" +
                            speed +
                            " 距离=" +
                            distance
                    );
                }

                @Override
                public void onGPSStatusChanged(String status) {
                    Log.d(TAG, "GPS状态：" + status);
                }
            }
        );

        if (success) {
            isGpsRunning = true;
            Log.i(TAG, "GPS启动成功（前台服务托管）");
            showToast(
                MyActivity.zh_cn ? "GPS启动成功" : "GPS started successfully"
            );
        } else {
            Log.e(TAG, "GPS启动失败");
            showToast(
                MyActivity.zh_cn
                    ? "GPS启动失败，请检查定位服务是否开启"
                    : "GPS startup failed, check if location service is enabled"
            );
        }

        return success;
    }

    // 新增：极简的吐司提示方法（适配中英文，运行在主线程）
    private void showToast(String message) {
        // 确保吐司在主线程显示（服务中调用需切换线程）
        new Handler(Looper.getMainLooper()).post(() -> {
            Toast.makeText(
                getApplicationContext(),
                message,
                Toast.LENGTH_SHORT
            ).show();
        });
    }

    // ========== 停止GPS方法（供C++调用） ==========
    public void stopGPS() {
        MyActivity.setVibrate();

        if (!isGpsRunning) {
            return;
        }

        gpsManager.stopGPS();
        isGpsRunning = false;
        Log.i(TAG, "GPS已停止（前台服务托管）");
    }

    public static MyService getInstance() {
        return instance;
    }

    public static double getLatitude() {
        MyService service = MyService.getInstance();
        if (service == null || service.gpsManager == null) {
            Log.w("MyActivity", "服务未启动或gpsManager未初始化，返回0");
            return Double.NaN;
        }

        return service.gpsManager.getLatitude();
    }

    public static double getLongitude() {
        MyService service = MyService.getInstance();
        if (service == null || service.gpsManager == null) {
            Log.w("MyActivity", "服务未启动或gpsManager未初始化，返回0");
            return Double.NaN;
        }

        return service.gpsManager.getLongitude();
    }

    public static double getMySpeed() {
        // 检查服务实例和GPSManager是否有效
        if (instance == null || instance.gpsManager == null) {
            Log.w(
                "MyService",
                "服务未启动或GPSManager未初始化，无法获取当前速度"
            );
            return Double.NaN; // 返回NaN，明确表示获取失败
        }
        // 调用gpsManager的getMySpeed()方法
        return instance.gpsManager.getMySpeed();
    }

    public static double getMaxSpeed() {
        if (instance == null || instance.gpsManager == null) {
            Log.w(
                "MyService",
                "服务未启动或GPSManager未初始化，无法获取最大速度"
            );
            return Double.NaN;
        }
        // 调用gpsManager的getMaxSpeed()方法
        return instance.gpsManager.getMaxSpeed();
    }

    private void updateGpsData() {
        // 1. 基础状态更新（优先标记GPS运行状态）
        if (isGpsRunning) {
            strGpsStatus = MyActivity.zh_cn
                ? "GPS正在运行中..."
                : "GPS is running...";
        } else {
            strGpsStatus = MyActivity.zh_cn ? "GPS已停止" : "GPS stopped";
            // GPS未运行时，重置所有数据为默认值，避免显示旧数据
            // resetGpsDataToDefault();
            return;
        }

        // 2. 空指针防护：检查GPSManager是否有效
        if (gpsManager == null) {
            Log.w(TAG, "updateGpsData: GPSManager未初始化，使用默认值");
            resetGpsDataToDefault();
            return;
        }

        // 3. 直接从GPSManager获取所有核心数据（单一数据源，无冗余）
        // 3.1 距离相关（GPSManager返回的是公里，无需额外转换）
        float totalDistance = gpsManager.getTotalDistance();
        strTotalDistance = String.format(Locale.US, "%.2f km", totalDistance);

        // 3.2 运动时间（毫秒转时分秒）
        long movingTimeMs = gpsManager.getMovingTime();
        long totalSeconds = movingTimeMs / 1000;
        long hours = totalSeconds / 3600;
        long minutes = (totalSeconds % 3600) / 60;
        long seconds = totalSeconds % 60;
        strRunTime = String.format(
            Locale.US,
            "%02d:%02d:%02d",
            hours,
            minutes,
            seconds
        );

        // 3.3 平均速度（避免除以0，单位：km/h）
        double avgSpeed = 0.0;
        if (movingTimeMs > 0) {
            avgSpeed = totalDistance / (movingTimeMs / 3600000.0); // 毫秒转小时
        }
        strAverageSpeed = String.format(Locale.US, "%.2f km/h", avgSpeed);

        // 3.4 最大速度（直接从GPSManager获取，单位：km/h）
        float maxSpeed = gpsManager.getMaxSpeed();
        strMaxSpeed = MyActivity.zh_cn
            ? String.format(Locale.US, "最大速度: %.2f km/h", maxSpeed)
            : String.format(Locale.US, "Max Speed: %.2f km/h", maxSpeed);

        // 3.5 海拔：直接从GPSManager获取previousAltitude（最新海拔）
        double altitude = gpsManager.getPreviousAltitude();
        strAltitude = MyActivity.zh_cn
            ? String.format(Locale.US, "海拔: %.2f m", altitude)
            : String.format(Locale.US, "Altitude: %.2f m", altitude);

        // 3.6 累计爬升/下降（直接从GPSManager获取，单位：米）
        float totalClimb = gpsManager.getTotalClimb();
        strTotalClimb = MyActivity.zh_cn
            ? String.format(Locale.US, "累计爬升: %.2f m", totalClimb)
            : String.format(Locale.US, "Total Climb: %.2f m", totalClimb);

        float totalDescent = gpsManager.getTotalDescent();
        strTotalDescent = MyActivity.zh_cn
            ? String.format(Locale.US, "累计下降: %.2f m", totalDescent)
            : String.format(Locale.US, "Total Descent: %.2f m", totalDescent);

        // 3.7 精细化GPS状态（基于GPSManager的就绪状态）
        strGpsStatus = gpsManager.isGpsReady()
            ? (MyActivity.zh_cn
                  ? "GPS已就绪（高精度）"
                  : "GPS Ready (High Precision)")
            : (MyActivity.zh_cn
                  ? "GPS未就绪（网络定位）"
                  : "GPS Not Ready (Network Location)");

        // 日志输出（便于调试）
        Log.d(
            TAG,
            String.format(
                Locale.US,
                "updateGpsData完成：距离=%.2fkm | 平均速度=%.2fkm/h | 最大速度=%.2fkm/h | 海拔=%.2fm",
                totalDistance,
                avgSpeed,
                maxSpeed,
                altitude
            )
        );
    }

    // ========== 辅助方法：重置GPS数据为默认值 ==========
    private void resetGpsDataToDefault() {
        strTotalDistance = MyActivity.zh_cn ? "0.00 公里" : "0.00 km";
        strRunTime = "00:00:00";
        strAverageSpeed = MyActivity.zh_cn ? "0.00 公里/小时" : "0.00 km/h";
        strMaxSpeed = MyActivity.zh_cn
            ? "最大速度: 0.00 公里/小时"
            : "Max Speed: 0.00 km/h";
        strAltitude = MyActivity.zh_cn ? "海拔: 0.00 米" : "Altitude: 0.00 m";
        strTotalClimb = MyActivity.zh_cn
            ? "累计爬升: 0.00 米"
            : "Total Climb: 0.00 m";
        strTotalDescent = MyActivity.zh_cn
            ? "累计下降: 0.00 米"
            : "Total Descent: 0.00 m";
    }

    public String getGpsStatus() {
        updateGpsData();
        return (
            strTotalDistance +
            "\n" +
            strRunTime +
            "\n" +
            strAverageSpeed +
            "\n" +
            strMaxSpeed +
            "\n" +
            strAltitude +
            "\n" +
            strTotalClimb +
            "\n" +
            strTotalDescent +
            "\n" +
            strGpsStatus
        );
    }

    /**
     * 服务内播放TTS文本（对外暴露的核心方法）
     */
    public void playMyTextInService(String text) {
        if (TextUtils.isEmpty(text)) {
            Log.w("TTS", "播放文本为空，跳过");
            return;
        }

        // 检查TTS引擎是否安装
        checkTTSEngine();

        synchronized (ttsLock) {
            if (isTtsInitializing) {
                pendingTtsText = text;
                Log.d("TTS", "TTS正在初始化，缓存待播放文本：" + text);
                return;
            }

            isTtsInitializing = true;
            pendingTtsText = text;

            // 新建TTSUtils实例（使用服务上下文）
            TTSUtils newTts = new TTSUtils(getApplicationContext());
            currentPlayingTts = newTts;

            newTts.initialize(
                new TTSUtils.InitCallback() {
                    @Override
                    public void onSuccess() {
                        synchronized (ttsLock) {
                            isTtsInitializing = false;
                            isTtsInitialized = true;
                        }

                        // 设置播放完成监听
                        newTts.setOnPlayCompleteListener(
                            new TTSUtils.OnPlayCompleteListener() {
                                @Override
                                public void onPlayComplete() {
                                    Log.d("TTS", "服务内TTS播放完成！");
                                    // 播放完成后释放资源
                                    newTts.shutdown();
                                    synchronized (ttsLock) {
                                        if (currentPlayingTts == newTts) {
                                            currentPlayingTts = null;
                                        }
                                    }
                                }

                                @Override
                                public void onPlayStopped() {
                                    Log.d("TTS", "服务内TTS播放被停止！");
                                    newTts.shutdown();
                                    synchronized (ttsLock) {
                                        if (currentPlayingTts == newTts) {
                                            currentPlayingTts = null;
                                        }
                                    }
                                }
                            }
                        );

                        // 播放缓存文本
                        String playText = null;
                        synchronized (ttsLock) {
                            playText = pendingTtsText;
                            pendingTtsText = null;
                        }
                        if (!TextUtils.isEmpty(playText)) {
                            newTts.speak(playText);
                            Log.d("TTS", "服务内播放文本：" + playText);
                        }
                    }

                    @Override
                    public void onError(String error) {
                        Log.e("TTS", "服务内TTS初始化失败：" + error);
                        synchronized (ttsLock) {
                            isTtsInitializing = false;
                            pendingTtsText = null;
                            if (currentPlayingTts == newTts) {
                                currentPlayingTts = null;
                            }
                        }
                        newTts.shutdown();
                    }
                }
            );
        }
    }

    /**
     * 停止服务内的TTS播放
     */
    public void stopPlayMyTextInService() {
        synchronized (ttsLock) {
            isTtsInitializing = false;
            pendingTtsText = null;

            if (currentPlayingTts != null) {
                currentPlayingTts.stop();
                currentPlayingTts.shutdown();
                currentPlayingTts = null;
                Log.d("TTS", "服务内已停止并释放TTS实例");
            }
        }
    }

    /**
     * 检查系统TTS引擎是否安装，未安装则引导用户安装
     */
    private void checkTTSEngine() {
        Intent checkIntent = new Intent(
            TextToSpeech.Engine.ACTION_CHECK_TTS_DATA
        );
        ComponentName comp = checkIntent.resolveActivity(getPackageManager());
        if (comp == null) {
            Log.w("TTS", "系统未安装TTS引擎，引导用户安装");
            Intent installIntent = new Intent(
                TextToSpeech.Engine.ACTION_INSTALL_TTS_DATA
            );
            installIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            try {
                startActivity(installIntent);
            } catch (Exception e) {
                Log.e("TTS", "跳转TTS引擎安装页面失败", e);
                String tipText = MyActivity.zh_cn
                    ? "请手动安装TTS语音引擎"
                    : "Please install the TTS voice engine manually";
                new Handler(Looper.getMainLooper()).post(() -> {
                    Toast.makeText(
                        getApplicationContext(),
                        tipText,
                        Toast.LENGTH_LONG
                    ).show();
                });
            }
        }
    }

    /**
     * 对外提供的静态调用方法（供Activity/其他组件调用）
     */
    public static void playText(String text) {
        MyService service = getInstance();
        if (service != null) {
            service.playMyTextInService(text);
        } else {
            Log.e("TTS", "MyService未启动，无法播放TTS");
        }
    }

    /**
     * 对外提供的静态停止方法
     */
    public static void stopTextPlay() {
        MyService service = getInstance();
        if (service != null) {
            service.stopPlayMyTextInService();
        }
    }

    // ========== 新增：将Activity的前台唤醒逻辑迁移到Service ==========
    /**
     * 唤醒屏幕并让应用显示在锁屏上层（Service中适配）
     */
    private void wakeUpScreen() {
        // 1. 获取PowerManager并唤醒屏幕
        PowerManager pm = (PowerManager) getSystemService(
            Context.POWER_SERVICE
        );
        if (pm != null) {
            int wakeLockFlags =
                PowerManager.SCREEN_BRIGHT_WAKE_LOCK |
                PowerManager.ACQUIRE_CAUSES_WAKEUP |
                PowerManager.ON_AFTER_RELEASE;
            PowerManager.WakeLock wakeLock = pm.newWakeLock(
                wakeLockFlags,
                "MyApp:WakeLockTag"
            );
            if (!wakeLock.isHeld()) {
                wakeLock.acquire(10000); // 持有10秒
            }
        }

        // 2. 适配Service中无法直接操作Window的问题，通过启动Activity来设置Window Flag
        Intent intent = new Intent(this, MyActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            // Android 9.0+ 新API
            PendingIntent pendingIntent = PendingIntent.getActivity(
                this,
                0,
                intent,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
            );
            try {
                pendingIntent.send();
            } catch (PendingIntent.CanceledException e) {
                Log.e(TAG, "唤醒屏幕PendingIntent发送失败", e);
            }
        } else {
            startActivity(intent);
        }
    }

    /**
     * 将应用移到前台（Service中实现）
     */
    private void bringToFront() {
        try {
            Intent intent = new Intent(this, MyActivity.class);
            // 核心Flag：保证复用已有实例，不重复创建
            intent.addFlags(
                Intent.FLAG_ACTIVITY_REORDER_TO_FRONT |
                    Intent.FLAG_ACTIVITY_BROUGHT_TO_FRONT |
                    Intent.FLAG_ACTIVITY_SINGLE_TOP |
                    Intent.FLAG_ACTIVITY_NO_USER_ACTION |
                    Intent.FLAG_ACTIVITY_NEW_TASK
            ); // Service启动Activity必须加NEW_TASK

            startActivity(intent);
            Log.d(TAG, "Service中通过Intent唤醒应用前台成功");

            // 延迟激活窗口
            new Handler(Looper.getMainLooper()).postDelayed(
                () -> {
                    // 尝试获取Activity实例并激活窗口
                    if (
                        MyActivity.m_instance != null &&
                        !MyActivity.m_instance.isFinishing() &&
                        !MyActivity.m_instance.isDestroyed()
                    ) {
                        Window window = MyActivity.m_instance.getWindow();
                        if (window != null) {
                            window.addFlags(
                                WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED
                            );
                            window.getDecorView().requestFocus();
                            window.getDecorView().postInvalidate();
                        }
                    }
                },
                200
            );
        } catch (Exception e) {
            Log.e(TAG, "Service中移应用到前台失败", e);
        }
    }

    /**
     * Service中核心方法：将应用带到前台
     */
    public void bringAppToForeground() {
        try {
            //wakeUpScreen();
            bringToFront();

            // 强制激活窗口 + 清除输入法焦点
            new Handler(Looper.getMainLooper()).post(() -> {
                if (
                    MyActivity.m_instance != null &&
                    !MyActivity.m_instance.isFinishing() &&
                    !MyActivity.m_instance.isDestroyed()
                ) {
                    hideSoftInput();
                    forceDisconnectInputMethod();
                    // 强制刷新UI
                    MyActivity.m_instance
                        .getWindow()
                        .getDecorView()
                        .postInvalidate();
                }
            });
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 隐藏软键盘（适配Service）
     */
    private void hideSoftInput() {
        if (MyActivity.m_instance == null) {
            Log.e(TAG, "hideSoftInput失败：MyActivity实例为空");
            return;
        }

        InputMethodManager imm = (InputMethodManager) getSystemService(
            Context.INPUT_METHOD_SERVICE
        );
        if (imm == null) return;

        View currentFocus = MyActivity.m_instance.getCurrentFocus();
        if (currentFocus != null) {
            imm.hideSoftInputFromWindow(
                currentFocus.getWindowToken(),
                InputMethodManager.HIDE_NOT_ALWAYS
            );
        }
    }

    /**
     * 强制断开输入法连接（适配Service）
     */
    // 正确：安卓官方的“断开输入法+隐藏键盘”方法
    // 优化：去掉Context参数，直接用MyService的上下文
    public static void forceDisconnectInputMethod() {
        try {
            // 直接用MyService的上下文（this.getApplicationContext()）
            Context context = instance.getApplicationContext();
            InputMethodManager imm =
                (InputMethodManager) context.getSystemService(
                    Context.INPUT_METHOD_SERVICE
                );

            // 隐藏软键盘+清除焦点
            if (context instanceof Activity) {
                // 兼容Activity/Service场景
                View focusView = ((Activity) context).getCurrentFocus();
                if (focusView != null) {
                    imm.hideSoftInputFromWindow(focusView.getWindowToken(), 0);
                    focusView.clearFocus();
                }
            }

            // 兜底关闭输入法
            imm.toggleSoftInput(InputMethodManager.HIDE_IMPLICIT_ONLY, 0);
        } catch (Exception e) {
            Log.e("MyService", "强制断开输入法失败", e);
        }
    }

    /**
     * 对外提供的静态方法：从任意地方调用，将应用带到前台
     */
    public static void bringAppToForegroundFromService() {
        MyService service = getInstance();
        if (service != null) {
            service.bringAppToForeground();
        } else {
            Log.e(TAG, "MyService未启动，无法执行bringAppToForeground");
        }
    }
    // ========== 前台唤醒逻辑迁移结束 ==========
}
