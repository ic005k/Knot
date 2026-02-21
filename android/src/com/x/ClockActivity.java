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
import java.lang.ref.WeakReference;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Properties;
import org.ini4j.Wini;

public class ClockActivity
    extends Activity
    implements View.OnClickListener, Application.ActivityLifecycleCallbacks
{

    private MediaPlayer player;

    private int mOriginalRingerMode; // 保存原始铃声模式

    private static String strInfo =
        "Todo|There are currently timed tasks pending.|0|Close";

    private AudioManager mAudioManager;
    private InternalConfigure internalConfigure;

    private Button btn_cancel;
    private Button btn_play_voice;
    public TextView text_info, text_title;
    private String voiceFile;

    // 静态弱引用（泛型指定 ClockActivity，初始化为 null）
    private static WeakReference<ClockActivity> m_instance_weak;
    private boolean isHomeKey = false;

    private boolean isReady = false; // 非静态成员变量

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
        overridePendingTransition(0, R.anim.exit_anim);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        m_instance_weak = new WeakReference<>(this);

        Application application = this.getApplication();
        application.registerActivityLifecycleCallbacks(this);
        mAudioManager = (AudioManager) getApplicationContext().getSystemService(
            Service.AUDIO_SERVICE
        );

        // 保存用户原始铃声模式
        mOriginalRingerMode = mAudioManager.getRingerMode();

        // 去除title
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        if (MyActivity.isDark) {
            this.setStatusBarColor("#19232D");
            getWindow()
                .getDecorView()
                .setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
            setContentView(R.layout.activity_clock_dark);
        } else {
            this.setStatusBarColor("#F3F3F3");
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

        // 读取Intent参数
        Intent intent = getIntent();
        if (intent != null && intent.hasExtra("SAVED_CLOCK_CONTENT")) {
            String savedContent = intent.getStringExtra("SAVED_CLOCK_CONTENT");
            if (!TextUtils.isEmpty(savedContent)) {
                this.strInfo = savedContent;
            } else {
                this.strInfo = MyService.strTodoAlarm;
            }
        } else {
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

        CallJavaNotify_3();

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
                // TTS逻辑
                String filename = "/storage/emulated/0/.Knot/msg.ini";
                internalConfigure = new InternalConfigure(this);
                try {
                    File iniFile = new File(filename);
                    if (iniFile.exists()) {
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

        // 注册Home键广播
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
                    isHomeKey = true;
                    System.out.println("ClockActivity HOME键被按下...");
                } else if (TextUtils.equals(reason, SYSTEM_HOME_KEY_LONG)) {
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
        restoreOriginalRingerMode();
        Log.d("ClockActivity", "MIUI 适配：onPause 还原铃声模式");
    }

    @Override
    public void onStop() {
        System.out.println("ClockActivity onStop...");
        super.onStop();
    }

    @Override
    public void onBackPressed() {
        MyActivity.stopPlayMyText();

        // 停止MediaPlayer
        if (player != null && player.isPlaying()) {
            player.stop();
            player.reset();
        }

        restoreOriginalRingerMode();

        AnimationWhenClosed();

        if (!MyActivity.isBackMainUI) {
            MyActivity.setMini();
        }

        super.onBackPressed();
    }

    @Override
    protected void onDestroy() {
        isReady = false;

        unregisterReceiver(mHomeKeyEvent);

        MyService.clearNotify(getApplicationContext());

        CallJavaNotify_4();

        // 释放MediaPlayer
        if (player != null) {
            if (player.isPlaying()) {
                player.stop();
            }
            player.release();
            player = null;
        }

        restoreOriginalRingerMode();

        // 注销生命周期回调
        Application application = this.getApplication();
        application.unregisterActivityLifecycleCallbacks(this);

        super.onDestroy();

        // 清空恢复标记
        synchronized (MyActivity.clockLock) {
            if (MyActivity.isNeedRestoreClock) {
                MyActivity.isNeedRestoreClock = false;
                MyActivity.savedClockContent = "";
                Log.d("ClockActivity", "正常关闭，清空恢复标记");
            }
        }

        // 释放弱引用
        if (m_instance_weak != null) {
            m_instance_weak.clear();
            m_instance_weak = null;
        }

        text_info = null;
        text_title = null;

        System.out.println("ClockActivity onDestroy...");
    }

    public static void close() {
        ClockActivity instance =
            m_instance_weak != null ? m_instance_weak.get() : null;
        if (
            instance != null &&
            !instance.isFinishing() &&
            !instance.isDestroyed()
        ) {
            instance.finish();
        }
    }

    public class InternalConfigure {

        private final Context context;
        private Properties properties;

        public InternalConfigure(Context context) {
            super();
            this.context = context;
            this.properties = new Properties();
        }

        public void saveFile(String filename, Properties properties)
            throws Exception {
            FileOutputStream fileOutputStream;
            File file = new File(filename);
            fileOutputStream = new FileOutputStream(file);
            for (String key : properties.stringPropertyNames()) {
                String s = key + " = " + properties.getProperty(key) + "\n";
                System.out.println(s);
                fileOutputStream.write(s.getBytes());
            }
            fileOutputStream.close();
        }

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

        public String getIniKey(String key) {
            if (properties.containsKey(key) == false) {
                return null;
            }
            return String.valueOf(properties.get(key));
        }
    }

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

    public int getMediaMaxVolume() {
        return mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
    }

    public int getMediaVolume() {
        return mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
    }

    public void setMediaVolume(int volume) {
        mAudioManager.setStreamVolume(
            AudioManager.STREAM_MUSIC,
            volume,
            AudioManager.FLAG_PLAY_SOUND | AudioManager.FLAG_SHOW_UI
        );
    }

    private void setStatusBarColor(String color) {
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
        // 释放旧的MediaPlayer
        if (player != null) {
            if (player.isPlaying()) {
                player.stop();
            }
            player.reset();
            player.release();
            player = null;
        }
        if (TextUtils.isEmpty(outputFile)) return;
        try {
            player = new MediaPlayer();
            player.setDataSource(outputFile);
            player.prepare();
            player.start();
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

    private void updateTextInternal(String message) {
        try {
            if (text_info == null) {
                Log.w("ClockActivity", "text_info控件未初始化，跳过更新");
                return;
            }

            String oldTxt = text_info.getText().toString();
            SimpleDateFormat formatter = new SimpleDateFormat(
                "yyyy-MM-dd HH:mm:ss"
            );
            Date date = new Date(System.currentTimeMillis());
            String strCurDT0 = formatter.format(date);
            String strCurDT = " ( " + strCurDT0 + " ) ";
            String newTxt = message + "\n" + strCurDT + "\n\n" + oldTxt;

            text_info.setText(newTxt);
            Log.d("ClockActivity", "UI文本更新成功：" + message);
        } catch (Exception e) {
            Log.e("ClockActivity", "UI更新异常", e);
        }
    }

    public static void safeUpdateTodoText(String message) {
        // 1. 修复错误1：通过实例访问非静态的isReady
        ClockActivity instance =
            m_instance_weak != null ? m_instance_weak.get() : null;
        if (instance == null || !instance.isReady) {
            // 关键修改：instance.isReady
            Log.d("ClockActivity", "实例未就绪或已被回收，跳过UI更新");
            return;
        }

        if (Looper.myLooper() != Looper.getMainLooper()) {
            instance.runOnUiThread(() -> instance.updateTextInternal(message));
            return;
        }
        instance.updateTextInternal(message);
    }

    public String getStrInfo() {
        return this.strInfo;
    }

    private void restoreOriginalRingerMode() {
        if (mAudioManager != null) {
            if (mAudioManager.getRingerMode() != mOriginalRingerMode) {
                mAudioManager.setRingerMode(mOriginalRingerMode);
                Log.d(
                    "ClockActivity",
                    "已还原铃声模式为：" + mOriginalRingerMode
                );
            }
        }
    }

    public static ClockActivity getInstance() {
        if (m_instance_weak == null) {
            return null;
        }
        ClockActivity instance = m_instance_weak.get();
        // 增加实例有效性判断
        if (
            instance == null || instance.isFinishing() || instance.isDestroyed()
        ) {
            return null;
        }
        return instance;
    }

    // 提供公共的静态方法，让外部类（如MyService）获取isReady状态
    public static boolean isClockActivityReady() {
        ClockActivity instance = getInstance();
        return instance != null && instance.isReady;
    }
}
