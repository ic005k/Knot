package com.x;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlertDialog;
import android.app.Application;
import android.app.PendingIntent;
import android.app.Service;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Color;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Bundle;
import android.os.FileObserver;
import android.os.Handler;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.method.ScrollingMovementMethod;
import android.text.style.BackgroundColorSpan;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.TextView;
import android.widget.TextView;
import android.widget.Toast;
import com.x.MyActivity;
import com.x.NoteEditor;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Locale;
import java.util.Map;
import java.util.Properties;
import java.util.logging.Logger;
import org.ini4j.Wini;

public class Desk_Exercise extends Activity {

    private String shortcut_ini = "/storage/emulated/0/.Knot/shortcut.ini";

    private static Context context;

    public static native void CallJavaNotify_0();

    public static native void CallJavaNotify_1();

    public static native void CallJavaNotify_2();

    public static native void CallJavaNotify_3();

    public static native void CallJavaNotify_4();

    public static native void CallJavaNotify_5();

    public static native void CallJavaNotify_6();

    public static native void CallJavaNotify_7();

    public static native void CallJavaNotify_8();

    public static native void CallJavaNotify_9();

    public static native void CallJavaNotify_10();

    public static native void CallJavaNotify_11();

    public static native void CallJavaNotify_12();

    public static native void CallJavaNotify_13();

    public static native void CallJavaNotify_14();

    private static boolean zh_cn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setBackgroundDrawable(null);

        // 直接处理业务逻辑
        processShortcutOperation();

        finish();
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }

    @Override
    protected void onDestroy() {
        System.out.println("onDestroy...");
        // goExercise();
        super.onDestroy();
    }

    public void openAppFromPackageName(String pname) {
        PackageManager packageManager = getPackageManager();
        Intent it = packageManager.getLaunchIntentForPackage(pname);
        startActivity(it);
    }

    public boolean isAppRun(String pName) {
        Context context = MyActivity.getMyAppContext();

        boolean isRun = false;
        int uid = getPackageUid(context, pName);
        if (uid > 0) {
            boolean rstA = isAppRunning(context, pName);
            boolean rstB = isProcessRunning(context, uid);
            // if (rstA || rstB) {
            if (rstB) {
                // 指定包名的程序正在运行中
                isRun = true;
            } else {
                // 指定包名的程序未在运行中
                isRun = false;
            }
        } else {
            // 应用未安装
        }
        return isRun;
    }

    public static boolean isAppRunning(Context context, String packageName) {
        ActivityManager am = (ActivityManager) context.getSystemService(
            Context.ACTIVITY_SERVICE
        );
        List<ActivityManager.RunningTaskInfo> list = am.getRunningTasks(100);
        if (list.size() <= 0) {
            return false;
        }
        for (ActivityManager.RunningTaskInfo info : list) {
            if (info.baseActivity.getPackageName().equals(packageName)) {
                return true;
            }
        }
        return false;
    }

    // 获取已安装应用的 uid，-1 表示未安装此应用或程序异常
    public static int getPackageUid(Context context, String packageName) {
        try {
            ApplicationInfo applicationInfo = context
                .getPackageManager()
                .getApplicationInfo(packageName, 0);
            if (applicationInfo != null) {
                Log.e("share", String.valueOf(applicationInfo.uid));
                return applicationInfo.uid;
            }
        } catch (Exception e) {
            return -1;
        }
        return -1;
    }

    public static boolean isProcessRunning(Context context, int uid) {
        ActivityManager am = (ActivityManager) context.getSystemService(
            Context.ACTIVITY_SERVICE
        );
        List<ActivityManager.RunningServiceInfo> runningServiceInfos =
            am.getRunningServices(200);
        if (runningServiceInfos.size() > 0) {
            for (ActivityManager.RunningServiceInfo appProcess : runningServiceInfos) {
                if (uid == appProcess.uid) {
                    return true;
                }
            }
        }
        return false;
    }

    public static boolean isZh(Context context) {
        Locale locale = context.getResources().getConfiguration().locale;
        String language = locale.getLanguage();
        if (language.endsWith("zh")) zh_cn = true;
        else zh_cn = false;

        return zh_cn;
    }

    private void goExercise() {
        boolean isRun = MyService.isReady; // isAppRun("com.x");

        if (!isRun) {
            try {
                File file = new File(shortcut_ini);
                if (!file.exists()) file.createNewFile();
                Wini ini = new Wini(file);

                ini.put("desk", "keyType", "exercise");
                ini.put("desk", "execDone", "false");

                ini.store();
            } catch (IOException e) {
                e.printStackTrace();
            }

            if (isZh(this)) Toast.makeText(
                this,
                getString(R.string.strTip_zh),
                Toast.LENGTH_LONG
            ).show();
            else Toast.makeText(
                this,
                getString(R.string.strTip),
                Toast.LENGTH_LONG
            ).show();

            // reopen app
            openAppFromPackageName("com.x");
        } else {
            try {
                File file = new File(shortcut_ini);
                if (!file.exists()) file.createNewFile();
                Wini ini = new Wini(file);

                ini.put("desk", "keyType", "exercise");
                ini.put("desk", "execDone", "true");

                ini.store();
            } catch (IOException e) {
                e.printStackTrace();
            }

            safeCallJavaNotify_8();
        }
    }

    private void processShortcutOperation() {
        // 后台执行耗时操作
        new Thread(() -> {
            boolean isServiceRunning = checkServiceRunning();
            updateConfigFile(isServiceRunning);

            runOnUiThread(() -> {
                if (!isServiceRunning) {
                    showLaunchPrompt();
                } else {
                    executeServiceAction();
                }
            });
        })
            .start();
    }

    private boolean checkServiceRunning() {
        return MyService.isReady; // 或其他服务检测逻辑
    }

    private void updateConfigFile(boolean serviceRunning) {
        try {
            File iniFile = new File(shortcut_ini);
            File parentDir = iniFile.getParentFile();

            // 确保目录存在
            if (parentDir != null && !parentDir.exists()) {
                parentDir.mkdirs();
            }

            // 使用 try-with-resources 确保文件关闭
            try (FileOutputStream fos = new FileOutputStream(iniFile)) {
                String config = String.format(
                    "[desk]\nkeyType=exercise\nexecDone=%s\n",
                    serviceRunning
                );
                fos.write(config.getBytes(StandardCharsets.UTF_8));
            }
        } catch (IOException e) {
            Log.e("Config", "Update failed", e);
        }
    }

    private void showLaunchPrompt() {
        // 显示国家化提示
        int resId = isZh(this) ? R.string.strTip_zh : R.string.strTip;
        String message = getString(resId);

        Toast.makeText(
            getApplicationContext(),
            message,
            Toast.LENGTH_LONG
        ).show();

        // 延迟启动应用（确保Toast显示）
        new Handler().postDelayed(
            () -> {
                startActivity(
                    new Intent(this, MyActivity.class).addFlags(
                        Intent.FLAG_ACTIVITY_NEW_TASK
                    )
                );
            },
            500
        );
    }

    private void executeServiceAction() {
        // 执行服务相关操作
        safeCallJavaNotify_8();
    }

    private void safeCallJavaNotify_8() {
        try {
            if (MyService.isReady) {
                MyService.CallJavaNotify_8();
            } else {
                Log.w(
                    "Desk_Exercise",
                    "MyService未就绪，跳过CallJavaNotify_8调用"
                );
            }
        } catch (Throwable e) {
            Log.e("Desk_Exercise", "调用CallJavaNotify_8失败", e);
        }
    }
}
