package com.x;

import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.Html;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;
import java.util.ArrayList;

public class MyEventSearchActivity extends AppCompatActivity {

    public static MyEventSearchActivity mInstance = null;
    private LinearLayout mRootLayout;
    private EditText mEtKeyword;
    private TextView mTvResultCount;
    private ListView mListView;
    private MyEventSearchAdapter mAdapter;
    private ArrayList<String> mResultList = new ArrayList<>();
    // 预留接收外部传入数组，暂不处理，用于后续扩展
    private ArrayList<String> mReservedEventList = new ArrayList<>();
    private boolean mIsDark;
    // 选中位置 -1代表无选中
    private int mSelectedPosition = -1;

    public static native void PublicJavaCallCpp(String cmd);

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

        //统一暗黑模式获取
        mIsDark = ImmersiveUtil.applyRealImmersive(this);

        //读取intent传入的event_list，只保存，不做任何业务逻辑处理，预留扩展
        Intent srcIntent = getIntent();
        if (srcIntent != null) {
            ArrayList<String> temp = srcIntent.getStringArrayListExtra(
                "event_list"
            );
            if (temp != null) {
                mReservedEventList.clear();
                mReservedEventList.addAll(temp);
            }
        }

        mRootLayout = new LinearLayout(this);
        mRootLayout.setOrientation(LinearLayout.VERTICAL);
        mRootLayout.setLayoutParams(
            new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        );

        //配色
        int bgColor, cardBg, selectBg, textMain, textSub, btnBg;
        if (mIsDark) {
            bgColor = 0xFF121212;
            cardBg = 0xFF1E1E1E;
            selectBg = 0xFF2C3E50;
            textMain = Color.WHITE;
            textSub = 0xFFAAAAAA;
            btnBg = 0xFF3478E8;
        } else {
            bgColor = 0xFFF7F7F7;
            cardBg = 0xFFFFFFFF;
            selectBg = 0xFFE0EDF7;
            textMain = 0xFF111111;
            textSub = 0xFF555555;
            btnBg = 0xFF2179D4;
        }
        mRootLayout.setBackgroundColor(bgColor);

        //----------顶部搜索栏 改用Emoji文本图标，图标GONE隐藏，代码保留以备后用----------
        LinearLayout topBar = new LinearLayout(this);
        topBar.setOrientation(LinearLayout.HORIZONTAL);
        topBar.setPadding(dp(12), dp(12), dp(12), dp(8));
        topBar.setGravity(Gravity.CENTER_VERTICAL);

        TextView tvEmoji1 = new TextView(this);
        tvEmoji1.setText(MyActivity.zh_cn ? "关键词：" : "Key Words:");
        tvEmoji1.setTextSize(18);
        tvEmoji1.setTextColor(textMain);
        tvEmoji1.setPadding(dp(4), dp(4), dp(8), dp(4));
        //tvEmoji1.setVisibility(View.GONE); // 暂时隐藏

        TextView tvEmoji2 = new TextView(this);
        tvEmoji2.setText("🔍");
        tvEmoji2.setTextSize(24);
        tvEmoji2.setTextColor(textMain);
        tvEmoji2.setPadding(dp(4), dp(4), dp(8), dp(4));
        tvEmoji2.setVisibility(View.GONE); // 暂时隐藏

        mEtKeyword = new EditText(this);
        mEtKeyword.setLayoutParams(
            new LinearLayout.LayoutParams(
                0,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                1.0f
            )
        );
        mEtKeyword.setTextColor(textMain);
        mEtKeyword.setHintTextColor(textSub);
        mEtKeyword.setHint("");
        mEtKeyword.setOnKeyListener((v, keyCode, event) -> {
            if (
                keyCode == KeyEvent.KEYCODE_ENTER &&
                event.getAction() == KeyEvent.ACTION_UP
            ) {
                triggerSearch();
                return true;
            }
            return false;
        });

        Button btnSearch = new Button(this);
        btnSearch.setText(MyActivity.zh_cn ? "搜索" : "Search");
        btnSearch.setBackgroundColor(btnBg);
        btnSearch.setTextColor(Color.WHITE);
        btnSearch.setOnClickListener(v -> triggerSearch());
        btnSearch.setVisibility(View.GONE); // 暂时隐藏

        topBar.addView(tvEmoji1);
        topBar.addView(tvEmoji2);
        topBar.addView(mEtKeyword);
        topBar.addView(btnSearch);
        mRootLayout.addView(topBar);

