package com.x;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListPopupWindow;
import android.widget.TextView;
import android.widget.TextView;
import androidx.activity.OnBackPressedCallback;
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

    private OnBackPressedCallback mBackCallback;

    // 静态实例引用
    public static MainEntrance mInstance = null;

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

    /** 菜单项：携带原始ID和显示文本 */
    private static class MenuItem {

        final int id;
        final String label;

        MenuItem(int id, String label) {
            this.id = id;
            this.label = label;
        }

        @NonNull
        @Override
        public String toString() {
            return label; // ArrayAdapter 调用 toString() 显示文本
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mInstance = this;

        // 注册返回拦截回调
        mBackCallback = new OnBackPressedCallback(true /* enabled */) {
            @Override
            public void handleOnBackPressed() {
                MyActivity.m_instance.moveTaskToBack(true);
            }
        };
        getOnBackPressedDispatcher().addCallback(this, mBackCallback);

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
            finish();
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
        // 隐藏主页按钮（保留引用，将来恢复只需 setVisibility(VISIBLE)）
        mTopBtnHome.setVisibility(View.GONE);

        // 此时剩下4个按钮，每个weight=1，总有效权重=4，自动均分整行，布局均匀。

        mTopBtnAdd = findViewById(R.id.top_btn_add_layout);
        ivTopAdd = findViewById(R.id.iv_top_add);
        tvTopAdd = findViewById(R.id.tv_top_add);
        mTopBtnAdd.setOnClickListener(v -> {
            // TODO
            PublicJavaCallCpp("topbtn_add");
            finish();
        });

        mTopBtnSearch = findViewById(R.id.top_btn_search_layout);
        ivTopSearch = findViewById(R.id.iv_top_search);
        tvTopSearch = findViewById(R.id.tv_top_search);
        mTopBtnSearch.setOnClickListener(v -> {
            PublicJavaCallCpp("topbtn_search");
            finish();
        });

        mTopBtnUpload = findViewById(R.id.top_btn_upload_layout);
        ivTopUpload = findViewById(R.id.iv_top_upload);
        tvTopUpload = findViewById(R.id.tv_top_upload);
        mTopBtnUpload.setOnClickListener(v -> {
            PublicJavaCallCpp("topbtn_upload");
            finish();
        });

        // ---------- 底部Tab容器绑定 ----------
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
            PublicJavaCallCpp("tab_todo");
            finish();
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
        if (mBackCallback != null) {
            mBackCallback.remove();
            mBackCallback = null;
        }

        mSelfWeakRef.clear();
        mInstance = null; // 清空，防止内存泄漏

        PublicJavaCallCpp("mainentrance_destroy");
    }

    /**
     * 弹出Android原生下拉菜单 ListPopupWindow
     * @param anchorView 锚点：顶部菜单按钮
     */
    private void showNativeMainMenu(View anchorView) {
        ListPopupWindow popupWindow = new ListPopupWindow(this);
        ArrayList<MenuItem> menuItems = buildMenuItems();

        ArrayAdapter<MenuItem> adapter = new ArrayAdapter<>(
            this,
            android.R.layout.simple_list_item_1,
            menuItems
        );
        popupWindow.setAdapter(adapter);
        popupWindow.setAnchorView(anchorView);
        popupWindow.setModal(true);

        // ========== 计算文本宽度 ==========
        android.graphics.Paint paint = new android.graphics.Paint();
        float textSizeSp = 16f;
        float density = getResources().getDisplayMetrics().scaledDensity;
        paint.setTextSize(textSizeSp * density);

        int maxWidth = 0;
        for (MenuItem item : menuItems) {
            int w = (int) paint.measureText(item.label);
            if (w > maxWidth) maxWidth = w;
        }
        int contentWidth = Math.max(maxWidth + dp2px(32), dp2px(220));

        popupWindow.setContentWidth(contentWidth);
        popupWindow.setWidth(contentWidth);
        popupWindow.setHorizontalOffset(dp2px(-8));

        // ✅ 通过 MenuItem.id 分发，不依赖 position
        popupWindow.setOnItemClickListener((parent, view, position, id) -> {
            MenuItem clicked = menuItems.get(position);
            handleMenuItemClick(clicked.id);
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
     * 构建菜单列表
     * 当前已隐藏：新建/删除/重命名标签页（取消注释即可恢复）
     */
    private ArrayList<MenuItem> buildMenuItems() {
        boolean zh = MyActivity.zh_cn;
        ArrayList<MenuItem> items = new ArrayList<>();

        // -------- 以下三项暂时隐藏，将来启用时取消注释即可 --------
        // items.add(new MenuItem(MENU_ID_ADD_TAB,    zh ? "增加标签页"       : "Add Tab"));
        // items.add(new MenuItem(MENU_ID_DELETE_TAB, zh ? "删除标签页"       : "Delete Tab"));
        // items.add(new MenuItem(MENU_ID_RENAME_TAB, zh ? "重命名标签页"     : "Rename Tab"));

        items.add(
            new MenuItem(MENU_ID_EXPORT_DATA, zh ? "导出数据" : "Export Data")
        );
        items.add(
            new MenuItem(MENU_ID_IMPORT_DATA, zh ? "导入数据" : "Import Data")
        );
        items.add(
            new MenuItem(MENU_ID_PREFERENCE, zh ? "偏好设置" : "Preferences")
        );
        items.add(
            new MenuItem(
                MENU_ID_CLOUD_BACKUP_RESTORE,
                zh ? "云备份与恢复数据" : "Cloud Backup & Restore"
            )
        );
        items.add(
            new MenuItem(
                MENU_ID_BACKUP_FILE_LIST,
                zh ? "备份文件列表" : "Backup File List"
            )
        );
        items.add(
            new MenuItem(
                MENU_ID_TAB_RECYCLE_BIN,
                zh ? "标签页回收箱" : "Tab Recycle Bin"
            )
        );
        items.add(new MenuItem(MENU_ID_ABOUT, zh ? "关于" : "About"));

        return items;
    }

    /**
     * 原生菜单项点击分发，全部留空占位，纯Java，不调用C++
     * @param pos 菜单条目索引
     */
    private void handleMenuItemClick(int pos) {
        switch (pos) {
            case MENU_ID_ADD_TAB:
                // 增加标签页
                //PublicJavaCallCpp("menu_id_add_tab");
                showAddTabDialog();

                break;
            case MENU_ID_DELETE_TAB:
                // 删除标签页
                int selIdxDel = MyActivity.mainTabLastSelectedPos;
                if (mCatAdapter != null) {
                    String rawItem = mCatAdapter.getRawItemAt(selIdxDel);
                    String displayTitle = mCatAdapter.getDisplayTitleAt(
                        selIdxDel
                    );
                    if (rawItem != null && !displayTitle.isEmpty()) {
                        showDeleteTabConfirmDialog(rawItem, displayTitle);
                    }
                }

                break;
            case MENU_ID_RENAME_TAB:
                // 重命名标签页
                int selIdx = MyActivity.mainTabLastSelectedPos;
                if (mCatAdapter != null) {
                    String rawItem = mCatAdapter.getRawItemAt(selIdx);
                    String displayTitle = mCatAdapter.getDisplayTitleAt(selIdx);
                    if (rawItem != null && !displayTitle.isEmpty()) {
                        showRenameTabDialog(rawItem, displayTitle);
                    }
                }

                break;
            case MENU_ID_EXPORT_DATA:
                // 导出数据
                PublicJavaCallCpp("menu_id_export_data");
                finish();
                break;
            case MENU_ID_IMPORT_DATA:
                // 导入数据
                PublicJavaCallCpp("menu_id_import_data");
                finish();
                break;
            case MENU_ID_PREFERENCE:
                // 偏好设置
                PublicJavaCallCpp("menu_id_preference");
                finish();
                break;
            case MENU_ID_CLOUD_BACKUP_RESTORE:
                // 云备份与恢复数据
                PublicJavaCallCpp("menu_id_cloud_backup_restore");
                finish();
                break;
            case MENU_ID_BACKUP_FILE_LIST:
                // 备份文件列表
                PublicJavaCallCpp("menu_id_backup_file_list");
                finish();
                break;
            case MENU_ID_TAB_RECYCLE_BIN:
                // 标签页回收箱
                PublicJavaCallCpp("menu_id_tab_recycle_bin");
                finish();
                break;
            case MENU_ID_ABOUT:
                // 关于
                PublicJavaCallCpp("menu_id_about");
                finish();
                break;
            default:
                break;
        }
    }

    @Override
    public void onBackPressed() {
        // 拦截，不调用super.onBackPressed()
    }

    //菜单对话框

    /**
     * 对话框1：新建标签页
     */
    public void showAddTabDialog() {
        runOnUiThread(() -> {
            AlertDialog.Builder builder = new AlertDialog.Builder(
                MainEntrance.this
            );
            LinearLayout container = new LinearLayout(MainEntrance.this);
            container.setOrientation(LinearLayout.VERTICAL);
            int dp16 = dp2px(16);
            container.setPadding(dp16, dp16, dp16, dp16);

            TextView tvHint = new TextView(MainEntrance.this);
            if (MyActivity.zh_cn) {
                tvHint.setText("请输入标签页名称:");
            } else {
                tvHint.setText("Please input tab name:");
            }
            tvHint.setTextSize(18);
            container.addView(tvHint);

            EditText etInput = new EditText(MainEntrance.this);
            if (MyActivity.zh_cn) {
                etInput.setText("标签页");
            } else {
                etInput.setText("Tab");
            }
            etInput.selectAll(); // 默认全选文本，和截图行为一致
            LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            );
            lp.topMargin = dp2px(12);
            etInput.setLayoutParams(lp);
            container.addView(etInput);

            builder.setView(container);

            // 按钮文本中英文
            String textOk = MyActivity.zh_cn ? "完成" : "Done";
            String textCancel = MyActivity.zh_cn ? "取消" : "Cancel";

            builder.setPositiveButton(textOk, (dialog, which) -> {
                String name = etInput.getText().toString().trim();
                // ==========【确定事件，留空，你自己写业务】==========
                // PublicJavaCallCpp("add_tab_confirm|==|" + name);
            });
            builder.setNegativeButton(textCancel, (dialog, which) -> {
                dialog.dismiss();
            });

            AlertDialog dlg = builder.create();
            dlg.show();
        });
    }

    /**
     * 对话框2：重命名标签页
     * @param rawItem 完整原始记录 title|==|flag
     * @param displayName UI展示用旧名称
     */
    public void showRenameTabDialog(
        final String rawItem,
        final String displayName
    ) {
        runOnUiThread(() -> {
            AlertDialog.Builder builder = new AlertDialog.Builder(
                MainEntrance.this
            );
            LinearLayout container = new LinearLayout(MainEntrance.this);
            container.setOrientation(LinearLayout.VERTICAL);
            int dp16 = dp2px(16);
            container.setPadding(dp16, dp16, dp16, dp16);
            TextView tvHint = new TextView(MainEntrance.this);
            if (MyActivity.zh_cn) {
                tvHint.setText("标签页名称:");
            } else {
                tvHint.setText("Tab name:");
            }
            tvHint.setTextSize(18);
            container.addView(tvHint);
            EditText etInput = new EditText(MainEntrance.this);
            etInput.setText(displayName);
            etInput.selectAll();
            LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            );
            lp.topMargin = dp2px(12);
            etInput.setLayoutParams(lp);
            container.addView(etInput);
            builder.setView(container);
            String textOk = MyActivity.zh_cn ? "确定" : "OK";
            String textCancel = MyActivity.zh_cn ? "取消" : "Cancel";
            builder.setPositiveButton(textOk, (dialog, which) -> {
                String newName = etInput.getText().toString().trim();
                // TODO：业务，可使用 rawItem + newName
                // PublicJavaCallCpp("rename_tab_confirm|==|" + rawItem + "|==|" + newName);
            });
            builder.setNegativeButton(textCancel, (dialog, which) -> {
                dialog.dismiss();
            });
            AlertDialog dlg = builder.create();
            dlg.show();
        });
    }

    /**
     * 对话框3：删除标签页确认弹窗
     * @param rawItem 完整原始记录 title|==|flag
     * @param displayName UI展示名称
     */
    public void showDeleteTabConfirmDialog(
        final String rawItem,
        final String displayName
    ) {
        runOnUiThread(() -> {
            AlertDialog.Builder builder = new AlertDialog.Builder(
                MainEntrance.this
            );
            String msg;
            if (MyActivity.zh_cn) {
                msg = "是否删除 " + displayName + " ?";
            } else {
                msg = "Delete " + displayName + " ?";
            }
            builder.setTitle("Knot");
            builder.setMessage(msg);
            String textOk = MyActivity.zh_cn ? "确定" : "OK";
            String textCancel = MyActivity.zh_cn ? "取消" : "Cancel";
            builder.setPositiveButton(textOk, (dialog, which) -> {
                // TODO：业务，使用 rawItem
                // PublicJavaCallCpp("delete_tab_confirm|==|" + rawItem);
            });
            builder.setNegativeButton(textCancel, (dialog, which) -> {
                dialog.dismiss();
            });
            AlertDialog dlg = builder.create();
            dlg.show();
        });
    }
}
