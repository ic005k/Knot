package com.x;

import com.x.MyActivity;

import android.app.Activity;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.IBinder;
import android.util.Log;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.app.Notification;
import android.app.PendingIntent;
import android.graphics.BitmapFactory;
import android.app.AlarmManager;
import android.app.NotificationManager;
import android.graphics.Color;
import android.app.NotificationChannel;
import android.net.Uri;
import androidx.core.app.NotificationCompat;
import android.provider.Settings;
import android.annotation.TargetApi;

import java.sql.Time;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;
import java.util.Locale;

import android.content.pm.ResolveInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.PackageInfo;
import android.content.ComponentName;

public class MyService extends Service {
    private static final String TAG = "MyService";
    private static final String ID = "channel_1";
    private static final String NAME = "F_SERVICE";

    private static PersistService mySensorSerivece;

    public native static void CallJavaNotify_0();

    public native static void CallJavaNotify_1();

    public native static void CallJavaNotify_2();

    public native static void CallJavaNotify_3();

    public native static void CallJavaNotify_4();

    public native static void CallJavaNotify_5();

    public native static void CallJavaNotify_6();

    public native static void CallJavaNotify_7();

    public native static void CallJavaNotify_8();

    public native static void CallJavaNotify_9();

    public native static void CallJavaNotify_10();

    public native static void CallJavaNotify_11();

    public native static void CallJavaNotify_12();

    public native static void CallJavaNotify_13();

    public native static void CallJavaNotify_14();

    private static SimpleDateFormat format = new SimpleDateFormat("HH:mm:ss");

    public static String strRun;
    public static String strStatus;
    public static String strPedometer;
    public static String strTodo;
    public static boolean zh_cn;
    public static Context context;

    public static boolean isZh(Context context) {
        Locale locale = context.getResources().getConfiguration().locale;
        String language = locale.getLanguage();
        if (language.endsWith("zh"))
            zh_cn = true;
        else
            zh_cn = false;

        if (zh_cn) {
            strRun = "运行中...";
            strStatus = "状态";
            strTodo = "待办事项";
            strPedometer = "计步器";

        } else {
            strRun = "Running...";
            strStatus = "Status";
            strTodo = "Todo";
            strPedometer = "Pedometer";
        }

        return zh_cn;
    }

    public static Runnable runnable = new Runnable() {
        @Override
        public void run() {

        }
    };

    @Override
    public IBinder onBind(Intent arg0) {
        // Auto-generated method stub
        Log.i(TAG, "Service on bind");// 服务被绑定
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "Service on create");// 服务被创建

        context = MyActivity.context;

