package com.x;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mInstance = this;
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
        myevent_rv_left_date_group.setLayoutManager(
            new LinearLayoutManager(this)
        );
        mLeftAdapter = new MyEventLeftGroupAdapter();
        myevent_rv_left_date_group.setAdapter(mLeftAdapter);
        // 右侧列表初始化
        myevent_rv_right_detail.setLayoutManager(new LinearLayoutManager(this));
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
        });
        myevent_btn_delete.setOnClickListener(v -> {
            // 删除事件
        });
        myevent_btn_add.setOnClickListener(v -> {
            // 新增事件
        });

        // 初始化UI颜色
        refreshUi();
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
        if (mLeftAdapter == null) return;

        ArrayList<MyEventDateGroup> outList = new ArrayList<>();
        for (String line : rawStrList) {
            if (line == null || line.isEmpty()) continue;
            String[] parts = line.split("\\|==|");
            // 根据你的协议顺序解析字段，示例：dateStr | dayItemCount | dayTotalValue
            MyEventDateGroup obj = new MyEventDateGroup();
            obj.dateStr = parts[0];
            obj.dayItemCount = Integer.parseInt(parts[1]);
            obj.dayTotalValue = Long.parseLong(parts[2]);
            outList.add(obj);
        }
        mLeftAdapter.setData(outList);
    }

    /**
     * JNI回调：刷新右侧当日明细列表，主线程调用
     * @param rawStrList C++返回，每条字符串以 |==| 分隔
     */
    public void refreshRightDetailList(ArrayList<String> rawStrList) {
        if (mRightAdapter == null) return;

        ArrayList<MyEventDetailItem> outList = new ArrayList<>();
        for (String line : rawStrList) {
            if (line == null || line.isEmpty()) continue;
            String[] parts = line.split("\\|==|");
            // 按照你约定的字段顺序：timeStr | eventValue | category | note
            MyEventDetailItem obj = new MyEventDetailItem();
            obj.timeStr = parts[0];
            obj.eventValue = Long.parseLong(parts[1]);
            obj.category = parts[2];
            obj.note = parts[3];
            outList.add(obj);
        }
        mRightAdapter.setDetailData(outList);
    }
}
