package com.x;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;

public class NotifyRouterActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            // ✅ 高版本：系统已通过 RESET_TASK_IF_NEEDED 恢复到栈顶
            // Router 的使命已完成，无需再启动任何 Activity
            // 直接 finish 让已恢复的栈顶窗口自然显示
        } else {
            // ✅ 低版本：仅尝试移前台，失败则冷启动
            ActivityManager am = (ActivityManager) getSystemService(
                Context.ACTIVITY_SERVICE
            );
            boolean taskMoved = false;
            try {
                if (am != null) {
                    am.moveTaskToFront(
                        getTaskId(),
                        ActivityManager.MOVE_TASK_WITH_HOME
                    );
                    taskMoved = true;
                }
            } catch (Exception e) {
                /* ignore */
            }

            if (!taskMoved) {
                Intent intent = new Intent(this, MyActivity.class);
                intent.addFlags(
                    Intent.FLAG_ACTIVITY_NEW_TASK |
                        Intent.FLAG_ACTIVITY_SINGLE_TOP
                );
                startActivity(intent);
            }
        }
        finish();
    }
}
