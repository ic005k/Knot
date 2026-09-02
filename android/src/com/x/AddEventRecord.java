package com.x;

import android.app.AlertDialog;
import android.content.Context;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Filter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.TextView;
import androidx.activity.OnBackPressedCallback;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import java.util.ArrayList;
import java.util.List;

public class AddEventRecord extends AppCompatActivity {

    public static AddEventRecord mInstance = null;

    private Button etTitle;
    private AutoCompleteTextView etCategory;
    private EditText etNote;
    private EditText etAmount;
    private TextView tvTimeDisplay;
    private SeekBar seekHour;
    private SeekBar seekMinute;

    private ImageView iconJe;
    private ImageView iconXq;
    private ImageView iconCategory;

    private Button btnClearCategory;
    private Button btnClearNote;
    private Button btnClearAmount;

    private Button btnCancel;
    private Button btnCategory;
    private Button btnConfirm;
    private TextView tvHourLabel;
    private TextView tvMinuteLabel;

    private int currentHour;
    private int currentMinute;
    private int currentSecond;

    public static native void PublicJavaCallCpp(String type);

    private OnBackPressedCallback mBackCallback;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mInstance = this;

        // 注册返回拦截回调
        mBackCallback = new OnBackPressedCallback(true /* enabled */) {
            @Override
            public void handleOnBackPressed() {
                PublicJavaCallCpp("cancel_add_event_record");
                finish();
            }
        };
        getOnBackPressedDispatcher().addCallback(this, mBackCallback);

        if (MyActivity.isDark) {
            setContentView(R.layout.activity_add_event_record_dark);
        } else {
            setContentView(R.layout.activity_add_event_record);
        }

        ImmersiveUtil.applyRealImmersive(this);

        bindViews();

        Bundle bundle = getIntent().getExtras();
        String titleText = "";
        String categoryText = "";
        String noteText = "";
        String amountText = "";
        String timeTagText = "";

        if (bundle != null) {
            titleText = bundle.getString("title_text", "");
            categoryText = bundle.getString("category_text", "");
            noteText = bundle.getString("note_text", "");
            amountText = bundle.getString("amount_text", "");
            timeTagText = bundle.getString("time_tag_text", "");
        }

        // 填充文本输入框
        etTitle.setText(titleText);
        etCategory.setText(categoryText);
        etNote.setText(noteText);
        etAmount.setText(amountText);

        // 完全由C++传入timeTagText初始化时间；Java不再读取系统时间
        parseTimeTag(timeTagText);
        initTimeUi();

