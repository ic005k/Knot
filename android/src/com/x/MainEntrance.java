package com.x;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListPopupWindow;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import java.lang.ref.WeakReference;
import java.util.ArrayList;

/**
 * 原生主入口窗口 MainEntrance
 * Java仅负责视图渲染、事件转发；业务逻辑全部在C++
 */
public class MainEntrance extends AppCompatActivity {

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

    // 原生菜单命令常量，仅Java内部使用
    public static final int MENU_ID_ADD_TAB = 0;
    public static final int MENU_ID_DELETE_TAB = 1;
    public static final int MENU_ID_RENAME_TAB = 2;
    public static final int MENU_ID_EXPORT_DATA = 3;
    public static final int MENU_ID_IMPORT_DATA = 4;
    public static final int MENU_ID_PREFERENCE = 5;
    public static final int MENU_ID_CLOUD_BACKUP_RESTORE = 6;
    public static final int MENU_ID_BACKUP_FILE_LIST = 7;
    public static final int MENU_ID_TAB_RECYCLE_BIN = 8;
    public static final int MENU_ID_ABOUT = 9;

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

        // ✅从全局MyActivity恢复上次选中状态
        int selPos = MyActivity.mainTabLastSelectedPos;
        if (selPos >= 0 && selPos < maintabList.size()) {
            mCatAdapter.setItemSelected(selPos, true);
        }

        mCatAdapter.setOnItemClickListener(position -> {
            android.util.Log.d("MAIN_ENTRANCE_CLICK", "pos=" + position);
            // 写入全局状态
            MyActivity.mainTabLastSelectedPos = position;
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
            showNativeMainMenu(v);
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
            PublicJavaCallCpp("topbtn_search");
            onBackPressed();
        });

        mTopBtnUpload = findViewById(R.id.top_btn_upload_layout);
        ivTopUpload = findViewById(R.id.iv_top_upload);
        tvTopUpload = findViewById(R.id.tv_top_upload);
        mTopBtnUpload.setOnClickListener(v -> {
            PublicJavaCallCpp("topbtn_upload");
            onBackPressed();
        });

        // ---------- 底部Tab容器绑定 ----------
        mTabRead = findViewById(R.id.tab_read_layout);
        ivTabRead = findViewById(R.id.iv_tab_read);
        tvTabRead = findViewById(R.id.tv_tab_read);
        mTabRead.setOnClickListener(v -> {
            PublicJavaCallCpp("tab_reader");
            onBackPressed();
        });

        mTabTodo = findViewById(R.id.tab_todo_layout);
        ivTabTodo = findViewById(R.id.iv_tab_todo);
        tvTabTodo = findViewById(R.id.tv_tab_todo);
        mTabTodo.setOnClickListener(v -> {
            PublicJavaCallCpp("tab_todo");
            onBackPressed();
        });

        mTabNote = findViewById(R.id.tab_note_layout);
        ivTabNote = findViewById(R.id.iv_tab_note);
        tvTabNote = findViewById(R.id.tv_tab_note);
        mTabNote.setOnClickListener(v -> {
            PublicJavaCallCpp("tab_notes");
            onBackPressed();
        });

        mTabSport = findViewById(R.id.tab_sport_layout);
        ivTabSport = findViewById(R.id.iv_tab_sport);
        tvTabSport = findViewById(R.id.tv_tab_sport);
        mTabSport.setOnClickListener(v -> {
            PublicJavaCallCpp("tab_steps");
            onBackPressed();
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

        // 同步暗黑模式到适配器
        mCatAdapter.setDarkMode(MyActivity.isDark);
    }

    /**
     * singleTop模式：桌面图标回到应用、后台切回时触发
     */
    /*@Override
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
    }*/

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mSelfWeakRef.clear();

        PublicJavaCallCpp("mainentrance_destroy");
    }

