package com.x;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import androidx.core.app.NotificationCompat;

public class TTSForegroundService extends Service {
    private static final String CHANNEL_ID = "tts_foreground_service_channel";
    private static final int NOTIFICATION_ID = 1234;

    private static TTSUtils ttsUtils;

    @Override
    public void onCreate() {
        super.onCreate();
        ttsUtils = TTSUtils.getInstance(getApplicationContext());
        createNotificationChannel();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent != null && intent.hasExtra("tts_text")) {
            String text = intent.getStringExtra("tts_text");

            // 启动前台服务
            startForeground(NOTIFICATION_ID, createNotification());

            if (!MyActivity.isTTSService) {
                if (ttsUtils != null) {
                    ttsUtils.speak(text);
                }
                MyActivity.isTTSService = true;
            }
        }
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private Notification createNotification() {
        return new NotificationCompat.Builder(this, CHANNEL_ID)
                .setContentTitle("Voice Broadcast Service")
                .setContentText("Running...")
                .setSmallIcon(android.R.drawable.ic_lock_silent_mode_off) // 使用系统图标或替换为应用图标
                .setPriority(NotificationCompat.PRIORITY_LOW)
                .build();
    }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel serviceChannel = new NotificationChannel(
                    CHANNEL_ID,
                    "Voice Broadcast Service",
                    NotificationManager.IMPORTANCE_LOW);
            NotificationManager manager = getSystemService(NotificationManager.class);
            if (manager != null) {
                manager.createNotificationChannel(serviceChannel);
            }
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (ttsUtils != null) {
            ttsUtils.shutdown();
        }
    }

    public static void requestSpeak(String text) {
        if (ttsUtils != null) {
            ttsUtils.speak(text);
        }
    }

}