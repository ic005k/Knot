package com.x;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import androidx.activity.OnBackPressedCallback;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;
import java.util.ArrayList;

public class NoteRecycleBinActivity extends AppCompatActivity {

    public static NoteRecycleBinActivity mInstance = null;
    private LinearLayout mRootLayout;
    private TextView mTvTitle;
    private Button mBtnSelectAll;
    private Button mBtnCancelSelect;
    private ListView mListView;
    private LinearLayout mBottomBtnContainer;
    private NoteRecycleBinAdapter mAdapter;
    private ArrayList<String> mAllRawList = new ArrayList<>();
    private ArrayList<String> mSelectedItems = new ArrayList<>();
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
                finish();
            }
        };
        getOnBackPressedDispatcher().addCallback(this, mBackCallback);
        // 使用ImmersiveUtil返回值获取暗黑状态，项目统一方案
        mIsDark = ImmersiveUtil.applyRealImmersive(this);

        mRootLayout = new LinearLayout(this);
        mRootLayout.setOrientation(LinearLayout.VERTICAL);
        mRootLayout.setLayoutParams(
            new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        );

        // ========== 亮色 / 暗黑两套配色 ==========
        int bgColor;
        int cardBgColor;
        int itemBgColor;
        int textColor;
        int pathColor;
        int btnNormalBg;
        int checkBoxTint;
        if (mIsDark) {
            // 暗黑模式
            bgColor = 0xFF121212;
            cardBgColor = 0xFF1E1E1E;
            itemBgColor = 0xFF2C3E50;
            textColor = Color.WHITE;
            pathColor = 0xFFAAAAAA;
            btnNormalBg = 0xFF3A3A3A;
            checkBoxTint = 0xFF3478E8;
        } else {
            // 亮色模式
            bgColor = 0xFFF7F7F7;
            cardBgColor = 0xFFFFFFFF;
            itemBgColor = 0xFFECEFF1;
            textColor = 0xFF212121;
            pathColor = 0xFF757575;
            btnNormalBg = 0xFFD8D8D8;
            checkBoxTint = 0xFF2179D4;
        }

        mRootLayout.setBackgroundColor(bgColor);

        // ---------------- 标题栏 分为两行：标题一行，按钮一行 ----------------
        LinearLayout titleBar = new LinearLayout(this);
        titleBar.setOrientation(LinearLayout.VERTICAL);
        titleBar.setPadding(dp(16), dp(16), dp(16), dp(12));
        titleBar.setBackgroundColor(cardBgColor);

        // 第一行：标题
        mTvTitle = new TextView(this);
        mTvTitle.setText(MyActivity.zh_cn ? "笔记回收箱" : "Note Recycle Bin");
        mTvTitle.setTextSize(18);
        mTvTitle.setTextColor(textColor);
        LinearLayout.LayoutParams titleLp = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        );
        titleLp.bottomMargin = dp(10);
        mTvTitle.setLayoutParams(titleLp);
        titleBar.addView(mTvTitle);

        // 第二行：按钮容器，按钮组水平居中
        LinearLayout btnRow = new LinearLayout(this);
        btnRow.setOrientation(LinearLayout.HORIZONTAL);
        btnRow.setGravity(Gravity.CENTER);
        LinearLayout.LayoutParams btnRowLp = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        );
        btnRowLp.topMargin = dp(4);
        btnRow.setLayoutParams(btnRowLp);

        mBtnSelectAll = createTextButton(
            MyActivity.zh_cn ? "全选" : "Select All",
            btnNormalBg,
            textColor
        );
        mBtnSelectAll.setOnClickListener(v -> selectAll());

        mBtnCancelSelect = createTextButton(
            MyActivity.zh_cn ? "取消全选" : "Deselect All",
            btnNormalBg,
            textColor
        );
        mBtnCancelSelect.setOnClickListener(v -> cancelSelectAll());

        btnRow.addView(mBtnSelectAll);
        btnRow.addView(mBtnCancelSelect);
        titleBar.addView(btnRow);

        mRootLayout.addView(titleBar);

        // ---------------- 列表 ----------------
        mListView = new ListView(this);
        LinearLayout.LayoutParams listLp = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            0,
            1.0f
        );
        listLp.setMargins(dp(16), dp(12), dp(16), dp(12));
        mListView.setLayoutParams(listLp);
        mListView.setSelector(android.R.color.transparent);
        mListView.setDividerHeight(dp(1));
        mListView.setDivider(
            ContextCompat.getDrawable(
                this,
                android.R.drawable.divider_horizontal_textfield
            )
        );
        mListView.setBackgroundColor(cardBgColor);

        // 传入cardBgColor作为未选中条目的底色
        mAdapter = new NoteRecycleBinAdapter(
            this,
            mAllRawList,
            mSelectedItems,
            itemBgColor,
            cardBgColor,
            textColor,
            pathColor,
            checkBoxTint
        );
        mListView.setAdapter(mAdapter);
        mRootLayout.addView(mListView);

        // ---------------- 底部按钮栏：纯文字按钮【移动】【删除】 ----------------
        mBottomBtnContainer = new LinearLayout(this);
        mBottomBtnContainer.setOrientation(LinearLayout.HORIZONTAL);
        mBottomBtnContainer.setGravity(Gravity.CENTER);
        mBottomBtnContainer.setPadding(dp(12), dp(12), dp(12), dp(16));
        mBottomBtnContainer.setBackgroundColor(cardBgColor);

        Button btnMove = createBottomTextButton(
            MyActivity.zh_cn ? "移动" : "Move",
            0xFF3478E8
        );
        btnMove.setOnClickListener(v -> moveSelected());

        Button btnDelete = createBottomTextButton(
            MyActivity.zh_cn ? "删除" : "Delete",
            0xFFE53935
        );
        btnDelete.setOnClickListener(v -> deleteSelected());

        mBottomBtnContainer.addView(btnMove);
        mBottomBtnContainer.addView(btnDelete);
        mRootLayout.addView(mBottomBtnContainer);

        setContentView(mRootLayout);

        ArrayList<String> initList = getIntent().getStringArrayListExtra(
            "recycle_note_list"
        );
        if (initList != null) {
            setNoteList(initList);
        }
    }

    public void setNoteList(ArrayList<String> list) {
        new Handler(Looper.getMainLooper()).post(() -> {
            if (isFinishing() || isDestroyed()) return;
            mAllRawList.clear();
            mSelectedItems.clear();
            mAllRawList.addAll(list);
            mAdapter.notifyDataSetChanged();
        });
    }

    private void selectAll() {
        mSelectedItems.clear();
        mSelectedItems.addAll(mAllRawList);
        mAdapter.notifyDataSetChanged();
    }

    private void cancelSelectAll() {
        mSelectedItems.clear();
        mAdapter.notifyDataSetChanged();
    }

    private void moveSelected() {
        if (mSelectedItems.isEmpty()) {
            new AlertDialog.Builder(this)
                .setTitle(MyActivity.zh_cn ? "未选择笔记" : "No Note Selected")
                .setMessage(
                    MyActivity.zh_cn
                        ? "请先选择要移动的笔记。"
                        : "Please select notes to move first."
                )
                .setPositiveButton(MyActivity.zh_cn ? "确定" : "OK", null)
                .show();
            return;
        }
        StringBuilder sb = new StringBuilder();
        for (String item : mSelectedItems) {
            if (sb.length() > 0) sb.append("|==|");
            sb.append(item);
        }
        PublicJavaCallCpp("note_recycle_bin_move|==|" + sb.toString());
    }

    private void deleteSelected() {
        if (mSelectedItems.isEmpty()) {
            new AlertDialog.Builder(this)
                .setTitle(MyActivity.zh_cn ? "未选择笔记" : "No Note Selected")
                .setMessage(
                    MyActivity.zh_cn
                        ? "请先选择要删除的笔记。"
                        : "Please select notes to delete first."
                )
                .setPositiveButton(MyActivity.zh_cn ? "确定" : "OK", null)
                .show();
            return;
        }
        new AlertDialog.Builder(this)
            .setTitle(MyActivity.zh_cn ? "确认删除" : "Confirm Delete")
            .setMessage(
                MyActivity.zh_cn
                    ? "选中笔记将被永久删除，不可恢复。"
                    : "Selected notes will be permanently deleted and cannot be restored."
            )
            .setPositiveButton(MyActivity.zh_cn ? "删除" : "Delete", (d, w) -> {
                StringBuilder sb = new StringBuilder();
                for (String item : mSelectedItems) {
                    if (sb.length() > 0) sb.append("|==|");
                    sb.append(item);
                }
                PublicJavaCallCpp(
                    "note_recycle_bin_delete|==|" + sb.toString()
                );
                mSelectedItems.clear();
                mAdapter.notifyDataSetChanged();
            })
            .setNegativeButton(MyActivity.zh_cn ? "取消" : "Cancel", null)
            .show();
    }

    /**
     * 顶部「全选/取消全选」按钮，支持传入背景色、文字色
     */
    private Button createTextButton(String text, int bgColor, int textColor) {
        Button btn = new Button(this);
        btn.setText(text);
        btn.setTextSize(13);
        btn.setTextColor(textColor);
        btn.setBackgroundColor(bgColor);
        btn.setPadding(dp(14), dp(10), dp(14), dp(10));
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            dp(44)
        );
        lp.leftMargin = dp(8);
        btn.setLayoutParams(lp);
        return btn;
    }

    /**
     * 底部纯文字按钮，两个按钮平分宽度
     */
    private Button createBottomTextButton(String text, int bgColor) {
        Button btn = new Button(this);
        btn.setText(text);
        btn.setTextSize(17);
        btn.setTextColor(Color.WHITE);
        btn.setBackgroundColor(bgColor);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
            0,
            dp(52),
            1.0f
        );
        lp.setMargins(dp(8), 0, dp(8), 0);
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

    private static class NoteRecycleBinAdapter extends ArrayAdapter<String> {

        private final Context mCtx;
        private final ArrayList<String> mList;
        private final ArrayList<String> mSelectedItems;
        private final int mSelectedBg;
        private final int mNormalBg;
        private final int mTextColor;
        private final int mPathColor;
        private final int mCheckBoxTint;

        public NoteRecycleBinAdapter(
            @NonNull Context context,
            ArrayList<String> objects,
            ArrayList<String> selectedItems,
            int selectedBg,
            int normalBg,
            int textColor,
            int pathColor,
            int checkBoxTint
        ) {
            super(context, 0, objects);
            mCtx = context;
            mList = objects;
            mSelectedItems = selectedItems;
            mSelectedBg = selectedBg;
            mNormalBg = normalBg;
            mTextColor = textColor;
            mPathColor = pathColor;
            mCheckBoxTint = checkBoxTint;
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
                itemLayout.setOrientation(LinearLayout.HORIZONTAL);
                itemLayout.setGravity(Gravity.CENTER_VERTICAL);
                itemLayout.setPadding(dp(14), dp(14), dp(14), dp(14));
                // 初始化不给固定背景，运行时动态赋值
                CheckBox checkBox = new CheckBox(mCtx);
                checkBox.setButtonTintList(
                    android.content.res.ColorStateList.valueOf(mCheckBoxTint)
                );
                // 扩大CheckBox触摸热区
                checkBox.setPadding(dp(8), dp(8), dp(8), dp(8));

                LinearLayout textWrap = new LinearLayout(mCtx);
                textWrap.setOrientation(LinearLayout.VERTICAL);
                textWrap.setPadding(dp(12), 0, 0, 0);

                TextView tvTitle = new TextView(mCtx);
                tvTitle.setTextSize(19);
                tvTitle.setMaxLines(1);
                tvTitle.setEllipsize(TextUtils.TruncateAt.END);

                TextView tvPath = new TextView(mCtx);
                tvPath.setTextSize(14);
                tvPath.setPadding(0, dp(6), 0, 0);
                tvPath.setMaxLines(1);
                tvPath.setEllipsize(TextUtils.TruncateAt.MIDDLE);

                textWrap.addView(tvTitle);
                textWrap.addView(tvPath);

                itemLayout.addView(checkBox);
                itemLayout.addView(textWrap);

                convertView = itemLayout;
                holder = new ViewHolder();
                holder.itemLayout = itemLayout;
                holder.checkBox = checkBox;
                holder.tvTitle = tvTitle;
                holder.tvPath = tvPath;
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            final String currentItem = mList.get(position);
            String[] parts = currentItem.split("===");
            String title = parts.length >= 1 ? parts[0] : "";
            String path = parts.length >= 2 ? parts[1] : "";

            holder.tvTitle.setText(title);
            holder.tvPath.setText(path);
            holder.tvTitle.setTextColor(mTextColor);
            holder.tvPath.setTextColor(mPathColor);

            boolean checked = mSelectedItems.contains(currentItem);
            // 根据选中状态动态切换条目背景
            if (checked) {
                holder.itemLayout.setBackgroundColor(mSelectedBg);
            } else {
                holder.itemLayout.setBackgroundColor(mNormalBg);
            }

            // 复用时先清空监听，防止setChecked触发旧回调错乱
            holder.checkBox.setOnCheckedChangeListener(null);
            holder.checkBox.setChecked(checked);

            holder.checkBox.setOnCheckedChangeListener(
                (buttonView, isChecked) -> {
                    if (isChecked) {
                        if (!mSelectedItems.contains(currentItem)) {
                            mSelectedItems.add(currentItem);
                        }
                    } else {
                        mSelectedItems.remove(currentItem);
                    }
                    notifyDataSetChanged();
                }
            );

            // 整行点击切换勾选，提升触摸灵敏度
            convertView.setOnClickListener(v -> {
                holder.checkBox.setChecked(!holder.checkBox.isChecked());
            });

            return convertView;
        }

        private int dp(int dpVal) {
            float density = mCtx.getResources().getDisplayMetrics().density;
            return (int) (dpVal * density + 0.5f);
        }

        private static class ViewHolder {

            LinearLayout itemLayout;
            CheckBox checkBox;
            TextView tvTitle;
            TextView tvPath;
        }
    }
}
