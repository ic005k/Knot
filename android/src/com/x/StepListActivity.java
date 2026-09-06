package com.x;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.Typeface;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;
import java.util.ArrayList;

public class StepListActivity extends AppCompatActivity {

    public static StepListActivity mInstance = null;
    private ListView mListView;
    private StepListAdapter mAdapter;
    private ArrayList<String> mDataList = new ArrayList<>();
    private boolean mIsDark;
    private OnBackPressedCallback mBackCallback;

    // JNI调用声明，确认项目已有该方法
    private native void PublicJavaCallCpp(String msg);

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

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setLayoutParams(
            new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        );

        int bgColor = mIsDark ? 0xFF121212 : 0xFFF5F5F5;
        int textColor = mIsDark ? Color.WHITE : Color.BLACK;
        int subColor = mIsDark ? 0xFFAAAAAA : 0xFF666666;
        root.setBackgroundColor(bgColor);

        // 标题行：标题 + AI分析按钮，同一行
        LinearLayout titleBar = new LinearLayout(this);
        titleBar.setOrientation(LinearLayout.HORIZONTAL);
        titleBar.setGravity(Gravity.CENTER_VERTICAL);
        titleBar.setPadding(dp(16), dp(16), dp(16), dp(8));

        TextView tvTitle = new TextView(this);
        tvTitle.setText(MyActivity.zh_cn ? "步数记录" : "Step Records");
        tvTitle.setTextSize(22);
        tvTitle.setTextColor(textColor);
        tvTitle.setLayoutParams(
            new LinearLayout.LayoutParams(
                0,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                1.0f
            )
        );

        Button btnAiAnalyse = new Button(this);
        btnAiAnalyse.setText(MyActivity.zh_cn ? "AI分析" : "AI Analyse");
        btnAiAnalyse.setPadding(dp(12), dp(6), dp(12), dp(6));
        btnAiAnalyse.setOnClickListener(v -> {
            PublicJavaCallCpp("step_ai_analyse");
        });

        titleBar.addView(tvTitle);
        titleBar.addView(btnAiAnalyse);
        root.addView(titleBar);

        LinearLayout headerRow = new LinearLayout(this);
        headerRow.setOrientation(LinearLayout.HORIZONTAL);
        headerRow.setPadding(dp(12), dp(8), dp(12), dp(8));
        headerRow.setBackgroundColor(mIsDark ? 0xFF1E1E1E : Color.WHITE);

        View headerPlaceholder = new View(this);
        headerPlaceholder.setLayoutParams(
            new LinearLayout.LayoutParams(
                dp(4),
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        );
        headerRow.addView(headerPlaceholder);

        String[] heads = MyActivity.zh_cn
            ? new String[] { "步数", "公里", "卡路里" }
            : new String[] { "Steps", "Km", "Cal" };
        for (int i = 0; i < heads.length; i++) {
            TextView h = new TextView(this);
            h.setText(heads[i]);
            h.setTextSize(14);
            h.setTextColor(subColor);
            h.setGravity(Gravity.CENTER);
            h.setLayoutParams(
                new LinearLayout.LayoutParams(
                    0,
                    ViewGroup.LayoutParams.WRAP_CONTENT,
                    1.0f
                )
            );
            h.setPadding(dp(10), 0, 0, 0);
            headerRow.addView(h);
        }
        root.addView(headerRow);

        mListView = new ListView(this);
        mListView.setLayoutParams(
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                0,
                1.0f
            )
        );
        mListView.setSelector(android.R.color.transparent);
        mListView.setDividerHeight(1);
        mListView.setDivider(
            mIsDark
                ? getResources().getDrawable(
                      android.R.drawable.divider_horizontal_textfield
                  )
                : getResources().getDrawable(
                      android.R.drawable.divider_horizontal_textfield
                  )
        );
        mAdapter = new StepListAdapter(this, mDataList, mIsDark);
        mListView.setAdapter(mAdapter);
        root.addView(mListView);

        setContentView(root);

