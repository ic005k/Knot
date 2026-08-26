package com.x;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import org.qtproject.qt.android.QtNative;

/**
 * 原生主入口窗口 MainEntrance
 * 过渡阶段：普通Activity，由Qt/C++通过JNI拉起，不作为App启动入口
 * Java仅负责视图渲染、事件转发；业务逻辑全部在C++
 */
public class MainEntrance extends Activity {

    private RecyclerView mRvMaintabGrid;
    private CategoryGridAdapter mCatAdapter;
    private WeakReference<MainEntrance> mSelfWeakRef;

    // 顶部栏布局容器
    private View mTopBtnMenu;
    private View mTopBtnHome;
    private View mTopBtnAdd;
    private View mTopBtnSearch;
    private View mTopBtnUpload;

    // 顶部图标 ImageView
    private ImageView ivTopMenu;
    private ImageView ivTopHome;
    private ImageView ivTopAdd;
    private ImageView ivTopSearch;
    private ImageView ivTopUpload;

    // 顶部标签文本
    private TextView tvTopMenu;
    private TextView tvTopHome;
    private TextView tvTopAdd;
    private TextView tvTopSearch;
    private TextView tvTopUpload;

    // 底部Tab布局容器
    private View mTabRead;
    private View mTabTodo;
    private View mTabNote;
    private View mTabSport;

    // 底部Tab图标 ImageView
    private ImageView ivTabRead;
    private ImageView ivTabTodo;
    private ImageView ivTabNote;
    private ImageView ivTabSport;

    // 底部Tab标签文本
    private TextView tvTabRead;
    private TextView tvTabTodo;
    private TextView tvTabNote;
    private TextView tvTabSport;

    private View layoutRoot;
    private View bottomTabLayout;

    public static native void PublicJavaCallCpp(String type);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // if (MyActivity.isDark) {
        setContentView(R.layout.activity_main_entrance_dark);
        //} else {
        // setContentView(R.layout.activity_main_entrance);
        //}
        ImmersiveUtil.applyRealImmersive(this);
        mSelfWeakRef = new WeakReference<>(this);

        // 网格初始化
        mRvMaintabGrid = findViewById(R.id.rv_maintab_grid);
        GridLayoutManager gridLayout = new GridLayoutManager(this, 3);
        mRvMaintabGrid.setLayoutManager(gridLayout);
        mCatAdapter = new CategoryGridAdapter();
        mRvMaintabGrid.setAdapter(mCatAdapter);

        // -------- 读取Intent传入maintab_list，交给Adapter渲染卡片 --------
        ArrayList<String> maintabList = getIntent().getStringArrayListExtra(
            "maintab_list"
        );
        if (maintabList == null) {
            maintabList = new ArrayList<>();
        }
        mCatAdapter.setStringListData(maintabList);

        mCatAdapter.setOnItemClickListener(position -> {
            android.util.Log.d("MAIN_ENTRANCE_CLICK", "pos=" + position);
            // 清除全部选中
            for (int i = 0; i < mCatAdapter.getItemCount(); i++) {
                mCatAdapter.setItemSelected(i, false);
            }
            mCatAdapter.setItemSelected(position, true);
            // JNI统一在这里调用，业务集中
            PublicJavaCallCpp("maintab_selected|==|" + position);
            onBackPressed();
        });

        // ---------- 顶部按钮容器绑定 ----------
        mTopBtnMenu = findViewById(R.id.top_btn_menu_layout);
        ivTopMenu = findViewById(R.id.iv_top_menu);
        tvTopMenu = findViewById(R.id.tv_top_menu);
        mTopBtnMenu.setOnClickListener(v -> {
            // TODO
        });

        mTopBtnHome = findViewById(R.id.top_btn_home_layout);
        ivTopHome = findViewById(R.id.iv_top_home);
        tvTopHome = findViewById(R.id.tv_top_home);
        mTopBtnHome.setOnClickListener(v -> {
            // TODO
        });

        mTopBtnAdd = findViewById(R.id.top_btn_add_layout);
        ivTopAdd = findViewById(R.id.iv_top_add);
        tvTopAdd = findViewById(R.id.tv_top_add);
        mTopBtnAdd.setOnClickListener(v -> {
            // TODO
            PublicJavaCallCpp("topbtn_add");
            onBackPressed();
        });

        mTopBtnSearch = findViewById(R.id.top_btn_search_layout);
        ivTopSearch = findViewById(R.id.iv_top_search);
        tvTopSearch = findViewById(R.id.tv_top_search);
        mTopBtnSearch.setOnClickListener(v -> {
            // TODO
        });

        mTopBtnUpload = findViewById(R.id.top_btn_upload_layout);
        ivTopUpload = findViewById(R.id.iv_top_upload);
        tvTopUpload = findViewById(R.id.tv_top_upload);
        mTopBtnUpload.setOnClickListener(v -> {
            // TODO
        });

        // ---------- 底部Tab容器绑定 ----------
        mTabRead = findViewById(R.id.tab_read_layout);
        ivTabRead = findViewById(R.id.iv_tab_read);
        tvTabRead = findViewById(R.id.tv_tab_read);
        mTabRead.setOnClickListener(v -> {
            // TODO
        });

