package com.x;

import android.app.Activity;
import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;

public class TodoRecycleActivity extends Activity {

    public static native void PublicJavaCallCpp(String type);

    private RecyclerView mRecyclerView;
    private TodoRecycleAdapter mAdapter;
    private TextView mTvTitle;
    private Button mBtnClearAll;
    private Button mBtnDelete;
    private Button mBtnRestore;

    // Activity本地持有回收站数据源
    private ArrayList<String> mRecycleData = new ArrayList<>();
    private boolean mIsDark = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_todo_recycle);
        mIsDark = ImmersiveUtil.applyRealImmersive(this);

        mRecyclerView = findViewById(R.id.recycler_view);
        mTvTitle = findViewById(R.id.tv_title);
        mBtnClearAll = findViewById(R.id.btn_clear_all);
        mBtnDelete = findViewById(R.id.btn_delete);
        mBtnRestore = findViewById(R.id.btn_restore);

        mRecyclerView.setLayoutManager(new LinearLayoutManager(this));
        mAdapter = new TodoRecycleAdapter();
        mRecyclerView.setAdapter(mAdapter);
        mAdapter.setDarkMode(mIsDark);

        updateButtonLanguage();
        applyWindowDarkStyle();

        // 读取Intent初始化本地数据源
        ArrayList<String> initList = getIntent().getStringArrayListExtra(
            "todorecycle_list"
        );
        if (initList != null) {
            mRecycleData.clear();
            mRecycleData.addAll(initList);
            mAdapter.setData(mRecycleData);
        }

        // ========== 按钮事件：全部业务逻辑在Java ==========
        // 清除所有
        mBtnClearAll.setOnClickListener(v -> {
            mRecycleData.clear();
            mAdapter.setData(mRecycleData);
        });

        // 删除选中卡片
        mBtnDelete.setOnClickListener(v -> {
            int sel = mAdapter.mSelectedPos;
            if (sel >= 0 && sel < mRecycleData.size()) {
                mRecycleData.remove(sel);
                mAdapter.setData(mRecycleData);
            }
        });

        // 恢复选中：提取todo文本抛给C++，本地删掉这条
        mBtnRestore.setOnClickListener(v -> {
            int sel = mAdapter.mSelectedPos;
            if (sel < 0 || sel >= mRecycleData.size()) return;

            String rawLine = mRecycleData.get(sel);
            String[] parts = rawLine.split("\\|==\\|");
            String todoText = "";
            if (parts.length >= 2) {
                todoText = parts[1];
            }
            if (todoText.isEmpty()) return;

            // Java本地移除这条
            mRecycleData.remove(sel);
            mAdapter.setData(mRecycleData);

            PublicJavaCallCpp("todo_recycle_restore|==|" + todoText);
        });
    }

    private void applyWindowDarkStyle() {
        if (mIsDark) {
            getWindow()
                .getDecorView()
                .findViewById(android.R.id.content)
                .setBackgroundColor(0xFF1E1E1E);
            mTvTitle.setTextColor(0xFFFFFFFF);
        } else {
            getWindow()
                .getDecorView()
                .findViewById(android.R.id.content)
                .setBackgroundColor(0xFFFFFFFF);
            mTvTitle.setTextColor(0xFF000000);
        }
    }

    public void updateButtonLanguage() {
        if (MyActivity.zh_cn) {
            mTvTitle.setText("待办事项回收站:");
            mBtnClearAll.setText("清除");
            mBtnDelete.setText("删除");
            mBtnRestore.setText("恢复");
        } else {
            mTvTitle.setText("Todo Recycle Bin:");
            mBtnClearAll.setText("Clear");
            mBtnDelete.setText("Delete");
            mBtnRestore.setText("Restore");
        }
    }

    public void setRecycleData(ArrayList<String> list) {
        mRecycleData.clear();
        mRecycleData.addAll(list);
        mAdapter.setData(mRecycleData);
    }

    public void setDarkMode(boolean dark) {
        mIsDark = dark;
        mAdapter.setDarkMode(dark);
        applyWindowDarkStyle();
    }

    public void clearSelect() {
        mAdapter.clearSelection();
    }
}
