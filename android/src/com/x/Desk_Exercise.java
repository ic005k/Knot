package com.x;

import org.ini4j.Wini;

import com.x.MyActivity;
import com.x.NoteEditor;

import android.content.ComponentName;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.net.Uri;
import android.os.FileObserver;

import android.text.method.ScrollingMovementMethod;
import java.util.List;
import java.util.ArrayList;

import android.content.IntentFilter;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.app.PendingIntent;
import android.text.TextUtils;
import android.app.AlertDialog;
import android.app.Service;
import android.content.DialogInterface;
import android.graphics.Color;
import android.media.MediaPlayer;
import android.os.Build;
import android.os.Bundle;
import android.app.Activity;
import android.appwidget.AppWidgetManager;
import android.content.Context;
import android.appwidget.AppWidgetProvider;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.view.WindowManager;
import android.view.Window;

import java.io.OutputStreamWriter;
import java.net.URI;
import java.io.BufferedWriter;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.InputStreamReader;

import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Properties;
import java.io.IOException;
import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.os.Handler;
import android.media.AudioManager;
import android.widget.TextView;

import java.util.Locale;

import android.app.Application;
import android.app.ActivityManager;
import android.content.pm.ApplicationInfo;

import java.util.logging.Logger;

import android.widget.Toast;

import android.text.SpannableStringBuilder;
import android.text.style.BackgroundColorSpan;
import android.text.style.ForegroundColorSpan;
import android.text.Spannable;
import android.text.Spanned;

public class Desk_Exercise extends Activity {

    private String shortcut_ini = "/storage/emulated/0/.Knot/shortcut.ini";

    private static Context context;

    public native static void CallJavaNotify_0();

    public native static void CallJavaNotify_1();

    public native static void CallJavaNotify_2();

    public native static void CallJavaNotify_3();

    public native static void CallJavaNotify_4();

    public native static void CallJavaNotify_5();

    public native static void CallJavaNotify_6();

    public native static void CallJavaNotify_7();

    public native static void CallJavaNotify_8();

    public native static void CallJavaNotify_9();

    public native static void CallJavaNotify_10();

    public native static void CallJavaNotify_11();

    public native static void CallJavaNotify_12();

    public native static void CallJavaNotify_13();

    public native static void CallJavaNotify_14();

