package com.x;

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

public class ClockActivity extends Activity implements View.OnClickListener, Application.ActivityLifecycleCallbacks {

    private MediaPlayer mediaPlayer;
    private static int curVol;
    private static String strInfo = "Todo|There are currently timed tasks pending.|0|Close";
    private static String strEnInfo = "Todo|There are currently timed tasks pending.|0|Close";
    private String strMute = "false";
    private boolean isRefreshAlarm = true;
    private AudioManager mAudioManager;
    private InternalConfigure internalConfigure;

    private Button btn_cancel;
    private TextView text_info;
    private static boolean zh_cn;

    private static Context context;
    private static ClockActivity m_instance;

    public static Context getContext() {
        return context;
    }

    public native static void CallJavaNotify_0();

    public native static void CallJavaNotify_1();

    public native static void CallJavaNotify_2();

    public native static void CallJavaNotify_3();

    public native static void CallJavaNotify_4();

    public static boolean isZh(Context context) {
        Locale locale = context.getResources().getConfiguration().locale;
        String language = locale.getLanguage();
        if (language.endsWith("zh"))
            zh_cn = true;
        else
            zh_cn = false;

        return zh_cn;
    }

    public static int setInfoText(String str) {
        strInfo = str;
        System.out.println("InfoText" + strInfo);
        return 1;
    }

