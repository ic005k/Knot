package com.x;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.os.Bundle;

/**
 * 通知点击路由中转站
 * 专门解决 EMUI 9.0 点击通知导致非栈底 Activity 被错误重建的问题
 *
 * 核心原理：仅将任务栈整体移至前台，不向任何 Activity 发送 Intent，
 * 从而避免栈顶原生窗口被干扰或清除。
 */
public class NotifyRouterActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // ✅ 关键：使用 moveTaskToFront 代替 startActivity
        // 这只会改变任务栈的前后台顺序，不会触发任何 Activity 的重建、销毁或 onNewIntent
        ActivityManager am = (ActivityManager) getSystemService(
            Context.ACTIVITY_SERVICE
        );
        if (am != null) {
            // getTaskId() 返回当前 Router 所在的任务ID
            // 由于 Router 与主应用同任务（未设置独立 taskAffinity），这就是主任务ID
            am.moveTaskToFront(
                getTaskId(),
                ActivityManager.MOVE_TASK_WITH_HOME
            );
        }

        // 立即关闭自己，不留痕迹
        finish();
    }
}
