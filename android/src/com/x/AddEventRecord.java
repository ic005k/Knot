package com.x;

import android.os.Bundle;
import android.text.Editable;
import android.text.TextUtils;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;

public class AddEventRecord extends AppCompatActivity {

    private Button etTitle;
    private EditText etCategory;
    private EditText etNote;
    private EditText etAmount;
    private TextView tvTimeDisplay;
    private SeekBar seekHour;
    private SeekBar seekMinute;

    private int currentHour;
    private int currentMinute;
    private int currentSecond;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_add_event_record);
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

        // 设置标题点击事件占位
        etTitle.setOnClickListener(v -> {
            // TODO: 点击标题业务，例如弹窗编辑标题
        });
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
                ed.append(append);
                etAmount.setSelection(ed.length());
            });
        }
        //退格
        Button backspaceBtn = findViewById(R.id.key_backspace);
        if (backspaceBtn != null) {
            backspaceBtn.setText("←");
            backspaceBtn.setOnClickListener(v -> {
                //退格逻辑
                String text = etAmount.getText().toString();
                if (!TextUtils.isEmpty(text)) {
                    etAmount.setText(text.substring(0, text.length() - 1));
                }
            });
        }
    }

    private void setupClearButtons() {
        findViewById(R.id.btn_clear_category).setOnClickListener(v ->
            etCategory.setText("")
        );
        findViewById(R.id.btn_clear_note).setOnClickListener(v ->
            etNote.setText("")
        );
        findViewById(R.id.btn_clear_amount).setOnClickListener(v ->
            etAmount.setText("")
        );
        findViewById(R.id.btn_cancel).setOnClickListener(v -> finish());

        findViewById(R.id.btn_category).setOnClickListener(v -> {
            // TODO:打开分类弹窗
        });

        findViewById(R.id.btn_confirm).setOnClickListener(v -> {
            // 直接拿界面展示的格式化时间字符串，与用户所见完全一致
            String finalTimeTag = tvTimeDisplay.getText().toString();

            String finalTitle = etTitle.getText().toString();
            String finalCategory = etCategory.getText().toString();
            String finalNote = etNote.getText().toString();
            String finalAmount = etAmount.getText().toString();

            // TODO：把 finalTitle、finalCategory、finalNote、finalAmount、finalTimeTag 回传给C++
            finish();
        });
    }
}
