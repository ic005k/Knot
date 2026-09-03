package com.x;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import androidx.activity.OnBackPressedCallback;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import java.util.ArrayList;

public class TabRecycleBinActivity extends AppCompatActivity {

    public static TabRecycleBinActivity mInstance = null;
    private LinearLayout mRootLayout;
    private TextView mTvTitle;
    private TextView mTvTotalHint;
    private ListView mListView;
    private LinearLayout mBottomBtnContainer;
    private RecycleBinItemAdapter mAdapter;
    private ArrayList<String> mRawDataList = new ArrayList<>();
    private int mSelectedPos = -1;
    private boolean mIsDark = false;

    public static native void PublicJavaCallCpp(String type);

    private OnBackPressedCallback mBackCallback;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mInstance = this;
        mBackCallback = new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                // Knot标准返回标志，保留原有命名
                PublicJavaCallCpp("cancel_add_event_record");
                finish();
            }
        };
        getOnBackPressedDispatcher().addCallback(this, mBackCallback);

        mIsDark = ImmersiveUtil.applyRealImmersive(this);
        // 根布局
        mRootLayout = new LinearLayout(this);
        mRootLayout.setOrientation(LinearLayout.VERTICAL);
        mRootLayout.setLayoutParams(
            new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        );
        int bgColor = mIsDark ? 0xFF121212 : 0xFFF5F5F5;
        mRootLayout.setBackgroundColor(bgColor);

        // ---------------- 标题行：标签页回收箱 总计:x ----------------
        LinearLayout titleRow = new LinearLayout(this);
        titleRow.setGravity(Gravity.CENTER_VERTICAL);
        titleRow.setPadding(dp(16), dp(16), dp(16), dp(8));
        mTvTitle = new TextView(this);
        mTvTitle.setText(MyActivity.zh_cn ? "标签页回收箱" : "Tab Recycle Bin");
        mTvTitle.setTextSize(22);
        mTvTitle.setTextColor(mIsDark ? Color.WHITE : Color.BLACK);
        mTvTotalHint = new TextView(this);
        mTvTotalHint.setTextSize(20);
        mTvTotalHint.setTextColor(mIsDark ? Color.WHITE : Color.BLACK);
        LinearLayout.LayoutParams titleLp = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        );
        titleLp.leftMargin = dp(12);
        titleRow.addView(mTvTitle);
        titleRow.addView(mTvTotalHint, titleLp);
        mRootLayout.addView(titleRow);

        // ---------------- ListView列表 ----------------
        mListView = new ListView(this);
        LinearLayout.LayoutParams listLp = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            0,
            1.0f
        );
        listLp.leftMargin = dp(8);
        listLp.rightMargin = dp(8);
        mListView.setLayoutParams(listLp);
        // 关闭ListView系统自带item按压选择器，防止覆盖我们自己设置的选中背景
        mListView.setSelector(android.R.color.transparent);
        mListView.setDividerHeight(dp(1));

        mAdapter = new RecycleBinItemAdapter(this, mRawDataList);
        mListView.setAdapter(mAdapter);
        mListView.setOnItemClickListener((parent, view, position, id) -> {
            mSelectedPos = position;
            mAdapter.notifyDataSetChanged();
        });
        mRootLayout.addView(mListView);

        // ---------------- 底部按钮栏：返回、删除、恢复 ----------------
        mBottomBtnContainer = new LinearLayout(this);
        mBottomBtnContainer.setOrientation(LinearLayout.HORIZONTAL);
        mBottomBtnContainer.setGravity(Gravity.CENTER);
        mBottomBtnContainer.setPadding(dp(12), dp(12), dp(12), dp(16));
        Button btnBack = createButton(MyActivity.zh_cn ? "返回" : "Back");
        btnBack.setOnClickListener(v -> {
            getOnBackPressedDispatcher().onBackPressed();
        });
        Button btnDelete = createButton(MyActivity.zh_cn ? "删除" : "Delete");
        btnDelete.setOnClickListener(v -> {
            if (mSelectedPos < 0) return;
            new AlertDialog.Builder(this)
                .setTitle(
                    MyActivity.zh_cn
                        ? "确认彻底删除"
                        : "Confirm Permanent Delete"
                )
                .setMessage(
                    MyActivity.zh_cn
                        ? "该条目将被永久删除，不可恢复！"
                        : "Permanently delete, cannot restore!"
                )
                .setPositiveButton(
                    MyActivity.zh_cn ? "删除" : "Delete",
                    (d, w) -> {
                        String item = mRawDataList.get(mSelectedPos);
                        PublicJavaCallCpp("tab_recycle_bin_delete|==|" + item);
                        mSelectedPos = -1;
                        mAdapter.notifyDataSetChanged();
                    }
                )
                .setNegativeButton(MyActivity.zh_cn ? "取消" : "Cancel", null)
                .show();
        });
        Button btnRestore = createButton(MyActivity.zh_cn ? "恢复" : "Restore");
        btnRestore.setOnClickListener(v -> {
            if (mSelectedPos < 0) return;
            String item = mRawDataList.get(mSelectedPos);
            PublicJavaCallCpp("tab_recycle_bin_restore|==|" + item);
            mSelectedPos = -1;
            mAdapter.notifyDataSetChanged();
        });
        mBottomBtnContainer.addView(btnBack);
        mBottomBtnContainer.addView(btnDelete);
        mBottomBtnContainer.addView(btnRestore);
        mRootLayout.addView(mBottomBtnContainer);

        setContentView(mRootLayout);
        //接收intent传入的回收箱数据
        ArrayList<String> initList = getIntent().getStringArrayListExtra(
            "recycle_list"
        );
        if (initList != null) {
            setRecycleData(initList);
        }
    }

    /**
     * JNI调用入口，刷新回收箱列表
     */
    public void setRecycleData(ArrayList<String> list) {
        new Handler(Looper.getMainLooper()).post(() -> {
            if (isFinishing() || isDestroyed()) return;
            mRawDataList.clear();
            mRawDataList.addAll(list);
            mSelectedPos = -1;
            mAdapter.notifyDataSetChanged();
            mTvTotalHint.setText(
                String.format(
                    MyActivity.zh_cn ? "总计：%d" : "Total: %d",
                    mRawDataList.size()
                )
            );
        });
    }

    private Button createButton(String text) {
        Button btn = new Button(this);
        btn.setText(text);
        btn.setTextSize(17);
        android.content.res.ColorStateList colorStateList =
            android.content.res.ColorStateList.valueOf(0xFF3478E8);
        btn.setBackgroundTintList(colorStateList);
        btn.setTextColor(Color.WHITE);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
            0,
            dp(52),
            1.0f
        );
        lp.setMargins(dp(4), 0, dp(4), 0);
        btn.setLayoutParams(lp);
        return btn;
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

    /**
     * 内部适配器，纯Java生成每一行item视图，无xml
     */
    private class RecycleBinItemAdapter extends ArrayAdapter<String> {

        private final Context mCtx;
        private final ArrayList<String> mList;

        public RecycleBinItemAdapter(
            @NonNull Context context,
            ArrayList<String> objects
        ) {
            super(context, 0, objects);
            mCtx = context;
            mList = objects;
        }

        @NonNull
        @Override
        public View getView(
            int position,
            View convertView,
            @NonNull ViewGroup parent
        ) {
            ViewHolder holder;
            if (convertView == null) {
                LinearLayout itemLayout = new LinearLayout(mCtx);
                itemLayout.setOrientation(LinearLayout.VERTICAL);
                itemLayout.setPadding(dp(14), dp(14), dp(14), dp(14));
                TextView tvTitle = new TextView(mCtx);
                tvTitle.setTextSize(19);
                TextView tvPath = new TextView(mCtx);
                tvPath.setTextSize(14);
                tvPath.setPadding(0, dp(8), 0, 0);
                itemLayout.addView(tvTitle);
                itemLayout.addView(tvPath);
                convertView = itemLayout;
                holder = new ViewHolder();
                holder.tvTitle = tvTitle;
                holder.tvPath = tvPath;
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }
            String line = mList.get(position);
            String[] parts = line.split("\\|==\\|");
            String title = parts.length >= 1 ? parts[0] : "";
            String pathText = parts.length >= 2 ? parts[1] : "";
            holder.tvTitle.setText(title);
            holder.tvPath.setText(pathText);

            //明暗两套选中背景色
            if (position == mSelectedPos) {
                if (mIsDark) {
                    convertView.setBackgroundColor(0xFF2b4860);
                } else {
                    convertView.setBackgroundColor(0xFFadd8e6);
                }
            } else {
                convertView.setBackgroundColor(Color.TRANSPARENT);
            }

            int textColor = mIsDark ? Color.WHITE : Color.BLACK;
            holder.tvTitle.setTextColor(textColor);
            holder.tvPath.setTextColor(mIsDark ? 0xFFcccccc : 0xFF444444);
            return convertView;
        }

        private class ViewHolder {

            TextView tvTitle;
            TextView tvPath;
        }
    }
}
