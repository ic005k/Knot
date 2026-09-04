package com.x;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.Editable;
import android.text.Html;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.view.Gravity;
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

public class NoteSearchActivity extends AppCompatActivity {

    private ProgressDialog mSearchWaitingDialog;

    public static NoteSearchActivity mInstance = null;
    private LinearLayout mRootLayout;
    private EditText mEtSearchInput;
    private Button mBtnExactSearch;
    private TextView mTvResultCount;
    private ListView mListView;
    private NoteSearchAdapter mAdapter;
    private ArrayList<String> mResultList = new ArrayList<>();
    //预留预加载数组，打开窗口intent传入，暂不渲染，供后续扩展
    private ArrayList<String> mPreloadItemList = new ArrayList<>();
    private boolean mIsDark;
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
                finish();
            }
        };
        getOnBackPressedDispatcher().addCallback(this, mBackCallback);

        mIsDark = ImmersiveUtil.applyRealImmersive(this);

        //读取intent预留扩展预加载数组，可选传入，不做UI渲染
        Intent srcIntent = getIntent();
        ArrayList<String> preloadList = srcIntent.getStringArrayListExtra(
            "preload_item_list"
        );
        if (preloadList != null) {
            mPreloadItemList.clear();
            mPreloadItemList.addAll(preloadList);
        }

        mRootLayout = new LinearLayout(this);
        mRootLayout.setOrientation(LinearLayout.VERTICAL);
        mRootLayout.setLayoutParams(
            new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        );

        int bgColor, cardBg, selectBg, textMain, textSub, btnBg;
        if (mIsDark) {
            bgColor = 0xFF121212;
            cardBg = 0xFF1E1E1E;
            selectBg = 0xFF2C3E50;
            textMain = 0xFFFFFFFF;
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

        //搜索等待弹窗
        mSearchWaitingDialog = new ProgressDialog(this);
        mSearchWaitingDialog.setMessage(
            MyActivity.zh_cn ? "正在搜索…" : "Searching…"
        );
        mSearchWaitingDialog.setCancelable(false); //不允许点击返回取消，等待后端返回结果

        //1.【页面最顶部：搜索结果计数，左对齐，匹配截图UI】
        mTvResultCount = new TextView(this);
        mTvResultCount.setGravity(Gravity.LEFT);
        mTvResultCount.setTextSize(18);
        mTvResultCount.setTextColor(textMain);
        mTvResultCount.setPadding(dp(12), dp(12), dp(12), dp(4));
        mTvResultCount.setText(
            MyActivity.zh_cn ? "笔记搜索结果：0" : "Result: 0"
        );
        mRootLayout.addView(mTvResultCount);

        //2.搜索输入框+搜索按钮同一行布局
        LinearLayout layoutSearchRow = new LinearLayout(this);
        layoutSearchRow.setOrientation(LinearLayout.HORIZONTAL);
        layoutSearchRow.setGravity(Gravity.CENTER_VERTICAL);
        LinearLayout.LayoutParams lpSearchRow = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        );
        lpSearchRow.setMargins(dp(12), dp(0), dp(12), dp(8));
        layoutSearchRow.setLayoutParams(lpSearchRow);

        mEtSearchInput = new EditText(this);
        LinearLayout.LayoutParams lpEdit = new LinearLayout.LayoutParams(
            0,
            ViewGroup.LayoutParams.WRAP_CONTENT,
            1.0f
        );
        lpEdit.setMargins(0, 0, dp(8), 0);
        mEtSearchInput.setLayoutParams(lpEdit);
        mEtSearchInput.setTextColor(textMain);
        mEtSearchInput.setHintTextColor(textSub);
        //文本变化：触发AI语义搜索
        mEtSearchInput.addTextChangedListener(
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
                    String kw = s.toString().trim();
                    PublicJavaCallCpp("note_ai_search|==|" + kw);
                    //新搜索重置选中
                    mSelectedPosition = -1;
                    mAdapter.setSelectedIndex(-1);
                }

                @Override
                public void afterTextChanged(Editable s) {}
            }
        );
        layoutSearchRow.addView(mEtSearchInput);

        mBtnExactSearch = new Button(this);
        mBtnExactSearch.setText(MyActivity.zh_cn ? "搜索" : "Search");
        mBtnExactSearch.setBackgroundColor(btnBg);
        mBtnExactSearch.setTextColor(0xFFFFFFFF);
        mBtnExactSearch.setOnClickListener(v -> {
            String kw = mEtSearchInput.getText().toString().trim();
            if (kw.isEmpty()) {
                return;
            }

            mSearchWaitingDialog.show(); //打开等待框
            //点击按钮直接执行精准匹配，不再需要复选框
            PublicJavaCallCpp("note_exact_search|==|" + kw);
            mSelectedPosition = -1;
            mAdapter.setSelectedIndex(-1);
        });
        layoutSearchRow.addView(mBtnExactSearch);
        mRootLayout.addView(layoutSearchRow);

        //3.列表
        mListView = new ListView(this);
        LinearLayout.LayoutParams lpList = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            0,
            1.0f
        );
        lpList.setMargins(dp(12), 0, dp(12), dp(12));
        mListView.setLayoutParams(lpList);
        mListView.setBackgroundColor(cardBg);
        mListView.setDividerHeight(dp(1));
        mListView.setSelector(android.R.color.transparent);

        mAdapter = new NoteSearchAdapter(
            this,
            mResultList,
            cardBg,
            selectBg,
            textMain,
            textSub
        );
        mListView.setAdapter(mAdapter);
        //条目点击事件：只更新UI选中，留空，不调用C++
        mListView.setOnItemClickListener((parent, view, position, id) -> {
            mSelectedPosition = position;
            mAdapter.setSelectedIndex(mSelectedPosition);
            //==== 留空，不再发送 PublicJavaCallCpp 调用 ====
        });
        mRootLayout.addView(mListView);

        //4.底部按钮栏：查看、编辑，按钮之间增加间隔
        LinearLayout layoutBottom = new LinearLayout(this);
        layoutBottom.setOrientation(LinearLayout.HORIZONTAL);
        layoutBottom.setGravity(Gravity.CENTER);
        layoutBottom.setLayoutParams(
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
        );
        layoutBottom.setPadding(dp(8), dp(8), dp(8), dp(8));

        Button btnView = new Button(this);
        btnView.setText(MyActivity.zh_cn ? "查看" : "View");
        btnView.setBackgroundColor(btnBg);
        btnView.setTextColor(0xFFFFFFFF);
        LinearLayout.LayoutParams lpBtnView = new LinearLayout.LayoutParams(
            0,
            dp(52),
            1.0f
        );
        lpBtnView.setMargins(0, 0, dp(6), 0);
        btnView.setLayoutParams(lpBtnView);
        btnView.setOnClickListener(v -> {
            if (mSelectedPosition >= 0) {
                PublicJavaCallCpp("note_search_view|==|" + mSelectedPosition);
            }
        });

        Button btnEdit = new Button(this);
        btnEdit.setText(MyActivity.zh_cn ? "编辑" : "Edit");
        btnEdit.setBackgroundColor(btnBg);
        btnEdit.setTextColor(0xFFFFFFFF);
        btnEdit.setLayoutParams(new LinearLayout.LayoutParams(0, dp(52), 1.0f));
        btnEdit.setOnClickListener(v -> {
            if (mSelectedPosition >= 0) {
                PublicJavaCallCpp("note_search_edit|==|" + mSelectedPosition);
            }
        });

        layoutBottom.addView(btnView);
        layoutBottom.addView(btnEdit);
        mRootLayout.addView(layoutBottom);

        setContentView(mRootLayout);
    }

    /** C++调用推送搜索结果，格式 title===previewHtml===filePath */
    public void setSearchResult(ArrayList<String> rawArray) {
        new Handler(Looper.getMainLooper()).post(() -> {
            if (isFinishing() || isDestroyed()) return;

            //关闭等待弹窗
            if (
                mSearchWaitingDialog != null && mSearchWaitingDialog.isShowing()
            ) {
                mSearchWaitingDialog.dismiss();
            }

            mResultList.clear();
            for (String item : rawArray) {
                if (!TextUtils.isEmpty(item)) {
                    mResultList.add(item);
                }
            }
            mSelectedPosition = -1;
            mAdapter.setSelectedIndex(-1);
            mTvResultCount.setText(
                MyActivity.zh_cn
                    ? "笔记搜索结果：" + mResultList.size()
                    : "Result: " + mResultList.size()
            );
            mAdapter.notifyDataSetChanged();
        });
    }

    /** 预留：C++主动设置选中条目 */
    public void setSelectPos(int pos) {
        mSelectedPosition = pos;
        mAdapter.setSelectedIndex(pos);
    }

    private int dp(int dpVal) {
        float density = getResources().getDisplayMetrics().density;
        return (int) (dpVal * density + 0.5f);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mBackCallback != null) mBackCallback.remove();
        //销毁弹窗
        if (mSearchWaitingDialog != null && mSearchWaitingDialog.isShowing()) {
            mSearchWaitingDialog.dismiss();
        }
        mInstance = null;
    }

    private static class NoteSearchAdapter extends ArrayAdapter<String> {

        private final Context mCtx;
        private final ArrayList<String> mData;
        private final int mNormalBg;
        private final int mSelectBg;
        private final int mTextMain;
        private final int mTextSub;
        private int mSelectedIndex = -1;

        public NoteSearchAdapter(
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
            TextView tvTitle; //粗体
            TextView tvPreview; //支持html高亮
            TextView tvPath; //中间省略
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;
            if (convertView == null) {
                LinearLayout itemRoot = new LinearLayout(mCtx);
                itemRoot.setOrientation(LinearLayout.VERTICAL);
                itemRoot.setPadding(dp(14), dp(12), dp(14), dp(12));

                TextView tvTitle = new TextView(mCtx);
                tvTitle.setTextSize(18);
                tvTitle.setTextColor(mTextMain);
                tvTitle.setTypeface(null, android.graphics.Typeface.BOLD);

                TextView tvPreview = new TextView(mCtx);
                tvPreview.setTextSize(15);
                tvPreview.setTextColor(mTextSub);
                tvPreview.setPadding(0, dp(4), 0, dp(4));

                TextView tvPath = new TextView(mCtx);
                tvPath.setTextSize(13);
                tvPath.setTextColor(mTextSub);
                tvPath.setEllipsize(android.text.TextUtils.TruncateAt.MIDDLE);
                tvPath.setSingleLine(true);

                itemRoot.addView(tvTitle);
                itemRoot.addView(tvPreview);
                itemRoot.addView(tvPath);

                convertView = itemRoot;
                holder = new ViewHolder();
                holder.rootItem = itemRoot;
                holder.tvTitle = tvTitle;
                holder.tvPreview = tvPreview;
                holder.tvPath = tvPath;
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            String line = mData.get(position);
            String[] parts = line.split("===");
            String title = parts.length >= 1 ? parts[0] : "";
            String preview = parts.length >= 2 ? parts[1] : "";
            String filePath = parts.length >= 3 ? parts[2] : "";

            //标题
            if (TextUtils.isEmpty(title)) {
                holder.tvTitle.setVisibility(View.GONE);
            } else {
                holder.tvTitle.setVisibility(View.VISIBLE);
                holder.tvTitle.setText(title);
            }

            //预览 html
            if (TextUtils.isEmpty(preview)) {
                holder.tvPreview.setVisibility(View.GONE);
            } else {
                holder.tvPreview.setVisibility(View.VISIBLE);
                if (
                    android.os.Build.VERSION.SDK_INT >=
                    android.os.Build.VERSION_CODES.N
                ) {
                    holder.tvPreview.setText(
                        Html.fromHtml(preview, Html.FROM_HTML_MODE_LEGACY)
                    );
                } else {
                    holder.tvPreview.setText(Html.fromHtml(preview));
                }
            }

            //文件路径，中间省略
            if (TextUtils.isEmpty(filePath)) {
                holder.tvPath.setVisibility(View.GONE);
            } else {
                holder.tvPath.setVisibility(View.VISIBLE);
                holder.tvPath.setText(filePath);
            }

            //选中背景
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
