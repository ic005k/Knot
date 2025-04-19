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
import android.app.Notification;
import android.app.NotificationManager;
import android.graphics.Color;
import android.app.NotificationChannel;
import android.support.v4.app.NotificationCompat;
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

    //private static SimpleDateFormat format = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
    private static SimpleDateFormat format = new SimpleDateFormat("HH:mm:ss");
    public static Timer timer;
    public static Timer timerAlarm;
    public static Handler handler;
    public static int sleep = 0;
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

    public static int startTimerAlarm() {
        stopTimerAlarm();

        handler = new Handler(Looper.getMainLooper());
        handler.post(runnable);//立即调用
        System.out.println("startTimerAlarm+++++++++++++++++++++++");
        return 1;
    }

    public static int stopTimerAlarm() {
        if (handler != null) {
            handler.removeCallbacks(runnable);

        }

        System.out.println("stopTimerAlarm+++++++++++++++++++++++");
        return 1;
    }

    public static int startTimer() {
        System.out.println("startTimer+++++++++++++++++++++++");

        timer = new Timer();
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                if (MyActivity.isStepCounter == 0)
                    CallJavaNotify_1();
                if (MyActivity.isStepCounter == 1)
                {

                }


            }
        }, 0, sleep);
        return 1;
    }


    public static int stopTimer() {
        if (timer != null) {
            timer.cancel();
            timer.purge();
            System.out.println("stopTimer+++++++++++++++++++++++");
        }
        return 1;
    }

    public static int setSleep1() {
        sleep = 200;
        stopTimer();
        startTimer();
        System.out.println("setSleep1+++++++++++++++++++++++");
        return 1;
    }

    public static int setSleep2() {
        sleep = 10;
        stopTimer();
        startTimer();
        System.out.println("setSleep2+++++++++++++++++++++++");
        return 1;
    }

    public static int setSleep3() {
        sleep = 5000;
        stopTimer();
        startTimer();
        System.out.println("setSleep3+++++++++++++++++++++++");
        return 1;
    }


    @Override
    public IBinder onBind(Intent arg0) {
        // Auto-generated method stub
        Log.i(TAG, "Service on bind");//服务被绑定
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "Service on create");//服务被创建

    }

    //服务在每次启动的时候调用的方法 如果某些行为在服务已启动的时候就执行，可以把处理逻辑写在这个方法里面
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
                    .setContentIntent(pendingIntent).build();

            startForeground(1337, notification);
        }

        return super.onStartCommand(intent, flags, startId);
    }

    //服务销毁的时候调用的方法 可以回收部分不再使用的资源
    @Override
    public void onDestroy() {
        Log.d("MyService", "onDestroy()-------");

        super.onDestroy();

    }

    @TargetApi(26)
    private void setForeground() {
        Context context;
        NotificationManager manager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        NotificationChannel channel = new NotificationChannel(ID, NAME, NotificationManager.IMPORTANCE_LOW);
        manager.createNotificationChannel(channel);
        Notification notification = new Notification.Builder(this, ID)
                .setContentTitle(strStatus)
                .setContentText(strRun)
                .setSmallIcon(R.drawable.icon)
                .setLargeIcon(BitmapFactory.decodeResource(getResources(), R.drawable.icon))
                .build();
        startForeground(1337, notification);
    }

    //----------------------------------------------
    //每天行走的步数通知
    private static NotificationManager m_notificationManager;
    private static Notification.Builder m_builder;

    public static void notify(Context context, String message) {
        try {
            m_notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);

            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                //int importance = NotificationManager.IMPORTANCE_DEFAULT;
                int importance = NotificationManager.IMPORTANCE_LOW; //这个低频道不包含任何声音，达到静音的效果
                NotificationChannel notificationChannel = new NotificationChannel("Knot", "Knot Notifier Steps", importance);
                m_notificationManager.createNotificationChannel(notificationChannel);

                m_builder = new Notification.Builder(context, notificationChannel.getId());
                //m_builder.setOnlyAlertOnce(true);
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

    //------------------------------------------------------------------------------
    // 待办事项定时任务通知
    private static NotificationManager m_notificationManagerAlarm;
    private static Notification.Builder m_builderAlarm;

    public static void notifyTodoAlarm(Context context, String message) {
        try {
            m_notificationManagerAlarm = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);

            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                int importance = NotificationManager.IMPORTANCE_DEFAULT;
                NotificationChannel notificationChannel = new NotificationChannel("Knot Alarm", "Knot Notifier Alarm", importance);
                m_notificationManagerAlarm.createNotificationChannel(notificationChannel);
                m_builderAlarm = new Notification.Builder(context, notificationChannel.getId());

            } else {
                m_builderAlarm = new Notification.Builder(context);
            }

            m_builderAlarm.setContentTitle(strTodo)
                    .setContentText(message)
                    .setSmallIcon(R.drawable.alarm)
                    .setColor(Color.GREEN)
                    .setAutoCancel(true)
                    .setDefaults(Notification.DEFAULT_ALL);

            Notification notification = m_builderAlarm.build();

            m_notificationManagerAlarm.notify(strTodo, 10, notification);

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
