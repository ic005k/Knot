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
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.res.TypedArray;
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
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.net.URI;
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

public class DateTimePicker extends Activity {

    private String datetime_ini = "/storage/emulated/0/.Knot/datetime.ini";

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
    private int y, m, d, h, mm;
    private static boolean isDark;
    private String dateFlag = "";

    /**
     * hook反射方向检查
     **/
    private static void fixOrientation(Activity activity) {
        try {
            Class activityClass = Activity.class;
            Field mActivityInfoField = activityClass.getDeclaredField(
                "mActivityInfo"
            );
            mActivityInfoField.setAccessible(true);
            ActivityInfo activityInfo = (ActivityInfo) mActivityInfoField.get(
                activity
            );
            // 设置屏幕不固定
            activityInfo.screenOrientation =
                ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED;
        } catch (Exception e) {}
    }

    /**
     * hook反射检查是否透明色或者悬浮
     **/
    private boolean isTranslucentOrFloating() {
        boolean isTranslucentOrFloating = false;
        try {
            int[] styleableRes = (int[]) Class.forName(
                "com.android.internal.R$styleable"
            )
                .getField("Window")
                .get(null);
            final TypedArray typedArray = obtainStyledAttributes(styleableRes);
            Method method = ActivityInfo.class.getMethod(
                "isTranslucentOrFloating",
                TypedArray.class
            );
            method.setAccessible(true);
            isTranslucentOrFloating = (boolean) method.invoke(null, typedArray);
            method.setAccessible(false);
        } catch (Exception e) {}
        return isTranslucentOrFloating;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // 切记：在父类oncreate()方法调用前调用该方法修改配置
        // 针对Android8.0出现的“java.lang.IllegalStateException: Only fullscreen opaque
        // activities can request orientation”问题
        // Android8.1之后已改变了这个现象
        if (Build.VERSION.SDK_INT == 26 && isTranslucentOrFloating()) {
            fixOrientation(this);
        }

        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        context = DateTimePicker.this;
        isZh(context);
        isDark = MyActivity.isDark;
        // HomeKey
        registerReceiver(
            mHomeKeyEvent,
            new IntentFilter(Intent.ACTION_CLOSE_SYSTEM_DIALOGS)
        );

        String strFlag = "";
        try {
            File file = new File(datetime_ini);
            if (!file.exists()) file.createNewFile();
            Wini ini = new Wini(file);
            strFlag = ini.get("DateTime", "flag");
            dateFlag = ini.get("DateTime", "dateFlag");
            y = Integer.valueOf(ini.get("DateTime", "y"));
            m = Integer.valueOf(ini.get("DateTime", "m"));
            d = Integer.valueOf(ini.get("DateTime", "d"));
            h = Integer.valueOf(ini.get("DateTime", "h"));
            mm = Integer.valueOf(ini.get("DateTime", "mm"));

            System.out.println(
                "ymd hm  " +
                    y +
                    " " +
                    m +
                    " " +
                    d +
                    " " +
                    h +
                    " " +
                    mm +
                    " isDark " +
                    isDark
            );
        } catch (IOException e) {
            e.printStackTrace();
        }

        if (strFlag.equals("ymd")) showYearMonthDayPicker();

        if (strFlag.equals("ym")) showYearMonthPicker();

        if (strFlag.equals("y")) showYearPicker();

        if (strFlag.equals("hm")) showTimerPicker();
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
    public void onBackPressed() {
        super.onBackPressed();
    }

    @Override
    protected void onDestroy() {
        System.out.println("onDestroy...");
        unregisterReceiver(mHomeKeyEvent);
        super.onDestroy();
    }

    public static boolean isZh(Context context) {
        Locale locale = context.getResources().getConfiguration().locale;
        String language = locale.getLanguage();
        if (language.endsWith("zh")) zh_cn = true;
        else zh_cn = false;

        return zh_cn;
    }

    /**
     * 年月日选择
     */
    public void showYearMonthDayPicker() {
        String strTitle = "";
        if (dateFlag.equals("start")) {
            if (zh_cn) strTitle = "起始日期";
            else strTitle = "Start Date";
        }

        if (dateFlag.equals("end")) {
            if (zh_cn) strTitle = "结束日期";
            else strTitle = "End Date";
        }

        if (dateFlag.equals("todo")) {
            if (zh_cn) strTitle = "选择日期";
            else strTitle = "Select Date";
        }

        BasisTimesUtils.showDatePickerDialog(
            context,
            !isDark,
            strTitle,
            y,
            m,
            d,
            new BasisTimesUtils.OnDatePickerListener() {
                @Override
                public void onConfirm(int year, int month, int dayOfMonth) {
                    // Toast.makeText(context, year + "-" + month + "-" + dayOfMonth,
                    // Toast.LENGTH_SHORT).show();

                    try {
                        File file = new File(datetime_ini);
                        if (!file.exists()) file.createNewFile();
                        Wini ini = new Wini(file);

                        ini.put("DateTime", "y", String.valueOf(year));
                        ini.put("DateTime", "m", String.valueOf(month));
                        ini.put("DateTime", "d", String.valueOf(dayOfMonth));

                        ini.store();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    CallJavaNotify_14();
                    DateTimePicker.this.finish();
                }

                @Override
                public void onCancel() {
                    // Toast.makeText(context, "Cancle", Toast.LENGTH_SHORT).show();
                    DateTimePicker.this.finish();
                }
            }
        );
    }

    /**
     * 时间选择
     */
    public void showTimerPicker() {
        String strTitle = "";
        if (zh_cn) strTitle = "设置时间";
        else strTitle = "Set Time";

        BasisTimesUtils.showTimerPickerDialog(
            context,
            !isDark,
            strTitle,
            h,
            mm,
            true,
            new BasisTimesUtils.OnTimerPickerListener() {
                @Override
                public void onConfirm(int hourOfDay, int minute) {
                    try {
                        File file = new File(datetime_ini);
                        if (!file.exists()) file.createNewFile();
                        Wini ini = new Wini(file);

                        ini.put("DateTime", "h", String.valueOf(hourOfDay));
                        ini.put("DateTime", "mm", String.valueOf(minute));

                        ini.store();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }

                    CallJavaNotify_14();
                    DateTimePicker.this.finish();
                }

                @Override
                public void onCancel() {
                    DateTimePicker.this.finish();
                }
            }
        );
    }

    /**
     * 年月选择
     */
    private void showYearMonthPicker() {
        String strTitle = "";
        if (zh_cn) strTitle = "设置年月";
        else strTitle = "Set Year Month";

        BasisTimesUtils.showDatePickerDialog(
            context,
            !isDark,
            strTitle,
            y,
            m,
            d,
            new BasisTimesUtils.OnDatePickerListener() {
                @Override
                public void onConfirm(int year, int month, int dayOfMonth) {
                    try {
                        File file = new File(datetime_ini);
                        if (!file.exists()) file.createNewFile();
                        Wini ini = new Wini(file);

                        ini.put("DateTime", "y", String.valueOf(year));
                        ini.put("DateTime", "m", String.valueOf(month));
                        ini.put("DateTime", "d", String.valueOf(dayOfMonth));

                        ini.store();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }

                    CallJavaNotify_14();
                    DateTimePicker.this.finish();
                }

                @Override
                public void onCancel() {
                    DateTimePicker.this.finish();
                }
            }
        ).setDayGone();
    }

    /**
     * 年选择
     */
    private void showYearPicker() {
        String strTitle = "";
        if (zh_cn) strTitle = "设置年";
        else strTitle = "Set Year";

        // 步骤1：获取 BasisTimesUtils 实例（真实返回类型，无编译错误）
        BasisTimesUtils basisTimesUtils = BasisTimesUtils.showDatePickerDialog(
            context,
            !isDark,
            strTitle,
            y,
            m,
            d,
            new BasisTimesUtils.OnDatePickerListener() {
                @Override
                public void onConfirm(int year, int month, int dayOfMonth) {
                    try {
                        File file = new File(datetime_ini);
                        if (!file.exists()) file.createNewFile();
                        Wini ini = new Wini(file);

                        ini.put("DateTime", "y", String.valueOf(year));
                        ini.put("DateTime", "m", String.valueOf(month));
                        ini.put("DateTime", "d", String.valueOf(dayOfMonth));

                        ini.store();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }

                    CallJavaNotify_14();
                    DateTimePicker.this.finish();
                }

                @Override
                public void onCancel() {
                    DateTimePicker.this.finish();
                }
            }
        );

        // 步骤2：分步调用隐藏方法，先隐藏日，再隐藏月，实现仅显示年
        basisTimesUtils.setDayGone(); // 隐藏日（原有方法）
        basisTimesUtils.setMonthGone(); // 隐藏月（新增方法，已在 BasisTimesUtils 中实现）
    }
}
