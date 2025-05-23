package com.x;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.os.Build;

public class AlarmReceiver extends BroadcastReceiver {
  @Override
  public void onReceive(Context context, Intent intent) {
    String message = intent.getStringExtra("alarmMessage");

    // Android 8.0+：发送全屏通知
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      MyService.notifyTodoAlarm(context, message);
      Log.d("AlarmManager", "定时提醒时间已到...... :" + message);
    } else {
      // Android 7.1 及以下：直接启动 Activity
      Intent activityIntent = new Intent(context, ClockActivity.class);
      activityIntent.putExtra("alarmMessage", message);
      activityIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
      context.startActivity(activityIntent);
    }
  }

}
