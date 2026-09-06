package com.x;

import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.HorizontalScrollView;
import android.widget.LinearLayout;
import android.widget.TextView;
import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;
import java.util.ArrayList;
import java.util.Calendar;

public class SportChartActivity extends AppCompatActivity {

    public static SportChartActivity mInstance = null;

    private Button mBtnYear;
    private Button mBtnYearMonth;

    private HorizontalScrollView mHScrollView;
    private LinearLayout mScrollContentContainer;

    private boolean mIsDark;
    private OnBackPressedCallback mBackCallback;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mInstance = this;

        mBackCallback = new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                PublicJavaCallCpp("cancel_sport_chart");
                finish();
            }
        };
        getOnBackPressedDispatcher().addCallback(this, mBackCallback);

        mIsDark = ImmersiveUtil.applyRealImmersive(this);

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setLayoutParams(
            new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        );

        int bgColor = mIsDark ? 0xFF121212 : 0xFFF5F5F5;
        int textColor = mIsDark ? 0xFFFFFFFF : 0xFF000000;
        int btnBgNormal = mIsDark ? 0xFF303030 : 0xFFE8E8E8;
        root.setBackgroundColor(bgColor);

        // ========== 顶部按钮栏：年汇总｜年月 ==========
        LinearLayout topBtnBar = new LinearLayout(this);
        topBtnBar.setOrientation(LinearLayout.HORIZONTAL);
        topBtnBar.setGravity(Gravity.CENTER);
        topBtnBar.setPadding(dp(12), dp(12), dp(12), dp(8));

        mBtnYear = new Button(this);
        mBtnYear.setText(MyActivity.zh_cn ? "年汇总" : "Year");
        mBtnYear.setBackgroundColor(btnBgNormal);
        mBtnYear.setTextColor(textColor);
        LinearLayout.LayoutParams lpBtnY = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        );
        lpBtnY.setMargins(0, 0, dp(12), 0);
        topBtnBar.addView(mBtnYear, lpBtnY);
        mBtnYear.setOnClickListener(v -> showYearSelectDialog());

        mBtnYearMonth = new Button(this);
        mBtnYearMonth.setText(MyActivity.zh_cn ? "年月" : "Year‑Month");
        mBtnYearMonth.setBackgroundColor(btnBgNormal);
        mBtnYearMonth.setTextColor(textColor);
        topBtnBar.addView(mBtnYearMonth);
        mBtnYearMonth.setOnClickListener(v -> showYearMonthSelectDialog());

        root.addView(topBtnBar);

        // ========== 横向滚动容器：每一项占满屏幕宽度 ==========
        mHScrollView = new HorizontalScrollView(this);
        mHScrollView.setLayoutParams(
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                0,
                1.0f
            )
        );
        mHScrollView.setFillViewport(true);

        mScrollContentContainer = new LinearLayout(this);
        mScrollContentContainer.setOrientation(LinearLayout.HORIZONTAL);
        mHScrollView.addView(mScrollContentContainer);
        root.addView(mHScrollView);

        setContentView(root);

        //读取Intent传入的ArrayList<String>，每条内部用===分割7个文本
        Intent intent = getIntent();
        ArrayList<String> initRecordList = intent.getStringArrayListExtra(
            "sport_record_list"
        );
        if (initRecordList != null) {
            refreshAllRecords(initRecordList);
        }
    }

    /**
     * 刷新全部记录列表：每条字符串内部 ===分割7个文本
     */
    public void refreshAllRecords(ArrayList<String> recordRawList) {
        runOnUiThread(() -> {
            mScrollContentContainer.removeAllViews();
            int screenW = getResources().getDisplayMetrics().widthPixels;
            int textColor = mIsDark ? 0xFFFFFFFF : 0xFF000000;
            int btnBgNormal = mIsDark ? 0xFF303030 : 0xFFE8E8E8;

            for (int idx = 0; idx < recordRawList.size(); idx++) {
                String rawItem = recordRawList.get(idx);
                String[] sevenTexts = rawItem.split("===");

                // 单条记录整体容器，宽度等于屏幕宽度
                LinearLayout itemRoot = new LinearLayout(this);
                itemRoot.setOrientation(LinearLayout.VERTICAL);
                LinearLayout.LayoutParams itemLp =
                    new LinearLayout.LayoutParams(
                        screenW,
                        ViewGroup.LayoutParams.MATCH_PARENT
                    );
                itemRoot.setLayoutParams(itemLp);
                itemRoot.setPadding(dp(12), dp(8), dp(12), dp(8));

                //7个统计文本
                LinearLayout statContainer = new LinearLayout(this);
                statContainer.setOrientation(LinearLayout.VERTICAL);
                for (int t = 0; t < 7; t++) {
                    TextView tv = new TextView(this);
                    tv.setTextSize(15);
                    tv.setTextColor(textColor);
                    tv.setPadding(0, dp(3), 0, dp(3));
                    if (t < sevenTexts.length) {
                        tv.setText(sevenTexts[t]);
                    } else {
                        tv.setText("");
                    }
                    statContainer.addView(tv);
                }
                itemRoot.addView(statContainer);

                //图表绘图View
                SportChartView chartView = new SportChartView(
                    this,
                    mIsDark,
                    idx
                );
                LinearLayout.LayoutParams chartLp =
                    new LinearLayout.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        0,
                        1.0f
                    );
                chartView.setLayoutParams(chartLp);
                itemRoot.addView(chartView);

                //底部内嵌按钮：轨迹、途径（属于当前这条记录）
                LinearLayout itemBottomBar = new LinearLayout(this);
                itemBottomBar.setOrientation(LinearLayout.HORIZONTAL);
                itemBottomBar.setGravity(Gravity.CENTER);
                itemBottomBar.setPadding(0, dp(6), 0, 0);

                Button btnTrack = new Button(this);
                btnTrack.setText(MyActivity.zh_cn ? "轨迹" : "Track");
                btnTrack.setBackgroundColor(btnBgNormal);
                btnTrack.setTextColor(textColor);
                LinearLayout.LayoutParams lpTrack =
                    new LinearLayout.LayoutParams(
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT
                    );
                lpTrack.setMargins(0, 0, dp(12), 0);
                btnTrack.setLayoutParams(lpTrack);
                final int curIdx = idx;
                btnTrack.setOnClickListener(v -> {
                    PublicJavaCallCpp("sport_chart_track|==|" + curIdx);
                });

                Button btnPath = new Button(this);
                btnPath.setText(MyActivity.zh_cn ? "途径" : "Path");
                btnPath.setBackgroundColor(btnBgNormal);
                btnPath.setTextColor(textColor);
                btnPath.setOnClickListener(v -> {
                    PublicJavaCallCpp("sport_chart_path|==|" + curIdx);
                });

                itemBottomBar.addView(btnTrack);
                itemBottomBar.addView(btnPath);
                itemRoot.addView(itemBottomBar);

                mScrollContentContainer.addView(itemRoot);
            }
            //刷新后滚动到最左侧，展示最新记录（下标0）
            mHScrollView.scrollTo(0, 0);
        });
    }

    /**
     * 设置某一条记录的速度double数组（保留原有单条接口）
     */
    public void setRecordSpeedData(int recordIndex, double[] speedArr) {
        runOnUiThread(() -> {
            View itemView = mScrollContentContainer.getChildAt(recordIndex);
            if (!(itemView instanceof LinearLayout)) return;
            LinearLayout itemLl = (LinearLayout) itemView;
            SportChartView chart = (SportChartView) itemLl.getChildAt(1);
            if (chart != null) {
                chart.setSpeedData(speedArr);
            }
        });
    }

    /**
     * 设置某一条记录的海拔double数组（保留原有单条接口）
     */
    public void setRecordAltData(int recordIndex, double[] altArr) {
        runOnUiThread(() -> {
            View itemView = mScrollContentContainer.getChildAt(recordIndex);
            if (!(itemView instanceof LinearLayout)) return;
            LinearLayout itemLl = (LinearLayout) itemView;
            SportChartView chart = (SportChartView) itemLl.getChildAt(1);
            if (chart != null) {
                chart.setAltitudeData(altArr);
            }
        });
    }

    /**
     * 【新增】批量刷新全部速度数据
     * @param speedList ArrayList里面每个元素是一条记录的double[]速度数组
     */
    public void refreshAllSpeedData(ArrayList<double[]> speedList) {
        runOnUiThread(() -> {
            for (int i = 0; i < speedList.size(); i++) {
                if (i >= mScrollContentContainer.getChildCount()) break;
                double[] arr = speedList.get(i);
                View itemView = mScrollContentContainer.getChildAt(i);
                if (!(itemView instanceof LinearLayout)) continue;
                LinearLayout itemLl = (LinearLayout) itemView;
                SportChartView chart = (SportChartView) itemLl.getChildAt(1);
                if (chart != null) {
                    chart.setSpeedData(arr);
                }
            }
        });
    }

    /**
     * 【新增】批量刷新全部海拔数据
     * @param altList ArrayList里面每个元素是一条记录的double[]海拔数组
     */
    public void refreshAllAltData(ArrayList<double[]> altList) {
        runOnUiThread(() -> {
            for (int i = 0; i < altList.size(); i++) {
                if (i >= mScrollContentContainer.getChildCount()) break;
                double[] arr = altList.get(i);
                View itemView = mScrollContentContainer.getChildAt(i);
                if (!(itemView instanceof LinearLayout)) continue;
                LinearLayout itemLl = (LinearLayout) itemView;
                SportChartView chart = (SportChartView) itemLl.getChildAt(1);
                if (chart != null) {
                    chart.setAltitudeData(arr);
                }
            }
        });
    }

    private void showYearSelectDialog() {
        Calendar cal = Calendar.getInstance();
        int currentYear = cal.get(Calendar.YEAR);
        ArrayList<Integer> yearList = new ArrayList<>();
        for (int y = currentYear - 10; y <= currentYear; y++) {
            yearList.add(y);
        }
        String[] items = new String[yearList.size()];
        for (int i = 0; i < items.length; i++) {
            items[i] = String.valueOf(yearList.get(i));
        }

        new AlertDialog.Builder(this)
            .setTitle(MyActivity.zh_cn ? "选择年份" : "Select Year")
            .setItems(items, (dialog, which) -> {
                int selYear = yearList.get(which);
                mBtnYear.setText(String.valueOf(selYear));
                PublicJavaCallCpp("sport_select_year|==|" + selYear);
            })
            .show();
    }

    private void showYearMonthSelectDialog() {
        Calendar cal = Calendar.getInstance();
        int currentYear = cal.get(Calendar.YEAR);
        int currentMonth = cal.get(Calendar.MONTH) + 1;
        ArrayList<String> ymList = new ArrayList<>();
        for (int y = currentYear - 2; y <= currentYear; y++) {
            for (int m = 1; m <= 12; m++) {
                if (y == currentYear && m > currentMonth) continue;
                ymList.add(y + "-" + m);
            }
        }
        String[] items = new String[ymList.size()];
        ymList.toArray(items);

        new AlertDialog.Builder(this)
            .setTitle(MyActivity.zh_cn ? "选择年月" : "Select Year‑Month")
            .setItems(items, (dialog, which) -> {
                String selYm = ymList.get(which);
                mBtnYearMonth.setText(selYm);
                PublicJavaCallCpp("sport_select_ym|==|" + selYm);
            })
            .show();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mInstance = null;
        if (mBackCallback != null) {
            mBackCallback.remove();
        }
    }

    private int dp(int dpVal) {
        float density = getResources().getDisplayMetrics().density;
        return (int) (dpVal * density + 0.5f);
    }

    private native void PublicJavaCallCpp(String msg);

    //==================== 单条目绘图View ====================
    public static class SportChartView extends View {

        private final boolean mDarkMode;
        private final float mDensity;

        @SuppressWarnings("unused")
        private final int mRecordIndex;

        private double[] mSpeedData;
        private double[] mAltData;

        private final Paint mPaintSpeedFill = new Paint();
        private final Paint mPaintAltLine = new Paint();
        private final Paint mPaintGrid = new Paint();

        private int mCountUp = 0;
        private int mCountFlat = 0;
        private int mCountDown = 0;

        public SportChartView(
            Context context,
            boolean isDark,
            int recordIndex
        ) {
            super(context);
            mDarkMode = isDark;
            mRecordIndex = recordIndex;
            mDensity = context.getResources().getDisplayMetrics().density;

            mPaintSpeedFill.setStyle(Paint.Style.FILL);
            mPaintAltLine.setStyle(Paint.Style.STROKE);
            mPaintAltLine.setStrokeWidth(dpInner(2));
            mPaintGrid.setStyle(Paint.Style.STROKE);
            mPaintGrid.setStrokeWidth(dpInner(1));

            if (mDarkMode) {
                mPaintGrid.setColor(0xFF444444);
                mPaintAltLine.setColor(0xFF4FC3F7);
            } else {
                mPaintGrid.setColor(0xFFCCCCCC);
                mPaintAltLine.setColor(0xFF0288D1);
            }
        }

        private int dpInner(int dpVal) {
            return (int) (dpVal * mDensity + 0.5f);
        }

        public void setSpeedData(double[] arr) {
            mSpeedData = arr;
            invalidate();
        }

        public void setAltitudeData(double[] arr) {
            mAltData = arr;
            computeSlopeStat(arr);
            invalidate();
        }

        private void computeSlopeStat(double[] alt) {
            mCountUp = 0;
            mCountFlat = 0;
            mCountDown = 0;
            if (alt == null || alt.length < 2) return;
            for (int i = 1; i < alt.length; i++) {
                double delta = alt[i] - alt[i - 1];
                if (delta > 0.1) {
                    mCountUp++;
                } else if (delta < -0.1) {
                    mCountDown++;
                } else {
                    mCountFlat++;
                }
            }
        }

        private int getSpeedColor(double speed, double maxSpeed) {
            if (maxSpeed < 0.01) return 0x804CAF50;
            double ratio = speed / maxSpeed;
            if (ratio < 0.33) {
                return 0x702196F3;
            } else if (ratio < 0.66) {
                return 0x70FFC107;
            } else {
                return 0x70F44336;
            }
        }

        @Override
        protected void onDraw(Canvas canvas) {
            super.onDraw(canvas);
            int w = getWidth();
            int h = getHeight();
            if (w < 10 || h < 10) return;

            canvas.drawLine(0, h / 2, w, h / 2, mPaintGrid);

            //速度山丘填充，下标0为本条记录最左侧（最新）
            if (mSpeedData != null && mSpeedData.length > 1) {
                int sampleCount = mSpeedData.length;
                int step = Math.max(1, sampleCount / w);

                double maxSp = 0;
                for (double v : mSpeedData) {
                    if (v > maxSp) maxSp = v;
                }

                Path path = new Path();
                path.moveTo(0, h);
                for (int i = 0; i < sampleCount; i += step) {
                    float x = (float) (((double) i / (sampleCount - 1)) * w);
                    float y;
                    if (maxSp < 0.001) {
                        y = h;
                    } else {
                        double sp = mSpeedData[i];
                        y = h - (float) ((sp / maxSp) * (h * 0.45));
                    }
                    path.lineTo(x, y);
                }
                path.lineTo(w, h);
                path.close();
                mPaintSpeedFill.setColor(getSpeedColor(maxSp, maxSp));
                canvas.drawPath(path, mPaintSpeedFill);
            }

            //海拔折线
            if (mAltData != null && mAltData.length > 1) {
                int sampleCount = mAltData.length;
                int step = Math.max(1, sampleCount / w);

                double minAlt = Double.MAX_VALUE;
                double maxAlt = -Double.MAX_VALUE;
                for (double v : mAltData) {
                    if (v < minAlt) minAlt = v;
                    if (v > maxAlt) maxAlt = v;
                }
                double rangeAlt = maxAlt - minAlt;
                if (rangeAlt < 0.1) rangeAlt = 1.0;

                Path altPath = new Path();
                boolean first = true;
                for (int i = 0; i < sampleCount; i += step) {
                    float x = (float) (((double) i / (sampleCount - 1)) * w);
                    float y = (float) (h * 0.55 -
                        ((mAltData[i] - minAlt) / rangeAlt) * (h * 0.45));
                    if (first) {
                        altPath.moveTo(x, y);
                        first = false;
                    } else {
                        altPath.lineTo(x, y);
                    }
                }
                canvas.drawPath(altPath, mPaintAltLine);
            }
        }
    }
}