    /**
     * 弹出Android原生下拉菜单 ListPopupWindow
     * @param anchorView 锚点：顶部菜单按钮
     */
    private void showNativeMainMenu(View anchorView) {
        ListPopupWindow popupWindow = new ListPopupWindow(this);
        String[] menuItems = getMainMenuItems();
        ArrayAdapter<String> adapter = new ArrayAdapter<>(
            this,
            android.R.layout.simple_list_item_1,
            menuItems
        );
        popupWindow.setAdapter(adapter);
        popupWindow.setAnchorView(anchorView);
        popupWindow.setModal(true);

        // ========== 解决文字被截断：计算文本实际宽度 ==========
        android.graphics.Paint paint = new android.graphics.Paint();
        // 直接用16sp，不再读取不存在的系统dimen
        float textSizeSp = 16f;
        float density = getResources().getDisplayMetrics().scaledDensity;
        paint.setTextSize(textSizeSp * density);

        int maxWidth = 0;
        for (String text : menuItems) {
            int w = (int) paint.measureText(text);
            if (w > maxWidth) {
                maxWidth = w;
            }
        }
        // 增加边距padding，避免文字贴边，最小宽度保护
        int contentWidth = maxWidth + dp2px(32);
        int minWidth = dp2px(220);
        contentWidth = Math.max(contentWidth, minWidth);

        popupWindow.setContentWidth(contentWidth);
        popupWindow.setWidth(contentWidth);
        // 向左偏移，防止菜单被屏幕右边界截断
        popupWindow.setHorizontalOffset(dp2px(-8));

        popupWindow.setOnItemClickListener((parent, view, position, id) -> {
            handleMenuItemClick(position);
            popupWindow.dismiss();
        });
        popupWindow.show();
    }

    /** dp转px工具 */
    private int dp2px(int dpValue) {
        final float scale = getResources().getDisplayMetrics().density;
        return (int) (dpValue * scale + 0.5f);
    }

    /**
     * 获取菜单文本数组，跟随全局语言切换中英文
     */
    private String[] getMainMenuItems() {
        if (MyActivity.zh_cn) {
            return new String[] {
                "增加标签页",
                "删除标签页",
                "重命名标签页",
                "导出数据",
                "导入数据",
                "偏好设置",
                "云备份与恢复数据",
                "备份文件列表",
                "标签页回收箱",
                "关于",
            };
        } else {
            return new String[] {
                "Add Tab",
                "Delete Tab",
                "Rename Tab",
                "Export Data",
                "Import Data",
                "Preferences",
                "Cloud Backup & Restore",
                "Backup File List",
                "Tab Recycle Bin",
                "About",
            };
        }
    }

    /**
     * 原生菜单项点击分发，全部留空占位，纯Java，不调用C++
     * @param pos 菜单条目索引
     */
    private void handleMenuItemClick(int pos) {
        switch (pos) {
            case MENU_ID_ADD_TAB:
                // 增加标签页
                PublicJavaCallCpp("menu_id_add_tab");
                onBackPressed();
                break;
            case MENU_ID_DELETE_TAB:
                // 删除标签页
                PublicJavaCallCpp("menu_id_delete_tab");
                onBackPressed();
                break;
            case MENU_ID_RENAME_TAB:
                // 重命名标签页
                PublicJavaCallCpp("menu_id_rename_tab");
                onBackPressed();
                break;
            case MENU_ID_EXPORT_DATA:
                // 导出数据
                PublicJavaCallCpp("menu_id_export_data");
                onBackPressed();
                break;
            case MENU_ID_IMPORT_DATA:
                // 导入数据
                PublicJavaCallCpp("menu_id_import_data");
                onBackPressed();
                break;
            case MENU_ID_PREFERENCE:
                // 偏好设置
                PublicJavaCallCpp("menu_id_preference");
                onBackPressed();
                break;
            case MENU_ID_CLOUD_BACKUP_RESTORE:
                // 云备份与恢复数据
                PublicJavaCallCpp("menu_id_cloud_backup_restore");
                onBackPressed();
                break;
            case MENU_ID_BACKUP_FILE_LIST:
                // 备份文件列表
                PublicJavaCallCpp("menu_id_backup_file_list");
                onBackPressed();
                break;
            case MENU_ID_TAB_RECYCLE_BIN:
                // 标签页回收箱
                PublicJavaCallCpp("menu_id_tab_recycle_bin");
                onBackPressed();
                break;
            case MENU_ID_ABOUT:
                // 关于
                PublicJavaCallCpp("menu_id_about");
                onBackPressed();
                break;
            default:
                break;
        }
    }
}
