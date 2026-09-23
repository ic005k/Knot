package com.x;

import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.NumberPicker;
import android.widget.ScrollView;
import android.widget.TextView;
import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;
import java.util.ArrayList;
import java.util.Calendar;

public class DataReportActivity extends AppCompatActivity {

    public static DataReportActivity mInstance = null;

    private TextView mTvTitle1;
    private TextView mTvTitle2;
    private ListView mListView;
    private ReportItemAdapter mAdapter;
    private int mSelectedPos = -1;
    private boolean mIsDark;

    private androidx.appcompat.app.AlertDialog mAiLoadingDialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mInstance = this;
        mIsDark = ImmersiveUtil.applyRealImmersive(this);

        OnBackPressedCallback backCallback = new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                //MyActivity.m_instance.PublicJavaCallCpp("cancel_data_report");
                finish();
            }
        };
        getOnBackPressedDispatcher().addCallback(this, backCallback);

        int bgColor = mIsDark ? 0xFF121212 : 0xFFF5F5F5;
        int textColor = mIsDark ? 0xFFFFFFFF : 0xFF000000;
        int btnBgNormal = mIsDark ? 0xFF303030 : 0xFFE8E8E8;
        int itemSelectBg = mIsDark ? 0xFF2A2A2A : 0xFFE0EDFB;

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setLayoutParams(
            new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        );
        root.setBackgroundColor(bgColor);

        // 标题1
        mTvTitle1 = new TextView(this);
        mTvTitle1.setTextSize(18);
        mTvTitle1.setTextColor(textColor);
        mTvTitle1.getPaint().setFakeBoldText(true);
        mTvTitle1.setPadding(dp(12), dp(12), dp(12), dp(4));
        root.addView(mTvTitle1);

        // 标题2
        mTvTitle2 = new TextView(this);
        mTvTitle2.setTextSize(16);
        mTvTitle2.setTextColor(textColor);
        mTvTitle2.setPadding(dp(12), dp(4), dp(12), dp(12));
        root.addView(mTvTitle2);

        // 按钮栏：年、年月、区间
        LinearLayout btnBar = new LinearLayout(this);
        btnBar.setOrientation(LinearLayout.HORIZONTAL);
        btnBar.setGravity(Gravity.CENTER_HORIZONTAL);
        btnBar.setPadding(dp(8), dp(4), dp(8), dp(4));

        Button btnYear = new Button(this);
        btnYear.setText(MyActivity.zh_cn ? "年" : "Year");
        btnYear.setBackgroundColor(btnBgNormal);
        btnYear.setTextColor(textColor);
        LinearLayout.LayoutParams lpY = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        );
        lpY.setMargins(dp(4), 0, dp(4), 0);
        btnYear.setLayoutParams(lpY);
        btnYear.setOnClickListener(v -> showYearPicker());
        btnBar.addView(btnYear);

        Button btnYm = new Button(this);
        btnYm.setText(MyActivity.zh_cn ? "年月" : "Year‑Month");
        btnYm.setBackgroundColor(btnBgNormal);
        btnYm.setTextColor(textColor);
        LinearLayout.LayoutParams lpYm = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        );
        lpYm.setMargins(dp(4), 0, dp(4), 0);
        btnYm.setLayoutParams(lpYm);
        btnYm.setOnClickListener(v -> showYearMonthPicker());
        btnBar.addView(btnYm);

        Button btnRange = new Button(this);
        btnRange.setText(MyActivity.zh_cn ? "区间" : "Range");
        btnRange.setBackgroundColor(btnBgNormal);
        btnRange.setTextColor(textColor);
        LinearLayout.LayoutParams lpR = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        );
        lpR.setMargins(dp(4), 0, dp(4), 0);
        btnRange.setLayoutParams(lpR);
        btnRange.setOnClickListener(v -> showDateRangePicker());
        btnBar.addView(btnRange);

        root.addView(btnBar);

        // 列表
        mListView = new ListView(this);
        mListView.setLayoutParams(
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                0,
                1.0f
            )
        );
        mListView.setPadding(dp(8), dp(4), dp(8), dp(4));
        mAdapter = new ReportItemAdapter(
            this,
            new ArrayList<>(),
            itemSelectBg,
            textColor
        );
        mListView.setAdapter(mAdapter);
        mListView.setOnItemClickListener((parent, view, position, id) -> {
            mSelectedPos = position;
            mAdapter.notifyDataSetChanged();
        });
        root.addView(mListView);

        // 底部按钮栏：详情、分类
        LinearLayout bottomBar = new LinearLayout(this);
        bottomBar.setOrientation(LinearLayout.HORIZONTAL);
        bottomBar.setGravity(Gravity.CENTER);
        bottomBar.setPadding(dp(8), dp(8), dp(8), dp(12));

        Button btnDetail = new Button(this);
        btnDetail.setText(MyActivity.zh_cn ? "详情" : "Detail");
        btnDetail.setBackgroundColor(btnBgNormal);
        btnDetail.setTextColor(textColor);
        LinearLayout.LayoutParams lpD = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        );
        lpD.setMargins(dp(6), 0, dp(12), 0);
        btnDetail.setLayoutParams(lpD);
        btnDetail.setOnClickListener(v -> {
            if (mSelectedPos >= 0) {
                MyActivity.m_instance.PublicJavaCallCpp(
                    "data_report_detail|==|" + mSelectedPos
                );
            }
        });
        bottomBar.addView(btnDetail);

        Button btnCategory = new Button(this);
        btnCategory.setText(MyActivity.zh_cn ? "分类" : "Category");
        btnCategory.setBackgroundColor(btnBgNormal);
        btnCategory.setTextColor(textColor);
        LinearLayout.LayoutParams lpC = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        );
        lpC.setMargins(dp(12), 0, dp(6), 0);
        btnCategory.setLayoutParams(lpC);
        btnCategory.setOnClickListener(v -> {
            MyActivity.m_instance.PublicJavaCallCpp("data_report_category");
        });
        bottomBar.addView(btnCategory);

        root.addView(bottomBar);

        setContentView(root);

        // 打开时接收数组，预留扩展，不做处理
        Intent intent = getIntent();
        ArrayList<String> initExtra = intent.getStringArrayListExtra(
            "report_extra"
        );
        if (initExtra != null) {
            // reserved for future
        }
    }

    /**
     * 刷新标题1、标题2
     * @param titles size=0:不更新；size>=1:标题1；size>=2:标题2
     */
    public void refreshTitles(ArrayList<String> titles) {
        runOnUiThread(() -> {
            if (titles.size() >= 1) {
                mTvTitle1.setText(titles.get(0));
            }
            if (titles.size() >= 2) {
                mTvTitle2.setText(titles.get(1));
            }
        });
    }

    /**
     * 刷新列表
     * 每条元素格式：日期===次数===金额
     */
    public void refreshReportList(ArrayList<String> itemList) {
        runOnUiThread(() -> {
            mSelectedPos = -1;
            mAdapter.setData(itemList);
            mAdapter.notifyDataSetChanged();
        });
    }

    // -------- 选择对话框 NumberPicker 上下滚轮 --------
    private void showYearPicker() {
        Calendar cal = Calendar.getInstance();
        int curYear = cal.get(Calendar.YEAR);
        int minY = curYear - 15;
        int maxY = curYear;

        LinearLayout container = new LinearLayout(this);
        container.setOrientation(LinearLayout.VERTICAL);
        container.setGravity(Gravity.CENTER);
        container.setPadding(dp(24), dp(16), dp(24), dp(16));
        NumberPicker np = new NumberPicker(this);
        np.setMinValue(minY);
        np.setMaxValue(maxY);
        np.setValue(curYear);
        np.setWrapSelectorWheel(false);
        container.addView(np);

        AlertDialog.Builder b = new AlertDialog.Builder(this);
        b.setTitle(MyActivity.zh_cn ? "选择年份" : "Select Year");
        b.setView(container);
        b.setPositiveButton(android.R.string.ok, (d, w) -> {
            int y = np.getValue();
            MyActivity.m_instance.PublicJavaCallCpp("data_report_year|==|" + y);
        });
        b.setNegativeButton(android.R.string.cancel, null);
        b.show();
    }

    private void showYearMonthPicker() {
        Calendar cal = Calendar.getInstance();
        int curY = cal.get(Calendar.YEAR);
        int curM = cal.get(Calendar.MONTH) + 1;
        int minY = curY - 2;
        int maxY = curY;

        LinearLayout container = new LinearLayout(this);
        container.setOrientation(LinearLayout.HORIZONTAL);
        container.setGravity(Gravity.CENTER);
        container.setPadding(dp(24), dp(16), dp(24), dp(16));

        NumberPicker npY = new NumberPicker(this);
        npY.setMinValue(minY);
        npY.setMaxValue(maxY);
        npY.setValue(curY);
        npY.setWrapSelectorWheel(false);
        LinearLayout.LayoutParams lpY = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        );
        lpY.setMargins(0, 0, dp(16), 0);
        npY.setLayoutParams(lpY);

        NumberPicker npM = new NumberPicker(this);
        npM.setMinValue(1);
        npM.setMaxValue(12);
        npM.setValue(curM);
        npM.setWrapSelectorWheel(true);

        container.addView(npY);
        container.addView(npM);

        AlertDialog.Builder b = new AlertDialog.Builder(this);
        b.setTitle(MyActivity.zh_cn ? "选择年月" : "Select Year‑Month");
        b.setView(container);
        b.setPositiveButton(android.R.string.ok, (d, w) -> {
            int y = npY.getValue();
            int m = npM.getValue();
            String val = String.format("%d-%02d", y, m);
            MyActivity.m_instance.PublicJavaCallCpp("data_report_ym|==|" + val);
        });
        b.setNegativeButton(android.R.string.cancel, null);
        b.show();
    }

    /**
     * 区间选择：起始年/月/日，结束年/月/日，全部NumberPicker滚轮
     */
    private void showDateRangePicker() {
        Calendar cal = Calendar.getInstance();
        int curY = cal.get(Calendar.YEAR);
        int curM = cal.get(Calendar.MONTH) + 1;
        int curD = cal.get(Calendar.DAY_OF_MONTH);
        int minY = curY - 10;

        LinearLayout rootLl = new LinearLayout(this);
        rootLl.setOrientation(LinearLayout.VERTICAL);
        rootLl.setGravity(Gravity.CENTER_HORIZONTAL);
        rootLl.setPadding(dp(16), dp(12), dp(16), dp(12));

        TextView tvStart = new TextView(this);
        tvStart.setText(MyActivity.zh_cn ? "起始日期" : "Start Date");
        tvStart.setTextSize(15);
        tvStart.setPadding(0, 0, 0, dp(6));
        rootLl.addView(tvStart);

        LinearLayout startRow = new LinearLayout(this);
        startRow.setOrientation(LinearLayout.HORIZONTAL);
        startRow.setGravity(Gravity.CENTER);

        NumberPicker npSYear = new NumberPicker(this);
        npSYear.setMinValue(minY);
        npSYear.setMaxValue(curY);
        npSYear.setValue(curY);
        npSYear.setWrapSelectorWheel(false);

        NumberPicker npSMonth = new NumberPicker(this);
        npSMonth.setMinValue(1);
        npSMonth.setMaxValue(12);
        npSMonth.setValue(curM);

        NumberPicker npSDay = new NumberPicker(this);
        npSDay.setMinValue(1);
        npSDay.setMaxValue(31);
        npSDay.setValue(curD);

        startRow.addView(npSYear);
        startRow.addView(npSMonth);
        startRow.addView(npSDay);
        rootLl.addView(startRow);

        TextView tvEnd = new TextView(this);
        tvEnd.setText(MyActivity.zh_cn ? "结束日期" : "End Date");
        tvEnd.setTextSize(15);
        tvEnd.setPadding(0, dp(12), 0, dp(6));
        rootLl.addView(tvEnd);

        LinearLayout endRow = new LinearLayout(this);
        endRow.setOrientation(LinearLayout.HORIZONTAL);
        endRow.setGravity(Gravity.CENTER);

        NumberPicker npEYear = new NumberPicker(this);
        npEYear.setMinValue(minY);
        npEYear.setMaxValue(curY);
        npEYear.setValue(curY);
        npEYear.setWrapSelectorWheel(false);

        NumberPicker npEMonth = new NumberPicker(this);
        npEMonth.setMinValue(1);
        npEMonth.setMaxValue(12);
        npEMonth.setValue(curM);

        NumberPicker npEDay = new NumberPicker(this);
        npEDay.setMinValue(1);
        npEDay.setMaxValue(31);
        npEDay.setValue(curD);

        endRow.addView(npEYear);
        endRow.addView(npEMonth);
        endRow.addView(npEDay);
        rootLl.addView(endRow);

        AlertDialog.Builder b = new AlertDialog.Builder(this);
        b.setTitle(MyActivity.zh_cn ? "选择日期区间" : "Select Date Range");
        b.setView(rootLl);
        b.setPositiveButton(android.R.string.ok, (d, w) -> {
            String s = String.format(
                "%d-%02d-%02d",
                npSYear.getValue(),
                npSMonth.getValue(),
                npSDay.getValue()
            );
            String e = String.format(
                "%d-%02d-%02d",
                npEYear.getValue(),
                npEMonth.getValue(),
                npEDay.getValue()
            );

            MyActivity.m_instance.PublicJavaCallCpp(
                "data_report_range|==|" + s + "|==|" + e
            );
        });
        b.setNegativeButton(android.R.string.cancel, null);
        b.show();
    }

    private int dp(int dpVal) {
        float density = getResources().getDisplayMetrics().density;
        return (int) (dpVal * density + 0.5f);
    }

    // 列表适配器
    private static class ReportItemAdapter extends ArrayAdapter<String> {

        private final int mSelectBg;
        private final int mTextColor;
        private ArrayList<String> mData;
        private final DataReportActivity mAct;

        public ReportItemAdapter(
            DataReportActivity act,
            ArrayList<String> list,
            int selectBg,
            int textColor
        ) {
            super(act, 0, list);
            mAct = act;
            mData = list;
            mSelectBg = selectBg;
            mTextColor = textColor;
        }

        public void setData(ArrayList<String> newData) {
            mData = newData;
            clear();
            addAll(mData);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            LinearLayout itemLl;
            if (convertView instanceof LinearLayout) {
                itemLl = (LinearLayout) convertView;
            } else {
                itemLl = new LinearLayout(getContext());
                itemLl.setOrientation(LinearLayout.VERTICAL);
                itemLl.setGravity(Gravity.CENTER_VERTICAL);
                itemLl.setPadding(
                    mAct.dp(12),
                    mAct.dp(6),
                    mAct.dp(12),
                    mAct.dp(6)
                );
            }
            itemLl.removeAllViews();

            if (mAct.mSelectedPos == position) {
                itemLl.setBackgroundColor(mSelectBg);
            } else {
                itemLl.setBackgroundColor(0x00000000);
            }

            String raw = getItem(position);
            String[] parts = raw.split("===");

            TextView tvDate = new TextView(getContext());
            tvDate.setTextSize(15);
            tvDate.setTextColor(mTextColor);
            tvDate.getPaint().setFakeBoldText(true);
            tvDate.setPadding(0, 0, 0, mAct.dp(2));
            if (parts.length >= 1) tvDate.setText(parts[0]);
            itemLl.addView(tvDate);

            TextView tvCount = new TextView(getContext());
            tvCount.setTextSize(14);
            tvCount.setTextColor(mTextColor);
            tvCount.setPadding(0, 0, 0, mAct.dp(2));
            if (parts.length >= 2) tvCount.setText(parts[1]);
            itemLl.addView(tvCount);

            TextView tvAmount = new TextView(getContext());
            tvAmount.setTextSize(14);
            tvAmount.setTextColor(mTextColor);
            tvAmount.setPadding(0, 0, 0, 0);
            if (parts.length >= 3) tvAmount.setText(parts[2]);
            itemLl.addView(tvAmount);

            return itemLl;
        }
    }

    /**
     * C++调用：详情弹窗
     * 传入ArrayList，按顺序依次创建TextView，元素为空字符串则隐藏该控件
     * @param detailItems 详情数据数组
     */
    public void showDetailDialog(ArrayList<String> detailItems) {
        runOnUiThread(() -> {
            final DataReportActivity act = DataReportActivity.this;
            int textColor = mIsDark ? 0xFFFFFFFF : 0xFF000000;
            int bgColor = mIsDark ? 0xFF1E1E1E : 0xFFFFFFFF;

            android.widget.ScrollView scrollView =
                new android.widget.ScrollView(act);
            scrollView.setPadding(dp(16), dp(12), dp(16), dp(12));

            LinearLayout container = new LinearLayout(act);
            container.setOrientation(LinearLayout.VERTICAL);
            container.setGravity(Gravity.CENTER_VERTICAL);

            for (String itemText : detailItems) {
                TextView tv = new TextView(act);
                tv.setTextSize(14);
                tv.setTextColor(textColor);
                tv.setPadding(0, dp(4), 0, dp(4));
                if (TextUtils.isEmpty(itemText)) {
                    tv.setVisibility(View.GONE);
                } else {
                    tv.setText(itemText);
                    tv.setVisibility(View.VISIBLE);
                }
                container.addView(tv);
            }
            scrollView.addView(container);

            // 限制滚动区域最大高度为屏幕高度70%，兼容低版本，替代setMaxHeight
            int maxH = (int) (act
                .getResources()
                .getDisplayMetrics()
                .heightPixels * 0.7);
            ViewGroup.LayoutParams lp = new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            );
            lp.height = maxH;
            scrollView.setLayoutParams(lp);

            android.app.AlertDialog.Builder builder =
                new android.app.AlertDialog.Builder(act);
            builder.setTitle(MyActivity.zh_cn ? "详情" : "Detail");
            builder.setView(scrollView);
            builder.setPositiveButton(android.R.string.ok, null);
            builder.show();
        });
    }

    /**
     * C++调用：分类统计弹窗
     * 单条格式：分类文本===百分比文本===金额文本
     * @param cateList 分类条目数组
     */
    /*public void showCategoryDialog(ArrayList<String> cateList) {
        runOnUiThread(() -> {
            final DataReportActivity act = DataReportActivity.this;
            int textColor = mIsDark ? 0xFFFFFFFF : 0xFF000000;
            int bgColor = mIsDark ? 0xFF1E1E1E : 0xFFFFFFFF;
            int itemBgSelect = mIsDark ? 0xFF2A2A2A : 0xFFE0EDFB;

            ListView listView = new ListView(act);
            listView.setBackgroundColor(bgColor);

            ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                act,
                0,
                cateList
            ) {
                @Override
                public View getView(
                    int position,
                    View convertView,
                    ViewGroup parent
                ) {
                    LinearLayout itemLl;
                    if (convertView instanceof LinearLayout) {
                        itemLl = (LinearLayout) convertView;
                    } else {
                        itemLl = new LinearLayout(act);
                        itemLl.setOrientation(LinearLayout.VERTICAL);
                        itemLl.setGravity(Gravity.CENTER_VERTICAL);
                        itemLl.setPadding(dp(12), dp(6), dp(12), dp(6));
                    }
                    itemLl.removeAllViews();
                    itemLl.setBackgroundColor(0x00000000);

                    String raw = getItem(position);
                    String[] parts = raw.split("===");

                    // 第一行：分类，粗体
                    TextView tvCate = new TextView(act);
                    tvCate.setTextSize(15);
                    tvCate.setTextColor(textColor);
                    tvCate.getPaint().setFakeBoldText(true);
                    tvCate.setPadding(0, 0, 0, dp(2));
                    if (parts.length >= 1) {
                        tvCate.setText(parts[0]);
                    }
                    itemLl.addView(tvCate);

                    // 第二行：百分比
                    TextView tvPercent = new TextView(act);
                    tvPercent.setTextSize(14);
                    tvPercent.setTextColor(textColor);
                    tvPercent.setPadding(0, 0, 0, dp(2));
                    if (parts.length >= 2) {
                        tvPercent.setText(parts[1]);
                    }
                    itemLl.addView(tvPercent);

                    // 第三行：金额
                    TextView tvAmount = new TextView(act);
                    tvAmount.setTextSize(14);
                    tvAmount.setTextColor(textColor);
                    tvAmount.setPadding(0, 0, 0, 0);
                    if (parts.length >= 3) {
                        tvAmount.setText(parts[2]);
                    }
                    itemLl.addView(tvAmount);

                    return itemLl;
                }
            };
            listView.setAdapter(adapter);

            AlertDialog.Builder builder = new AlertDialog.Builder(act);
            builder.setTitle(
                MyActivity.zh_cn ? "分类统计" : "Category Statistic"
            );
            builder.setView(listView);
            builder.setPositiveButton(android.R.string.ok, null);
            builder.show();
        });
        }*/
    public void showCategoryDialog(ArrayList<String> cateList) {
        runOnUiThread(() -> {
            final DataReportActivity act = DataReportActivity.this;
            int textColor = mIsDark ? 0xFFFFFFFF : 0xFF000000;
            int bgColor = mIsDark ? 0xFF1E1E1E : 0xFFFFFFFF;
            int btnBgNormal = mIsDark ? 0xFF303030 : 0xFFE8E8E8;
            int itemBgSelect = mIsDark ? 0xFF2A2A2A : 0xFFE0EDFB;
            // 记录选中条目索引
            final int[] selectedPos = { -1 };

            ListView listView = new ListView(act);
            listView.setBackgroundColor(bgColor);
            ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                act,
                0,
                cateList
            ) {
                @Override
                public View getView(
                    int position,
                    View convertView,
                    ViewGroup parent
                ) {
                    LinearLayout itemLl;
                    if (convertView instanceof LinearLayout) {
                        itemLl = (LinearLayout) convertView;
                    } else {
                        itemLl = new LinearLayout(act);
                        itemLl.setOrientation(LinearLayout.VERTICAL);
                        itemLl.setGravity(Gravity.CENTER_VERTICAL);
                        itemLl.setPadding(dp(12), dp(6), dp(12), dp(6));
                    }
                    itemLl.removeAllViews();
                    // 选中背景
                    if (position == selectedPos[0]) {
                        itemLl.setBackgroundColor(itemBgSelect);
                    } else {
                        itemLl.setBackgroundColor(0x00000000);
                    }
                    String raw = getItem(position);
                    String[] parts = raw.split("===");
                    // 第一行：分类，粗体
                    TextView tvCate = new TextView(act);
                    tvCate.setTextSize(15);
                    tvCate.setTextColor(textColor);
                    tvCate.getPaint().setFakeBoldText(true);
                    tvCate.setPadding(0, 0, 0, dp(2));
                    if (parts.length >= 1) {
                        tvCate.setText(parts[0]);
                    }
                    itemLl.addView(tvCate);
                    // 第二行：百分比
                    TextView tvPercent = new TextView(act);
                    tvPercent.setTextSize(14);
                    tvPercent.setTextColor(textColor);
                    tvPercent.setPadding(0, 0, 0, dp(2));
                    if (parts.length >= 2) {
                        tvPercent.setText(parts[1]);
                    }
                    itemLl.addView(tvPercent);
                    // 第三行：金额
                    TextView tvAmount = new TextView(act);
                    tvAmount.setTextSize(14);
                    tvAmount.setTextColor(textColor);
                    tvAmount.setPadding(0, 0, 0, 0);
                    if (parts.length >= 3) {
                        tvAmount.setText(parts[2]);
                    }
                    itemLl.addView(tvAmount);
                    return itemLl;
                }
            };
            listView.setAdapter(adapter);
            // 条目点击，切换选中
            listView.setOnItemClickListener((parent, view, position, id) -> {
                selectedPos[0] = position;
                adapter.notifyDataSetChanged();
            });

            // ========== 自定义底部按钮栏：AI分析 + 详细，平分宽度 ==========
            LinearLayout btnContainer = new LinearLayout(act);
            btnContainer.setOrientation(LinearLayout.HORIZONTAL);
            btnContainer.setWeightSum(2);
            btnContainer.setPadding(dp(12), dp(8), dp(12), dp(4));
            Button btnAi = new Button(act);
            btnAi.setText(MyActivity.zh_cn ? "AI分析" : "AI Analysis");
            btnAi.setBackgroundColor(btnBgNormal);
            btnAi.setTextColor(textColor);
            LinearLayout.LayoutParams lpAi = new LinearLayout.LayoutParams(
                0,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                1.0f
            );
            lpAi.setMargins(dp(4), 0, dp(4), 0);
            btnAi.setLayoutParams(lpAi);
            btnAi.setOnClickListener(v -> {
                showAiLoadingDialog();
                MyActivity.m_instance.PublicJavaCallCpp(
                    "data_category_ai_analysis"
                );
            });

            // 按钮改为【详细】
            Button btnDetail = new Button(act);
            btnDetail.setText(MyActivity.zh_cn ? "详细" : "Detail");
            btnDetail.setBackgroundColor(btnBgNormal);
            btnDetail.setTextColor(textColor);
            LinearLayout.LayoutParams lpDetail = new LinearLayout.LayoutParams(
                0,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                1.0f
            );
            lpDetail.setMargins(dp(4), 0, dp(4), 0);
            btnDetail.setLayoutParams(lpDetail);

            btnContainer.addView(btnAi);
            btnContainer.addView(btnDetail);

            // 外层垂直容器，ListView + 按钮栏
            LinearLayout dialogRoot = new LinearLayout(act);
            dialogRoot.setOrientation(LinearLayout.VERTICAL);
            dialogRoot.addView(listView);
            dialogRoot.addView(btnContainer);

            AlertDialog.Builder builder = new AlertDialog.Builder(act);
            builder.setTitle(
                MyActivity.zh_cn ? "分类统计" : "Category Statistic"
            );
            builder.setView(dialogRoot);
            AlertDialog dialog = builder.create();

            btnDetail.setOnClickListener(v -> {
                if (selectedPos[0] >= 0) {
                    String rawItem = cateList.get(selectedPos[0]);
                    String[] parts = rawItem.split("===");
                    String cateText = "";
                    if (parts.length >= 1) {
                        cateText = parts[0];
                    }
                    // 回调C++，带上分类文本
                    MyActivity.m_instance.PublicJavaCallCpp(
                        "data_category_detail|==|" + cateText
                    );
                }
                //dialog.dismiss();
            });
            dialog.show();
        });
    }

    /**
     * 显示AI分析等待弹窗，带转圈ProgressBar
     */
    public void showAiLoadingDialog() {
        runOnUiThread(() -> {
            if (isFinishing()) return;
            if (mAiLoadingDialog != null && mAiLoadingDialog.isShowing()) {
                return;
            }
            // 调用MyActivity公共方法，拿到dialog实例保存到本页面成员
            mAiLoadingDialog = MyActivity.showAiLoadingDialog(this);
        });
    }

    /**
     * 关闭AI等待弹窗
     */
    public void dismissAiLoadingDialog() {
        runOnUiThread(() -> {
            MyActivity.dismissAiLoadingDialog(mAiLoadingDialog);
        });
    }

    /**
     * C++调用：AI分析结果弹窗，支持简易MD渲染，支持 # ~ #### 标题
     * @param mdList 数组，仅第0项存放Markdown原始文本
     */
    public void showAiMarkdownDialog(ArrayList<String> mdList) {
        // 先关闭本页面的loading弹窗
        dismissAiLoadingDialog();
        // 调用MyActivity公共静态方法，传入当前this作为上下文
        MyActivity.showAiMarkdownDialog(this, mdList);
    }

    /**
     * C++调用：分类详情弹窗
     * 单条格式：日期文本===金额文本===备注文本
     * @param cateDetailList 详情条目数组
     */
    public void showCateDetailDialog(ArrayList<String> cateDetailList) {
        runOnUiThread(() -> {
            final DataReportActivity act = DataReportActivity.this;
            int textColor = mIsDark ? 0xFFFFFFFF : 0xFF000000;
            int bgColor = mIsDark ? 0xFF1E1E1E : 0xFFFFFFFF;

            ListView listView = new ListView(act);
            listView.setBackgroundColor(bgColor);
            ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                act,
                0,
                cateDetailList
            ) {
                @Override
                public View getView(
                    int position,
                    View convertView,
                    ViewGroup parent
                ) {
                    LinearLayout itemLl;
                    if (convertView instanceof LinearLayout) {
                        itemLl = (LinearLayout) convertView;
                    } else {
                        itemLl = new LinearLayout(act);
                        itemLl.setOrientation(LinearLayout.VERTICAL);
                        itemLl.setGravity(Gravity.CENTER_VERTICAL);
                        itemLl.setPadding(dp(12), dp(8), dp(12), dp(8));
                    }
                    itemLl.removeAllViews();
                    itemLl.setBackgroundColor(0x00000000);
                    String raw = getItem(position);
                    String[] parts = raw.split("===");

                    // 第1行：日期
                    TextView tvDate = new TextView(act);
                    tvDate.setTextSize(14);
                    tvDate.setTextColor(textColor);
                    tvDate.setPadding(0, 0, 0, dp(3));
                    if (parts.length >= 1) {
                        tvDate.setText(parts[0]);
                    }
                    itemLl.addView(tvDate);

                    // 第2行：金额
                    TextView tvMoney = new TextView(act);
                    tvMoney.setTextSize(14);
                    tvMoney.setTextColor(textColor);
                    tvMoney.setPadding(0, 0, 0, dp(3));
                    if (parts.length >= 2) {
                        tvMoney.setText(parts[1]);
                    }
                    itemLl.addView(tvMoney);

                    // 第3行：详细备注，为空则隐藏
                    TextView tvRemark = new TextView(act);
                    tvRemark.setTextSize(14);
                    tvRemark.setTextColor(textColor);
                    tvRemark.setPadding(0, 0, 0, 0);
                    if (parts.length >= 3) {
                        String remarkText = parts[2];
                        if (TextUtils.isEmpty(remarkText)) {
                            tvRemark.setVisibility(View.GONE);
                        } else {
                            tvRemark.setText(remarkText);
                            tvRemark.setVisibility(View.VISIBLE);
                        }
                    } else {
                        tvRemark.setVisibility(View.GONE);
                    }
                    itemLl.addView(tvRemark);

                    return itemLl;
                }
            };
            listView.setAdapter(adapter);

            // 底部按钮：确定
            LinearLayout btnContainer = new LinearLayout(act);
            btnContainer.setOrientation(LinearLayout.HORIZONTAL);
            btnContainer.setPadding(dp(12), dp(8), dp(12), dp(4));
            btnContainer.setGravity(Gravity.CENTER);

            Button btnOk = new Button(act);
            btnOk.setText(MyActivity.zh_cn ? "确定" : "OK");
            btnOk.setBackgroundColor(mIsDark ? 0xFF303030 : 0xFFE8E8E8);
            btnOk.setTextColor(textColor);
            LinearLayout.LayoutParams lpOk = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            );
            lpOk.setMargins(dp(4), 0, dp(4), 0);
            btnOk.setLayoutParams(lpOk);

            // 外层容器
            LinearLayout dialogRoot = new LinearLayout(act);
            dialogRoot.setOrientation(LinearLayout.VERTICAL);
            dialogRoot.addView(listView);
            dialogRoot.addView(btnContainer);

            AlertDialog.Builder builder = new AlertDialog.Builder(act);
            builder.setTitle(MyActivity.zh_cn ? "分类详情" : "Category Detail");
            builder.setView(dialogRoot);
            AlertDialog dialog = builder.create();
            btnOk.setOnClickListener(v -> dialog.dismiss());
            btnContainer.addView(btnOk);

            dialog.show();
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mInstance = null;
    }
}
