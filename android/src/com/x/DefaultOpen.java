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
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.database.Cursor;
import android.graphics.Color;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Bundle;
import android.os.Environment;
import android.os.FileObserver;
import android.os.Handler;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.provider.MediaStore.Images;
import android.provider.MediaStore.Images.Media;
import android.provider.OpenableColumns;
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
import android.webkit.MimeTypeMap;
import android.widget.Button;
import android.widget.TextView;
import android.widget.TextView;
import android.widget.TextView;
import android.widget.Toast;
import com.x.FileUtils;
import com.x.GetRealPath;
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
import java.nio.file.Files;
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

public class DefaultOpen extends Activity {

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

    private void goDefaultOpen() {
        Intent intent = getIntent();
        uri = intent.getData();

        if (uri != null) {
            String filePath = "";
            String strUri = Uri.decode(uri.toString());

            // 适配 MIUI 文件管理器
            if (
                "content".equals(uri.getScheme()) &&
                "com.mi.android.globalFileexplorer.myprovider".equals(
                    uri.getAuthority()
                )
            ) {
                filePath = handleMiuiFileManagerUri(uri);
            }
            // 适配 LineageOS / 原生安卓文件管理器
            else if ("content".equals(uri.getScheme())) {
                filePath = getFilePathForAndroid11Plus(context, uri);
            }
            // 适配 file:// 类型
            else if (strUri.contains("file://")) {
                filePath = strUri;
            }
            // 兜底
            else {
                filePath = GetRealPath.getFilePathFromContentUri(context, uri);
            }

            // ========== 防崩溃核心（必加） ==========
            if (filePath == null) {
                filePath = "";
            }
            filePath = filePath.replace("file://", "");

            System.err.println("最终解析路径: " + filePath);

            // 如果最终路径为空，直接关闭页面，避免崩溃
            if (filePath.isEmpty()) {
                DefaultOpen.this.finish();
                return;
            }

            File file_path = new File(filePath);
            if (file_path.exists()) {
                Toast.makeText(
                    context,
                    "已打开：" + filePath,
                    Toast.LENGTH_LONG
                ).show();

                String filename = "/storage/emulated/0/.Knot/choice_book.ini";
                try {
                    File file = new File(filename);
                    if (!file.exists()) file.createNewFile();
                    Wini ini = new Wini(file);
                    ini.put("book", "file", filePath);
                    ini.put("book", "type", "defaultopen");
                    ini.store();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            } else {
                DefaultOpen.this.finish();
            }
        } else {
            DefaultOpen.this.finish();
        }

        boolean isRun = MyService.isReady;

        if (!isRun) {
            try {
                File file = new File(shortcut_ini);
                if (!file.exists()) file.createNewFile();
                Wini ini = new Wini(file);
                ini.put("desk", "keyType", "defaultopen");
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

            openAppFromPackageName("com.x");
        } else {
            try {
                File file = new File(shortcut_ini);
                if (!file.exists()) file.createNewFile();
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
        Cursor cursor = context
            .getContentResolver()
            .query(uri, null, null, null, null);
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
        String[] filePathColumn = {
            MediaStore.MediaColumns.DATA,
            MediaStore.MediaColumns.DISPLAY_NAME,
        };
        ContentResolver contentResolver = context.getContentResolver();
        Cursor cursor = contentResolver.query(
            uri,
            filePathColumn,
            null,
            null,
            null
        );
        if (cursor != null) {
            cursor.moveToFirst();
            try {
                filePath = cursor.getString(
                    cursor.getColumnIndex(filePathColumn[0])
                );
                return filePath;
            } catch (Exception e) {
            } finally {
                cursor.close();
            }
        }
        return "";
    }

    // 新增方法：专门处理MIUI文件管理器的URI
    private String handleMiuiFileManagerUri(Uri uri) {
        List<String> segments = uri.getPathSegments();
        if (segments.size() > 1 && "external".equals(segments.get(0))) {
            // 构建真实路径: /storage/emulated/0/ + 剩余路径部分
            return (
                Environment.getExternalStorageDirectory() +
                "/" +
                TextUtils.join("/", segments.subList(1, segments.size()))
            );
        }
        return uri.getPath(); // 回退到普通处理
    }

    // 适配 LineageOS / 原生安卓 10+ 路径解析
    private String getFilePathForAndroid11Plus(Context context, Uri uri) {
        if (DocumentsContract.isDocumentUri(context, uri)) {
            if (
                "com.android.externalstorage.documents".equals(
                    uri.getAuthority()
                )
            ) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];

                if ("primary".equalsIgnoreCase(type)) {
                    return (
                        Environment.getExternalStorageDirectory() +
                        "/" +
                        split[1]
                    );
                }
            }
        }
        return null;
    }
}
