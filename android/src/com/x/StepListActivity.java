package com.x;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.Typeface;
import android.os.Bundle;
import android.view.Gravity;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
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

        TextView tvTitle = new TextView(this);
        tvTitle.setText(MyActivity.zh_cn ? "步数记录" : "Step Records");
        tvTitle.setTextSize(22);
        tvTitle.setTextColor(textColor);
        tvTitle.setPadding(dp(16), dp(16), dp(16), dp(8));
        root.addView(tvTitle);

        LinearLayout headerRow = new LinearLayout(this);
        headerRow.setOrientation(LinearLayout.HORIZONTAL);
        headerRow.setPadding(dp(12), dp(8), dp(12), dp(8));
        headerRow.setBackgroundColor(mIsDark ? 0xFF1E1E1E : Color.WHITE);
        String[] heads = MyActivity.zh_cn
            ? new String[] { "日期", "步数", "公里", "卡路里" }
            : new String[] { "Date", "Steps", "Km", "Cal" };
        for (int i = 0; i < heads.length; i++) {
            TextView h = new TextView(this);
            h.setText(heads[i]);
            h.setTextSize(14);
            h.setTextColor(subColor);
            h.setGravity(i == 0 ? Gravity.CENTER : Gravity.LEFT);
            h.setLayoutParams(
                new LinearLayout.LayoutParams(
                    0,
                    ViewGroup.LayoutParams.WRAP_CONTENT,
                    1.0f
                )
            );
            // 表头日期列右侧增加间距
            if (i == 0) {
                h.setPadding(0, 0, dp(10), 0);
            }
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
                LinearLayout row = new LinearLayout(getContext());
                row.setOrientation(LinearLayout.HORIZONTAL);
                row.setPadding(dp(12), dp(12), dp(12), dp(12));
                row.setGravity(Gravity.CENTER_VERTICAL);

                TextView tvDate = new TextView(getContext());
                tvDate.setTextSize(15);
                tvDate.setTypeface(null, Typeface.BOLD);
                tvDate.setGravity(Gravity.CENTER);
                // 日期单元格右侧增加10dp空白，解决和步数拥挤
                tvDate.setPadding(0, 0, dp(10), 0);

                TextView tvSteps = new TextView(getContext());
                tvSteps.setTextSize(15);
                tvSteps.setGravity(Gravity.LEFT);

                TextView tvKm = new TextView(getContext());
                tvKm.setTextSize(15);
                tvKm.setGravity(Gravity.LEFT);

                TextView tvCal = new TextView(getContext());
                tvCal.setTextSize(15);
                tvCal.setGravity(Gravity.LEFT);

                tvDate.setLayoutParams(
                    new LinearLayout.LayoutParams(
                        0,
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        1.0f
                    )
                );
                tvSteps.setLayoutParams(
                    new LinearLayout.LayoutParams(
                        0,
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        1.0f
                    )
                );
                tvKm.setLayoutParams(
                    new LinearLayout.LayoutParams(
                        0,
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        1.0f
                    )
                );
                tvCal.setLayoutParams(
                    new LinearLayout.LayoutParams(
                        0,
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        1.0f
                    )
                );

                row.addView(tvDate);
                row.addView(tvSteps);
                row.addView(tvKm);
                row.addView(tvCal);

                int textColor = mIsDark ? Color.WHITE : Color.BLACK;
                tvDate.setTextColor(textColor);
                tvSteps.setTextColor(textColor);
                tvKm.setTextColor(textColor);
                tvCal.setTextColor(textColor);

                convertView = row;
                holder = new ViewHolder();
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
            String steps = parts.length >= 2 ? parts[1] : "";
            String km = parts.length >= 3 ? parts[2] : "";
            String cal = parts.length >= 4 ? parts[3] : "";

            holder.tvDate.setText(date);
            holder.tvSteps.setText(steps);
            holder.tvKm.setText(km);
            holder.tvCal.setText(cal);

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

            TextView tvDate;
            TextView tvSteps;
            TextView tvKm;
            TextView tvCal;
        }
    }
}
