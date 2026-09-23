package com.x;

import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.RectF;
import android.os.Bundle;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.VelocityTracker;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.HorizontalScrollView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.NumberPicker;
import android.widget.TextView;
import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;
import java.util.ArrayList;
import java.util.Calendar;

public class SportChartActivity extends AppCompatActivity {

    public static SportChartActivity mInstance = null;
    private Button mBtnYear;
    private Button mBtnYearMonth;
    private SnapHorizontalScrollView mHScrollView;
    private LinearLayout mScrollContentContainer;
    private boolean mIsDark;
    private OnBackPressedCallback mBackCallback;
    private int mScreenWidth;

    private native void PublicJavaCallCpp(String msg);

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
        mScreenWidth = getResources().getDisplayMetrics().widthPixels;

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
        mBtnYear.setText(MyActivity.zh_cn ? "汇总" : "Summary");
        mBtnYear.setBackgroundColor(btnBgNormal);
        mBtnYear.setTextColor(textColor);
        LinearLayout.LayoutParams lpBtnY = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        );
        lpBtnY.setMargins(0, 0, dp(12), 0);
        topBtnBar.addView(mBtnYear, lpBtnY);
        mBtnYear.setOnClickListener(v -> {
            PublicJavaCallCpp("sport_select_year|==|");
        });

        mBtnYearMonth = new Button(this);
        mBtnYearMonth.setText(MyActivity.zh_cn ? "年月" : "Year‑Month");
        mBtnYearMonth.setBackgroundColor(btnBgNormal);
        mBtnYearMonth.setTextColor(textColor);
        topBtnBar.addView(mBtnYearMonth);
        mBtnYearMonth.setOnClickListener(v -> showYearMonthSelectDialog());
        root.addView(topBtnBar);

        // ========== 自定义分页吸附横向滚动容器 ==========
        mHScrollView = new SnapHorizontalScrollView(this);
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

        Intent intent = getIntent();
        ArrayList<String> initRecordList = intent.getStringArrayListExtra(
            "sport_record_list"
        );
        if (initRecordList != null) {
            refreshAllRecords(initRecordList);
        }
    }

    public void refreshAllRecords(ArrayList<String> recordRawList) {
        runOnUiThread(() -> {
            mScrollContentContainer.removeAllViews();
            int screenW = mScreenWidth;
            int textColor = mIsDark ? 0xFFFFFFFF : 0xFF000000;
            int btnBgNormal = mIsDark ? 0xFF303030 : 0xFFE8E8E8;
            for (int idx = 0; idx < recordRawList.size(); idx++) {
                String rawItem = recordRawList.get(idx);
                String[] sevenTexts = rawItem.split("===");
                LinearLayout itemRoot = new LinearLayout(this);
                itemRoot.setOrientation(LinearLayout.VERTICAL);
                LinearLayout.LayoutParams itemLp =
                    new LinearLayout.LayoutParams(
                        screenW,
                        ViewGroup.LayoutParams.MATCH_PARENT
                    );
                itemRoot.setLayoutParams(itemLp);
                itemRoot.setPadding(dp(12), dp(8), dp(12), dp(8));

                LinearLayout statContainer = new LinearLayout(this);
                statContainer.setOrientation(LinearLayout.VERTICAL);
                for (int t = 0; t < sevenTexts.length; t++) {
                    if (t == 6) continue;
                    TextView tv = new TextView(this);
                    tv.setTextSize(15);
                    tv.setTextColor(textColor);
                    tv.setPadding(0, dp(3), 0, dp(3));
                    tv.setText(sevenTexts[t]);
                    statContainer.addView(tv);
                }
                itemRoot.addView(statContainer);

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
                btnTrack.setOnClickListener(v ->
                    PublicJavaCallCpp("sport_chart_track|==|" + curIdx)
                );

                Button btnPath = new Button(this);
                btnPath.setText(MyActivity.zh_cn ? "途径" : "Path");
                btnPath.setBackgroundColor(btnBgNormal);
                btnPath.setTextColor(textColor);
                btnPath.setOnClickListener(v ->
                    PublicJavaCallCpp("sport_chart_path|==|" + curIdx)
                );

                itemBottomBar.addView(btnTrack);
                itemBottomBar.addView(btnPath);
                itemRoot.addView(itemBottomBar);

                mScrollContentContainer.addView(itemRoot);
            }

            int itemCount = mScrollContentContainer.getChildCount();
            if (itemCount > 0) {
                int targetScrollX = (itemCount - 1) * mScreenWidth;
                mHScrollView.post(() ->
                    mHScrollView.scrollTo(targetScrollX, 0)
                );
            }
        });
    }

    public void setRecordSpeedData(int recordIndex, double[] speedArr) {
        runOnUiThread(() -> {
            View itemView = mScrollContentContainer.getChildAt(recordIndex);
            if (!(itemView instanceof LinearLayout)) return;
            LinearLayout itemLl = (LinearLayout) itemView;
            SportChartView chart = (SportChartView) itemLl.getChildAt(1);
            if (chart != null) chart.setSpeedData(speedArr);
        });
    }

    public void setRecordAltData(int recordIndex, double[] altArr) {
        runOnUiThread(() -> {
            View itemView = mScrollContentContainer.getChildAt(recordIndex);
            if (!(itemView instanceof LinearLayout)) return;
            LinearLayout itemLl = (LinearLayout) itemView;
            SportChartView chart = (SportChartView) itemLl.getChildAt(1);
            if (chart != null) chart.setAltitudeData(altArr);
        });
    }

    public void refreshAllSpeedData(ArrayList<double[]> speedList) {
        runOnUiThread(() -> {
            for (int i = 0; i < speedList.size(); i++) {
                if (i >= mScrollContentContainer.getChildCount()) break;
                View itemView = mScrollContentContainer.getChildAt(i);
                if (!(itemView instanceof LinearLayout)) continue;
                SportChartView chart = (SportChartView) (
                    (LinearLayout) itemView
                ).getChildAt(1);
                if (chart != null) chart.setSpeedData(speedList.get(i));
            }
        });
    }

    public void refreshAllAltData(ArrayList<double[]> altList) {
        runOnUiThread(() -> {
            for (int i = 0; i < altList.size(); i++) {
                if (i >= mScrollContentContainer.getChildCount()) break;
                View itemView = mScrollContentContainer.getChildAt(i);
                if (!(itemView instanceof LinearLayout)) continue;
                SportChartView chart = (SportChartView) (
                    (LinearLayout) itemView
                ).getChildAt(1);
                if (chart != null) chart.setAltitudeData(altList.get(i));
            }
        });
    }

    private void showYearMonthSelectDialog() {
        Calendar cal = Calendar.getInstance();
        int currentYear = cal.get(Calendar.YEAR);
        int currentMonth = cal.get(Calendar.MONTH) + 1;

        LinearLayout container = new LinearLayout(this);
        container.setOrientation(LinearLayout.HORIZONTAL);
        container.setGravity(Gravity.CENTER);
        container.setPadding(dp(24), dp(16), dp(24), dp(16));

        NumberPicker npYear = new NumberPicker(this);
        npYear.setMinValue(currentYear - 2);
        npYear.setMaxValue(currentYear);
        npYear.setValue(currentYear);
        npYear.setWrapSelectorWheel(false);
        LinearLayout.LayoutParams lpYear = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        );
        lpYear.setMargins(0, 0, dp(16), 0);
        npYear.setLayoutParams(lpYear);

        NumberPicker npMonth = new NumberPicker(this);
        npMonth.setMinValue(1);
        npMonth.setMaxValue(12);
        npMonth.setValue(currentMonth);
        npMonth.setWrapSelectorWheel(true);

        container.addView(npYear);
        container.addView(npMonth);

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(MyActivity.zh_cn ? "选择年月" : "Select Year‑Month");
        builder.setView(container);
        builder.setPositiveButton(android.R.string.ok, (dialog, which) -> {
            String selYm = npYear.getValue() + "-" + npMonth.getValue();
            mBtnYearMonth.setText(selYm);
            PublicJavaCallCpp("sport_select_ym|==|" + selYm);
        });
        builder.setNegativeButton(android.R.string.cancel, null);
        builder.show();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mInstance = null;
        if (mBackCallback != null) mBackCallback.remove();
    }

    private int dp(int dpVal) {
        return (int) (dpVal * getResources().getDisplayMetrics().density +
            0.5f);
    }

    public void showSummaryDialog(ArrayList<String> summaryList) {
        runOnUiThread(() -> {
            int textColor = mIsDark ? 0xFFFFFFFF : 0xFF000000;
            int bgColor = mIsDark ? 0xFF1E1E1E : 0xFFFFFFFF;

            LinearLayout rootContainer = new LinearLayout(this);
            rootContainer.setOrientation(LinearLayout.VERTICAL);
            rootContainer.setBackgroundColor(bgColor);

            TextView tvTitle = new TextView(this);
            tvTitle.setText(MyActivity.zh_cn ? "汇总" : "Summary");
            tvTitle.setTextSize(18);
            tvTitle.setTextColor(textColor);
            tvTitle.setPadding(dp(12), dp(16), dp(12), dp(12));
            rootContainer.addView(tvTitle);

            ListView listView = new ListView(this);
            listView.setBackgroundColor(bgColor);
            listView.setAdapter(
                new ArrayAdapter<String>(this, 0, summaryList) {
                    @Override
                    public View getView(
                        int position,
                        View convertView,
                        ViewGroup parent
                    ) {
                        LinearLayout itemLl =
                            convertView instanceof LinearLayout
                                ? (LinearLayout) convertView
                                : new LinearLayout(getContext());
                        itemLl.setOrientation(LinearLayout.VERTICAL);
                        itemLl.setPadding(dp(12), dp(14), dp(12), dp(14));
                        itemLl.removeAllViews();

                        TextView tvLine = new TextView(getContext());
                        tvLine.setTextSize(15);
                        tvLine.setTextColor(textColor);
                        tvLine.setText(getItem(position));
                        itemLl.addView(tvLine);
                        return itemLl;
                    }
                }
            );
            rootContainer.addView(listView);

            new AlertDialog.Builder(this)
                .setView(rootContainer)
                .setPositiveButton(android.R.string.ok, null)
                .show();
        });
    }

    public void showPathDialog(ArrayList<String> pathItemList) {
        runOnUiThread(() -> {
            int textColor = mIsDark ? 0xFFFFFFFF : 0xFF000000;
            int bgColor = mIsDark ? 0xFF1E1E1E : 0xFFFFFFFF;

            LinearLayout rootContainer = new LinearLayout(this);
            rootContainer.setOrientation(LinearLayout.VERTICAL);
            rootContainer.setBackgroundColor(bgColor);

            TextView tvTitle = new TextView(this);
            tvTitle.setText(MyActivity.zh_cn ? "途径点" : "Path Points");
            tvTitle.setTextSize(18);
            tvTitle.setTextColor(textColor);
            tvTitle.setPadding(dp(12), dp(16), dp(12), dp(12));
            rootContainer.addView(tvTitle);

            ListView listView = new ListView(this);
            listView.setBackgroundColor(bgColor);
            listView.setAdapter(
                new ArrayAdapter<String>(this, 0, pathItemList) {
                    @Override
                    public View getView(
                        int position,
                        View convertView,
                        ViewGroup parent
                    ) {
                        LinearLayout itemLl =
                            convertView instanceof LinearLayout
                                ? (LinearLayout) convertView
                                : new LinearLayout(getContext());
                        itemLl.setOrientation(LinearLayout.VERTICAL);
                        itemLl.setPadding(dp(12), dp(8), dp(12), dp(8));
                        itemLl.removeAllViews();

                        String[] parts = getItem(position).split("===");

                        TextView tvTime = new TextView(getContext());
                        tvTime.setTextSize(15);
                        tvTime.setTextColor(textColor);
                        tvTime.getPaint().setFakeBoldText(true);
                        tvTime.setText(parts.length >= 1 ? parts[0] : "");
                        itemLl.addView(tvTime);

                        TextView tvLatLon = new TextView(getContext());
                        tvLatLon.setTextSize(14);
                        tvLatLon.setTextColor(textColor);
                        tvLatLon.setText(parts.length >= 2 ? parts[1] : "");
                        itemLl.addView(tvLatLon);

                        TextView tvAddr = new TextView(getContext());
                        tvAddr.setTextSize(14);
                        tvAddr.setTextColor(textColor);
                        tvAddr.setText(parts.length >= 3 ? parts[2] : "");
                        itemLl.addView(tvAddr);

                        return itemLl;
                    }
                }
            );
            rootContainer.addView(listView);

            new AlertDialog.Builder(this)
                .setView(rootContainer)
                .setPositiveButton(android.R.string.ok, null)
                .show();
        });
    }

    private class SnapHorizontalScrollView extends HorizontalScrollView {

        private static final float SNAP_THRESHOLD_RATIO = 0.20f;
        private static final int VELOCITY_FLING_THRESHOLD = 300;
        private static final int ANIMATION_THRESHOLD_PX = 2;

        private boolean mIsUserScrolling = false;
        private VelocityTracker mVelocityTracker;

        public SnapHorizontalScrollView(Context context) {
            super(context);
            setOverScrollMode(OVER_SCROLL_NEVER);
            mVelocityTracker = VelocityTracker.obtain();
        }

        @Override
        public boolean onTouchEvent(MotionEvent ev) {
            mVelocityTracker.addMovement(ev);
            int action = ev.getAction();
            if (action == MotionEvent.ACTION_DOWN) {
                mIsUserScrolling = true;
                mVelocityTracker.clear();
            } else if (
                action == MotionEvent.ACTION_UP ||
                action == MotionEvent.ACTION_CANCEL
            ) {
                mIsUserScrolling = false;
                mVelocityTracker.computeCurrentVelocity(1000);
                float velocityX = mVelocityTracker.getXVelocity();
                post(() -> snapToPageIfNeeded(velocityX));
            }
            return super.onTouchEvent(ev);
        }

        @Override
        protected void onScrollChanged(int l, int t, int oldl, int oldt) {
            super.onScrollChanged(l, t, oldl, oldt);
            if (!mIsUserScrolling && !isScrolling()) {
                snapToPageIfNeeded(0f);
            }
        }

        private boolean isScrolling() {
            return awakenScrollBars(0, false);
        }

        private void snapToPageIfNeeded(float velocityX) {
            if (getChildCount() == 0) return;
            int scrollX = getScrollX();
            int pageWidth = mScreenWidth;
            if (pageWidth <= 0) return;

            int currentPage = scrollX / pageWidth;
            float fraction = (float) (scrollX % pageWidth) / pageWidth;
            int targetPage;

            if (Math.abs(velocityX) > VELOCITY_FLING_THRESHOLD) {
                targetPage = velocityX < 0 ? currentPage + 1 : currentPage;
            } else {
                if (fraction >= 1.0f - SNAP_THRESHOLD_RATIO) targetPage =
                    currentPage + 1;
                else if (fraction <= SNAP_THRESHOLD_RATIO) targetPage =
                    currentPage;
                else targetPage = Math.round((float) scrollX / pageWidth);
            }

            int maxIndex = mScrollContentContainer.getChildCount() - 1;
            targetPage = Math.max(0, Math.min(targetPage, maxIndex));
            int targetX = targetPage * pageWidth;

            if (Math.abs(scrollX - targetX) > ANIMATION_THRESHOLD_PX) {
                smoothScrollTo(targetX, 0);
            }
        }

        @Override
        protected void onDetachedFromWindow() {
            super.onDetachedFromWindow();
            if (mVelocityTracker != null) {
                mVelocityTracker.recycle();
                mVelocityTracker = null;
            }
        }
    }

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
        private final Paint mPaintLabel = new Paint();
        private final Paint mPaintLabelBg = new Paint();

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
            mPaintSpeedFill.setAntiAlias(true);

            mPaintAltLine.setStyle(Paint.Style.STROKE);
            mPaintAltLine.setStrokeWidth(dpInner(2));
            mPaintAltLine.setAntiAlias(true);

            mPaintGrid.setStyle(Paint.Style.STROKE);
            mPaintGrid.setStrokeWidth(dpInner(1));

            mPaintLabel.setTextSize(12 * mDensity);
            mPaintLabel.setAntiAlias(true);

            mPaintLabelBg.setStyle(Paint.Style.FILL);
            mPaintLabelBg.setAntiAlias(true);

            if (mDarkMode) {
                mPaintGrid.setColor(0xFF444444);
                mPaintAltLine.setColor(0xFF4FC3F7);
                mPaintLabel.setColor(0xFFEEEEEE);
                mPaintLabelBg.setColor(0xAA000000);
            } else {
                mPaintGrid.setColor(0xFFCCCCCC);
                mPaintAltLine.setColor(0xFF0288D1);
                mPaintLabel.setColor(0xFF333333);
                mPaintLabelBg.setColor(0xAAFFFFFF);
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
            invalidate();
        }

        private static int getGradientColor(float ratio) {
            ratio = Math.max(0, Math.min(1, ratio));
            int blue = Color.argb(180, 33, 150, 243);
            int green = Color.argb(180, 76, 175, 80);
            int orange = Color.argb(180, 255, 152, 0);
            int red = Color.argb(180, 244, 67, 54);

            if (ratio < 0.33f) return evaluateColor(ratio / 0.33f, blue, green);
            else if (ratio < 0.66f) return evaluateColor(
                (ratio - 0.33f) / 0.33f,
                green,
                orange
            );
            else return evaluateColor((ratio - 0.66f) / 0.34f, orange, red);
        }

        private static int evaluateColor(
            float fraction,
            int startColor,
            int endColor
        ) {
            int startA = (startColor >> 24) & 0xff,
                startR = (startColor >> 16) & 0xff;
            int startG = (startColor >> 8) & 0xff,
                startB = startColor & 0xff;
            int endA = (endColor >> 24) & 0xff,
                endR = (endColor >> 16) & 0xff;
            int endG = (endColor >> 8) & 0xff,
                endB = endColor & 0xff;

            return Color.argb(
                startA + (int) (fraction * (endA - startA)),
                startR + (int) (fraction * (endR - startR)),
                startG + (int) (fraction * (endG - startG)),
                startB + (int) (fraction * (endB - startB))
            );
        }

        /**
         * 绘制带半透明背景的标签
         */
        private void drawLabel(Canvas canvas, String text, float x, float y) {
            float textWidth = mPaintLabel.measureText(text);
            float textSize = mPaintLabel.getTextSize();
            float padding = dpInner(4);

            // 绘制背景圆角矩形
            RectF bgRect = new RectF(
                x - padding,
                y - textSize - padding,
                x + textWidth + padding,
                y + padding
            );
            canvas.drawRoundRect(bgRect, dpInner(4), dpInner(4), mPaintLabelBg);

            // 绘制文字
            canvas.drawText(text, x, y, mPaintLabel);
        }

        @Override
        protected void onDraw(Canvas canvas) {
            super.onDraw(canvas);
            int w = getWidth();
            int h = getHeight();
            if (w < 10 || h < 10) return;

            float midY = h / 2f;
            float labelX = dpInner(8);
            float labelPaddingTop = dpInner(16);

            // 绘制中间的分隔线
            canvas.drawLine(0, midY, w, midY, mPaintGrid);

            // ========== 1. 绘制海拔图（上半区）==========
            if (mAltData != null && mAltData.length > 1) {
                int sampleCount = mAltData.length;
                double minAlt = Double.MAX_VALUE;
                double maxAlt = -Double.MAX_VALUE;
                for (double v : mAltData) {
                    if (v < minAlt) minAlt = v;
                    if (v > maxAlt) maxAlt = v;
                }
                double rangeAlt = maxAlt - minAlt;
                if (rangeAlt < 0.1) rangeAlt = 1.0;

                Path altPath = new Path();
                for (int i = 0; i < sampleCount; i++) {
                    float x = (float) (((double) i / (sampleCount - 1)) * w);
                    float y = (float) (midY -
                        ((mAltData[i] - minAlt) / rangeAlt) * (midY * 0.9));
                    if (i == 0) altPath.moveTo(x, y);
                    else altPath.lineTo(x, y);
                }
                canvas.drawPath(altPath, mPaintAltLine);

                // 海拔标签：上半区左上角
                String altLabel = MyActivity.zh_cn ? "海拔" : "Alt";
                drawLabel(canvas, altLabel, labelX, labelPaddingTop);
            }

            // ========== 2. 绘制速度图（下半区）==========
            if (mSpeedData != null && mSpeedData.length > 1) {
                int sampleCount = mSpeedData.length;
                double maxSp = 0;
                for (double v : mSpeedData) if (v > maxSp) maxSp = v;

                for (int i = 0; i < sampleCount - 1; i++) {
                    double sp1 = mSpeedData[i];
                    double sp2 = mSpeedData[i + 1];

                    float x1 = (float) (((double) i / (sampleCount - 1)) * w);
                    float x2 = (float) (((double) (i + 1) / (sampleCount - 1)) *
                        w);

                    // 下半区高度为 midY，底部为 h，顶部为 midY
                    float y1 =
                        maxSp > 0
                            ? h - (float) ((sp1 / maxSp) * (midY * 0.9))
                            : h;
                    float y2 =
                        maxSp > 0
                            ? h - (float) ((sp2 / maxSp) * (midY * 0.9))
                            : h;

                    Path segmentPath = new Path();
                    segmentPath.moveTo(x1, h);
                    segmentPath.lineTo(x1, y1);
                    segmentPath.lineTo(x2, y2);
                    segmentPath.lineTo(x2, h);
                    segmentPath.close();

                    float ratio = maxSp > 0 ? (float) (sp1 / maxSp) : 0;
                    mPaintSpeedFill.setColor(getGradientColor(ratio));
                    canvas.drawPath(segmentPath, mPaintSpeedFill);
                }

                // 速度标签：下半区左上角
                String speedLabel = MyActivity.zh_cn ? "速度" : "Speed";
                drawLabel(canvas, speedLabel, labelX, midY + labelPaddingTop);
            }
        }
    }
}
