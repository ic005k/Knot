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
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.provider.Settings;
import android.text.TextUtils; // ✅ 正确的导入位置
import android.util.Log;
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

    // ========== 新增：主题切换相关变量 ==========
    private static final String TAG_THEME = "ThemeChange";
    private BroadcastReceiver themeChangeReceiver;
    private Handler themeHandler = new Handler(Looper.getMainLooper());
    private String savedClockContent = ""; // 保存切换前的提醒内容
    // ==========================================

    // ========== 新增GPS逻辑：核心变量 ==========
    public volatile GPSManager gpsManager; // GPS管理器实例
    private static MyService instance;
    private boolean isGpsRunning = false; // GPS运行状态标记
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

        // 权限设置页面需要在 Activity 的任务栈中启动，必须用 Activity 上下文,而不是getMyAppContext()
        Context context = MyActivity.getMyAppContext();

        if (MyActivity.zh_cn) {
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

        // 注册闹钟接收器
        IntentFilter filter = new IntentFilter(ACTION_TODO_ALARM);
        registerReceiver(myalarmReceiver, filter);

        // 计步器
        mySensorSerivece = new PersistService(MyService.this); // 传入Service上下文
        mSensorManager = (SensorManager) getSystemService(
            Context.SENSOR_SERVICE
        );
        countSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_STEP_COUNTER);
        initStepSensor();

        // 定时闹钟
        // 检查并请求权限
        if (
            Build.VERSION.SDK_INT >= Build.VERSION_CODES.S &&
            !hasExactAlarmPermission(context)
        ) {
            requestExactAlarmPermission(context); // 直接传Context，不再强转
            return;
        }

        alarmManager = (AlarmManager) getSystemService(ALARM_SERVICE);

        // ========== 新增GPS逻辑：初始化GPSManager ==========
        gpsManager = GPSManager.getInstance(getApplicationContext());
        // ==========================================

        // ========== 新增：注册主题切换广播接收器 ==========
        registerThemeChangeReceiver();
        // ==========================================

        isReady = true;
    }

    // ========== 新增：注册主题切换广播接收器 ==========
    private void registerThemeChangeReceiver() {
        themeChangeReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                Log.d(TAG_THEME, "收到广播：" + action);

                // 监听系统主题切换相关广播
                if (
                    Intent.ACTION_CONFIGURATION_CHANGED.equals(action) ||
                    "android.intent.action.THEME_CHANGED".equals(action)
                ) {
                    handleThemeChange();
                }
            }
        };

        // 注册配置变化广播（包含主题切换）
        IntentFilter themeFilter = new IntentFilter();
        themeFilter.addAction(Intent.ACTION_CONFIGURATION_CHANGED);
        themeFilter.addAction("android.intent.action.THEME_CHANGED"); // 部分厂商自定义的主题切换广播
        registerReceiver(themeChangeReceiver, themeFilter);
    }

    // ========== 新增：处理主题切换逻辑 ==========
    private void handleThemeChange() {
        Log.d(TAG_THEME, "检测到主题切换，开始处理ClockActivity");

        // 1. 检查ClockActivity是否正在显示
        if (ClockActivity.isReady) {
            Log.d(TAG_THEME, "ClockActivity正在显示，准备关闭并重启");

            // 2. 保存当前ClockActivity的内容
            try {
                // ✅ 修正：使用公共方法获取实例，不再直接访问私有变量
                ClockActivity instance = ClockActivity.getInstance();
                if (instance != null) {
                    savedClockContent = instance.getStrInfo();
                    Log.d(TAG_THEME, "保存的提醒内容：" + savedClockContent);
                }
            } catch (Exception e) {
                Log.e(TAG_THEME, "保存ClockActivity内容失败", e);
                savedClockContent = strTodoAlarm; // 备用方案：使用全局的提醒内容
            }

            // 3. 先关闭当前的ClockActivity
            ClockActivity.close();

            // 4. 延迟500ms（等待主题切换完成）后重新打开ClockActivity
            themeHandler.postDelayed(
                () -> {
                    restartClockActivity();
                },
                500
            );
        } else {
            Log.d(TAG_THEME, "ClockActivity未显示，无需处理");
        }
    }

    // ==========================================

    // ========== 新增：重启ClockActivity ==========
    private void restartClockActivity() {
        Log.d(TAG_THEME, "准备重启ClockActivity，内容：" + savedClockContent);

        if (TextUtils.isEmpty(savedClockContent)) {
            Log.w(TAG_THEME, "无保存的提醒内容，使用默认值");
            savedClockContent = strTodoAlarm;
        }

        try {
            Context context = getApplicationContext();
            Intent intent = new Intent(context, ClockActivity.class);
            intent.setFlags(
                Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP
            );
            intent.putExtra("SAVED_CLOCK_CONTENT", savedClockContent); // 传递保存的内容
            context.startActivity(intent);
            Log.d(TAG_THEME, "ClockActivity重启成功");
        } catch (Exception e) {
            Log.e(TAG_THEME, "重启ClockActivity失败", e);

            // 备用方案：通过通知重新打开
            notifyTodoAlarm(getApplicationContext(), savedClockContent);
        }
    }

    // ==========================================

    // 服务在每次启动的时候调用的方法 如果某些行为在服务已启动的时候就执行，可以把处理逻辑写在这个方法里面
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d("MyService", "onStartCommand()-------");

        // ========== 新增GPS逻辑：处理GPS启停指令 ==========
        if (intent != null) {
            String action = intent.getAction();
            if ("com.x.ACTION_START_GPS".equals(action)) {
                startGPS(); // 启动GPS
            } else if ("com.x.ACTION_STOP_GPS".equals(action)) {
                stopGPS(); // 停止GPS
            }
        }
        // ==========================================

        if (Build.VERSION.SDK_INT >= 26) {
            setForeground();
        } else {
            // android 8.0 以下
            Intent notificationIntent = new Intent(this, MyActivity.class);
            PendingIntent pendingIntent = PendingIntent.getActivity(
                this,
                0,
                notificationIntent,
                0
            );
            Notification notification = new NotificationCompat.Builder(this)
                .setSmallIcon(R.drawable.icon)
                .setLargeIcon(
                    BitmapFactory.decodeResource(
                        getResources(),
                        R.drawable.icon
                    )
                )
                .setContentTitle("Knot")
                .setContentText(strRun)
                .setContentIntent(pendingIntent) // 通知栏点击
                .build();

            startForeground(1337, notification);
        }

        return super.onStartCommand(intent, flags, startId);
    }

    // 服务销毁的时候调用的方法 可以回收部分不再使用的资源
    @Override
    public void onDestroy() {
        isReady = false;

        Log.d("MyService", "onDestroy()-------");

        // ========== 新增GPS逻辑：服务销毁时停止GPS ==========
        stopGPS();
        // ==========================================

        // ========== 新增：注销主题切换广播接收器 ==========
        if (themeChangeReceiver != null) {
            try {
                unregisterReceiver(themeChangeReceiver);
            } catch (Exception e) {
                Log.w(TAG_THEME, "注销主题切换广播失败", e);
            }
            themeChangeReceiver = null;
        }
        // 清空Handler中的延迟任务
        themeHandler.removeCallbacksAndMessages(null);
        // ==========================================

        super.onDestroy();

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
    }

    @TargetApi(26)
    private void setForeground() {
        // 创建点击意图
        Intent notificationIntent = new Intent(this, MyActivity.class);
        notificationIntent.setFlags(
            Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_CLEAR_TOP
        );

        // 适配不同版本的PendingIntent标志
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

        // 创建通知渠道 (Android 8.0+)
        String channelId = ID;
        String channelName = "Knot Service";
        NotificationChannel channel = new NotificationChannel(
            channelId,
            channelName,
            NotificationManager.IMPORTANCE_LOW
        );
        channel.setShowBadge(false);
        channel.setLockscreenVisibility(Notification.VISIBILITY_PRIVATE);

        NotificationManager manager = (NotificationManager) getSystemService(
            NOTIFICATION_SERVICE
        );
        if (manager != null) {
            manager.createNotificationChannel(channel);
        }

        // 构建通知
        Notification notification = new Notification.Builder(this, channelId)
            .setContentTitle(strStatus)
            .setContentText(strRun)
            .setSmallIcon(R.drawable.icon)
            .setLargeIcon(
                BitmapFactory.decodeResource(getResources(), R.drawable.icon)
            )
            .setContentIntent(pendingIntent) // 关键：添加点击意图
            .setAutoCancel(true) // 点击后自动清除
            .setOnlyAlertOnce(true) // 避免重复提示
            .setVisibility(Notification.VISIBILITY_PUBLIC)
            .build();

        // 启动前台服务
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
    // 待办事项定时任务通知（使用 Full-Screen Intent）
    public static void notifyTodoAlarm(Context context, String message) {
        try {
            NotificationManager m_notificationManagerAlarm =
                (NotificationManager) context.getSystemService(
                    Context.NOTIFICATION_SERVICE
                );

            // 创建跳转 Activity 的 Intent
            Intent activityIntent = new Intent(context, ClockActivity.class);

            // 修改启动标志 (Intent.FLAG_ACTIVITY_CLEAR_TOP)
            activityIntent.setFlags(
                Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP
            );

            // 构建 PendingIntent（适配 Android 12+ 不可变性）
            int flags = PendingIntent.FLAG_UPDATE_CURRENT;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                flags |= PendingIntent.FLAG_IMMUTABLE;
            }
            // 为通知创建请求码
            int requestCode = (int) System.currentTimeMillis();
            PendingIntent pendingIntent = PendingIntent.getActivity(
                context,
                requestCode,
                activityIntent,
                flags
            );

            Notification.Builder m_builderAlarm = null;

            // 创建通知渠道（Android 8.0+）
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                String channelId = "knot_alarm_channel";
                String channelName = "Knot Alarm";
                int importance = NotificationManager.IMPORTANCE_HIGH;
                NotificationChannel channel = new NotificationChannel(
                    channelId,
                    channelName,
                    importance
                );
                channel.setDescription("Alarm notifications");
                channel.enableLights(true);
                channel.setLightColor(Color.RED);
                m_notificationManagerAlarm.createNotificationChannel(channel);

                // 构建通知（Android 8.0+）
                m_builderAlarm = new Notification.Builder(context, channelId)
                    .setContentTitle(strTodo)
                    .setContentText(message)
                    .setSmallIcon(R.drawable.alarm)
                    .setColor(Color.GREEN)
                    .setAutoCancel(true)
                    .setContentIntent(pendingIntent) // 添加内容点击
                    .setFullScreenIntent(pendingIntent, true) // 关键：启用全屏 Intent
                    .setPriority(Notification.PRIORITY_MAX); // 最高优先级
            } else {
                // Android 7.1 及以下
                m_builderAlarm = new Notification.Builder(context)
                    .setContentTitle(strTodo)
                    .setContentText(message)
                    .setSmallIcon(R.drawable.alarm)
                    .setColor(Color.GREEN)
                    .setAutoCancel(true)
                    .setContentIntent(pendingIntent) // 添加内容点击
                    .setFullScreenIntent(pendingIntent, true)
                    .setPriority(Notification.PRIORITY_MAX);
            }

            // 发送通知
            m_notificationManagerAlarm.notify(
                "knot_alarm_tag",
                10,
                m_builderAlarm.build()
            );
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // 不依赖静态变量，每次重新获取NotificationManager
    public static void clearNotify(Context context) {
        try {
            NotificationManager notificationManager =
                (NotificationManager) context.getSystemService(
                    Context.NOTIFICATION_SERVICE
                );
            if (notificationManager != null) {
                notificationManager.cancel("knot_alarm_tag", 10);
                Log.d(TAG, "待办通知已清除");
            }
        } catch (Exception e) {
            Log.e(TAG, "清除待办通知失败", e);
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

    public static void initStepSensor() {
        // 1. 权限校验
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            Context context = MyActivity.getMyAppContext();
            if (
                context != null &&
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

        // 2. 补充空判 + 重新获取传感器（静态变量countSensor）
        if (mSensorManager == null) {
            Context context = MyActivity.getMyAppContext();
            if (context != null) {
                mSensorManager = (SensorManager) context.getSystemService(
                    Context.SENSOR_SERVICE
                );
            }
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
        Intent receiverIntent = new Intent(appContext, AlarmReceiver.class);
        receiverIntent.setAction(ACTION_TODO_ALARM); // 显式设置 Action
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

    // 检查是否有设置精确闹钟的权限
    private boolean hasExactAlarmPermission(Context context) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S) {
            return true; // API 31 以下不需要此权限
        }

        AlarmManager alarmManager = (AlarmManager) context.getSystemService(
            Context.ALARM_SERVICE
        );
        return alarmManager.canScheduleExactAlarms();
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
        Intent receiverIntent = new Intent(appContext, AlarmReceiver.class);
        receiverIntent.setAction(ACTION_TODO_ALARM);
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

                // 直接在前台服务中处理闹钟事件
                notifyTodoAlarm(context, message);

                if (ClockActivity.isReady) {
                    ClockActivity.safeUpdateTodoText(message);
                    CallJavaNotify_3();
                }
            }
        }
    };

    // ========== 新增GPS逻辑：启动GPS方法（供C++调用） ==========
    public boolean startGPS() {
        if (isGpsRunning) {
            Log.w(TAG, "GPS已在运行，无需重复启动");
            return true;
        }

        // 检查定位权限
        Context context = getApplicationContext();
        String finePerm = Manifest.permission.ACCESS_FINE_LOCATION;
        String coarsePerm = Manifest.permission.ACCESS_COARSE_LOCATION;
        boolean hasFine =
            ContextCompat.checkSelfPermission(context, finePerm) ==
            PackageManager.PERMISSION_GRANTED;
        boolean hasCoarse =
            ContextCompat.checkSelfPermission(context, coarsePerm) ==
            PackageManager.PERMISSION_GRANTED;

        if (!hasFine && !hasCoarse) {
            Log.e(TAG, "缺少定位权限，启动GPS失败");
            return false;
        }

        // 启动GPSManager
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
        } else {
            Log.e(TAG, "GPS启动失败");
        }

        return success;
    }

    // ========== 新增GPS逻辑：停止GPS方法（供C++调用） ==========
    public void stopGPS() {
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
}
