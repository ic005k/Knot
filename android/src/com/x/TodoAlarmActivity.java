package com.x;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.NumberPicker;
import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;
import java.lang.ref.WeakReference;
import java.util.ArrayList;

/**
 * Todo定时弹窗页面
 * Java仅视图渲染+事件转发，业务逻辑全部C++
 */
public class TodoAlarmActivity extends AppCompatActivity {

    private static boolean isDark = false;
    private OnBackPressedCallback mBackCallback;
    public static TodoAlarmActivity mInstance = null;
    private WeakReference<TodoAlarmActivity> mSelfWeakRef;
    //周选择开关
    private SwitchCompat swWeek1, swWeek2, swWeek3, swWeek4, swWeek5, swWeek6, swWeek7, swWeekAll;
    //日期滚轮
    private NumberPicker npYear, npMonth, npDay;
    //时间滚轮
    private NumberPicker npHour, npMinute;
    private SwitchCompat swTts;
    private Button btnTest, btnBack, btnDeleteAlarm, btnSetAlarm;
    private View layoutAlarmRoot;

    public static native void PublicJavaCallCpp(String type);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mInstance = this;
        mBackCallback = new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                PublicJavaCallCpp("todo_alarm_back");
                finish();
            }
        };
        getOnBackPressedDispatcher().addCallback(this, mBackCallback);
        setContentView(R.layout.activity_todo_alarm);
        isDark = ImmersiveUtil.applyRealImmersive(this);
        mSelfWeakRef = new WeakReference<>(this);
        layoutAlarmRoot = findViewById(R.id.layout_alarm_root);
        bindViews();
        initPickerRange();
        refreshUi();

        //===== 读取C++传入的初始化参数，赋值UI初始值 =====
        ArrayList<String> initList = getIntent().getStringArrayListExtra(
            "todo_alarm_list"
        );
        if (initList != null && initList.size() >= 9) {
            // 直接按list下标取，不再split
            int w1 = parseSafeInt(initList.get(0));
            int w2 = parseSafeInt(initList.get(1));
            int w3 = parseSafeInt(initList.get(2));
            int w4 = parseSafeInt(initList.get(3));
            int w5 = parseSafeInt(initList.get(4));
            int w6 = parseSafeInt(initList.get(5));
            int w7 = parseSafeInt(initList.get(6));

            String strDate = initList.get(7);
            String strTime = initList.get(8);

            //日期默认兜底 2026‑1‑1
            int y = 2026,
                m = 1,
                d = 1;
            String[] dateSplit = strDate.split("-");
            if (dateSplit.length >= 3) {
                y = parseSafeInt(dateSplit[0]);
                m = parseSafeInt(dateSplit[1]);
                d = parseSafeInt(dateSplit[2]);
            }

            //时间默认兜底 00:00
            int h = 0,
                mi = 0;
            String[] timeSplit = strTime.split(":");
            if (timeSplit.length >= 2) {
                h = parseSafeInt(timeSplit[0]);
                mi = parseSafeInt(timeSplit[1]);
            }

            final int fy = y;
            final int fm = m;
            final int fd = d;
            final int fh = h;
            final int fmi = mi;
            final int fw1 = w1;
            final int fw2 = w2;
            final int fw3 = w3;
            final int fw4 = w4;
            final int fw5 = w5;
            final int fw6 = w6;
            final int fw7 = w7;

            runOnUiThread(() -> {
                if (isFinishing() || isDestroyed()) return;
                npYear.setValue(fy);
                npMonth.setValue(fm);
                npDay.setValue(fd);
                npHour.setValue(fh);
                npMinute.setValue(fmi);

                swWeek1.setChecked(fw1 == 1);
                swWeek2.setChecked(fw2 == 1);
                swWeek3.setChecked(fw3 == 1);
                swWeek4.setChecked(fw4 == 1);
                swWeek5.setChecked(fw5 == 1);
                swWeek6.setChecked(fw6 == 1);
                swWeek7.setChecked(fw7 == 1);

                boolean allChecked =
                    fw1 == 1 &&
                    fw2 == 1 &&
                    fw3 == 1 &&
                    fw4 == 1 &&
                    fw5 == 1 &&
                    fw6 == 1 &&
                    fw7 == 1;
                swWeekAll.setChecked(allChecked);
            });
        }

        /////////////////////////////////////////
    }

    /**安全int解析，解析失败返回0*/
    private int parseSafeInt(String s) {
        try {
            return Integer.parseInt(s.trim());
        } catch (Exception e) {
            return 0;
        }
    }

    private void bindViews() {
        swWeek1 = findViewById(R.id.sw_week1);
        swWeek2 = findViewById(R.id.sw_week2);
        swWeek3 = findViewById(R.id.sw_week3);
        swWeek4 = findViewById(R.id.sw_week4);
        swWeek5 = findViewById(R.id.sw_week5);
        swWeek6 = findViewById(R.id.sw_week6);
        swWeek7 = findViewById(R.id.sw_week7);
        swWeekAll = findViewById(R.id.sw_week_all);
        npYear = findViewById(R.id.np_year);
        npMonth = findViewById(R.id.np_month);
        npDay = findViewById(R.id.np_day);
        npHour = findViewById(R.id.np_hour);
        npMinute = findViewById(R.id.np_minute);
        swTts = findViewById(R.id.sw_tts);
        btnTest = findViewById(R.id.btn_test);
        btnBack = findViewById(R.id.btn_back);
        btnDeleteAlarm = findViewById(R.id.btn_delete_alarm);
        btnSetAlarm = findViewById(R.id.btn_set_alarm);
        btnBack.setOnClickListener(v -> {
            PublicJavaCallCpp("todo_alarm_back");
            finish();
        });
        btnTest.setOnClickListener(v -> onTestClicked());
        btnDeleteAlarm.setOnClickListener(v -> onDeleteAlarmClicked());
        btnSetAlarm.setOnClickListener(v -> onSetAlarmClicked());
        swWeekAll.setOnCheckedChangeListener((compoundButton, b) ->
            onWeekAllChecked(b)
        );
    }

    /**
     * 初始化NumberPicker取值范围
     */
    private void initPickerRange() {
        npYear.setMinValue(2024);
        npYear.setMaxValue(2035);
        npMonth.setMinValue(1);
        npMonth.setMaxValue(12);
        npDay.setMinValue(1);
        npDay.setMaxValue(31);
        npHour.setMinValue(0);
        npHour.setMaxValue(23);
        npMinute.setMinValue(0);
        npMinute.setMaxValue(59);
    }

    private void updateAllTexts() {
        boolean zh = MyActivity.zh_cn;
        if (zh) {
            btnBack.setText("返回");
            btnDeleteAlarm.setText("删除定时");
            btnSetAlarm.setText("设置定时");
            btnTest.setText("测试");
        } else {
            btnBack.setText("Back");
            btnDeleteAlarm.setText("Delete Alarm");
            btnSetAlarm.setText("Set Alarm");
            btnTest.setText("Test");
        }
    }

    private void updateAllColor() {
        int rootBgColor;
        if (isDark) {
            rootBgColor = 0xFF121212;
        } else {
            rootBgColor = 0xFFF8F8F8;
        }
        layoutAlarmRoot.setBackgroundColor(rootBgColor);
    }

    private void refreshUi() {
        updateAllTexts();
        updateAllColor();
    }

    //=========== 回调占位，事件全部转发C++ ===========
    private void onWeekAllChecked(boolean allChecked) {
        PublicJavaCallCpp("todo_alarm_week_all|==|" + (allChecked ? 1 : 0));
    }

    private void onTestClicked() {
        PublicJavaCallCpp("todo_alarm_test");
    }

    private void onDeleteAlarmClicked() {
        PublicJavaCallCpp("todo_alarm_delete");
    }

    private void onSetAlarmClicked() {
        int year = npYear.getValue();
        int month = npMonth.getValue();
        int day = npDay.getValue();
        int hour = npHour.getValue();
        int minute = npMinute.getValue();
        int weekBits = getWeekBits();
        int ttsEnable = swTts.isChecked() ? 1 : 0;
        // 直接转发全部控件状态给C++，业务逻辑在C++处理
        PublicJavaCallCpp(
            "todo_alarm_set|==|" +
                year +
                "|==|" +
                month +
                "|==|" +
                day +
                "|==|" +
                hour +
                "|==|" +
                minute +
                "|==|" +
                weekBits +
                "|==|" +
                ttsEnable
        );
    }

    /**
     * 读取全部周选择开关，计算weekBits掩码返回
     *
     * @return int weekBits
     */
    public int getWeekBits() {
        int bits = 0;
        if (swWeek1.isChecked()) bits |= 1 << 0;
        if (swWeek2.isChecked()) bits |= 1 << 1;
        if (swWeek3.isChecked()) bits |= 1 << 2;
        if (swWeek4.isChecked()) bits |= 1 << 3;
        if (swWeek5.isChecked()) bits |= 1 << 4;
        if (swWeek6.isChecked()) bits |= 1 << 5;
        if (swWeek7.isChecked()) bits |= 1 << 6;
        return bits;
    }

    /**
     * C++调用，UI回显已存在闹钟参数
     */
    public void setUiValue(
        int year,
        int month,
        int day,
        int hour,
        int minute,
        int weekBits,
        boolean ttsEnable
    ) {
        runOnUiThread(() -> {
            if (isFinishing() || isDestroyed()) return;
            npYear.setValue(year);
            npMonth.setValue(month);
            npDay.setValue(day);
            npHour.setValue(hour);
            npMinute.setValue(minute);
            swTts.setChecked(ttsEnable);
            // 根据掩码回写周开关
            swWeek1.setChecked((weekBits & (1 << 0)) != 0);
            swWeek2.setChecked((weekBits & (1 << 1)) != 0);
            swWeek3.setChecked((weekBits & (1 << 2)) != 0);
            swWeek4.setChecked((weekBits & (1 << 3)) != 0);
            swWeek5.setChecked((weekBits & (1 << 4)) != 0);
            swWeek6.setChecked((weekBits & (1 << 5)) != 0);
            swWeek7.setChecked((weekBits & (1 << 6)) != 0);
            swWeekAll.setChecked(weekBits == 0b1111111);
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mBackCallback != null) {
            mBackCallback.remove();
            mBackCallback = null;
        }
        mSelfWeakRef.clear();
        mInstance = null;
        PublicJavaCallCpp("todo_alarm_activity_destroy");
        if (
            MyActivity.m_instance != null &&
            !MyActivity.m_instance.isFinishing() &&
            !MyActivity.m_instance.isDestroyed()
        ) {
            MyActivity.m_instance.runOnUiThread(() -> {
                MyActivity.m_instance.getWindow().getDecorView().requestFocus();
                MyActivity.m_instance
                    .getWindow()
                    .getDecorView()
                    .postInvalidate();
                MyService.forceDisconnectInputMethod();
            });
        }
    }

    @Override
    public void onBackPressed() {}
}