    private void bindViews(String str) {
        text_info = (TextView) findViewById(R.id.text_info);
        text_info.setText(str);

        btn_cancel = (Button) findViewById(R.id.btn_cancel);
        if (zh_cn)
            btn_cancel.setText("关闭");
        else
            btn_cancel.setText("Close");
        btn_cancel.setOnClickListener(this);

    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_cancel:
                // ClockActivity.this.finish();
                // btn_cancel.setVisibility(View.GONE);

                onBackPressed();
                break;
        }
    }

    private void AnimationWhenClosed() {
        // 淡出效果
        //overridePendingTransition(android.R.anim.fade_in, android.R.anim.fade_out);

        // 或者使用底部滑出效果(自定义文件exit_anim.xml)
        overridePendingTransition(0, R.anim.exit_anim);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        context = getApplicationContext();
        //this.getWindow().setWindowAnimations(R.style.WindowAnim);
        isZh(context);
        m_instance = this;
        registerReceiver(mHomeKeyEvent, new IntentFilter(Intent.ACTION_CLOSE_SYSTEM_DIALOGS));
        Application application = this.getApplication();
        application.registerActivityLifecycleCallbacks(this);
        mAudioManager = (AudioManager) context.getSystemService(Service.AUDIO_SERVICE);
        curVol = getMediaVolume();

        String filename = "/data/data/com.x/files/msg.ini";
        internalConfigure = new InternalConfigure(this);
        try {

            internalConfigure.readFrom(filename);
        } catch (Exception e) {
            System.err.println("Error : reading msg.ini");
            e.printStackTrace();
        }

        //去除title(App Name)
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        //去掉Activity上面的状态栏(Show Time...)
        //getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        this.setStatusBarColor("#F3F3F3");  //灰
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);

        //SimpleDateFormat formatter = new SimpleDateFormat("yyyy-MM-dd 'at' HH:mm:ss z");
        SimpleDateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        Date date = new Date(System.currentTimeMillis());
        String strCurDT0 = formatter.format(date);
        String strCurDT = " ( " + strCurDT0 + " ) ";

        int maxVol = getMediaMaxVolume();
        strMute = internalConfigure.getIniKey("mute");
        System.out.println("Mute: " + strMute);
        double vol = 0;
        mediaPlayer = new MediaPlayer();
        if (strMute.equals("false")) {
            vol = maxVol * 0.75;
            setMediaVolume((int) Math.round(vol));

            try {
                mediaPlayer.setDataSource("/data/data/com.x/files/msg.mp3");
                mediaPlayer.prepare();
                mediaPlayer.start();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        System.out.println("maxVol:  " + maxVol + "    setvol:  " + vol);

        if (!fileIsExists(filename)) {
            String strCount = internalConfigure.getIniKey("count");
            int count = Integer.parseInt(strCount);
            for (int i = 0; i < count; i++) {
                String str = internalConfigure.getIniKey("msg" + String.valueOf(i + 1));
                String[] arr1 = str.split("\\|");
                String str1 = arr1[0];
                if (str.contains(strCurDT0)) {
                    strInfo = str;
                    break;
                }

                System.out.println("Read Ini: " + str);
                System.out.println(str1 + "    " + strCurDT0);
            }
        }

        if (strEnInfo.equals(strInfo)) {
            isRefreshAlarm = false;
            strInfo = internalConfigure.getIniKey("msg");

        }

        System.out.println("Info Text: " + strInfo);

        String[] array = strInfo.split("\\|");
        String str1 = array[0];
        String str2 = array[1];
        String str3 = array[3];
        String strTodo;
        if (zh_cn)
            strTodo = "待办事项：";
        else
            strTodo = "Todo: ";

        //显示一个警报框，目前已弃用，采用全屏幕显示
        /*new AlertDialog.Builder(ClockActivity.this).setTitle(str1).setMessage(str2 + "\n\n\n" + strCurDT)
                .setPositiveButton(str3, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        if (strMute.equals("false")) {
                            mediaPlayer.stop();
                        }

                        ClockActivity.this.finish();
                    }
                }).show();*/


        setContentView(R.layout.activity_main);
        bindViews(str1 + "\n\n" + strTodo + str2 + "\n\n\n" + strCurDT);

        if (isRefreshAlarm) {
            CallJavaNotify_3();
        }

        MyService.notifyTodoAlarm(context, str2);

        System.out.println("闹钟已开始+++++++++++++++++++++++");

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

                    System.out.println("ClockActivity HOME键被按下...");
                    onBackPressed();
                } else if (TextUtils.equals(reason, SYSTEM_HOME_KEY_LONG)) {
                    // 表示长按home键,显示最近使用的程序
                    System.out.println("ClockActivity 长按HOME键...");
                    onBackPressed();
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
        super.onBackPressed();

        AnimationWhenClosed();
    }

    @Override
    protected void onDestroy() {

        if (strMute.equals("false")) {
            mediaPlayer.stop();
        }
        if (strMute.equals("false") && curVol != -1) {
            setMediaVolume(curVol);
        }

        unregisterReceiver(mHomeKeyEvent);
        MyService.clearNotify();

        if (!isRefreshAlarm) {
            android.os.Process.killProcess(android.os.Process.myPid());
        } else {
            CallJavaNotify_4();
        }

        super.onDestroy();

        System.out.println("ClockActivity onDestroy...");

    }

    public static void close() {
        if (m_instance != null)
            m_instance.finish();
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
            //设置Context.MODE_PRIVATE表示每次调用该方法会覆盖原来的文件数据
            FileOutputStream fileOutputStream;// = context.openFileOutput(filename, Context.MODE_PRIVATE);
            File file = new File(filename);
            fileOutputStream = new FileOutputStream(file);
            //通过properties.stringPropertyNames()获得所有key的集合Set，里面是String对象
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
            //String line;
            //while ((line = br.readLine()) != null) {
            //    System.out.println(line);
            //}

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

    //fileName 为文件名称 返回true为存在
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

    public String readText(String filename) throws Exception {
        FileInputStream fileInputStream;
        File file = new File(filename);
        fileInputStream = new FileInputStream(file);

        InputStreamReader reader = new InputStreamReader(fileInputStream, "UTF-8");
        BufferedReader br = new BufferedReader(reader);
        String line = br.readLine();
        //while ((line = br.readLine()) != null) {
        //    System.out.println(line);
        //}

        br.close();
        reader.close();
        fileInputStream.close();

        return line;
    }

    //获取最大多媒体音量
    public int getMediaMaxVolume() {
        return mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
    }

    //获取当前多媒体音量
    public int getMediaVolume() {
        return mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
    }

    // 设置多媒体音量
    public void setMediaVolume(int volume) {
        mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, //音量类型
                volume,
                AudioManager.FLAG_PLAY_SOUND
                        | AudioManager.FLAG_SHOW_UI);
    }

    private void setStatusBarColor(String color) {
        // 需要安卓版本大于5.0以上
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            getWindow().setStatusBarColor(Color.parseColor(color));
        }
    }

    @Override
    public void onActivityCreated(Activity activity, Bundle savedInstanceState) {

    }

    @Override
    public void onActivityStarted(Activity activity) {

    }

    @Override
    public void onActivityResumed(Activity activity) {

    }

    @Override
    public void onActivityPaused(Activity activity) {

    }

    @Override
    public void onActivityStopped(Activity activity) {
        System.out.println("ClockActivity onActivityStopped...");
    }

    @Override
    public void onActivitySaveInstanceState(Activity activity, Bundle outState) {

    }

    @Override
    public void onActivityDestroyed(Activity activity) {

    }

}
