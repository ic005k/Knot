package com.x;

import android.app.Activity;
import android.content.Context;
import android.view.inputmethod.InputMethodManager;
import android.os.IBinder;
import android.util.Log;

public class InputMethodHelper {
    private static final String TAG = "InputMethodHelper";
    private static InputMethodManager m_imm;
    
    public static void deepResetInputMethod(Activity activity) {
        if (activity == null) {
            Log.w(TAG, "Activity is null");
            return;
        }
        
        // 获取输入法管理器
        if (m_imm == null) {
            m_imm = (InputMethodManager) activity.getSystemService(Context.INPUT_METHOD_SERVICE);
        }
        
        // 获取窗口令牌
        IBinder windowToken = activity.getWindow().getDecorView().getWindowToken();
        
        // 深度重置
        resetInputMethodService(windowToken);
        clearInputMethodState();
    }
    
    public static void resetNativeReferences() {
        // 重置 JNI 引用
        nativeResetReferences();
    }
    
    private static native void nativeResetReferences();
    
    private static void resetInputMethodService(IBinder windowToken) {
        if (m_imm != null && windowToken != null) {
            try {
                // 隐藏输入法
                m_imm.hideSoftInputFromWindow(windowToken, InputMethodManager.HIDE_NOT_ALWAYS);
                
                // 重启输入法服务
                m_imm.restartInput(null);
                
                // 清除内部状态 (使用反射)
                try {
                    java.lang.reflect.Field servedViewField = InputMethodManager.class.getDeclaredField("mServedView");
                    servedViewField.setAccessible(true);
                    servedViewField.set(m_imm, null);
                    
                    java.lang.reflect.Field nextServedViewField = InputMethodManager.class.getDeclaredField("mNextServedView");
                    nextServedViewField.setAccessible(true);
                    nextServedViewField.set(m_imm, null);
                } catch (Exception e) {
                    Log.w(TAG, "Failed to reset input method state via reflection", e);
                }
            } catch (Exception e) {
                Log.e(TAG, "Error resetting input method service", e);
            }
        }
    }
    
    private static void clearInputMethodState() {
        // 清除输入法内部状态
        try {
            // 使用反射清除更多状态
            java.lang.reflect.Field servedConnField = InputMethodManager.class.getDeclaredField("mServedInputConnection");
            servedConnField.setAccessible(true);
            servedConnField.set(m_imm, null);
        } catch (Exception e) {
            Log.w(TAG, "Failed to clear input method connection state", e);
        }
    }
}