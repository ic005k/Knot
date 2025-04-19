package com.x;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class AlarmReceiver extends BroadcastReceiver{
    @Override
    public void onReceive(Context context, Intent intent) {
        Intent i = new Intent(context,ClockActivity.class);
        i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        Log.e("闹钟管理","运行中...");
        context.startActivity(i);
        System.out.println("闹钟接收服务已运行+++++++++++++++++++++++");
    }
}
