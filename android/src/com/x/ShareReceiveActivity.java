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
import android.os.AsyncTask;
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

public class ShareReceiveActivity extends Activity
        implements View.OnClickListener {
    private TextView tv;
    private Button btnAddToTodo;
    private Button btnAppendNote;
    private Button btnInsertNote;
    private Button btnFreePaste;
    public static String strData;
    private String share_ini = "/storage/emulated/0/.Knot/myshare.ini";

    private InternalConfigure internalConfigure;
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

    private String type;
    private String action;

    private String cursorText;
    private String strUri = "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        context = getApplicationContext();

        // 去除title(App Name)
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        setContentView(R.layout.activity_share_receive);
        tv = (TextView) findViewById(R.id.text_info);
        tv.setMovementMethod(ScrollingMovementMethod.getInstance());

        btnAddToTodo = (Button) findViewById(R.id.btnAddToTodo);
        btnAppendNote = (Button) findViewById(R.id.btnAppendNote);
        btnInsertNote = (Button) findViewById(R.id.btnInsertNote);
        btnFreePaste = (Button) findViewById(R.id.btnFreePaste);

        if (MyActivity.zh_cn) {
            btnAddToTodo.setText("增加到待办事项");
            btnAppendNote.setText("追加到当前笔记");
            btnInsertNote.setText("插入到当前笔记");
            btnFreePaste.setText("选择笔记并自由粘贴");
        } else {
            btnAddToTodo.setText("Add to Todo");
            btnAppendNote.setText("Append to Current Note");
            btnInsertNote.setText("Insert to Current Note");
            btnFreePaste.setText("Choose Notes and Paste Freely");
        }

        btnAddToTodo.setOnClickListener(this);
        btnAppendNote.setOnClickListener(this);
        btnInsertNote.setOnClickListener(this);
        btnFreePaste.setOnClickListener(this);

        String file2 = "/storage/emulated/0/.Knot/cursor_text.txt";
        cursorText = readTextFile(file2);

        // 获取intent
        Intent intent = getIntent();
        action = intent.getAction();
        type = intent.getType();
        System.out.println("type=" + type + "  action=" + action);

        tv.setText(type + "\n\n" + action + "\n\nSorry, this feature is not currently supported.");

        if (type.startsWith("image/")) {
            btnAddToTodo.setVisibility(View.GONE);
            btnFreePaste.setVisibility(View.GONE);
            btnAppendNote.setEnabled(false);
            btnInsertNote.setEnabled(false);
        }

        // 接收单个文件
        if (Intent.ACTION_SEND.equals(action) && type != null) {
            if ("text/plain".equals(type)) {
                handlerText(intent);
                System.out.println("strData=" + strData);
                tv.setText(type + "\n\n" + action + "\n\ncursor pos: " + cursorText + "\n\n" + strData);
                setInsertFlag();

            } else

            if (type.startsWith("image/")) {
                MyAsyncTask myAsyncTask = new MyAsyncTask();
                myAsyncTask.execute();

            } else

            if (type.startsWith("*/*") || type.startsWith("application/")) {
                Toast.makeText(this, "Sorry, this feature is not currently supported.", 9000).show();
                ShareReceiveActivity.this.finish();
            }
        }

        // 接收多个文件
        if (Intent.ACTION_SEND_MULTIPLE.equals(action) && type != null) {
            if (type.startsWith("image/")) {
                MyAsyncTask myAsyncTask = new MyAsyncTask();
                myAsyncTask.execute();

            } else

            if (type.startsWith("*/*") || type.startsWith("application/")) {
                Toast.makeText(this, "Sorry, this feature is not currently supported.", 9000).show();
                ShareReceiveActivity.this.finish();
            }
        }

        // HomeKey
        registerReceiver(mHomeKeyEvent, new IntentFilter(Intent.ACTION_CLOSE_SYSTEM_DIALOGS));

    }

    private BroadcastReceiver mHomeKeyEvent = new BroadcastReceiver() {
        String SYSTEM_REASON = "reason";
        String SYSTEM_HOME_KEY = "homekey";
        String SYSTEM_HOME_KEY_LONG = "recentapps";

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_CLOSE_SYSTEM_DIALOGS)) {
                String reason = intent.getStringExtra(SYSTEM_REASON);
                if (TextUtils.equals(reason, SYSTEM_HOME_KEY)) {
                    // 表示按了home键,程序直接进入到后台
                    System.out.println("NoteEditor HOME键被按下...");

                    onBackPressed();
                } else if (TextUtils.equals(reason, SYSTEM_HOME_KEY_LONG)) {
                    // 表示长按home键,显示最近使用的程序
                    System.out.println("NoteEditor 长按HOME键...");

                    onBackPressed();
                }
            }
        }
    };

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnAddToTodo:
                btnAddToTodo.setBackgroundColor(getResources().getColor(R.color.red));
                NoteEditor.closeNoteEditorView();
                try {
                    File file = new File(share_ini);
                    if (!file.exists())
                        file.createNewFile();
                    Wini ini = new Wini(file);
                    ini.put("share", "method", "todo");
                    ini.store();
                } catch (IOException e) {
                    e.printStackTrace();
                }

                if (type.startsWith("text/")) {
                    goReceiveString();
                }

                onBackPressed();
                break;

            case R.id.btnAppendNote:
                btnAppendNote.setBackgroundColor(getResources().getColor(R.color.red));
                NoteEditor.closeNoteEditorView();
                try {
                    File file = new File(share_ini);
                    if (!file.exists())
                        file.createNewFile();
                    Wini ini = new Wini(file);
                    ini.put("share", "method", "appendNote");
                    ini.store();
                } catch (IOException e) {
                    e.printStackTrace();
                }

                if (type.startsWith("text/")) {
                    goReceiveString();
                }

                if (type.startsWith("image/")) {
                    goReceiveImage();
                }

                onBackPressed();
                break;

            case R.id.btnInsertNote:
                btnInsertNote.setBackgroundColor(getResources().getColor(R.color.red));
                NoteEditor.closeNoteEditorView();
                try {
                    File file = new File(share_ini);
                    if (!file.exists())
                        file.createNewFile();
                    Wini ini = new Wini(file);
                    ini.put("share", "method", "insertNote");
                    ini.store();
                } catch (IOException e) {
                    e.printStackTrace();
                }

                if (type.startsWith("text/")) {
                    goReceiveString();
                }

                if (type.startsWith("image/")) {
                    goReceiveImage();
                }

                onBackPressed();
                break;

            case R.id.btnFreePaste:
                btnFreePaste.setBackgroundColor(getResources().getColor(R.color.red));
                NoteEditor.closeNoteEditorView();
                try {
                    File file = new File(share_ini);
                    if (!file.exists())
                        file.createNewFile();
                    Wini ini = new Wini(file);
                    ini.put("share", "method", "freePaste");
                    ini.store();
                } catch (IOException e) {
                    e.printStackTrace();
                }

                if (type.startsWith("text/")) {
                    goReceiveString();
                }

                if (type.startsWith("image/")) {
                    goReceiveImage();
                }

                onBackPressed();
                break;
        }
    }

    // 该方法用于获取intent所包含的文本信息，并显示到APP的Activity界面上
    public void handlerText(Intent intent) {
        String mainTxt = intent.getStringExtra(Intent.EXTRA_TEXT);
        String title = "";// = intent.getStringExtra(Intent.EXTRA_TITLE);
        if (title.length() > 0)
            strData = title + "\n\n" + mainTxt;
        else
            strData = mainTxt;

    }

    void dealPicStream(Intent intent) {
        Uri uri = intent.getParcelableExtra(Intent.EXTRA_STREAM);
    }

    void dealMultiplePicStream(Intent intent) {
        ArrayList<Uri> arrayList = intent.getParcelableArrayListExtra(intent.EXTRA_STREAM);
    }

    void goReceiveString() {
        System.out.println("strData=" + strData);

        boolean isRun = MyService.isReady;// isAppRun("com.x");

        if (!isRun) {
            saveReceiveShare("text/plain", strData, "false");

            if (MyActivity.zh_cn)
                Toast.makeText(this, getString(R.string.strTip_zh), Toast.LENGTH_LONG).show();
            else
                Toast.makeText(this, getString(R.string.strTip), Toast.LENGTH_LONG).show();

            // reopen app
            openAppFromPackageName("com.x");

        } else {
            saveReceiveShare("text/plain", strData, "true");
            CallJavaNotify_5();
        }

    }

    void goReceiveImage() {
        boolean isRun = MyService.isReady;// isAppRun("com.x");

        if (!isRun) {
            saveReceiveShare("image/*", "", "false");

            if (MyActivity.zh_cn)
                Toast.makeText(this, getString(R.string.strTip_zh), Toast.LENGTH_LONG).show();
            else
                Toast.makeText(this, getString(R.string.strTip), Toast.LENGTH_LONG).show();

            // reopen app
            openAppFromPackageName("com.x");

        } else {
            saveReceiveShare("image/*", "", "true");
            CallJavaNotify_5();

        }

    }

    void saveReceiveShare(String shareType, String strData, String shareDone) {
        try {
            File file = new File(share_ini);
            if (!file.exists())
                file.createNewFile();
            Wini ini = new Wini(file);

            ini.put("share", "shareType", shareType);
            ini.put("share", "shareDone", shareDone);

            ini.store();
        } catch (IOException e) {
            e.printStackTrace();
        }

        String filename = "/storage/emulated/0/.Knot/share_text.txt";
        writeTextFile(strData, filename);

    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();

        ShareReceiveActivity.this.finish();
    }

    @Override
    protected void onDestroy() {
        System.out.println("onDestroy...");
        unregisterReceiver(mHomeKeyEvent);
        // android.os.Process.killProcess(android.os.Process.myPid());
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

    public class InternalConfigure {
        private final Context context;
        private Properties properties;

        public InternalConfigure(Context context) {
            super();
            this.context = context;
        }

        /**
         * 保存文件filename为文件名，filecontent为存入的文件内容
         * 例:configureActivity.saveFiletoSD("text.ini","");
         */
        public void saveFile(String filename, Properties properties) throws Exception {
            // 设置Context.MODE_PRIVATE表示每次调用该方法会覆盖原来的文件数据
            FileOutputStream fileOutputStream;// = context.openFileOutput(filename, Context.MODE_PRIVATE);
            File file = new File(filename);
            fileOutputStream = new FileOutputStream(file);
            // 通过properties.stringPropertyNames()获得所有key的集合Set，里面是String对象
            for (String key : properties.stringPropertyNames()) {
                String s = key + " = " + properties.getProperty(key) + "\n";
                System.out.println(s);
                fileOutputStream.write(s.getBytes());
            }
            fileOutputStream.close();
        }

        /**
         * 读取文件
         */
        public void readFrom(String filename) throws Exception {
            properties = new Properties();

            FileInputStream fileInputStream;// = context.openFileInput(filename);

            File file = new File(filename);
            fileInputStream = new FileInputStream(file);

            InputStreamReader reader = new InputStreamReader(fileInputStream, "UTF-8");
            BufferedReader br = new BufferedReader(reader);

            // debug:
            // String line;
            // while ((line = br.readLine()) != null) {
            // System.out.println(line);
            // }

            properties.load(br);

            br.close();
            reader.close();
            fileInputStream.close();
        }

        /**
         * 返回指定key对应的value
         */
        public String getIniKey(String key) {
            if (properties.containsKey(key) == false) {
                return null;
            }
            return String.valueOf(properties.get(key));
        }
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

    // 读取分享的文件并把文件导入到私有目录
    public boolean readFileFromShare() {
        Intent intent = getIntent();
        String type = intent.getType();
        ArrayList<Uri> uris = new ArrayList<Uri>();
        if (intent.getAction().equalsIgnoreCase(Intent.ACTION_SEND)) {
            Uri uri0 = intent.getParcelableExtra(Intent.EXTRA_STREAM);
            uris.add(uri0);
        }

        if (Intent.ACTION_SEND_MULTIPLE.equals(action)) {
            uris = intent.getParcelableArrayListExtra(Intent.EXTRA_STREAM);
        }

        String filename = null;
        for (int i = 0; i < uris.size(); i++) {
            Uri uri = uris.get(i);
            strUri = strUri + "\n\n" + uri;
            filename = "/storage/emulated/0/.Knot/img" + String.valueOf(i) + ".png";

            if (uri != null) {
                try {
                    File outFile = new File(filename);
                    InputStream inputStream = getContentResolver().openInputStream(uri);
                    FileOutputStream fos = new FileOutputStream(outFile);
                    byte[] buf = new byte[1024];
                    int readCount = 0;
                    while ((readCount = inputStream.read(buf)) != -1) {
                        fos.write(buf, 0, readCount);
                    }
                    fos.flush();
                    inputStream.close();
                    fos.close();

                } catch (IOException e) {
                    e.printStackTrace();
                    return false;
                }
            }
        }

        try {
            File file = new File(share_ini);
            if (!file.exists())
                file.createNewFile();
            Wini ini = new Wini(file);
            ini.put("share", "imgCount", uris.size());
            ini.store();
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }

        return true;
    }

    public class MyAsyncTask extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            // 在这里执行需要等待的操作
            readFileFromShare();

            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            // 在这里执行完成后的操作
            tv.setText(type + "\n\n" + action + "\n\ncursor pos: " + cursorText + "\n\n" + strUri);
            setInsertFlag();
            btnAppendNote.setEnabled(true);
            btnInsertNote.setEnabled(true);

        }
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

    private void setInsertFlag() {
        // Set insert flag
        String strTV = tv.getText().toString();
        int indexTV = strTV.indexOf(cursorText);
        System.out.println("strTV=" + strTV + " cursorText=" + cursorText + "  indexTV=" + String.valueOf(indexTV));
        if (indexTV >= 0) {
            int start = indexTV;
            int end = indexTV + cursorText.length();
            SpannableStringBuilder style = new SpannableStringBuilder(strTV);
            style.setSpan(new BackgroundColorSpan(Color.parseColor("#FFE4C4")), start, end,
                    Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            style.setSpan(new ForegroundColorSpan(Color.parseColor("#000000")), start, end,
                    Spannable.SPAN_EXCLUSIVE_INCLUSIVE);

            indexTV = strTV.indexOf("|");
            start = indexTV;
            end = indexTV + 1;
            style.setSpan(new BackgroundColorSpan(Color.parseColor("#FFE4C4")), start, end,
                    Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            style.setSpan(new ForegroundColorSpan(Color.parseColor("#FF0000")), start, end,
                    Spannable.SPAN_EXCLUSIVE_INCLUSIVE);

            tv.setText(style);
        }
    }

}
