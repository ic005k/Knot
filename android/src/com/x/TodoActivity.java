package com.x;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
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
    private TodoCardAdapter mTodoAdapter;

    private EditText etTodoInput;
    private ImageView ivTodoAdd;
    private ImageView ivTodoClear;

    //底部Tab控件
    private View mTabRead;
    private View mTabTodo;
    private View mTabNote;
    private View mTabSport;
    private ImageView ivTabRead;
    private ImageView ivTabTodo;
    private ImageView ivTabNote;
    private ImageView ivTabSport;
    private TextView tvTabRead;
    private TextView tvTabTodo;
    private TextView tvTabNote;
    private TextView tvTabSport;

    private View layoutTodoRoot;
    private View bottomTabLayout;

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
        bottomTabLayout = findViewById(R.id.bottom_tab_layout);

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

        //条目点击事件全部转发给C++
        mTodoAdapter.setOnItemActionListener((pos, actionCode) -> {
            PublicJavaCallCpp(
                "todo_item_action|==|" + pos + "|==|" + actionCode
            );
        });

        //绑定底部Tab
        bindBottomTabViews();
        refreshUi();
    }

    private void bindBottomTabViews() {
        mTabRead = findViewById(R.id.tab_read_layout);
        ivTabRead = findViewById(R.id.iv_tab_read);
        tvTabRead = findViewById(R.id.tv_tab_read);
        mTabRead.setOnClickListener(v -> {
            PublicJavaCallCpp("tab_reader");
            finish();
        });

        mTabTodo = findViewById(R.id.tab_todo_layout);
        ivTabTodo = findViewById(R.id.iv_tab_todo);
        tvTabTodo = findViewById(R.id.tv_tab_todo);
        mTabTodo.setOnClickListener(v -> {
            //当前页面，不需要动作
        });

        mTabNote = findViewById(R.id.tab_note_layout);
        ivTabNote = findViewById(R.id.iv_tab_note);
        tvTabNote = findViewById(R.id.tv_tab_note);
        mTabNote.setOnClickListener(v -> {
            PublicJavaCallCpp("tab_notes");
            finish();
        });

        mTabSport = findViewById(R.id.tab_sport_layout);
        ivTabSport = findViewById(R.id.iv_tab_sport);
        tvTabSport = findViewById(R.id.tv_tab_sport);
        mTabSport.setOnClickListener(v -> {
            PublicJavaCallCpp("tab_steps");
            finish();
        });
    }

    private void updateAllTexts() {
        boolean zh = MyActivity.zh_cn;
        if (zh) {
            etTodoInput.setHint("输入待办事项文本");
            tvTabRead.setText("阅读");
            tvTabTodo.setText("待办");
            tvTabNote.setText("笔记");
            tvTabSport.setText("运动");
        } else {
            etTodoInput.setHint("Input todo item");
            tvTabRead.setText("Read");
            tvTabTodo.setText("Todo");
            tvTabNote.setText("Note");
            tvTabSport.setText("Sport");
        }
    }

    private void updateAllColor() {
        int iconColor;
        int textColor;
        int rootBgColor;
        int bottomTabBgColor;
        if (isDark) {
            iconColor = 0xFFFFFFFF;
            textColor = 0xFFFFFFFF;
            rootBgColor = 0xFF121212;
            bottomTabBgColor = 0xFF1E1E1E;
        } else {
            iconColor = 0xFF000000;
            textColor = 0xFF000000;
            rootBgColor = 0xFFF5F5F5;
            bottomTabBgColor = 0xFFFFFFFF;
        }

        layoutTodoRoot.setBackgroundColor(rootBgColor);
        bottomTabLayout.setBackgroundColor(bottomTabBgColor);

        //顶部图标着色
        ivTodoAdd.setColorFilter(iconColor);
        ivTodoClear.setColorFilter(iconColor);

        //底部tab图标
        ivTabRead.setColorFilter(iconColor);
        ivTabTodo.setColorFilter(iconColor);
        ivTabNote.setColorFilter(iconColor);
        ivTabSport.setColorFilter(iconColor);

        //文字颜色
        tvTabRead.setTextColor(textColor);
        tvTabTodo.setTextColor(textColor);
        tvTabNote.setTextColor(textColor);
        tvTabSport.setTextColor(textColor);

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
        /**
         * @param pos 条目下标
         * @param actionCode 动作码：star/copy/edit/alarm/delete/done
         */
        void onAction(int pos, String actionCode);
    }
}
