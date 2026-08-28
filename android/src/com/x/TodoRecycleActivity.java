package com.x;

import android.app.Activity;
import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;

public class TodoRecycleActivity extends Activity {

    public interface OnTodoRecycleActionListener {
        void onRecycleClearAll();
        void onRecycleDeleteItem(int index);
        void onRecycleRestoreItem(int index);
    }

    private RecyclerView mRecyclerView;
    private TodoRecycleAdapter mAdapter;
    private TextView mTvTitle;
    private Button mBtnClearAll;
    private Button mBtnDelete;
    private Button mBtnRestore;
    public OnTodoRecycleActionListener mListener;
    private static boolean isDark = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_todo_recycle);
        isDark = ImmersiveUtil.applyRealImmersive(this);

        mRecyclerView = findViewById(R.id.recycler_view);
        mTvTitle = findViewById(R.id.tv_title);
        mBtnClearAll = findViewById(R.id.btn_clear_all);
        mBtnDelete = findViewById(R.id.btn_delete);
        mBtnRestore = findViewById(R.id.btn_restore);

        mRecyclerView.setLayoutManager(new LinearLayoutManager(this));
        mAdapter = new TodoRecycleAdapter();
        mRecyclerView.setAdapter(mAdapter);

        // 初始化标题+按钮中英文
        updateButtonLanguage();

        // ========== 读取Intent传入的回收站列表 ==========
        ArrayList<String> initList = getIntent().getStringArrayListExtra(
            "todorecycle_list"
        );
        if (initList != null) {
            mAdapter.setData(initList);
        }

        mBtnClearAll.setOnClickListener(v -> {
            if (mListener != null) {
                mListener.onRecycleClearAll();
            }
        });

        mBtnDelete.setOnClickListener(v -> {
            int sel = mAdapter.mSelectedPos;
            if (sel != -1 && mListener != null) {
                mListener.onRecycleDeleteItem(sel);
            }
        });

        mBtnRestore.setOnClickListener(v -> {
            int sel = mAdapter.mSelectedPos;
            if (sel != -1 && mListener != null) {
                mListener.onRecycleRestoreItem(sel);
            }
        });
    }

    /**
     * 根据全局 MyActivity.zh_cn 变量更新标题以及三个按钮文本
     */
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
        mAdapter.setData(list);
    }

    public void setDarkMode(boolean dark) {
        mAdapter.setDarkMode(dark);
    }

    public void setActionListener(OnTodoRecycleActionListener listener) {
        mListener = listener;
    }

    public void clearSelect() {
        mAdapter.clearSelection();
    }
}
