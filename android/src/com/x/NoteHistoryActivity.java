package com.x;

import android.app.Activity;
import android.os.Build;
import android.os.Bundle;
import android.text.Html;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import java.util.ArrayList;

public class NoteHistoryActivity extends Activity {

    public static NoteHistoryActivity mInstance = null;
    private ListView mListView;
    private TextView mTvHtmlPreview;
    private ArrayList<String> mTimeList;
    // 新增：记录当前选中条目索引
    private int mSelectedIndex = -1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mInstance = this;
        boolean isDark = ImmersiveUtil.applyRealImmersive(this);
        int textColor = isDark ? 0xFFFFFFFF : 0xFF000000;
        int bgColor = isDark ? 0xFF1E1E1E : 0xFFFFFFFF;
        // 选中高亮背景
        int selectedBg = isDark ? 0xFF383838 : 0xFFE8E8E8;

        // 从Intent拿到传入的修改时间数组
        mTimeList = getIntent().getStringArrayListExtra("timeList");
        if (mTimeList == null) {
            mTimeList = new ArrayList<>();
        }
        // 根布局
        LinearLayout rootLl = new LinearLayout(this);
        rootLl.setOrientation(LinearLayout.VERTICAL);
        rootLl.setBackgroundColor(bgColor);
        rootLl.setPadding(dp(16), dp(12), dp(16), dp(12));
        // 1.顶部标题标签
        TextView tvTitle = new TextView(this);
        tvTitle.setText(
            MyActivity.zh_cn ? "笔记修改历史" : "Note Revision History"
        );
        tvTitle.setTextSize(18);
        tvTitle.setTextColor(textColor);
        tvTitle.setPadding(0, 0, 0, dp(12));
        rootLl.addView(tvTitle);
        // 内容容器：ListView + HTML预览，平分剩余垂直空间
        LinearLayout contentLl = new LinearLayout(this);
        contentLl.setOrientation(LinearLayout.VERTICAL);
        LinearLayout.LayoutParams contentLp = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            0
        );
        contentLp.weight = 1;
        rootLl.addView(contentLl, contentLp);
        // ListView 占一半高度
        mListView = new ListView(this);
        mListView.setBackgroundColor(bgColor);
        LinearLayout.LayoutParams listLp = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            0
        );
        listLp.weight = 1;
        contentLl.addView(mListView, listLp);
        // HTML预览TextView，另一半高度，支持滚动
        mTvHtmlPreview = new TextView(this);
        mTvHtmlPreview.setTextColor(textColor);
        mTvHtmlPreview.setPadding(dp(8), dp(8), dp(8), dp(8));
        mTvHtmlPreview.setTextSize(14);
        mTvHtmlPreview.setMovementMethod(
            android.text.method.ScrollingMovementMethod.getInstance()
        );
        LinearLayout.LayoutParams previewLp = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            0
        );
        previewLp.weight = 1;
        contentLl.addView(mTvHtmlPreview, previewLp);

        // 列表适配器
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(
            this,
            0,
            mTimeList
        ) {
            @Override
            public View getView(
                int position,
                View convertView,
                ViewGroup parent
            ) {
                LinearLayout itemLl;
                if (convertView == null) {
                    itemLl = new LinearLayout(NoteHistoryActivity.this);
                    itemLl.setOrientation(LinearLayout.HORIZONTAL);
                    itemLl.setGravity(Gravity.CENTER_VERTICAL);
                    itemLl.setPadding(0, dp(10), 0, dp(10));
                } else {
                    itemLl = (LinearLayout) convertView;
                }
                itemLl.removeAllViews();

                // 选中/未选中背景
                if (position == mSelectedIndex) {
                    itemLl.setBackgroundColor(selectedBg);
                } else {
                    itemLl.setBackgroundColor(0x00000000); //透明
                }

                // 左侧 修改时间文本
                TextView tvTime = new TextView(NoteHistoryActivity.this);
                tvTime.setText(getItem(position));
                tvTime.setTextColor(textColor);
                tvTime.setTextSize(14);
                LinearLayout.LayoutParams timeLp =
                    new LinearLayout.LayoutParams(
                        0,
                        ViewGroup.LayoutParams.WRAP_CONTENT
                    );
                timeLp.weight = 1;
                itemLl.addView(tvTime, timeLp);
                // 右侧旧文本按钮
                Button btnOld = new Button(NoteHistoryActivity.this);
                btnOld.setText(MyActivity.zh_cn ? "旧文本" : "Old Text");
                btnOld.setTextSize(12);
                btnOld.setOnClickListener(v -> {
                    mSelectedIndex = position;
                    notifyDataSetChanged();
                    MyActivity.mInstance.PublicJavaCallCpp(
                        "note_history_oldtext|==|" + position
                    );
                });
                itemLl.addView(btnOld);
                // 点击整行条目
                itemLl.setOnClickListener(v -> {
                    mSelectedIndex = position;
                    notifyDataSetChanged();
                    MyActivity.mInstance.PublicJavaCallCpp(
                        "note_history_select|==|" + position
                    );
                });
                return itemLl;
            }
        };
        mListView.setAdapter(adapter);
        setContentView(rootLl);
    }

    /**
     * C++ JNI调用，刷新HTML预览，只取数组第0项
     */
    public void refreshNoteHistoryHtml(ArrayList<String> htmlArray) {
        runOnUiThread(() -> {
            if (htmlArray == null || htmlArray.isEmpty()) return;
            String htmlContent = htmlArray.get(0);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                mTvHtmlPreview.setText(
                    Html.fromHtml(htmlContent, Html.FROM_HTML_MODE_COMPACT)
                );
            } else {
                mTvHtmlPreview.setText(Html.fromHtml(htmlContent));
            }
        });
    }

    // dp转px工具方法，沿用项目现有
    private int dp(int dpVal) {
        float scale = getResources().getDisplayMetrics().density;
        return (int) (dpVal * scale + 0.5f);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mInstance = null;
    }
}
