package com.xhh.pdfui;

import com.x.MyActivity;
import com.x.MyService;
import com.x.R;

import android.app.Activity;
import android.content.Intent;

import android.os.Bundle;
import android.view.WindowManager;
import android.view.Window;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.xhh.pdfui.tree.TreeAdapter;
import com.xhh.pdfui.tree.TreeNodeData;

import java.util.List;

import android.text.TextUtils;
import android.content.IntentFilter;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.content.Context;

/**
 * UI页面：PDF目录
 * <p>
 * 1、用于显示Pdf目录信息
 * 2、点击tree item，带回Pdf页码到前一个页面
 * <p>
 * 作者：齐行超
 * 日期：2019.08.07
 */

@SuppressWarnings("unchecked")

public class PDFCatelogueActivity extends AppCompatActivity implements TreeAdapter.TreeEvent {

    RecyclerView recyclerView;
    Button btn_back;
    TextView lblTitle;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        setTheme(R.style.AppThemeprice);
        super.onCreate(savedInstanceState);
        // UIUtils.initWindowStyle(getWindow(), getSupportActionBar());
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.activity_catelogue);

        initView();// 初始化控件
        setEvent();// 设置事件
        loadData();// 加载数据

        // HomeKey
        registerReceiver(mHomeKeyEvent, new IntentFilter(Intent.ACTION_CLOSE_SYSTEM_DIALOGS));
    }

    private BroadcastReceiver mHomeKeyEvent = new BroadcastReceiver() {
        String SYSTEM_REASON = "reason";
        String SYSTEM_HOME_KEY = "homekey";
        String SYSTEM_HOME_KEY_LONG = "recentapps";

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_CLOSE_SYSTEM_DIALOGS)) {
                String reason = intent.getStringExtra(SYSTEM_REASON);
                if (TextUtils.equals(reason, SYSTEM_HOME_KEY)) {
                    // 表示按了home键,程序直接进入到后台
                    System.out.println("NoteEditor HOME键被按下...");

                } else if (TextUtils.equals(reason, SYSTEM_HOME_KEY_LONG)) {
                    // 表示长按home键,显示最近使用的程序
                    System.out.println("NoteEditor 长按HOME键...");

                }
            }
        }
    };

    /**
     * 初始化控件
     */
    private void initView() {
        btn_back = findViewById(R.id.btn_back);
        recyclerView = findViewById(R.id.rv_tree);

        lblTitle = findViewById(R.id.lblTitle);
        if (MyService.zh_cn)
            lblTitle.setText("目录");
        else
            lblTitle.setText("Catalogue");
    }

    /**
     * 设置事件
     */
    private void setEvent() {
        btn_back.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                PDFCatelogueActivity.this.finish();
            }
        });
    }

    /**
     * 加载数据
     */
    private void loadData() {
        // 从intent中获得传递的数据
        Intent intent = getIntent();
        List<TreeNodeData> catelogues = (List<TreeNodeData>) intent.getSerializableExtra("catelogues");

        // 使用RecyclerView加载数据
        LinearLayoutManager llm = new LinearLayoutManager(this);
        llm.setOrientation(LinearLayoutManager.VERTICAL);
        recyclerView.setLayoutManager(llm);
        TreeAdapter adapter = new TreeAdapter(this, catelogues);
        adapter.setTreeEvent(this);
        recyclerView.setAdapter(adapter);
    }

    /**
     * 点击tree item，带回Pdf页码到前一个页面
     *
     * @param data tree节点数据
     */
    @Override
    public void onSelectTreeNode(TreeNodeData data) {
        Intent intent = new Intent();
        intent.putExtra("pageNum", data.getPageNum());
        setResult(Activity.RESULT_OK, intent);
        finish();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mHomeKeyEvent);

    }
}
