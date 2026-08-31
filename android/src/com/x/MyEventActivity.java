package com.x;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;

public class MyEventActivity extends AppCompatActivity {

    public static MyEventActivity mInstance = null;

    private boolean isDark;
    private View layout_myevent_root;
    private ImageView myevent_btn_report;
    private ImageView myevent_btn_ai;
    private ImageView myevent_btn_edit;
    private ImageView myevent_btn_delete;
    private ImageView myevent_btn_add;
    private TextView myevent_tv_title;
    private RecyclerView myevent_rv_left_date_group;
    private RecyclerView myevent_rv_right_detail;
    private TextView myevent_tv_total;
    private MyEventLeftGroupAdapter mLeftAdapter;
    private MyEventRightDetailAdapter mRightAdapter;

    public static native void PublicJavaCallCpp(String type);

    private OnBackPressedCallback mBackCallback;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mInstance = this;

        mBackCallback = new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                PublicJavaCallCpp("cancel_add_event_record");
                finish();
            }
        };
        getOnBackPressedDispatcher().addCallback(this, mBackCallback);

        setContentView(R.layout.activity_myevent);
        isDark = ImmersiveUtil.applyRealImmersive(this);

        layout_myevent_root = findViewById(R.id.layout_myevent_root);
        myevent_btn_report = findViewById(R.id.myevent_btn_report);
        myevent_btn_ai = findViewById(R.id.myevent_btn_ai);
        myevent_btn_edit = findViewById(R.id.myevent_btn_edit);
        myevent_btn_delete = findViewById(R.id.myevent_btn_delete);
        myevent_btn_add = findViewById(R.id.myevent_btn_add);
        myevent_tv_title = findViewById(R.id.myevent_tv_title);
        myevent_rv_left_date_group = findViewById(
            R.id.myevent_rv_left_date_group
        );
        myevent_rv_right_detail = findViewById(R.id.myevent_rv_right_detail);
        myevent_tv_total = findViewById(R.id.myevent_tv_total);

        Intent intent = getIntent();
        if (intent != null) {
            ArrayList<String> receiveList = intent.getStringArrayListExtra(
                "event_item_list"
            );
            if (receiveList != null && !receiveList.isEmpty()) {
                String titleStr = receiveList.get(0);
                myevent_tv_title.setText(titleStr);
            } else {
                myevent_tv_title.setText("My Event");
            }
        }

        // 左侧列表初始化
        LinearLayoutManager leftLayoutManager = new LinearLayoutManager(this);
        leftLayoutManager.setMeasurementCacheEnabled(false);
        myevent_rv_left_date_group.setLayoutManager(leftLayoutManager);
        mLeftAdapter = new MyEventLeftGroupAdapter();
        myevent_rv_left_date_group.setAdapter(mLeftAdapter);

        // 右侧列表初始化
        LinearLayoutManager rightLayoutManager = new LinearLayoutManager(this);
        rightLayoutManager.setMeasurementCacheEnabled(false);
        myevent_rv_right_detail.setLayoutManager(rightLayoutManager);
        mRightAdapter = new MyEventRightDetailAdapter();
        myevent_rv_right_detail.setAdapter(mRightAdapter);

        mLeftAdapter.setClickListener((pos, data) -> {
            // TODO: QtNative.runOnQtThread() 调用C++，按dateStr获取事件明细，回调后更新mRightAdapter
            // 调用C++获取事件明细
            PublicJavaCallCpp("get_maindatedetail|==|" + pos);
        });

        myevent_btn_report.setOnClickListener(v -> {
            // 报表
        });
        myevent_btn_ai.setOnClickListener(v -> {
            // AI分析
        });
        myevent_btn_edit.setOnClickListener(v -> {
            // 修改事件
            int selectPos0 = mLeftAdapter.getSelectedPosition();
            if (selectPos0 < 0) {
                return;
            }

            int selectPos1 = mRightAdapter.getSelectedPosition();
            if (selectPos1 < 0) {
                return;
            }
            PublicJavaCallCpp(
                "edit_datadetail|==|" + selectPos0 + "|==|" + selectPos1
            );
        });

        myevent_btn_delete.setOnClickListener(v -> {
            // 删除事件
            boolean zh = MyActivity.zh_cn;
            String title = zh ? "确认删除" : "Confirm Delete";
            String message = zh
                ? "今天的最后一条记录将被删除。"
                : "The last record of today will be deleted.";
            String btnOk = zh ? "确定" : "OK";
            String btnCancel = zh ? "取消" : "Cancel";

            new androidx.appcompat.app.AlertDialog.Builder(this)
                .setTitle(title)
                .setMessage(message)
                .setPositiveButton(btnOk, (dialog, which) -> {
                    // 用户确认后才调用C++删除
                    PublicJavaCallCpp("del_datadetail");
                })
                .setNegativeButton(btnCancel, null)
                .show();
        });

        myevent_btn_add.setOnClickListener(v -> {
            // 新增事件
            PublicJavaCallCpp("add_datadetail");
        });

        // 初始化UI颜色
        refreshUi();

        View root = findViewById(android.R.id.content);
        if (root != null) {
            root.post(() -> {
                if (isFinishing()) return;
                // 放到Android主线下一轮事件循环，视图inflate/layout完成后再通知C++刷新
                PublicJavaCallCpp("refresh_alldata");
            });
        }
    }

    private void updateAllColor() {
        int iconColor;
        int rootBgColor;
        if (isDark) {
            iconColor = 0xFFFFFFFF;
            rootBgColor = 0xFF121212;
        } else {
            iconColor = 0xFF000000;
            rootBgColor = 0xFFF5F5F5;
        }
        layout_myevent_root.setBackgroundColor(rootBgColor);

        // 顶部全部图标着色
        myevent_btn_report.setColorFilter(iconColor);
        myevent_btn_ai.setColorFilter(iconColor);
        myevent_btn_edit.setColorFilter(iconColor);
        myevent_btn_delete.setColorFilter(iconColor);
        myevent_btn_add.setColorFilter(iconColor);

        // 同步暗黑模式给两个适配器
        mLeftAdapter.setDarkMode(isDark);
        mRightAdapter.setDarkMode(isDark);
    }

    private void refreshUi() {
        // 这里后续可以放文本更新逻辑 updateAllTexts();
        updateAllColor();
    }

    /**
     * JNI回调：刷新左侧全部日期分组列表，主线程调用
     * @param rawStrList C++返回，每条字符串以 |==| 分隔
     */
    public void refreshLeftGroupList(ArrayList<String> rawStrList) {
        // 如果不在主线程，post到主线程再执行
        if (
            !Thread.currentThread().equals(Looper.getMainLooper().getThread())
        ) {
            new Handler(Looper.getMainLooper()).post(() ->
                refreshLeftGroupList(rawStrList)
            );
            return;
        }
        if (mLeftAdapter == null || isFinishing()) return;

        ArrayList<MyEventDateGroup> outList = new ArrayList<>();
        for (String line : rawStrList) {
            if (line == null || line.isEmpty()) continue;
            String[] parts = line.split("\\|==\\|");
            MyEventDateGroup obj = new MyEventDateGroup();
            if (parts.length >= 1) obj.dateStr = parts[0];
            if (parts.length >= 2) obj.dayItemCount = parts[1];
            if (parts.length >= 3) obj.dayTotalValue = parts[2];
            outList.add(obj);
        }
        mLeftAdapter.setData(outList);
    }

    /**
     * JNI回调：刷新右侧当日明细列表，主线程调用
     * @param rawStrList C++返回，每条字符串以 |==| 分隔
     */
    public void refreshRightDetailList(ArrayList<String> rawStrList) {
        if (
            !Thread.currentThread().equals(Looper.getMainLooper().getThread())
        ) {
            new Handler(Looper.getMainLooper()).post(() ->
                refreshRightDetailList(rawStrList)
            );
            return;
        }
        if (mRightAdapter == null || isFinishing()) return;

        ArrayList<MyEventDetailItem> outList = new ArrayList<>();
        for (String line : rawStrList) {
            if (line == null || line.isEmpty()) continue;
            String[] parts = line.split("\\|==\\|");
            MyEventDetailItem obj = new MyEventDetailItem();
            if (parts.length >= 1) obj.timeStr = parts[0];
            if (parts.length >= 2) obj.eventValue = parts[1];
            if (parts.length >= 3) obj.category = parts[2];
            if (parts.length >= 4) obj.note = parts[3];
            outList.add(obj);
        }
        mRightAdapter.setDetailData(outList);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mInstance = null;
    }
}
