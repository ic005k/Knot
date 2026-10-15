package com.x;

import android.app.Activity;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;

public class ImmersiveUtil {

    // Qt主Activity硬编码配色
    private static final int STATUS_BAR_DARK = Color.parseColor("#19232D");
    private static final int NAV_BAR_DARK = Color.parseColor("#121212");
    private static final int STATUS_BAR_LIGHT = Color.parseColor("#F3F3F3");
    private static final int NAV_BAR_LIGHT = Color.parseColor("#FFFFFF");

    // DecorView 背景色条的 tag（用私有常量，避免和 contentView 上的 setTag(Boolean.TRUE) 冲突）
    private static final int TAG_KEY_STATUS = 0x7f0f0001;
    private static final int TAG_KEY_NAV = 0x7f0f0002;

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

        View contentView = activity.findViewById(android.R.id.content);
        if (contentView != null) {
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
     * 【Qt MyActivity专用调用】兼容 Android 15 edge-to-edge：
     * 系统栏着色 API 在 API 35 已失效，改为在 DecorView 底层垫两条目标色条，
     * 状态栏/导航栏透明区透出即为目标色；字色仍用 setAppearanceLight*。
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

        final int statusColor = isAppDark ? STATUS_BAR_DARK : STATUS_BAR_LIGHT;
        final int navColor = isAppDark ? NAV_BAR_DARK : NAV_BAR_LIGHT;

        window.addFlags(
            WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS
        );
        window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
        window.clearFlags(
            WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION
        );
        // 仍设一次：API<35 生效，API35 无害（被系统忽略）
        window.setStatusBarColor(statusColor);
        window.setNavigationBarColor(navColor);

        View decorView = window.getDecorView();
        View contentView = activity.findViewById(android.R.id.content);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.setDecorFitsSystemWindows(false);

            // Qt 内容 padding 避让（保持原逻辑）
            if (contentView != null && contentView.getTag() == null) {
                contentView.setTag(Boolean.TRUE);
                contentView.setOnApplyWindowInsetsListener((v, insets) -> {
                    int top = insets
                        .getInsets(WindowInsetsCompat.Type.systemBars())
                        .top;
                    int bottom = insets
                        .getInsets(WindowInsetsCompat.Type.systemBars())
                        .bottom;
                    v.setPadding(0, top, 0, bottom);
                    return insets;
                });
            }

            // ★ 关键：DecorView 上垫/更新两条背景色条
            final View statusStrip = ensureBar(
                decorView,
                TAG_KEY_STATUS,
                statusColor,
                true
            );
            final View navStrip = ensureBar(
                decorView,
                TAG_KEY_NAV,
                navColor,
                false
            );
            if (statusStrip != null) statusStrip.setBackgroundColor(
                statusColor
            );
            if (navStrip != null) navStrip.setBackgroundColor(navColor);

            // insets 变化时同步色条高度（顶部=状态栏高，底部=导航栏高）
            decorView.setOnApplyWindowInsetsListener((v, insets) -> {
                android.graphics.Insets bars = insets.getInsets(
                    android.view.WindowInsets.Type.systemBars()
                );
                if (statusStrip != null) {
                    ViewGroup.LayoutParams lp = statusStrip.getLayoutParams();
                    if (lp != null) {
                        lp.height = bars.top;
                        statusStrip.setLayoutParams(lp);
                    }
                }
                if (navStrip != null) {
                    ViewGroup.LayoutParams lp = navStrip.getLayoutParams();
                    if (lp != null) {
                        lp.height = bars.bottom;
                        navStrip.setLayoutParams(lp);
                    }
                }
                return insets; // 不消费，继续派发
            });
            ViewCompat.requestApplyInsets(decorView);
        } else {
            // API 21~29
            int vis = decorView.getSystemUiVisibility();
            vis &= ~(
                View.SYSTEM_UI_FLAG_FULLSCREEN |
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_IMMERSIVE |
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
                View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
            );
            vis |= View.SYSTEM_UI_FLAG_LAYOUT_STABLE;
            decorView.setSystemUiVisibility(vis);
        }

        WindowInsetsControllerCompat controller =
            new WindowInsetsControllerCompat(window, decorView);
        controller.setAppearanceLightStatusBars(!isAppDark);
        controller.setAppearanceLightNavigationBars(!isAppDark);
    }

    // 在 DecorView 上确保存在一条贴顶/贴底背景色条（addView(bar,0) 放最底层，不挡 Qt）
    private static View ensureBar(
        View decorView,
        int tagKey,
        int color,
        boolean isTop
    ) {
        if (!(decorView instanceof ViewGroup)) return null;
        ViewGroup decor = (ViewGroup) decorView;
        View bar = decor.findViewWithTag(tagKey);
        if (bar == null) {
            bar = new View(decor.getContext());
            bar.setTag(tagKey);
            bar.setBackgroundColor(color);
            android.widget.FrameLayout.LayoutParams flp =
                new android.widget.FrameLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    0
                );
            flp.gravity = isTop ? Gravity.TOP : Gravity.BOTTOM;
            bar.setLayoutParams(flp);
            decor.addView(bar, 0); // 最底层
        }
        return bar;
    }

    private static boolean isDarkColor(int color) {
        float r = Color.red(color) / 255f;
        float g = Color.green(color) / 255f;
        float b = Color.blue(color) / 255f;
        float luminance = 0.299f * r + 0.587f * g + 0.114f * b;
        return luminance <= 0.5f;
    }
}
