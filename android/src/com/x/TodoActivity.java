package com.x;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import java.lang.ref.WeakReference;
import java.util.ArrayList;

/**
 * Todo待办页面
 * Java仅视图渲染+事件转发，业务逻辑全部C++
 */
public class TodoActivity extends AppCompatActivity {

    private static boolean isDark = false;
    private OnBackPressedCallback mBackCallback;
    public static TodoActivity mInstance = null;
    private WeakReference<TodoActivity> mSelfWeakRef;
    private RecyclerView mRvTodoList;
    public TodoCardAdapter mTodoAdapter;
    private EditText etTodoInput;
    private ImageView ivTodoAdd;
    private ImageView ivTodoClear;
    private View layoutTodoRoot;

    public static native void PublicJavaCallCpp(String type);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mInstance = this;
        mBackCallback = new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                PublicJavaCallCpp("cancel_add_event_record");
                finish();
            }
        };
        getOnBackPressedDispatcher().addCallback(this, mBackCallback);
        setContentView(R.layout.activity_todo);
        isDark = ImmersiveUtil.applyRealImmersive(this);
        mSelfWeakRef = new WeakReference<>(this);

        layoutTodoRoot = findViewById(R.id.layout_todo_root);
        //输入栏
        etTodoInput = findViewById(R.id.et_todo_input);
        ivTodoAdd = findViewById(R.id.iv_todo_add);
        ivTodoClear = findViewById(R.id.iv_todo_clear);

        ivTodoAdd.setOnClickListener(v -> {
            String text = etTodoInput.getText().toString().trim();
            PublicJavaCallCpp("todo_add|==|" + text);
            etTodoInput.setText("");
        });
        ivTodoClear.setOnClickListener(v -> {
            PublicJavaCallCpp("todo_clear_input");
            etTodoInput.setText("");
        });

        //Todo列表初始化
        mRvTodoList = findViewById(R.id.rv_todo_list);
        mRvTodoList.setLayoutManager(new LinearLayoutManager(this));
        mTodoAdapter = new TodoCardAdapter();
        mRvTodoList.setAdapter(mTodoAdapter);

        //接收Intent传入todo列表数据
        ArrayList<String> todoDataList = getIntent().getStringArrayListExtra(
            "todo_list"
        );
        if (todoDataList == null) {
            todoDataList = new ArrayList<>();
        }
        mTodoAdapter.setStringListData(todoDataList);

        refreshUi();

        mTodoAdapter.setOnItemActionListener(pos -> {
            String rawItem = mTodoAdapter.getItemAt(pos);
            if (rawItem == null) return;
            String[] parts = rawItem.split("\\|==\\|");
            if (parts.length < 3) return;
            String oldText = parts[2];
            showTodoEditDialog(pos, oldText);
        });
    }

    private void updateAllTexts() {
        boolean zh = MyActivity.zh_cn;
        if (zh) {
            etTodoInput.setHint("输入待办事项文本");
        } else {
            etTodoInput.setHint("Input todo item");
        }
    }

    private void updateAllColor() {
        int iconColor;
        int rootBgColor;
        if (isDark) {
            iconColor = 0xFFFFFFFF;
            rootBgColor = 0xFF121212;
        } else {
            iconColor = 0xFF000000;
            rootBgColor = 0xFFF5F5F5;
        }
        layoutTodoRoot.setBackgroundColor(rootBgColor);
        //顶部图标着色
        ivTodoAdd.setColorFilter(iconColor);
        ivTodoClear.setColorFilter(iconColor);
        //同步暗黑模式到适配器
        mTodoAdapter.setDarkMode(isDark);
    }

    private void refreshUi() {
        updateAllTexts();
        updateAllColor();
    }

    /** C++ JNI调用，刷新todo列表 */
    public void refreshTodoList(ArrayList<String> list) {
        runOnUiThread(() -> {
            if (mTodoAdapter != null && !isFinishing() && !isDestroyed()) {
                mTodoAdapter.setStringListData(list);
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mBackCallback != null) {
            mBackCallback.remove();
            mBackCallback = null;
        }
        mSelfWeakRef.clear();
        mInstance = null;
        PublicJavaCallCpp("todo_activity_destroy");
        if (
            MyActivity.m_instance != null &&
            !MyActivity.m_instance.isFinishing() &&
            !MyActivity.m_instance.isDestroyed()
        ) {
            MyActivity.m_instance.runOnUiThread(() -> {
                MyActivity.m_instance.getWindow().getDecorView().requestFocus();
                MyActivity.m_instance
                    .getWindow()
                    .getDecorView()
                    .postInvalidate();
                MyService.forceDisconnectInputMethod();
            });
        }
    }

    @Override
    public void onBackPressed() {}

    public interface OnTodoItemActionListener {
        /** 请求编辑，直接触发弹窗 */
        void onRequestEdit(int pos);
    }

    /**
     * 弹出todo编辑框，直接依附当前Activity，明暗跟随isDark
     * @param pos 待编辑条目下标
     * @param oldText 原始文本
     */
    private void showTodoEditDialog(final int pos, String oldText) {
        runOnUiThread(() -> {
            android.widget.EditText et = new android.widget.EditText(this);
            et.setText(oldText);
            // 标准多行：允许自动换行，初始1行，内容变多自动增高
            et.setInputType(android.text.InputType.TYPE_TEXT_FLAG_MULTI_LINE);
            et.setSingleLine(false);
            et.setMinHeight(80); // 最小像素高度，避免输入框挤得太小，不再强制minLines
            et.setMaxLines(8); // 最多8行，超过后滚动，不再继续长高

            android.widget.LinearLayout ll = new android.widget.LinearLayout(
                this
            );
            android.widget.LinearLayout.LayoutParams lp =
                new android.widget.LinearLayout.LayoutParams(
                    android.widget.LinearLayout.LayoutParams.MATCH_PARENT,
                    android.widget.LinearLayout.LayoutParams.WRAP_CONTENT
                );
            lp.setMargins(48, 24, 48, 24);
            ll.setLayoutParams(lp);
            ll.addView(et);

            android.app.AlertDialog.Builder b =
                new android.app.AlertDialog.Builder(this);
            boolean zh = MyActivity.zh_cn;
            if (zh) {
                b.setTitle("编辑待办");
            } else {
                b.setTitle("Edit Todo");
            }
            b.setView(ll);

            if (isDark) {
                ll.setBackgroundColor(0xFF282828);
                et.setTextColor(0xFFEFEFEF);
                et.setHintTextColor(0xFFAAAAAA);
            } else {
                ll.setBackgroundColor(0xFFFFFFFF);
                et.setTextColor(0xFF222222);
                et.setHintTextColor(0xFF888888);
            }

            String strOk = zh ? "确定" : "OK";
            String strCancel = zh ? "取消" : "Cancel";

            b.setPositiveButton(strOk, (dialog, which) -> {
                String newText = et.getText().toString();
                PublicJavaCallCpp(
                    "todo_confirm_edit|==|" + pos + "|==|" + newText
                );
                dialog.dismiss();
            });
            b.setNegativeButton(strCancel, (dialog, which) -> dialog.dismiss());

            b.create().show();
        });
    }
}
