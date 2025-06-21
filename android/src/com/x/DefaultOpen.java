package com.x;

import org.ini4j.Wini;

import com.x.FileUtils;
import com.x.MyActivity;
import com.x.NoteEditor;
import com.x.GetRealPath;

import java.nio.file.Files;
import android.content.ComponentName;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;

import android.provider.DocumentsContract;
import android.os.Environment;
import android.webkit.MimeTypeMap;
import android.content.ContentUris;
import android.provider.OpenableColumns;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.net.Uri;
import android.os.FileObserver;
import android.database.Cursor;
import android.text.method.ScrollingMovementMethod;
import java.util.List;
import java.util.ArrayList;
import android.provider.MediaStore;
import android.provider.MediaStore.Images;
import android.provider.MediaStore.Images.Media;
import android.content.ContentResolver;
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

public class DefaultOpen extends Activity {

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
    private Uri uri;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        context = DefaultOpen.this;
        goDefaultOpen();

    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();

    }

    @Override
    protected void onDestroy() {
        System.out.println("onDestroy...");

        super.onDestroy();

    }

    public void openAppFromPackageName(String pname) {
        PackageManager packageManager = getPackageManager();
        Intent it = packageManager.getLaunchIntentForPackage(pname);
        startActivity(it);
    }

    public boolean isAppRun(String pName) {
        Context context = MyActivity.mycontext;

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

    private void goDefaultOpen() {
        Intent intent = getIntent();
        uri = intent.getData();

        if (uri != null) {
            String filePath = "";
            String strUri = Uri.decode(uri.toString());
            if (strUri.contains("file://"))
                filePath = strUri;
            else
                // filePath = getFileFromContentUri(context, uri);
                filePath = GetRealPath.getFilePathFromContentUri(context, uri);

            filePath = filePath.replace("file://", "");

            System.err.println(uri + "    path:" + filePath);

            File file_path = new File(filePath);
            if (file_path.exists()) {

                Toast.makeText(context, filePath, Toast.LENGTH_LONG).show();

                String filename = "/storage/emulated/0/.Knot/choice_book.ini";
                try {
                    File file = new File(filename);
                    if (!file.exists())
                        file.createNewFile();
                    Wini ini = new Wini(file);
                    ini.put("book", "file", filePath);
                    ini.put("book", "type", "defaultopen");
                    ini.store();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            } else
                DefaultOpen.this.finish();

        } else
            DefaultOpen.this.finish();

        boolean isRun = isAppRun("com.x");

        if (!isRun) {
            try {
                File file = new File(shortcut_ini);
                if (!file.exists())
                    file.createNewFile();
                Wini ini = new Wini(file);

                ini.put("desk", "keyType", "defaultopen");
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

                ini.put("desk", "keyType", "defaultopen");
                ini.put("desk", "execDone", "true");

                ini.store();
            } catch (IOException e) {
                e.printStackTrace();
            }

            CallJavaNotify_9();
        }

        DefaultOpen.this.finish();
    }

    // 非媒体文件中查找
    private static String getFilePathForNonMediaUri(Context context, Uri uri) {
        String filePath = "";
        Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
        if (cursor != null) {
            if (cursor.moveToFirst()) {
                int columnIndex = cursor.getColumnIndexOrThrow("_data");
                filePath = cursor.getString(columnIndex);
            }
            cursor.close();
        }
        return filePath;
    }

    /**
     * Android 10 以上适配 另一种写法
     * 
     * @param context
     * @param uri
     * @return
     */
    private static String getFileFromContentUri(Context context, Uri uri) {
        if (uri == null) {
            return null;
        }
        String filePath;
        String[] filePathColumn = { MediaStore.MediaColumns.DATA, MediaStore.MediaColumns.DISPLAY_NAME };
        // String[] filePathColumn = { MediaStore.DownloadColumns.DATA,
        // MediaStore.DownloadColumns.DISPLAY_NAME };
        ContentResolver contentResolver = context.getContentResolver();
        Cursor cursor = contentResolver.query(uri, filePathColumn, null,
                null, null);
        if (cursor != null) {
            cursor.moveToFirst();
            try {
                filePath = cursor.getString(cursor.getColumnIndex(filePathColumn[0]));
                return filePath;
            } catch (Exception e) {
            } finally {
                cursor.close();
            }
        }
        return "";
    }

}
