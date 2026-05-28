package com.x;

import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import androidx.appcompat.app.AppCompatActivity;

public class ImmersiveUtil {

    // ==================== 【一行调用：真正沉浸式】 ====================
    public static void applyRealImmersive(AppCompatActivity activity) {
        if (activity == null) return;
        Window window = activity.getWindow();
        if (window == null) return;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            // 允许自定义系统栏
            window.addFlags(
                WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS
            );
            window.clearFlags(
                WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS
            );
            window.clearFlags(
                WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION
            );

            // ========== 关键：自动读取当前页面 XML 背景色 ==========
            View rootView = activity
                .findViewById(android.R.id.content)
                .getRootView();
            int bgColor = Color.BLACK; // 默认黑
            if (rootView.getBackground() instanceof ColorDrawable) {
                bgColor = ((ColorDrawable) rootView.getBackground()).getColor();
            }

            // ========== 自动设置状态栏/导航栏颜色 ==========
            window.setStatusBarColor(bgColor);
            window.setNavigationBarColor(bgColor);

            // ========== 自动判断文字颜色（深底白字 / 浅底黑字） ==========
            boolean isDark = isDarkColor(bgColor);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                if (isDark) {
                    window.getDecorView().setSystemUiVisibility(0);
                } else {
                    window
                        .getDecorView()
                        .setSystemUiVisibility(
                            View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR |
                                View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR
                        );
                }
            }
        }

        // ========== 自动内边距避让（不顶状态栏） ==========
        activity
            .findViewById(android.R.id.content)
            .setOnApplyWindowInsetsListener((v, insets) -> {
                int top = insets.getSystemWindowInsetTop();
                int bottom = insets.getSystemWindowInsetBottom();
                v.setPadding(0, top, 0, bottom);
                return insets;
            });
    }

    // ==================== 判断颜色深浅 ====================
    private static boolean isDarkColor(int color) {
        float r = Color.red(color) / 255f;
        float g = Color.green(color) / 255f;
        float b = Color.blue(color) / 255f;
        float luminance = (0.299f * r) + (0.587f * g) + (0.114f * b);
        return luminance <= 0.5f;
    }
}
