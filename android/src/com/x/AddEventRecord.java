package com.x;

import android.content.Context;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextUtils;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Filter;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import java.util.ArrayList;
import java.util.List;

public class AddEventRecord extends AppCompatActivity {

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

    private int currentHour;
    private int currentMinute;
    private int currentSecond;

    public static native void PublicJavaCallCpp(String type);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

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

        // 根据全局语言标记动态设置 hint
        if (MyActivity.zh_cn) {
            etCategory.setHint("输入分类");
            etNote.setHint("输入备注");
        } else {
            etCategory.setHint("Enter category");
            etNote.setHint("Enter note");
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
            // TODO: 点击标题业务，例如弹窗编辑标题
            sendDataToCpp();
            PublicJavaCallCpp("select_tab");
            onBackPressed();
        });

        findViewById(R.id.btn_cancel).setOnClickListener(v -> finish());

        findViewById(R.id.btn_category).setOnClickListener(v -> {
            // TODO:打开分类弹窗
            sendDataToCpp();
            PublicJavaCallCpp("open_category_dialog");
            onBackPressed();
        });

        findViewById(R.id.btn_confirm).setOnClickListener(v -> {
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
}
