package com.x;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Application;
import android.app.PendingIntent;
import android.app.Service;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.TextView;
import com.x.MyActivity;
import com.x.NoteEditor;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Locale;
import java.util.Locale;
import java.util.Map;
import java.util.Properties;
import org.ini4j.Wini;

public class ClockActivity
    extends Activity
    implements View.OnClickListener, Application.ActivityLifecycleCallbacks {

    private MediaPlayer player;

    private static String strInfo =
        "Todo|There are currently timed tasks pending.|0|Close";

    private boolean isRefreshAlarm = true;
    private AudioManager mAudioManager;
    private InternalConfigure internalConfigure;

    private Button btn_cancel;
    private Button btn_play_voice;
    public TextView text_info, text_title;
    private String voiceFile;

    private static Context context;
    private static ClockActivity m_instance;
    private boolean isHomeKey = false;

    public static Context getContext() {
        return context;
    }

    public static volatile boolean isReady = false;

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

    private void bindViews(String strTitle, String str) {
        text_title = (TextView) findViewById(R.id.text_title);
        text_title.setText(strTitle);

        text_info = (TextView) findViewById(R.id.text_info);
        text_info.setText(str);

        btn_cancel = (Button) findViewById(R.id.btn_cancel);
        if (MyActivity.zh_cn) btn_cancel.setText("关闭");
        else btn_cancel.setText("Close");
        btn_cancel.setOnClickListener(this);

        btn_play_voice = (Button) findViewById(R.id.btn_play_voice);
        if (MyActivity.zh_cn) btn_play_voice.setText("播放语音");
        else btn_play_voice.setText("Play Voice");
        btn_play_voice.setOnClickListener(this);
        btn_play_voice.setVisibility(View.GONE);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_cancel:
                String iniFile = "/storage/emulated/0/.Knot/alarm.ini";
                try {
                    File file = new File(iniFile);
                    File parentDir = file.getParentFile();
                    if (!parentDir.exists()) {
                        parentDir.mkdirs(); // 创建多级父目录
                    }
                    if (!file.exists()) file.createNewFile();
                    Wini ini = new Wini(file);
                    ini.put("action", "backMain", "true");
                    ini.store();
                } catch (IOException e) {
                    e.printStackTrace();
                }

                MyActivity.isBackMainUI = true;

                onBackPressed();
                break;
            case R.id.btn_play_voice:
                playRecord(voiceFile);
                break;
        }
    }

    private void AnimationWhenClosed() {
        // 淡出效果
        // overridePendingTransition(android.R.anim.fade_in, android.R.anim.fade_out);

        // 或者使用底部滑出效果(自定义文件exit_anim.xml)
        overridePendingTransition(0, R.anim.exit_anim);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        context = getApplicationContext();

        m_instance = this;

        MyActivity.alarmWindows.add(m_instance);

        Application application = this.getApplication();
        application.registerActivityLifecycleCallbacks(this);
        mAudioManager = (AudioManager) context.getSystemService(
            Service.AUDIO_SERVICE
        );

        // 去除title(App Name)
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        if (MyActivity.isDark) {
            this.setStatusBarColor("#19232D"); // 深色
            getWindow()
                .getDecorView()
                .setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
            setContentView(R.layout.activity_clock_dark);
        } else {
            this.setStatusBarColor("#F3F3F3"); // 灰
            getWindow()
                .getDecorView()
                .setSystemUiVisibility(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
            setContentView(R.layout.activity_clock);
        }

        SimpleDateFormat formatter = new SimpleDateFormat(
            "yyyy-MM-dd HH:mm:ss"
        );
        Date date = new Date(System.currentTimeMillis());
        String strCurDT0 = formatter.format(date);
        String strCurDT = " ( " + strCurDT0 + " ) ";

        // 原有代码
        // strInfo = MyService.strTodoAlarm;

        // 替换为（条件分支，不影响原有逻辑）
        Intent intent = getIntent();
        if (intent != null && intent.hasExtra("SAVED_CLOCK_CONTENT")) {
            // 仅在「切换主题恢复打开」时生效（有传参）
            String savedContent = intent.getStringExtra("SAVED_CLOCK_CONTENT");
            if (!TextUtils.isEmpty(savedContent)) {
                this.strInfo = savedContent;
            } else {
                // 无有效保存内容，仍走原有逻辑
                this.strInfo = MyService.strTodoAlarm;
            }
        } else {
            // 日常正常打开ClockActivity（无传参），完全走原有逻辑
            this.strInfo = MyService.strTodoAlarm;
        }

        System.out.println("Info Text: " + strInfo);

        String[] array = strInfo.split("\\|");
        String str1 = array.length > 0 ? array[0] : "Todo";
        String str2 = array.length > 1 ? array[1] : "No content";
        String str3 = array.length > 3 ? array[3] : "Close";
        String strTodo;
        if (MyActivity.zh_cn) strTodo = "待办事项：\n";
        else strTodo = "Todo: \n";

        bindViews(str1, str2 + "\n" + strCurDT + "\n");

        if (isRefreshAlarm) {
            CallJavaNotify_3();
        }

        String mystr = str2;
        boolean isVoice = false;
        String[] the_splic = mystr.split(" ");

        if (the_splic[0] != null) {
            if (the_splic[0].equals("语音") || the_splic[0].equals("Voice")) {
                isVoice = true;
            }

            if (isVoice) {
                String[] _splic = mystr.split("\n");
                voiceFile =
                    "/storage/emulated/0/KnotData/memo/voice/" + _splic[1];
                playRecord(voiceFile);
                btn_play_voice.setVisibility(View.VISIBLE);
            } else {
                // TTS
                String filename = "/storage/emulated/0/.Knot/msg.ini";
                internalConfigure = new InternalConfigure(this);
                try {
                    File iniFile = new File(filename);
                    if (iniFile.exists()) {
                        // 先判断文件存在
                        internalConfigure.readFrom(filename);
                    } else {
                        Log.w("ClockActivity", "msg.ini不存在，使用默认配置");
                    }
                } catch (Exception e) {
                    System.err.println("Error : reading msg.ini");
                    e.printStackTrace();
                }
                String strVoice = internalConfigure.getIniKey("voice");
                String strValue = "true";
                if (strVoice != null && strVoice.equals(strValue)) {
                    MyActivity.stopPlayMyText();
                    MyActivity.playMyText(str2);
                    System.out.println("播放文本：" + str2);
                }
            }
        }

        System.out.println("闹钟已开始+++++++++++++++++++++++");

        // HomeKey
        registerReceiver(
            mHomeKeyEvent,
            new IntentFilter(Intent.ACTION_CLOSE_SYSTEM_DIALOGS)
        );

        isReady = true;
        MyActivity.isBackMainUI = false;
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
                    isHomeKey = true;

                    System.out.println("ClockActivity HOME键被按下...");
                } else if (TextUtils.equals(reason, SYSTEM_HOME_KEY_LONG)) {
                    // 表示长按home键,显示最近使用的程序
                    isHomeKey = true;

                    System.out.println("ClockActivity 长按HOME键...");
                }
            }
        }
    };

    @Override
    public void onPause() {
        System.out.println("ClockActivity onPause...");
        super.onPause();
    }

    @Override
    public void onStop() {
        System.out.println("ClockActivity onStop...");

        super.onStop();
    }

    @Override
    public void onBackPressed() {
        MyActivity.stopPlayMyText();

        // 安全停止MediaPlayer：判断状态+重置
        if (player != null && player.isPlaying()) {
            player.stop();
            player.reset(); // 重置状态，避免后续操作异常
        }

        AnimationWhenClosed();

        if (!MyActivity.isBackMainUI) {
            MyActivity.setMini();
        }

        super.onBackPressed();
    }

    @Override
    protected void onDestroy() {
        isReady = false;

        if (!isHomeKey) MyActivity.alarmWindows.remove(m_instance);

        unregisterReceiver(mHomeKeyEvent);

        MyService.clearNotify(getApplicationContext()); // 传入上下文

        if (!isRefreshAlarm) {
            android.os.Process.killProcess(android.os.Process.myPid());
        } else {
            CallJavaNotify_4();
        }

        // 释放MediaPlayer（补充完整状态判断）
        if (player != null) {
            if (player.isPlaying()) {
                player.stop();
            }
            player.release();
            player = null;
        }

        // 注销生命周期回调
        Application application = this.getApplication();
        application.unregisterActivityLifecycleCallbacks(this);

        super.onDestroy();

        // 若不是「主题切换关闭」（即正常关闭），清空恢复标记
        synchronized (MyActivity.clockLock) {
            // 检查当前是否是“需要恢复”的状态，且当前窗口是被正常关闭（而非主题切换）
            if (MyActivity.isNeedRestoreClock) {
                // 正常关闭时，清空标记（避免误恢复）
                MyActivity.isNeedRestoreClock = false;
                MyActivity.savedClockContent = "";
                Log.d("ClockActivity", "正常关闭，清空恢复标记");
            }
        }

        m_instance = null; // 释放静态引用
        text_info = null; // 释放静态控件引用
        text_title = null;

        System.out.println("ClockActivity onDestroy...");
    }

    public static void close() {
        if (m_instance != null) {
            m_instance.finish();
        }
    }

    public class InternalConfigure {

        private final Context context;
        private Properties properties;

        public InternalConfigure(Context context) {
            super();
            this.context = context;
            this.properties = new Properties(); // 提前初始化，避免null
        }

        /**
         * 保存文件filename为文件名，filecontent为存入的文件内容
         * 例:configureActivity.saveFiletoSD("text.ini","");
         */
        public void saveFile(String filename, Properties properties)
            throws Exception {
            // 设置Context.MODE_PRIVATE表示每次调用该方法会覆盖原来的文件数据
            FileOutputStream fileOutputStream; // = context.openFileOutput(filename, Context.MODE_PRIVATE);
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
            FileInputStream fileInputStream;

            File file = new File(filename);
            fileInputStream = new FileInputStream(file);

            InputStreamReader reader = new InputStreamReader(
                fileInputStream,
                "UTF-8"
            );
            BufferedReader br = new BufferedReader(reader);

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

    // fileName 为文件名称 返回true为存在
    public boolean fileIsExists(String fileName) {
        try {
            File f = new File(fileName);
            if (f.exists()) {
                Log.i("测试", "文件存在：" + fileName);
                return true;
            } else {
                Log.i("测试", "没有这个文件: " + fileName);
                return false;
            }
        } catch (Exception e) {
            Log.i("测试", "出现异常！");
            e.printStackTrace();
            return false;
        }
    }

    // 获取最大多媒体音量
    public int getMediaMaxVolume() {
        return mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
    }

    // 获取当前多媒体音量
    public int getMediaVolume() {
        return mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
    }

    // 设置多媒体音量
    public void setMediaVolume(int volume) {
        mAudioManager.setStreamVolume(
            AudioManager.STREAM_MUSIC, // 音量类型
            volume,
            AudioManager.FLAG_PLAY_SOUND | AudioManager.FLAG_SHOW_UI
        );
    }

    private void setStatusBarColor(String color) {
        // 需要安卓版本大于5.0以上
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            getWindow().addFlags(
                WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS
            );
            getWindow().setStatusBarColor(Color.parseColor(color));
        }
    }

    @Override
    public void onActivityCreated(
        Activity activity,
        Bundle savedInstanceState
    ) {}

    @Override
    public void onActivityStarted(Activity activity) {}

    @Override
    public void onActivityResumed(Activity activity) {}

    @Override
    public void onActivityPaused(Activity activity) {}

    @Override
    public void onActivityStopped(Activity activity) {
        System.out.println("ClockActivity onActivityStopped...");
    }

    @Override
    public void onActivitySaveInstanceState(
        Activity activity,
        Bundle outState
    ) {}

    @Override
    public void onActivityDestroyed(Activity activity) {}

    private void playRecord(String outputFile) {
        // 先释放旧实例，避免状态冲突
        if (player != null) {
            if (player.isPlaying()) {
                player.stop();
            }
            player.reset();
            player.release();
            player = null;
        }
        // 空值校验：避免传入null路径
        if (TextUtils.isEmpty(outputFile)) return;
        try {
            player = new MediaPlayer();
            player.setDataSource(outputFile);
            player.prepare();
            player.start();
            // 监听播放完成，自动释放资源
            player.setOnCompletionListener(
                new MediaPlayer.OnCompletionListener() {
                    @Override
                    public void onCompletion(MediaPlayer mp) {
                        mp.reset();
                    }
                }
            );
        } catch (Exception ex) {
            ex.printStackTrace();
            // 异常时确保释放资源
            if (player != null) {
                player.release();
                player = null;
            }
        }
    }

    private boolean isNumer(String str) {
        if (str == null) return false;
        for (char c : str.toCharArray()) {
            if (!Character.isDigit(c)) return false;
        }
        return true;
    }

    /**
     * 内部UI更新逻辑（仅内部调用，已做前置校验）
     */
    private void updateTextInternal(String message) {
        // 移除static
        try {
            // 3. 校验：text_info控件是否为空
            if (text_info == null) {
                Log.w("ClockActivity", "text_info控件未初始化，跳过更新");
                return;
            }

            // 拼接文本（复用原有逻辑）
            String oldTxt = text_info.getText().toString();
            SimpleDateFormat formatter = new SimpleDateFormat(
                "yyyy-MM-dd HH:mm:ss"
            );
            Date date = new Date(System.currentTimeMillis());
            String strCurDT0 = formatter.format(date);
            String strCurDT = " ( " + strCurDT0 + " ) ";
            String newTxt = message + "\n" + strCurDT + "\n\n" + oldTxt;

            // 安全更新文本
            text_info.setText(newTxt);
            Log.d("ClockActivity", "UI文本更新成功：" + message);
        } catch (Exception e) {
            Log.e("ClockActivity", "UI更新异常", e);
        }
    }

    public static void safeUpdateTodoText(String message) {
        // 1. 校验：ClockActivity实例是否存活
        if (m_instance == null || !isReady) {
            Log.d("ClockActivity", "实例未就绪，跳过UI更新");
            return;
        }

        // 2. 校验：是否在主线程（UI操作必须在主线程）
        if (Looper.myLooper() != Looper.getMainLooper()) {
            // 非主线程：通过Handler切换到主线程
            m_instance.runOnUiThread(() ->
                m_instance.updateTextInternal(message)
            ); // 改为m_instance调用
            return;
        }

        // 主线程：直接更新
        m_instance.updateTextInternal(message); // 改为m_instance调用
    }

    // 暴露strInfo给MyActivity（原有strInfo为private，需添加getter）
    public String getStrInfo() {
        return this.strInfo;
    }
}
