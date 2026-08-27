package com.x;

import android.app.Activity;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import androidx.core.view.WindowInsetsControllerCompat;

public class ImmersiveUtil {

    // Qt主Activity硬编码配色
    private static final int STATUS_BAR_DARK = Color.parseColor("#19232D");
    private static final int NAV_BAR_DARK = Color.parseColor("#121212");
    private static final int STATUS_BAR_LIGHT = Color.parseColor("#F3F3F3");
    private static final int NAV_BAR_LIGHT = Color.parseColor("#FFFFFF");

    /**
     * 【原生Java Activity调用】原版逻辑：读取页面背景色，content设置padding避让系统栏，内容不会顶状态栏
     */
    public static boolean applyRealImmersive(Activity activity) {
        if (
            activity == null || activity.isFinishing() || activity.isDestroyed()
        ) return false;
        if (!Looper.getMainLooper().isCurrentThread()) {
            new Handler(Looper.getMainLooper()).post(() ->
                applyRealImmersive(activity)
            );
            return false;
        }

        Window window = activity.getWindow();
        if (window == null) return false;

        boolean isDark = false;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            window.addFlags(
                WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS
            );
            window.clearFlags(
                WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS
            );
            window.clearFlags(
                WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION
            );

            View rootView = activity
                .findViewById(android.R.id.content)
                .getRootView();
            int bgColor = Color.BLACK;
            if (rootView.getBackground() instanceof ColorDrawable) {
                bgColor = ((ColorDrawable) rootView.getBackground()).getColor();
            }

            window.setStatusBarColor(bgColor);
            window.setNavigationBarColor(bgColor);

            isDark = isDarkColor(bgColor);
            WindowInsetsControllerCompat controller =
                new WindowInsetsControllerCompat(window, window.getDecorView());
            controller.setAppearanceLightStatusBars(!isDark);
            controller.setAppearanceLightNavigationBars(!isDark);
        }

        // ========== 仅原生Activity执行：给content设置padding避让系统栏 ==========
        View contentView = activity.findViewById(android.R.id.content);
        if (contentView != null) {
            // 使用View单tag标记是否已经安装insets listener
            if (contentView.getTag() == null) {
                contentView.setTag(Boolean.TRUE);
                contentView.setOnApplyWindowInsetsListener((v, insets) -> {
                    v.setPadding(
                        0,
                        insets.getSystemWindowInsetTop(),
                        0,
                        insets.getSystemWindowInsetBottom()
                    );
                    return insets;
                });
            }
        }

        return isDark;
    }

    /**
     * 【Qt MyActivity专用调用】双参数版本：只设置系统栏颜色+图标，完全跳过content padding/insets逻辑，Qt自己处理渲染避让
     * @param activity Qt主Activity
     * @param isAppDark 应用暗黑模式开关
     */
    public static void applyRealImmersive(
        Activity activity,
        boolean isAppDark
    ) {
        if (
            activity == null || activity.isFinishing() || activity.isDestroyed()
        ) return;
        if (!Looper.getMainLooper().isCurrentThread()) {
            new Handler(Looper.getMainLooper()).post(() ->
                applyRealImmersive(activity, isAppDark)
            );
            return;
        }

        Window window = activity.getWindow();
        if (window == null) return;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            window.addFlags(
                WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS
            );
            window.clearFlags(
                WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS
            );
            window.clearFlags(
                WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION
            );

            window.setStatusBarColor(
                isAppDark ? STATUS_BAR_DARK : STATUS_BAR_LIGHT
            );
            window.setNavigationBarColor(
                isAppDark ? NAV_BAR_DARK : NAV_BAR_LIGHT
            );
        }

        WindowInsetsControllerCompat controller =
            new WindowInsetsControllerCompat(window, window.getDecorView());
        controller.setAppearanceLightStatusBars(!isAppDark);
        controller.setAppearanceLightNavigationBars(!isAppDark);

        // !!! Qt分支：绝不执行contentView的OnApplyWindowInsetsListener，不修改content padding，完全交给Qt内部处理
    }

    private static boolean isDarkColor(int color) {
        float r = Color.red(color) / 255f;
        float g = Color.green(color) / 255f;
        float b = Color.blue(color) / 255f;
        float luminance = 0.299f * r + 0.587f * g + 0.114f * b;
        return luminance <= 0.5f;
    }
}