        mTabTodo = findViewById(R.id.tab_todo_layout);
        ivTabTodo = findViewById(R.id.iv_tab_todo);
        tvTabTodo = findViewById(R.id.tv_tab_todo);
        mTabTodo.setOnClickListener(v -> {
            // TODO
        });

        mTabNote = findViewById(R.id.tab_note_layout);
        ivTabNote = findViewById(R.id.iv_tab_note);
        tvTabNote = findViewById(R.id.tv_tab_note);
        mTabNote.setOnClickListener(v -> {
            // TODO
        });

        mTabSport = findViewById(R.id.tab_sport_layout);
        ivTabSport = findViewById(R.id.iv_tab_sport);
        tvTabSport = findViewById(R.id.tv_tab_sport);
        mTabSport.setOnClickListener(v -> {
            // TODO
        });

        layoutRoot = findViewById(R.id.layout_root);
        bottomTabLayout = findViewById(R.id.bottom_tab_layout);

        // 初始化全部UI：文字+图标颜色
        refreshUi();
    }

    /**
     * 更新所有文本内容，根据全局 MyActivity.zh_cn 切换中英文
     */
    private void updateAllTexts() {
        if (MyActivity.zh_cn) {
            // 中文
            tvTopMenu.setText("菜单");
            tvTopHome.setText("主页");
            tvTopAdd.setText("增加");
            tvTopSearch.setText("查找");
            tvTopUpload.setText("上传");

            tvTabRead.setText("阅读");
            tvTabTodo.setText("待办");
            tvTabNote.setText("笔记");
            tvTabSport.setText("运动");
        } else {
            // English
            tvTopMenu.setText("Menu");
            tvTopHome.setText("Home");
            tvTopAdd.setText("Add");
            tvTopSearch.setText("Search");
            tvTopUpload.setText("Upload");

            tvTabRead.setText("Read");
            tvTabTodo.setText("Todo");
            tvTabNote.setText("Note");
            tvTabSport.setText("Sport");
        }
    }

    /**
     * 更新全部图标着色、所有文字颜色，跟随全局明暗主题 MyActivity.isDark
     */
    /**
     * 更新全部图标着色、所有文字颜色，跟随全局明暗主题 MyActivity.isDark
     */
    private void updateAllColor() {
        int iconColor;
        int textColor;
        int rootBgColor;
        int bottomTabBgColor;

        if (MyActivity.isDark) {
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

        // 布局背景
        layoutRoot.setBackgroundColor(rootBgColor);
        bottomTabLayout.setBackgroundColor(bottomTabBgColor);

        // 顶部图标
        ivTopMenu.setColorFilter(iconColor);
        ivTopHome.setColorFilter(iconColor);
        ivTopAdd.setColorFilter(iconColor);
        ivTopSearch.setColorFilter(iconColor);
        ivTopUpload.setColorFilter(iconColor);

        // 底部图标
        ivTabRead.setColorFilter(iconColor);
        ivTabTodo.setColorFilter(iconColor);
        ivTabNote.setColorFilter(iconColor);
        ivTabSport.setColorFilter(iconColor);

        // 顶部文字颜色
        tvTopMenu.setTextColor(textColor);
        tvTopHome.setTextColor(textColor);
        tvTopAdd.setTextColor(textColor);
        tvTopSearch.setTextColor(textColor);
        tvTopUpload.setTextColor(textColor);

        // 底部Tab文字颜色
        tvTabRead.setTextColor(textColor);
        tvTabTodo.setTextColor(textColor);
        tvTabNote.setTextColor(textColor);
        tvTabSport.setTextColor(textColor);
    }

    /**
     * 统一UI刷新入口：刷新多语言文本 + 明暗颜色
     */
    private void refreshUi() {
        updateAllTexts();
        updateAllColor();
    }

    /**
     * singleTop模式：桌面图标回到应用、后台切回时触发
     */
    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        setIntent(intent);
        ArrayList<String> newTabList = intent.getStringArrayListExtra(
            "maintab_list"
        );
        if (newTabList != null) {
            runOnUiThread(() -> mCatAdapter.setStringListData(newTabList));
        }
        runOnUiThread(this::refreshUi);
    }

    /**
     * JNI接口：供C++调用，传入QStringList序列化JSON字符串，刷新网格
     */
    @SuppressWarnings("unused")
    public void refreshMaintabGrid(String jsonArray) {
        MainEntrance act = mSelfWeakRef.get();
        if (act == null || act.isFinishing() || act.isDestroyed()) {
            return;
        }
        act.runOnUiThread(() -> {
            // TODO: parseAndSetData 业务自行实现
        });
    }

    /**
     * JNI暴露接口：C++切换明暗模式后调用
     */
    @SuppressWarnings("unused")
    public void notifyThemeChanged() {
        MainEntrance act = mSelfWeakRef.get();
        if (act == null || act.isFinishing() || act.isDestroyed()) {
            return;
        }
        act.runOnUiThread(act::refreshUi);
    }

    /**
     * JNI暴露接口：C++切换语言后调用，刷新中英文标签
     */
    @SuppressWarnings("unused")
    public void notifyLanguageChanged() {
        MainEntrance act = mSelfWeakRef.get();
        if (act == null || act.isFinishing() || act.isDestroyed()) {
            return;
        }
        act.runOnUiThread(act::refreshUi);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mSelfWeakRef.clear();
    }
}
