package com.x;

import android.os.Handler;
import android.os.Looper;
import android.util.Log;

public class QtStateManager {

    private static volatile QtStateManager instance;
    private boolean isQtReady = true; // Qt是否就绪（默认就绪）
    private static final long LOCK_DELAY = 3000; // 暗黑模式切换后，锁定3秒

    private QtStateManager() {}

    public static QtStateManager getInstance() {
        if (instance == null) {
            synchronized (QtStateManager.class) {
                instance = new QtStateManager();
            }
        }
        return instance;
    }

    // 暗黑模式切换时调用：锁定Qt交互
    public void lockQtInteraction() {
        isQtReady = false;
        Log.w(
            "QtStateManager",
            "暗黑模式切换，锁定Java-C++交互 " + LOCK_DELAY + "ms"
        );

        // 3秒后自动解锁（等Qt渲染上下文初始化完成）
        new Handler(Looper.getMainLooper()).postDelayed(
            () -> {
                isQtReady = true;
                Log.i("QtStateManager", "Qt已就绪，解锁Java-C++交互");
            },
            LOCK_DELAY
        );
    }

    // 判断是否可以和Qt交互
    public boolean canInteractWithQt() {
        return isQtReady;
    }
}