        // 计步器
        mySensorSerivece = new PersistService();
        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        countSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_STEP_COUNTER);
        initStepSensor();

        // 定时闹钟
        // 检查并请求权限
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S && !hasExactAlarmPermission(context)) {
            // 引导用户到设置页面授予权限
            requestExactAlarmPermission((Activity) context);
            return;
        }

        alarmManager = (AlarmManager) getSystemService(ALARM_SERVICE);

    }

    // 服务在每次启动的时候调用的方法 如果某些行为在服务已启动的时候就执行，可以把处理逻辑写在这个方法里面
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        Log.d("MyService", "onStartCommand()-------");

        if (Build.VERSION.SDK_INT >= 26) {
            setForeground();
        } else {
            // android 8.0 以下
            Intent notificationIntent = new Intent(this, MyActivity.class);
            PendingIntent pendingIntent = PendingIntent.getActivity(this, 0,
                    notificationIntent, 0);
            Notification notification = new NotificationCompat.Builder(this)
                    .setSmallIcon(R.drawable.icon)
                    .setLargeIcon(BitmapFactory.decodeResource(getResources(), R.drawable.icon))
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
        Log.d("MyService", "onDestroy()-------");

        super.onDestroy();

        if (isStepCounter == 1) {
            if (mySensorSerivece != null) {
                mSensorManager.unregisterListener(mySensorSerivece);
            }
        }

    }

    @TargetApi(26)
    private void setForeground() {
        // 创建点击意图
        Intent notificationIntent = new Intent(this, MyActivity.class);
        notificationIntent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_CLEAR_TOP);

        // 适配不同版本的PendingIntent标志
        int pendingIntentFlags = PendingIntent.FLAG_UPDATE_CURRENT;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            pendingIntentFlags |= PendingIntent.FLAG_IMMUTABLE;
        }

        PendingIntent pendingIntent = PendingIntent.getActivity(
                this,
                0,
                notificationIntent,
                pendingIntentFlags);

        // 创建通知渠道 (Android 8.0+)
        String channelId = ID;
        String channelName = "Knot Service";
        NotificationChannel channel = new NotificationChannel(
                channelId,
                channelName,
                NotificationManager.IMPORTANCE_LOW);
        channel.setShowBadge(false);
        channel.setLockscreenVisibility(Notification.VISIBILITY_PRIVATE);

        NotificationManager manager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        if (manager != null) {
            manager.createNotificationChannel(channel);
        }

        // 构建通知
        Notification notification = new Notification.Builder(this, channelId)
                .setContentTitle(strStatus)
                .setContentText(strRun)
                .setSmallIcon(R.drawable.icon)
                .setLargeIcon(BitmapFactory.decodeResource(getResources(), R.drawable.icon))
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
            m_notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);

            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                // int importance = NotificationManager.IMPORTANCE_DEFAULT;
                int importance = NotificationManager.IMPORTANCE_LOW; // 这个低频道不包含任何声音，达到静音的效果
                NotificationChannel notificationChannel = new NotificationChannel("Knot", "Knot Notifier Steps",
                        importance);
                m_notificationManager.createNotificationChannel(notificationChannel);

                m_builder = new Notification.Builder(context, notificationChannel.getId());
                // m_builder.setOnlyAlertOnce(true);
            } else {
                m_builder = new Notification.Builder(context);
            }

            m_builder.setContentTitle(strPedometer)
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
    // 待办事项定时任务通知
    private static NotificationManager m_notificationManagerAlarm;
    private static Notification.Builder m_builderAlarm;

    // 待办事项定时任务通知（使用 Full-Screen Intent）
    public static void notifyTodoAlarm(Context context, String message) {
        try {
            m_notificationManagerAlarm = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);

            // 创建跳转 Activity 的 Intent
            Intent activityIntent = new Intent(context, ClockActivity.class);
            activityIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);

            // 构建 PendingIntent（适配 Android 12+ 不可变性）
            int flags = PendingIntent.FLAG_UPDATE_CURRENT;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                flags |= PendingIntent.FLAG_IMMUTABLE;
            }
            PendingIntent pendingIntent = PendingIntent.getActivity(
                    context, 0, activityIntent, flags);

            // 创建通知渠道（Android 8.0+）
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                String channelId = "knot_alarm_channel";
                String channelName = "Knot Alarm";
                int importance = NotificationManager.IMPORTANCE_HIGH; // 必须为 HIGH 才能触发全屏
                NotificationChannel channel = new NotificationChannel(
                        channelId, channelName, importance);
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
                        .setFullScreenIntent(pendingIntent, true)
                        .setPriority(Notification.PRIORITY_MAX);
            }

            // 发送通知
            m_notificationManagerAlarm.notify("knot_alarm_tag", 10, m_builderAlarm.build());

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void clearNotify() {
        if (m_notificationManagerAlarm != null) {
            // 使用和发送通知时相同的标签 "knot_alarm_tag"
            m_notificationManagerAlarm.cancel("knot_alarm_tag", 10);
            System.out.println("MyService Clear Notiry...");
        }
    }

    /////////////////////// Steps Sensor /////////////////////////////////////

    class PersistService extends Service implements SensorEventListener {
        public BroadcastReceiver mReceiver = new BroadcastReceiver() {

            @Override
            public void onReceive(Context context, Intent intent) {
                if (mSensorManager != null) { // 取消监听后重写监听，以保持后台运行
                    mSensorManager.unregisterListener(PersistService.this);
                    mSensorManager.registerListener(PersistService.this,
                            mSensorManager.getDefaultSensor(Sensor.TYPE_STEP_COUNTER),
                            SensorManager.SENSOR_DELAY_NORMAL);
                }
            }
        };

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            Log.i(TAG, "PersistService.onAccuracyChanged().");
        }

        @Override
        public void onSensorChanged(SensorEvent sensorEvent) {
            // 做个判断传感器类型很重要，这可以过滤掉杂音（比如可能来自其它传感器的值）
            if (sensorEvent.sensor.getType() == Sensor.TYPE_STEP_COUNTER) {
                // float[] values = sensorEvent.values;
                stepCounts = (long) sensorEvent.values[0];
            }
        }

        @Override
        public IBinder onBind(Intent intent) {
            // Auto-generated method stub
            return null;
        }
    }

    private Sensor countSensor;
    public static int isStepCounter = -1;
    public static float stepCounts;
    private static SensorManager mSensorManager;

    public void initStepSensor() {
        if (countSensor != null) {
            if (mSensorManager != null) {
                mSensorManager.unregisterListener(mySensorSerivece);
                mSensorManager.registerListener(
                        mySensorSerivece,
                        mSensorManager.getDefaultSensor(Sensor.TYPE_STEP_COUNTER),
                        SensorManager.SENSOR_DELAY_NORMAL);
            }
            isStepCounter = 1;
        } else
            isStepCounter = 0;
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
    // private static PendingIntent pi;
    private static PendingIntent pendingIntentAlarm;

    public static int startAlarm(String str) {
        // 特殊转义字符，必须加"\\"（“.”和“|”都是转义字符）
        String[] array = str.split("\\|");
        for (int i = 0; i < array.length; i++)
            System.out.println(array[i]);

        String strTime = array[0];
        String strText = array[1];
        String strTotalS = array[2];

        Calendar c = Calendar.getInstance();
        c.setTimeInMillis(System.currentTimeMillis());

        int ts = Integer.parseInt(strTotalS);
        c.add(Calendar.SECOND, ts);

        // 使用应用上下文，避免Activity引用导致的内存泄漏
        Context appContext = context.getApplicationContext();

        int flags = PendingIntent.FLAG_UPDATE_CURRENT;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            flags |= PendingIntent.FLAG_IMMUTABLE;
        }

        // Intent intent = new Intent(appContext, ClockActivity.class);
        // pi = PendingIntent.getActivity(
        // appContext,
        // 0,
        // intent,
        // flags);

        // 创建定时触发的 BroadcastReceiver Intent
        Intent receiverIntent = new Intent(appContext, AlarmReceiver.class);
        receiverIntent.setAction(ACTION_TODO_ALARM); // 显式设置 Action
        receiverIntent.putExtra("alarmMessage", strText);

        // 唯一请求码（不考虑 PendingIntent 复用，业务逻辑：新定时覆盖旧定时）
        int requestCode = 0;

        pendingIntentAlarm = PendingIntent.getBroadcast(
                appContext,
                requestCode,
                receiverIntent,
                flags);

        // 设置闹钟
        alarmManager.setExactAndAllowWhileIdle(
                AlarmManager.RTC_WAKEUP,
                c.getTimeInMillis(),
                pendingIntentAlarm);

        Log.e("Alarm Manager", c.getTimeInMillis() + "");
        Log.e("Alarm Manager", str);

        System.out.println(ts);
        System.out.println("startAlarm+++++++++++++++++++++++");

        return 1;
    }

    public static int stopAlarm() {
        if (alarmManager != null) {
            if (pendingIntentAlarm != null) {
                alarmManager.cancel(pendingIntentAlarm);
            }

            System.out.println("stopAlarm+++++++++++++++++++++++");
        }

        return 1;
    }

    // 检查是否有设置精确闹钟的权限
    private boolean hasExactAlarmPermission(Context context) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S) {
            return true; // API 31 以下不需要此权限
        }

        AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        return alarmManager.canScheduleExactAlarms();
    }

    // 请求精确闹钟权限
    private void requestExactAlarmPermission(Activity activity) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S) {
            return; // API 31 以下不需要此权限
        }

        Intent intent = new Intent();
        intent.setAction(Settings.ACTION_REQUEST_SCHEDULE_EXACT_ALARM);
        intent.setData(Uri.parse("package:" + activity.getPackageName()));
        activity.startActivityForResult(intent, 1001); // 1001 是请求码，可自定义
    }

    public static int startPreciseAlarm(String str) {
        String[] array = str.split("\\|");
        for (int i = 0; i < array.length; i++)
            System.out.println(array[i]);

        String strTime = array[0];
        String strText = array[1];
        String strTotalS = array[2];

        Calendar c = Calendar.getInstance();
        c.setTimeInMillis(System.currentTimeMillis());

        int ts = Integer.parseInt(strTotalS);
        c.add(Calendar.SECOND, ts);

        // 使用应用上下文，避免Activity引用导致的内存泄漏
        Context appContext = context.getApplicationContext();

        // 1. 使用 setAlarmClock() 提高优先级
        Intent showIntent = new Intent(appContext, ClockActivity.class);
        PendingIntent showPending = PendingIntent.getActivity(
                appContext, 0, showIntent, PendingIntent.FLAG_IMMUTABLE);

        AlarmManager.AlarmClockInfo clockInfo = new AlarmManager.AlarmClockInfo(c.getTimeInMillis(), showPending);

        // 2. 创建精确触发的广播Intent
        Intent receiverIntent = new Intent(appContext, AlarmReceiver.class);
        receiverIntent.setAction(ACTION_TODO_ALARM);
        receiverIntent.putExtra("alarmMessage", strText);

        int requestCode = 0;
        PendingIntent pending = PendingIntent.getBroadcast(
                appContext,
                requestCode,
                receiverIntent,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

        // 3. 设置闹钟
        alarmManager.setAlarmClock(clockInfo, pending);

        return 1;

    }

    //////////////////////////////////////////////////////////////////////

}
