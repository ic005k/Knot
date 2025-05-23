package com.x;

import com.x.MyActivity;

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

import android.app.NotificationManager;
import android.graphics.Color;
import android.app.NotificationChannel;

import androidx.core.app.NotificationCompat;

import android.annotation.TargetApi;

import java.sql.Time;
import java.text.SimpleDateFormat;
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

    /*
     * public static void notifyTodoAlarm(Context context, String message) {
     * try {
     * m_notificationManagerAlarm = (NotificationManager)
     * context.getSystemService(Context.NOTIFICATION_SERVICE);
     * 
     * if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
     * int importance = NotificationManager.IMPORTANCE_DEFAULT;
     * NotificationChannel notificationChannel = new
     * NotificationChannel("Knot Alarm", "Knot Notifier Alarm",
     * importance);
     * m_notificationManagerAlarm.createNotificationChannel(notificationChannel);
     * m_builderAlarm = new Notification.Builder(context,
     * notificationChannel.getId());
     * 
     * } else {
     * m_builderAlarm = new Notification.Builder(context);
     * }
     * 
     * m_builderAlarm.setContentTitle(strTodo)
     * .setContentText(message)
     * .setSmallIcon(R.drawable.alarm)
     * .setColor(Color.GREEN)
     * .setAutoCancel(true)
     * .setDefaults(Notification.DEFAULT_ALL);
     * 
     * Notification notification = m_builderAlarm.build();
     * 
     * m_notificationManagerAlarm.notify(strTodo, 10, notification);
     * 
     * } catch (Exception e) {
     * e.printStackTrace();
     * }
     * }
     */

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
            m_notificationManagerAlarm.cancel(strTodo, 10);
            System.out.println("MyService Clear Notiry...");
        }
    }
}
