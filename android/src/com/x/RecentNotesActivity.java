package com.x;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import androidx.activity.OnBackPressedCallback;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;
import java.util.ArrayList;

public class RecentNotesActivity extends AppCompatActivity {

    public static RecentNotesActivity mInstance = null;
    private LinearLayout mRootLayout;
    private TextView mTvTitle;
    private EditText mSearchInput;
    private ListView mListView;
    private LinearLayout mBottomBtnContainer;
    private RecentNotesAdapter mAdapter;
    private ArrayList<String> mAllRawList = new ArrayList<>();
    private ArrayList<String> mFilteredList = new ArrayList<>();
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
                //PublicJavaCallCpp("cancel_add_event_record");
                finish();
            }
        };
        getOnBackPressedDispatcher().addCallback(this, mBackCallback);
        mIsDark = ImmersiveUtil.applyRealImmersive(this);
        mRootLayout = new LinearLayout(this);
        mRootLayout.setOrientation(LinearLayout.VERTICAL);
        mRootLayout.setLayoutParams(
            new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        );
        int bgColor = mIsDark ? 0xFF121212 : 0xFFF5F5F5;
        int textColor = mIsDark ? Color.WHITE : Color.BLACK;
        int hintColor = mIsDark ? 0xFFAAAAAA : 0xFF666666;
        int cardBgColor = mIsDark ? 0xFF1E1E1E : Color.WHITE;
        mRootLayout.setBackgroundColor(bgColor);

        // ---------------- 标题 ----------------
        mTvTitle = new TextView(this);
        mTvTitle.setText(MyActivity.zh_cn ? "最近的笔记" : "Recent Notes");
        mTvTitle.setTextSize(22);
        mTvTitle.setTextColor(textColor);
        mTvTitle.setPadding(dp(16), dp(16), dp(16), dp(8));
        mRootLayout.addView(mTvTitle);

        // ---------------- 搜索输入框 ----------------
        LinearLayout searchWrap = new LinearLayout(this);
        searchWrap.setOrientation(LinearLayout.HORIZONTAL);
        searchWrap.setGravity(Gravity.CENTER_VERTICAL);
        searchWrap.setBackgroundResource(android.R.drawable.edit_text);
        searchWrap.setBackgroundTintList(
            android.content.res.ColorStateList.valueOf(
                mIsDark ? 0xFF3A3A3A : 0xFFE0E0E0
            )
        );
        searchWrap.setPadding(dp(12), dp(12), dp(12), dp(12));
        searchWrap.setGravity(Gravity.CENTER_VERTICAL);

        mSearchInput = new EditText(this);
        mSearchInput.setBackground(null);
        mSearchInput.setTextSize(16);
        mSearchInput.setTextColor(textColor);
        mSearchInput.setHint(
            MyActivity.zh_cn ? "输入笔记标题关键字" : "Search note title"
        );
        mSearchInput.setHintTextColor(hintColor);
        mSearchInput.setSingleLine(true);

        Drawable searchIcon = ContextCompat.getDrawable(
            this,
            android.R.drawable.ic_menu_search
        );
        if (searchIcon != null) {
            searchIcon.setBounds(0, 0, dp(20), dp(20));
            searchIcon.setTint(hintColor);
            mSearchInput.setCompoundDrawablesRelativeWithIntrinsicBounds(
                null,
                null,
                searchIcon,
                null
            );
        }

        LinearLayout.LayoutParams searchLp = new LinearLayout.LayoutParams(
            0,
            ViewGroup.LayoutParams.WRAP_CONTENT,
            1.0f
        );
        mSearchInput.setLayoutParams(searchLp);
        searchWrap.addView(mSearchInput);

        LinearLayout.LayoutParams wrapLp = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        );
        wrapLp.setMargins(dp(16), 0, dp(16), dp(12));
        searchWrap.setLayoutParams(wrapLp);
        mRootLayout.addView(searchWrap);

        // ---------------- 笔记列表 ----------------
        mListView = new ListView(this);
        LinearLayout.LayoutParams listLp = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            0,
            1.0f
        );
        listLp.leftMargin = dp(16);
        listLp.rightMargin = dp(16);
        mListView.setLayoutParams(listLp);
        mListView.setSelector(android.R.color.transparent);
        mListView.setDividerHeight(1);
        mListView.setDivider(
            ContextCompat.getDrawable(
                this,
                android.R.drawable.divider_horizontal_textfield
            )
        );
        mListView.setBackgroundColor(cardBgColor);

        mAdapter = new RecentNotesAdapter(this, mFilteredList, mIsDark);
        mListView.setAdapter(mAdapter);
        mListView.setOnItemClickListener((parent, view, position, id) -> {
            mSelectedPos = position;
            mAdapter.setSelectedPosition(position);
            mAdapter.notifyDataSetChanged();
        });
        mRootLayout.addView(mListView);

        // ---------------- 底部按钮栏 ----------------
        mBottomBtnContainer = new LinearLayout(this);
        mBottomBtnContainer.setOrientation(LinearLayout.HORIZONTAL);
        mBottomBtnContainer.setGravity(Gravity.CENTER);
        mBottomBtnContainer.setPadding(dp(12), dp(12), dp(12), dp(16));

        Button btnBack = createButton(MyActivity.zh_cn ? "返回" : "Back");
        btnBack.setOnClickListener(v -> {
            getOnBackPressedDispatcher().onBackPressed();
        });
        btnBack.setVisibility(View.GONE); // 暂时隐藏

        Button btnView = createButton(MyActivity.zh_cn ? "查看" : "View");
        btnView.setOnClickListener(v -> {
            if (mSelectedPos < 0) return;
            String item = mFilteredList.get(mSelectedPos);
            String[] parts = item.split("===");
            String filePath = parts.length >= 2 ? parts[1] : "";
            PublicJavaCallCpp("recent_note_view|==|" + filePath);
        });

        Button btnEdit = createButton(MyActivity.zh_cn ? "编辑" : "Edit");
        btnEdit.setOnClickListener(v -> {
            if (mSelectedPos < 0) return;
            String item = mFilteredList.get(mSelectedPos);
            String[] parts = item.split("===");
            String filePath = parts.length >= 2 ? parts[1] : "";
            PublicJavaCallCpp("recent_note_edit|==|" + filePath);
        });

        mBottomBtnContainer.addView(btnBack);
        mBottomBtnContainer.addView(btnView);
        mBottomBtnContainer.addView(btnEdit);
        mRootLayout.addView(mBottomBtnContainer);

        setContentView(mRootLayout);

        // 接收初始数据
        ArrayList<String> initList = getIntent().getStringArrayListExtra(
            "recent_note_list"
        );
        if (initList != null) {
            setNoteList(initList);
        }

        // 搜索筛选
        mSearchInput.addTextChangedListener(
            new TextWatcher() {
                @Override
                public void beforeTextChanged(
                    CharSequence s,
                    int start,
                    int count,
                    int after
                ) {}

                @Override
                public void onTextChanged(
                    CharSequence s,
                    int start,
                    int before,
                    int count
                ) {
                    filterList(s.toString());
                }

                @Override
                public void afterTextChanged(Editable s) {}
            }
        );
    }

    /**
     * JNI / 外部入口：设置最近笔记列表
     * 每条格式：标题===路径
     */
    public void setNoteList(ArrayList<String> list) {
        new Handler(Looper.getMainLooper()).post(() -> {
            if (isFinishing() || isDestroyed()) return;
            mAllRawList.clear();
            mAllRawList.addAll(list);
            filterList(mSearchInput.getText().toString());
        });
    }

    private void filterList(String keyword) {
        mFilteredList.clear();
        mSelectedPos = -1;
        mAdapter.setSelectedPosition(-1);
        if (keyword == null || keyword.trim().isEmpty()) {
            mFilteredList.addAll(mAllRawList);
        } else {
            String lower = keyword.toLowerCase();
            for (String item : mAllRawList) {
                String[] parts = item.split("===");
                String title = parts.length >= 1 ? parts[0] : "";
                String path = parts.length >= 2 ? parts[1] : "";
                if (
                    title.toLowerCase().contains(lower) ||
                    path.toLowerCase().contains(lower)
                ) {
                    mFilteredList.add(item);
                }
            }
        }
        mAdapter.notifyDataSetChanged();
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

    private static class RecentNotesAdapter extends ArrayAdapter<String> {

        private final Context mCtx;
        private final ArrayList<String> mList;
        private final boolean mIsDark;
        private int mSelectedPosition = -1;

        public RecentNotesAdapter(
            @NonNull Context context,
            ArrayList<String> objects,
            boolean isDark
        ) {
            super(context, 0, objects);
            mCtx = context;
            mList = objects;
            mIsDark = isDark;
        }

        public void setSelectedPosition(int pos) {
            mSelectedPosition = pos;
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
                tvTitle.setMaxLines(1);
                tvTitle.setEllipsize(TextUtils.TruncateAt.END);

                TextView tvPath = new TextView(mCtx);
                tvPath.setTextSize(14);
                tvPath.setPadding(0, dp(6), 0, 0);
                tvPath.setMaxLines(1);
                // 路径中间省略
                tvPath.setEllipsize(TextUtils.TruncateAt.MIDDLE);

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
            String[] parts = line.split("===");
            String title = parts.length >= 1 ? parts[0] : "";
            String path = parts.length >= 2 ? parts[1] : "";

            holder.tvTitle.setText(title);
            holder.tvPath.setText(path);

            int textColor = mIsDark ? Color.WHITE : Color.BLACK;
            int pathColor = mIsDark ? 0xFFAAAAAA : 0xFF666666;
            holder.tvTitle.setTextColor(textColor);
            holder.tvPath.setTextColor(pathColor);

            // 选中背景，明暗两套配色，和你之前笔记本列表保持风格统一
            if (position == mSelectedPosition) {
                if (mIsDark) {
                    convertView.setBackgroundColor(0xFF2b4060);
                } else {
                    convertView.setBackgroundColor(0xFFcce0f8);
                }
            } else {
                convertView.setBackgroundColor(Color.TRANSPARENT);
            }
            return convertView;
        }

        private int dp(int dpVal) {
            float density = mCtx.getResources().getDisplayMetrics().density;
            return (int) (dpVal * density + 0.5f);
        }

        private static class ViewHolder {

            TextView tvTitle;
            TextView tvPath;
        }
    }
}