        setupKeyboardClicks();
        setupSeekBarListener();
        setupClearButtons();
    }

    private void bindViews() {
        etTitle = findViewById(R.id.et_title);
        etCategory = findViewById(R.id.et_category);
        etNote = findViewById(R.id.et_note);
        etAmount = findViewById(R.id.et_amount);
        tvTimeDisplay = findViewById(R.id.tv_time_display);
        seekHour = findViewById(R.id.seek_hour);
        seekMinute = findViewById(R.id.seek_minute);

        iconJe = findViewById(R.id.icon_je);
        iconXq = findViewById(R.id.icon_xq);
        iconCategory = findViewById(R.id.icon_category);

        btnClearCategory = findViewById(R.id.btn_clear_category);
        btnClearNote = findViewById(R.id.btn_clear_note);
        btnClearAmount = findViewById(R.id.btn_clear_amount);

        // 按钮、时间标签
        btnCancel = findViewById(R.id.btn_cancel);
        btnCategory = findViewById(R.id.btn_category);
        btnConfirm = findViewById(R.id.btn_confirm);
        tvHourLabel = findViewById(R.id.tv_hour_label);
        tvMinuteLabel = findViewById(R.id.tv_minute_label);

        // 根据全局语言标记动态设置 hint
        if (MyActivity.zh_cn) {
            etCategory.setHint("输入分类");
            etNote.setHint("输入备注");

            btnCancel.setText("取消");
            btnCategory.setText("分类");
            btnConfirm.setText("确定");

            tvHourLabel.setText("时");
            tvMinuteLabel.setText("分");
        } else {
            etCategory.setHint("Enter category");
            etNote.setHint("Enter note");

            btnCancel.setText("Cancel");
            btnCategory.setText("Category");
            btnConfirm.setText("Confirm");

            tvHourLabel.setText("Hour");
            tvMinuteLabel.setText("Minute");
        }
        etAmount.setHint("");

        // ==========自动完成适配器==========
        refreshCategoryAutoComplete();

        int iconColor;
        if (MyActivity.isDark) {
            iconColor = 0xFFFFFFFF; //暗黑：白色图标
        } else {
            iconColor = 0xFF000000; //亮色：黑色图标
        }

        iconJe.setColorFilter(iconColor);
        iconXq.setColorFilter(iconColor);
        iconCategory.setColorFilter(iconColor);

        btnClearCategory.setTextColor(iconColor);
        btnClearNote.setTextColor(iconColor);
        btnClearAmount.setTextColor(iconColor);
    }

    /**
     * 解析C++传过来 HH:mm:ss 格式时间字符串
     * 解析失败/空，兜底 00:00:00
     */
    private void parseTimeTag(String timeTagText) {
        currentHour = 0;
        currentMinute = 0;
        currentSecond = 0;

        if (TextUtils.isEmpty(timeTagText)) {
            return;
        }
        try {
            String[] parts = timeTagText.split(":");
            if (parts.length >= 1) {
                currentHour = Integer.parseInt(parts[0]);
            }
            if (parts.length >= 2) {
                currentMinute = Integer.parseInt(parts[1]);
            }
            if (parts.length >= 3) {
                currentSecond = Integer.parseInt(parts[2]);
            }
            // 边界钳位
            currentHour = Math.max(0, Math.min(23, currentHour));
            currentMinute = Math.max(0, Math.min(59, currentMinute));
            currentSecond = Math.max(0, Math.min(59, currentSecond));
        } catch (Exception ignored) {
            // 解析异常保持兜底0值
        }
    }

    /**
     * 设置SeekBar范围与初始进度、刷新时间文本
     */
    private void initTimeUi() {
        seekHour.setMax(23);
        seekHour.setProgress(currentHour);
        seekMinute.setMax(59);
        seekMinute.setProgress(currentMinute);
        refreshTimeText();
    }

    private void refreshTimeText() {
        String timeStr = String.format(
            "%02d:%02d:%02d",
            currentHour,
            currentMinute,
            currentSecond
        );
        tvTimeDisplay.setText(timeStr);
    }

    private void setupSeekBarListener() {
        seekHour.setOnSeekBarChangeListener(
            new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged(
                    SeekBar seekBar,
                    int progress,
                    boolean fromUser
                ) {
                    if (fromUser) {
                        currentHour = progress;
                        refreshTimeText();
                    }
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {}

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {}
            }
        );
        seekMinute.setOnSeekBarChangeListener(
            new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged(
                    SeekBar seekBar,
                    int progress,
                    boolean fromUser
                ) {
                    if (fromUser) {
                        currentMinute = progress;
                        refreshTimeText();
                    }
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {}

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {}
            }
        );
    }

    /**
     * 数字键盘按键，输出内容到金额输入框
     */
    private void setupKeyboardClicks() {
        int[] keyIds = {
            R.id.key_0,
            R.id.key_1,
            R.id.key_2,
            R.id.key_3,
            R.id.key_4,
            R.id.key_5,
            R.id.key_6,
            R.id.key_7,
            R.id.key_8,
            R.id.key_9,
            R.id.key_dot,
        };
        for (int id : keyIds) {
            Button btn = findViewById(id);
            if (btn == null) continue;
            btn.setOnClickListener(v -> {
                String append = ((Button) v).getText().toString();
                Editable ed = etAmount.getText();
                String current = ed.toString();

                boolean allow = true;
                if (".".equals(append)) {
                    // 不允许多个小数点
                    if (current.contains(".")) {
                        allow = false;
                    }
                } else {
                    // 小数点后最多2位
                    int dotPos = current.indexOf('.');
                    if (dotPos != -1) {
                        int decimalCount = current.length() - dotPos - 1;
                        if (decimalCount >= 2) {
                            allow = false;
                        }
                    }
                }

                if (allow) {
                    ed.append(append);
                    etAmount.setSelection(ed.length());
                }
            });
        }
        //退格
        Button backspaceBtn = findViewById(R.id.key_backspace);
        if (backspaceBtn != null) {
            backspaceBtn.setText("←");
            backspaceBtn.setOnClickListener(v -> {
                String text = etAmount.getText().toString();
                if (!TextUtils.isEmpty(text)) {
                    etAmount.setText(text.substring(0, text.length() - 1));
                }
            });
        }
    }

    private void setupClearButtons() {
        btnClearCategory.setOnClickListener(v -> etCategory.setText(""));
        btnClearNote.setOnClickListener(v -> etNote.setText(""));
        btnClearAmount.setOnClickListener(v -> etAmount.setText(""));

        // 设置标题点击事件
        etTitle.setOnClickListener(v -> {
            // TODO: 点击标题业务
            CharSequence titleText = etTitle.getText();
            String text = titleText != null ? titleText.toString() : "";

            // 以"增加"开头 或者 以"Add"开头，才继续执行；否则直接返回
            if (!text.startsWith("增加") && !text.startsWith("Add")) {
                return;
            }

            sendDataToCpp();
            PublicJavaCallCpp("select_tab");
            finish();
        });

        btnCancel.setOnClickListener(v -> {
            PublicJavaCallCpp("cancel_add_event_record");
            finish();
        });

        btnCategory.setOnClickListener(v -> {
            // TODO:打开分类弹窗
            //sendDataToCpp();
            PublicJavaCallCpp("open_category_dialog");
            //finish();

            PublicJavaCallCpp("open_category_select");
        });

        btnConfirm.setOnClickListener(v -> {
            sendDataToCpp();
            PublicJavaCallCpp("add_event_record");
            finish();
        });
    }

    private void sendDataToCpp() {
        // 直接拿界面展示的格式化时间字符串，与用户所见完全一致
        String finalTimeTag = tvTimeDisplay.getText().toString();

        String finalTitle = etTitle.getText().toString();
        String finalCategory = etCategory.getText().toString();
        String finalNote = etNote.getText().toString();
        String finalAmount = etAmount.getText().toString();

        // TODO：把 finalTitle、finalCategory、finalNote、finalAmount、finalTimeTag 回传给C++
        MyActivity.m_instance.setTempSwapStr(
            finalCategory +
                "|==|" +
                finalNote +
                "|==|" +
                finalAmount +
                "|==|" +
                finalTimeTag
        );
    }

    private void refreshCategoryAutoComplete() {
        List<String> categoryList = getHistoryCategoryFromCpp();
        ContainsArrayAdapter adapter = new ContainsArrayAdapter(
            AddEventRecord.this,
            R.layout.dropdown_item,
            categoryList
        );
        etCategory.setAdapter(adapter);
    }

    private List<String> getHistoryCategoryFromCpp() {
        List<String> list = new ArrayList<>();
        String raw = MyActivity.m_instance.getTempSwapStr();
        if (raw == null || raw.isEmpty()) {
            return list;
        }
        // 使用 |==| 分割字符串
        String[] items = raw.split("\\|==\\|");
        for (String s : items) {
            String trim = s.trim();
            // 过滤空字符串（末尾 |==| 分割出来会产生空项）
            if (!trim.isEmpty()) {
                list.add(trim);
            }
        }
        return list;
    }

    // 自定义的包含过滤的适配器 /////////////////////////////////////////////////
    class ContainsArrayAdapter extends ArrayAdapter<String> {

        private final List<String> mOriginalList;
        private List<String> mFilteredList;

        public ContainsArrayAdapter(
            Context context,
            int resource,
            List<String> objects
        ) {
            super(context, resource, objects);
            mOriginalList = new ArrayList<>(objects);
            mFilteredList = new ArrayList<>(objects);
        }

        @Override
        public int getCount() {
            return mFilteredList.size();
        }

        @Override
        public String getItem(int position) {
            return mFilteredList.get(position);
        }

        @NonNull
        @Override
        public Filter getFilter() {
            return new Filter() {
                @Override
                protected FilterResults performFiltering(
                    CharSequence constraint
                ) {
                    FilterResults results = new FilterResults();
                    List<String> temp = new ArrayList<>();
                    if (constraint == null || constraint.length() == 0) {
                        temp.addAll(mOriginalList);
                    } else {
                        String key = constraint.toString().toLowerCase();
                        for (String s : mOriginalList) {
                            if (s.toLowerCase().contains(key)) {
                                temp.add(s);
                            }
                        }
                    }
                    results.values = temp;
                    results.count = temp.size();
                    return results;
                }

                @Override
                protected void publishResults(
                    CharSequence constraint,
                    FilterResults results
                ) {
                    mFilteredList = (List<String>) results.values;
                    notifyDataSetChanged();
                }
            };
        }
    }

    ///////////////////////////////////////////////////////////////////
    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (mBackCallback != null) {
            mBackCallback.remove();
            mBackCallback = null;
        }

        mInstance = null;
    }

    /**
     * 由C++直接回调调用，传入分类字符串数组
     */
    /**
     * 由C++直接回调调用，传入分类字符串数组
     */
    public void showCategorySelectDialog(final ArrayList<String> catList) {
        // 切到Android UI主线程，防止Qt子线程调用崩溃
        if (!isMainThread()) {
            runOnUiThread(() -> showCategorySelectDialog(catList));
            return;
        }

        //防御判空
        if (catList == null || catList.isEmpty()) {
            return;
        }
        Context ctx = AddEventRecord.this;
        LinearLayout rootLayout = new LinearLayout(ctx);
        rootLayout.setOrientation(LinearLayout.VERTICAL);
        int dp12 = dip2px(ctx, 12);
        rootLayout.setPadding(dp12, dp12, dp12, dp12);
        //顶部：重命名按钮 + 输入框
        LinearLayout topRow = new LinearLayout(ctx);
        topRow.setOrientation(LinearLayout.HORIZONTAL);
        topRow.setGravity(Gravity.CENTER_VERTICAL);
        topRow.setLayoutParams(
            new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
        );
        Button btnRename = new Button(ctx);
        btnRename.setText(MyActivity.zh_cn ? "重命名" : "Rename");
        EditText etInput = new EditText(ctx);
        LinearLayout.LayoutParams etLp = new LinearLayout.LayoutParams(
            0,
            LinearLayout.LayoutParams.WRAP_CONTENT,
            1.0f
        );
        etLp.setMargins(dip2px(ctx, 8), 0, 0, 0);
        etInput.setLayoutParams(etLp);
        etInput.setText(etCategory.getText());
        topRow.addView(btnRename);
        topRow.addView(etInput);
        rootLayout.addView(topRow);
        //ListView，系统内置item
        ListView listView = new ListView(ctx);
        LinearLayout.LayoutParams listLp = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            0,
            1.0f
        );
        listLp.setMargins(0, dip2px(ctx, 12), 0, dip2px(ctx, 8));
        listView.setLayoutParams(listLp);

        class CatListAdapter extends ArrayAdapter<String> {

            private int mSelectedIndex = -1;

            public CatListAdapter(
                Context context,
                int resource,
                java.util.List<String> objects
            ) {
                super(context, resource, objects);
            }

            public void setSelectIndex(int pos) {
                mSelectedIndex = pos;
                notifyDataSetChanged();
            }

            public int getSelectIndex() {
                return mSelectedIndex;
            }

            public String getSelectedItem() {
                if (mSelectedIndex >= 0 && mSelectedIndex < getCount()) {
                    return getItem(mSelectedIndex);
                }
                return "";
            }

            @NonNull
            @Override
            public View getView(
                int position,
                @Nullable View convertView,
                @NonNull ViewGroup parent
            ) {
                View v = super.getView(position, convertView, parent);
                TextView tv = v.findViewById(android.R.id.text1);
                boolean sel = position == mSelectedIndex;
                if (MyActivity.isDark) {
                    if (sel) {
                        v.setBackgroundColor(0xFF3A3A3A);
                        tv.setTextColor(0xFFFFFFFF);
                    } else {
                        v.setBackgroundColor(0xFF1E1E1E);
                        tv.setTextColor(0xFFBBBBBB);
                    }
                } else {
                    if (sel) {
                        v.setBackgroundColor(0xFFB8DAF5);
                        tv.setTextColor(0xFF000000);
                    } else {
                        v.setBackgroundColor(0xFFFFFFFF);
                        tv.setTextColor(0xFF222222);
                    }
                }
                return v;
            }
        }
        CatListAdapter adapter = new CatListAdapter(
            ctx,
            android.R.layout.simple_list_item_1,
            catList
        );

        listView.setAdapter(adapter);
        listView.setOnItemClickListener((parent, view, position, id) -> {
            adapter.setSelectIndex(position);
            String item = catList.get(position);
            etInput.setText(item);
        });
        rootLayout.addView(listView);
        //总计文本
        TextView tvTotal = new TextView(ctx);
        String totalStr;
        if (MyActivity.zh_cn) {
            totalStr = "总计：" + catList.size();
        } else {
            totalStr = "Total: " + catList.size();
        }
        tvTotal.setText(totalStr);
        tvTotal.setPadding(0, 0, 0, dip2px(ctx, 8));
        rootLayout.addView(tvTotal);
        //底部三个按钮
        LinearLayout bottomRow = new LinearLayout(ctx);
        bottomRow.setOrientation(LinearLayout.HORIZONTAL);
        bottomRow.setLayoutParams(
            new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
        );
        Button btnCancel = new Button(ctx);
        btnCancel.setText(MyActivity.zh_cn ? "取消" : "Cancel");
        Button btnDelete = new Button(ctx);
        btnDelete.setText(MyActivity.zh_cn ? "删除" : "Delete");
        Button btnOk = new Button(ctx);
        btnOk.setText(MyActivity.zh_cn ? "确定" : "Confirm");
        int margin4 = dip2px(ctx, 4);
        LinearLayout.LayoutParams btnLp = new LinearLayout.LayoutParams(
            0,
            LinearLayout.LayoutParams.WRAP_CONTENT,
            1.0f
        );
        btnLp.setMargins(0, 0, margin4, 0);
        btnCancel.setLayoutParams(btnLp);
        btnLp = new LinearLayout.LayoutParams(
            0,
            LinearLayout.LayoutParams.WRAP_CONTENT,
            1.0f
        );
        btnLp.setMargins(margin4, 0, margin4, 0);
        btnDelete.setLayoutParams(btnLp);
        btnLp = new LinearLayout.LayoutParams(
            0,
            LinearLayout.LayoutParams.WRAP_CONTENT,
            1.0f
        );
        btnLp.setMargins(margin4, 0, 0, 0);
        btnOk.setLayoutParams(btnLp);
        bottomRow.addView(btnCancel);
        bottomRow.addView(btnDelete);
        bottomRow.addView(btnOk);
        rootLayout.addView(bottomRow);
        AlertDialog.Builder builder = new AlertDialog.Builder(ctx);
        builder.setView(rootLayout);
        AlertDialog dialog = builder.create();
        btnCancel.setOnClickListener(v -> dialog.dismiss());
        btnOk.setOnClickListener(v -> {
            String selText = adapter.getSelectedItem();
            if (!selText.isEmpty()) {
                etCategory.setText(selText);
            }
            dialog.dismiss();
        });

        btnDelete.setOnClickListener(v -> {
            final int pos = adapter.getSelectIndex();
            if (pos < 0) {
                return;
            }
            // 弹出二次确认框
            new AlertDialog.Builder(ctx)
                .setTitle(MyActivity.zh_cn ? "确认删除" : "Confirm Delete")
                .setMessage(
                    MyActivity.zh_cn
                        ? "确定要删除该分类吗？"
                        : "Are you sure to delete this category?"
                )
                .setPositiveButton(
                    MyActivity.zh_cn ? "删除" : "Delete",
                    (d, which) -> {
                        // 用户确认，才调用C++执行删除
                        PublicJavaCallCpp("category_delete|==|" + pos);
                        dialog.dismiss(); // 关闭外层分类选择弹窗
                    }
                )
                .setNegativeButton(MyActivity.zh_cn ? "取消" : "Cancel", null)
                .show();
        });

        btnRename.setOnClickListener(v -> {
            int pos = adapter.getSelectIndex();
            String newName = etInput.getText().toString();
            if (pos >= 0) {
                PublicJavaCallCpp(
                    "category_rename|==|" + pos + "|==|" + newName
                );
            }
            dialog.dismiss();
        });
        dialog.show();
    }

    /** 判断当前是否Android主线程 */
    private boolean isMainThread() {
        return (
            android.os.Looper.myLooper() == android.os.Looper.getMainLooper()
        );
    }

    private int dip2px(Context context, int dpValue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int) (dpValue * scale + 0.5f);
    }
}