        Intent intent = getIntent();
        ArrayList<String> dataList = intent.getStringArrayListExtra(
            "step_list_data"
        );
        if (dataList != null) {
            mDataList.clear();
            mDataList.addAll(dataList);
            mAdapter.notifyDataSetChanged();
            final int count = mDataList.size();
            if (count > 0) {
                mListView.post(() -> mListView.setSelection(count - 1));
            }
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

    private static class StepListAdapter extends ArrayAdapter<String> {

        private final boolean mIsDark;
        private final int COLOR_GOOD_LIGHT = 0xFF4CAF50;
        private final int COLOR_GOOD_DARK = 0xFF66BB6A;

        public StepListAdapter(
            android.content.Context ctx,
            ArrayList<String> data,
            boolean isDark
        ) {
            super(ctx, 0, data);
            mIsDark = isDark;
        }

        @Override
        public android.view.View getView(
            int position,
            android.view.View convertView,
            ViewGroup parent
        ) {
            ViewHolder holder;
            if (convertView == null) {
                LinearLayout rowRoot = new LinearLayout(getContext());
                rowRoot.setOrientation(LinearLayout.HORIZONTAL);
                rowRoot.setPadding(dp(12), dp(12), dp(12), dp(12));
                rowRoot.setGravity(Gravity.CENTER_VERTICAL);

                View markView = new View(getContext());
                LinearLayout.LayoutParams markLp =
                    new LinearLayout.LayoutParams(
                        dp(4),
                        ViewGroup.LayoutParams.MATCH_PARENT
                    );
                markView.setLayoutParams(markLp);

                LinearLayout contentContainer = new LinearLayout(getContext());
                contentContainer.setOrientation(LinearLayout.VERTICAL);
                contentContainer.setLayoutParams(
                    new LinearLayout.LayoutParams(
                        0,
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        1.0f
                    )
                );
                contentContainer.setPadding(dp(10), 0, 0, 0);

                TextView tvDate = new TextView(getContext());
                tvDate.setTextSize(15);
                tvDate.setTypeface(null, Typeface.BOLD);
                tvDate.setGravity(Gravity.CENTER);
                tvDate.setPadding(0, 0, 0, dp(8));

                LinearLayout valueRow = new LinearLayout(getContext());
                valueRow.setOrientation(LinearLayout.HORIZONTAL);

                TextView tvSteps = new TextView(getContext());
                tvSteps.setTextSize(15);
                tvSteps.setGravity(Gravity.CENTER);
                tvSteps.setLayoutParams(
                    new LinearLayout.LayoutParams(
                        0,
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        1.0f
                    )
                );

                TextView tvKm = new TextView(getContext());
                tvKm.setTextSize(15);
                tvKm.setGravity(Gravity.CENTER);
                tvKm.setLayoutParams(
                    new LinearLayout.LayoutParams(
                        0,
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        1.0f
                    )
                );

                TextView tvCal = new TextView(getContext());
                tvCal.setTextSize(15);
                tvCal.setGravity(Gravity.CENTER);
                tvCal.setLayoutParams(
                    new LinearLayout.LayoutParams(
                        0,
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        1.0f
                    )
                );

                valueRow.addView(tvSteps);
                valueRow.addView(tvKm);
                valueRow.addView(tvCal);

                contentContainer.addView(tvDate);
                contentContainer.addView(valueRow);

                rowRoot.addView(markView);
                rowRoot.addView(contentContainer);

                int textColor = mIsDark ? Color.WHITE : Color.BLACK;
                tvDate.setTextColor(textColor);
                tvSteps.setTextColor(textColor);
                tvKm.setTextColor(textColor);
                tvCal.setTextColor(textColor);

                convertView = rowRoot;
                holder = new ViewHolder();
                holder.markView = markView;
                holder.tvDate = tvDate;
                holder.tvSteps = tvSteps;
                holder.tvKm = tvKm;
                holder.tvCal = tvCal;
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            String line = getItem(position);
            String[] parts = line.split("===");
            String date = parts.length >= 1 ? parts[0] : "";
            String stepsStr = parts.length >= 2 ? parts[1] : "0";
            String km = parts.length >= 3 ? parts[2] : "";
            String cal = parts.length >= 4 ? parts[3] : "";
            String thresholdStr = parts.length >= 5 ? parts[4] : "0";

            holder.tvDate.setText(date);
            holder.tvSteps.setText(stepsStr);
            holder.tvKm.setText(km);
            holder.tvCal.setText(cal);

            boolean isReachGoal = false;
            try {
                int stepsVal = Integer.parseInt(stepsStr.trim());
                int thresholdVal = Integer.parseInt(thresholdStr.trim());
                if (stepsVal >= thresholdVal) {
                    isReachGoal = true;
                }
            } catch (NumberFormatException ignored) {}

            if (isReachGoal) {
                holder.markView.setVisibility(View.VISIBLE);
                holder.markView.setBackgroundColor(
                    mIsDark ? COLOR_GOOD_DARK : COLOR_GOOD_LIGHT
                );
            } else {
                holder.markView.setVisibility(View.GONE);
            }

            if (mIsDark) {
                convertView.setBackgroundColor(
                    position % 2 == 0 ? 0xFF1E1E1E : 0xFF202020
                );
            } else {
                convertView.setBackgroundColor(
                    position % 2 == 0 ? Color.WHITE : 0xFFF2F2F2
                );
            }
            return convertView;
        }

        private int dp(int dpVal) {
            float density = getContext()
                .getResources()
                .getDisplayMetrics()
                .density;
            return (int) (dpVal * density + 0.5f);
        }

        private static class ViewHolder {

            View markView;
            TextView tvDate;
            TextView tvSteps;
            TextView tvKm;
            TextView tvCal;
        }
    }
}
