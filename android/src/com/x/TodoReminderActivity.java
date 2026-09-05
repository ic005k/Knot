package com.x;

import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Gravity;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;
import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;
import java.util.ArrayList;

public class TodoReminderActivity extends AppCompatActivity {

    public static TodoReminderActivity mInstance = null;

    private TextView mTvTimeLabel; //左上角 定时文本 [0]
    private TextView mTvContent; //中间居中提醒内容 [1]
    private TextView mTvActivateTime; //右下角激活时间，小字体 [2]

    private boolean mIsDark;
    private OnBackPressedCallback mBackCallback;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mInstance = this;

        mBackCallback = new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                MyService.clearNotify(TodoReminderActivity.this);
                finish();
            }
        };
        getOnBackPressedDispatcher().addCallback(this, mBackCallback);

        mIsDark = ImmersiveUtil.applyRealImmersive(this);

        //根布局
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setLayoutParams(
            new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        );

        int bgColor = mIsDark ? 0xFF121212 : 0xFFF7F7F7;
        int textMain = mIsDark ? 0xFFFFFFFF : 0xFF111111;
        int textSub = mIsDark ? 0xFFAAAAAA : 0xFF555555;
        root.setBackgroundColor(bgColor);

        //顶部行布局：左上角文本，右下角留空位
        LinearLayout topRow = new LinearLayout(this);
        topRow.setOrientation(LinearLayout.HORIZONTAL);
        topRow.setLayoutParams(
            new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
        );
        topRow.setPadding(dp(16), dp(16), dp(16), dp(8));

        mTvTimeLabel = new TextView(this);
        mTvTimeLabel.setTextSize(17);
        mTvTimeLabel.setTextColor(textMain);
        topRow.addView(mTvTimeLabel);

        root.addView(topRow);

        //中间内容容器，占满剩余高度，实现内容完全居中
        LinearLayout centerContainer = new LinearLayout(this);
        centerContainer.setLayoutParams(
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                0,
                1.0f
            )
        );
        centerContainer.setGravity(Gravity.CENTER);

        mTvContent = new TextView(this);
        mTvContent.setTextSize(20);
        mTvContent.setTextColor(textMain);
        mTvContent.setGravity(Gravity.CENTER);
        mTvContent.setPadding(dp(24), dp(12), dp(24), dp(12));
        centerContainer.addView(mTvContent);

        root.addView(centerContainer);

        //底部行布局：右下角激活时间
        LinearLayout bottomRow = new LinearLayout(this);
        bottomRow.setOrientation(LinearLayout.HORIZONTAL);
        bottomRow.setLayoutParams(
            new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
        );
        bottomRow.setGravity(Gravity.END);
        bottomRow.setPadding(dp(16), dp(8), dp(16), dp(20));

        mTvActivateTime = new TextView(this);
        mTvActivateTime.setTextSize(12);
        mTvActivateTime.setTextColor(textSub);
        bottomRow.addView(mTvActivateTime);

        root.addView(bottomRow);

        setContentView(root);

        //读取传入ArrayList，下标0、1、2
        Intent intent = getIntent();
        ArrayList<String> dataList = intent.getStringArrayListExtra(
            "todo_reminder_data"
        );
        if (dataList != null && dataList.size() >= 3) {
            mTvTimeLabel.setText(dataList.get(0));
            mTvContent.setText(dataList.get(1));
            mTvActivateTime.setText(dataList.get(2));
        } else {
            mTvTimeLabel.setText("ERR: dataList null");
            mTvContent.setText("没有收到提醒数据");
            mTvActivateTime.setText("-");
        }
    }

    private int dp(int dpVal) {
        float density = getResources().getDisplayMetrics().density;
        return (int) (dpVal * density + 0.5f);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mBackCallback != null) {
            mBackCallback.remove();
        }
        mInstance = null;
    }
}