        //结果计数文本
        mTvResultCount = new TextView(this);
        mTvResultCount.setGravity(Gravity.CENTER);
        mTvResultCount.setTextSize(16);
        mTvResultCount.setTextColor(textSub);
        mTvResultCount.setPadding(0, dp(4), 0, dp(8));
        mRootLayout.addView(mTvResultCount);

        //----------ListView列表----------
        mListView = new ListView(this);
        LinearLayout.LayoutParams listLp = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            0,
            1.0f
        );
        listLp.setMargins(dp(12), 0, dp(12), dp(12));
        mListView.setLayoutParams(listLp);
        mListView.setBackgroundColor(cardBg);
        mListView.setDividerHeight(dp(1));
        mListView.setSelector(android.R.color.transparent);

        mAdapter = new MyEventSearchAdapter(
            this,
            mResultList,
            cardBg,
            selectBg,
            textMain,
            textSub
        );
        mListView.setAdapter(mAdapter);

        //列表点击事件：切换选中，通知C++
        mListView.setOnItemClickListener((parent, view, position, id) -> {
            mSelectedPosition = position;
            mAdapter.setSelectedIndex(mSelectedPosition);
            // 通知C++选中条目索引
            PublicJavaCallCpp("search_item_click|==|" + position);
        });

        mRootLayout.addView(mListView);

        //----------底部返回按钮----------
        Button btnBack = new Button(this);
        btnBack.setText(MyActivity.zh_cn ? "搜索" : "Search");
        btnBack.setBackgroundColor(btnBg);
        btnBack.setTextColor(Color.WHITE);
        btnBack.setLayoutParams(
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                dp(52)
            )
        );
        btnBack.setOnClickListener(v -> {
            triggerSearch();
        });
        mRootLayout.addView(btnBack);

        setContentView(mRootLayout);
    }

    /** 触发搜索，调用C++ */
    private void triggerSearch() {
        String kw = mEtKeyword.getText().toString().trim();
        PublicJavaCallCpp("myevent_search|==|" + kw);
        //搜索新结果，重置选中状态
        mSelectedPosition = -1;
        mAdapter.setSelectedIndex(-1);
    }

    /** C++回调推送搜索结果：全部有效条目，不再限制5条，仅过滤空字符串 */
    public void setSearchResult(ArrayList<String> rawArray) {
        new Handler(Looper.getMainLooper()).post(() -> {
            if (isFinishing() || isDestroyed()) return;
            mResultList.clear();
            for (String item : rawArray) {
                if (!TextUtils.isEmpty(item)) {
                    mResultList.add(item);
                }
            }
            //收到新搜索结果，重置选中
            mSelectedPosition = -1;
            mAdapter.setSelectedIndex(-1);

            mTvResultCount.setText(
                MyActivity.zh_cn
                    ? "结果：" + mResultList.size()
                    : "Result: " + mResultList.size()
            );
            mAdapter.notifyDataSetChanged();
        });
    }

    private int dp(int dpVal) {
        float density = getResources().getDisplayMetrics().density;
        return (int) (dpVal * density + 0.5f);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mBackCallback != null) mBackCallback.remove();
        mInstance = null;
    }

    private static class MyEventSearchAdapter extends ArrayAdapter<String> {

        private final Context mCtx;
        private final ArrayList<String> mData;
        private final int mNormalBg;
        private final int mSelectBg;
        private final int mTextMain;
        private final int mTextSub;
        private int mSelectedIndex = -1;

        public MyEventSearchAdapter(
            Context ctx,
            ArrayList<String> data,
            int normalBg,
            int selectBg,
            int mainText,
            int subText
        ) {
            super(ctx, 0, data);
            mCtx = ctx;
            mData = data;
            mNormalBg = normalBg;
            mSelectBg = selectBg;
            mTextMain = mainText;
            mTextSub = subText;
        }

        public void setSelectedIndex(int idx) {
            mSelectedIndex = idx;
            notifyDataSetChanged();
        }

        static class ViewHolder {

            LinearLayout rootItem;
            TextView tv0; // str_tab 粗体
            TextView tv1; // str0 粗体
            TextView tv2; // text1 html
            TextView tv3; // text2 html
            TextView tv4; // text3 html
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;
            if (convertView == null) {
                LinearLayout itemRoot = new LinearLayout(mCtx);
                itemRoot.setOrientation(LinearLayout.VERTICAL);
                itemRoot.setPadding(dp(14), dp(12), dp(14), dp(12));

                TextView tv0 = new TextView(mCtx);
                tv0.setTextSize(18);
                tv0.setTextColor(mTextMain);
                tv0.setTypeface(null, android.graphics.Typeface.BOLD);

                TextView tv1 = new TextView(mCtx);
                tv1.setTextSize(18);
                tv1.setTextColor(mTextMain);
                tv1.setTypeface(null, android.graphics.Typeface.BOLD);
                tv1.setPadding(0, dp(4), 0, dp(4));

                TextView tv2 = new TextView(mCtx);
                tv2.setTextSize(15);
                tv2.setTextColor(mTextSub);
                tv2.setPadding(0, dp(2), 0, dp(2));

                TextView tv3 = new TextView(mCtx);
                tv3.setTextSize(15);
                tv3.setTextColor(mTextSub);
                tv3.setPadding(0, dp(2), 0, dp(2));

                TextView tv4 = new TextView(mCtx);
                tv4.setTextSize(15);
                tv4.setTextColor(mTextSub);
                tv4.setPadding(0, dp(2), 0, dp(2));

                itemRoot.addView(tv0);
                itemRoot.addView(tv1);
                itemRoot.addView(tv2);
                itemRoot.addView(tv3);
                itemRoot.addView(tv4);

                convertView = itemRoot;
                holder = new ViewHolder();
                holder.rootItem = itemRoot;
                holder.tv0 = tv0;
                holder.tv1 = tv1;
                holder.tv2 = tv2;
                holder.tv3 = tv3;
                holder.tv4 = tv4;
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            String line = mData.get(position);
            String[] parts = line.split("===");
            String s0 = parts.length >= 1 ? parts[0] : "";
            String s1 = parts.length >= 2 ? parts[1] : "";
            String s2 = parts.length >= 3 ? parts[2] : "";
            String s3 = parts.length >= 4 ? parts[3] : "";
            String s4 = parts.length >= 5 ? parts[4] : "";

            // tv0 粗体
            if (TextUtils.isEmpty(s0)) {
                holder.tv0.setVisibility(View.GONE);
            } else {
                holder.tv0.setVisibility(View.VISIBLE);
                holder.tv0.setText(s0);
            }
            // tv1 粗体
            if (TextUtils.isEmpty(s1)) {
                holder.tv1.setVisibility(View.GONE);
            } else {
                holder.tv1.setVisibility(View.VISIBLE);
                holder.tv1.setText(s1);
            }
            // tv2 html解析
            if (TextUtils.isEmpty(s2)) {
                holder.tv2.setVisibility(View.GONE);
            } else {
                holder.tv2.setVisibility(View.VISIBLE);
                if (
                    android.os.Build.VERSION.SDK_INT >=
                    android.os.Build.VERSION_CODES.N
                ) {
                    holder.tv2.setText(
                        Html.fromHtml(s2, Html.FROM_HTML_MODE_LEGACY)
                    );
                } else {
                    holder.tv2.setText(Html.fromHtml(s2));
                }
            }
            // tv3 html解析
            if (TextUtils.isEmpty(s3)) {
                holder.tv3.setVisibility(View.GONE);
            } else {
                holder.tv3.setVisibility(View.VISIBLE);
                if (
                    android.os.Build.VERSION.SDK_INT >=
                    android.os.Build.VERSION_CODES.N
                ) {
                    holder.tv3.setText(
                        Html.fromHtml(s3, Html.FROM_HTML_MODE_LEGACY)
                    );
                } else {
                    holder.tv3.setText(Html.fromHtml(s3));
                }
            }
            // tv4 html解析
            if (TextUtils.isEmpty(s4)) {
                holder.tv4.setVisibility(View.GONE);
            } else {
                holder.tv4.setVisibility(View.VISIBLE);
                if (
                    android.os.Build.VERSION.SDK_INT >=
                    android.os.Build.VERSION_CODES.N
                ) {
                    holder.tv4.setText(
                        Html.fromHtml(s4, Html.FROM_HTML_MODE_LEGACY)
                    );
                } else {
                    holder.tv4.setText(Html.fromHtml(s4));
                }
            }

            //选中背景切换
            if (position == mSelectedIndex) {
                holder.rootItem.setBackgroundColor(mSelectBg);
            } else {
                holder.rootItem.setBackgroundColor(mNormalBg);
            }
            return convertView;
        }

        private int dp(int dpVal) {
            float density = mCtx.getResources().getDisplayMetrics().density;
            return (int) (dpVal * density + 0.5f);
        }
    }
}