    private static boolean zh_cn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        Desk_Exercise.this.finish();

    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();

    }

    @Override
    protected void onDestroy() {
        System.out.println("onDestroy...");
        goExercise();
        super.onDestroy();

    }

    public void openAppFromPackageName(String pname) {
        PackageManager packageManager = getPackageManager();
        Intent it = packageManager.getLaunchIntentForPackage(pname);
        startActivity(it);
    }

    private void doStartApplicationWithPackageName(String packagename) {
        System.out.println("自启动开始...");
        // 通过包名获取此APP详细信息，包括Activities、services、versioncode、name等等
        PackageInfo packageinfo = null;
        try {
            packageinfo = getPackageManager().getPackageInfo(packagename, 0);
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        if (packageinfo == null) {
            return;
        }

        // 创建一个类别为CATEGORY_LAUNCHER的该包名的Intent
        Intent resolveIntent = new Intent(Intent.ACTION_MAIN, null);
        resolveIntent.addCategory(Intent.CATEGORY_LAUNCHER);
        resolveIntent.setPackage(packageinfo.packageName);

        // 通过getPackageManager()的queryIntentActivities方法遍历
        List<ResolveInfo> resolveinfoList = getPackageManager()
                .queryIntentActivities(resolveIntent, 0);

        ResolveInfo resolveinfo = resolveinfoList.iterator().next();
        if (resolveinfo != null) {
            // packagename = 参数packname
            String packageName = resolveinfo.activityInfo.packageName;
            // 这个就是我们要找的该APP的LAUNCHER的Activity[组织形式：packagename.mainActivityname]
            String className = resolveinfo.activityInfo.name;
            // LAUNCHER Intent
            Intent intent = new Intent(Intent.ACTION_MAIN);
            intent.addCategory(Intent.CATEGORY_LAUNCHER);

            // 设置ComponentName参数1:packagename参数2:MainActivity路径
            ComponentName cn = new ComponentName(packageName, className);

            intent.setComponent(cn);
            startActivity(intent);

            System.out.println("启动自己已完成...");
        }

        System.out.println("过程完成...");
    }

    private void startLocalApp(String packageNameTarget) {
        Log.i("Wmx logs::", "-----------------------开始启动第三方 APP=" + packageNameTarget);

        PackageManager packageManager = getPackageManager();
        Intent intent = packageManager.getLaunchIntentForPackage(packageNameTarget);
        intent.addCategory(Intent.CATEGORY_LAUNCHER);
        intent.setFlags(Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED | Intent.FLAG_ACTIVITY_NEW_TASK);

        /**
         * android.intent.action.MAIN：打开另一程序
         */
        intent.setAction("android.intent.action.MAIN");
        /**
         * FLAG_ACTIVITY_SINGLE_TOP:
         * 如果当前栈顶的activity就是要启动的activity,则不会再启动一个新的activity
         */
        intent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
        startActivity(intent);

    }

    public boolean isAppRun(String pName) {
        Context context = MyActivity.context;

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

    /**
     * 方法描述：判断某一应用是否正在运行
     * Created by cafeting on 2017/2/4.
     *
     * @param context     上下文
     * @param packageName 应用的包名
     * @return true 表示正在运行，false 表示没有运行
     */
    public static boolean isAppRunning(Context context, String packageName) {
        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
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
            ApplicationInfo applicationInfo = context.getPackageManager().getApplicationInfo(packageName, 0);
            if (applicationInfo != null) {
                Log.e("share", String.valueOf(applicationInfo.uid));
                return applicationInfo.uid;
            }
        } catch (Exception e) {
            return -1;
        }
        return -1;
    }

    /**
     * 判断某一 uid 的程序是否有正在运行的进程，即是否存活
     * Created by cafeting on 2017/2/4.
     *
     * @param context 上下文
     * @param uid     已安装应用的 uid
     * @return true 表示正在运行，false 表示没有运行
     */
    public static boolean isProcessRunning(Context context, int uid) {
        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningServiceInfo> runningServiceInfos = am.getRunningServices(200);
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
        if (language.endsWith("zh"))
            zh_cn = true;
        else
            zh_cn = false;

        return zh_cn;
    }

    public String readTextFile(String filename) {
        try {
            File file = new File(filename);
            StringBuffer strBuf = new StringBuffer();
            BufferedReader bufferedReader = new BufferedReader(
                    new InputStreamReader(new FileInputStream(file), "UTF-8"));
            int tempchar;
            while ((tempchar = bufferedReader.read()) != -1) {
                strBuf.append((char) tempchar);
            }
            bufferedReader.close();
            return strBuf.toString();
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        return null;
    }

    public void writeTextFile(String content, String filename) {
        try {
            File file = new File(filename);

            if (file.exists()) {
                file.delete();
            }
            file.createNewFile();
            // 获取该文件的缓冲输出流
            BufferedWriter bufferedWriter = new BufferedWriter(
                    new OutputStreamWriter(new FileOutputStream(file), "UTF-8"));
            // 写入信息
            bufferedWriter.write(content);
            bufferedWriter.flush();// 清空缓冲区
            bufferedWriter.close();// 关闭输出流
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    private void goExercise() {
        boolean isRun = isAppRun("com.x");

        if (!isRun) {
            try {
                File file = new File(shortcut_ini);
                if (!file.exists())
                    file.createNewFile();
                Wini ini = new Wini(file);

                ini.put("desk", "keyType", "exercise");
                ini.put("desk", "execDone", "false");

                ini.store();
            } catch (IOException e) {
                e.printStackTrace();
            }

            if (isZh(this))
                Toast.makeText(this, getString(R.string.strTip_zh), Toast.LENGTH_LONG).show();
            else
                Toast.makeText(this, getString(R.string.strTip), Toast.LENGTH_LONG).show();

            // reopen app
            openAppFromPackageName("com.x");

        } else {
            try {
                File file = new File(shortcut_ini);
                if (!file.exists())
                    file.createNewFile();
                Wini ini = new Wini(file);

                ini.put("desk", "keyType", "exercise");
                ini.put("desk", "execDone", "true");

                ini.store();
            } catch (IOException e) {
                e.printStackTrace();
            }

            CallJavaNotify_8();
        }
    }

}
