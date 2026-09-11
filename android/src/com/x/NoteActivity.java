package com.x;

import android.app.Activity;
import android.app.AlertDialog;
import android.os.Bundle;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.PopupMenu;
import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;

public class NoteActivity extends AppCompatActivity {

    public static NoteActivity mInstance = null;

    private androidx.appcompat.app.AlertDialog mAiLoadingDialog = null;

    private boolean mIsDark = false;
    //顶部按钮
    private ImageView mBtnBookMenu;
    private ImageView mBtnFavorite;
    private ImageView mBtnNoteMenu;
    private ImageView mBtnNewNote;
    //列表
    private RecyclerView mRvBookList;
    private RecyclerView mRvNoteList;
    //底部按钮
    private ImageView mBtnSearch;
    private ImageView mBtnView;
    private ImageView mBtnEdit;
    private ImageView mBtnRecycle;
    private NoteBookAdapter mBookAdapter;
    private NoteEntryAdapter mNoteAdapter;

    public static native void PublicJavaCallCpp(String type);

    private OnBackPressedCallback mBackCallback;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mInstance = this;

        mBackCallback = new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                PublicJavaCallCpp("back_note");
                finish();
            }
        };
        getOnBackPressedDispatcher().addCallback(this, mBackCallback);

        setContentView(R.layout.activity_note);
        mIsDark = ImmersiveUtil.applyRealImmersive(this);
        bindView();
        initRecyclerView();

        // ===== 读取传入的笔记本列表，窗口创建自动加载左侧列表 =====
        ArrayList<String> notebookList = getIntent().getStringArrayListExtra(
            "notebook_list"
        );
        if (notebookList != null) {
            //setNoteBookList(notebookList);
            setNoteEntryList(notebookList);
        }

        setupClickListeners();
        applyUiTheme();

        // 隐藏笔记本菜单按钮
        mBtnBookMenu.setVisibility(View.GONE);
    }

    private void bindView() {
        mBtnBookMenu = findViewById(R.id.note_btn_book_menu);
        mBtnFavorite = findViewById(R.id.note_btn_favorite);
        mBtnNoteMenu = findViewById(R.id.note_btn_note_menu);
        mBtnNewNote = findViewById(R.id.note_btn_new_note);
        mRvBookList = findViewById(R.id.note_rv_book_list);
        mRvNoteList = findViewById(R.id.note_rv_note_list);
        mBtnSearch = findViewById(R.id.note_btn_search);
        mBtnView = findViewById(R.id.note_btn_view);
        mBtnEdit = findViewById(R.id.note_btn_edit);
        mBtnRecycle = findViewById(R.id.note_btn_recycle);
    }

    private void initRecyclerView() {
        mBookAdapter = new NoteBookAdapter();
        mRvBookList.setLayoutManager(new LinearLayoutManager(this));
        mRvBookList.setAdapter(mBookAdapter);
        mNoteAdapter = new NoteEntryAdapter();
        mRvNoteList.setLayoutManager(new LinearLayoutManager(this));
        mRvNoteList.setAdapter(mNoteAdapter);
        // 隐藏左侧笔记本列表
        mRvBookList.setVisibility(View.GONE);
        // 隐藏笔记本与笔记之间的竖分隔线
        findViewById(R.id.note_view_divider_center).setVisibility(View.GONE);

        // ✅笔记列表点击回调
        mNoteAdapter.setListener((pos, title) -> {
            PublicJavaCallCpp("note_click|==|" + pos);
        });
    }

    private void setupClickListeners() {
        // ========== 笔记本菜单按钮（代码保留，按钮已隐藏） ==========
        mBtnBookMenu.setOnClickListener(v -> {
            PopupMenu popup = new PopupMenu(NoteActivity.this, mBtnBookMenu);
            boolean isZh = MyActivity.zh_cn;
            String item1 = isZh ? "新建笔记本" : "New Notebook";
            String item2 = isZh ? "新建子笔记本" : "New Sub Notebook";
            String itemRename = isZh ? "重命名" : "Rename";
            String itemDeleteBook = isZh ? "删除笔记本" : "Delete Notebook";
            String item3 = isZh ? "统计" : "Statistics";
            popup.getMenu().add(0, 1, 0, item1);
            popup.getMenu().add(0, 2, 1, item2);
            popup.getMenu().add(0, 4, 2, itemRename);
            popup.getMenu().add(0, 5, 3, itemDeleteBook);
            popup.getMenu().add(0, 3, 4, item3);
            popup.setOnMenuItemClickListener(item -> {
                int id = item.getItemId();
                if (id == 1) {
                    // 新建一级笔记本，无需选中
                    showBookNameInputDialog(isZh, false);
                    return true;
                } else if (id == 2) {
                    // 新建子笔记本，检查父笔记本选中状态
                    int selectedBookPos = mBookAdapter.getSelectedPosition();
                    if (selectedBookPos == -1) {
                        showTipDialog(
                            isZh
                                ? "请先选择一个笔记本"
                                : "Please select a notebook first"
                        );
                        return true;
                    }
                    showBookNameInputDialog(isZh, true);
                    return true;
                } else if (id == 4) {
                    // 笔记本重命名
                    int selectedBookPos = mBookAdapter.getSelectedPosition();
                    if (selectedBookPos == -1) {
                        showTipDialog(
                            isZh
                                ? "请先选择一个笔记本"
                                : "Please select a notebook first"
                        );
                        return true;
                    }
                    showBookRenameDialog(isZh, selectedBookPos);
                    return true;
                } else if (id == 5) {
                    // 删除笔记本
                    int selectedBookPos = mBookAdapter.getSelectedPosition();
                    if (selectedBookPos == -1) {
                        showTipDialog(
                            isZh
                                ? "请先选择一个笔记本"
                                : "Please select a notebook first"
                        );
                        return true;
                    }
                    String bookRaw = mBookAdapter.getItemAt(selectedBookPos);
                    String[] parts = bookRaw.split("\\|==\\|");
                    String bookName = parts.length > 0 ? parts[0] : "";
                    showBookDeleteConfirmDialog(
                        isZh,
                        selectedBookPos,
                        bookName
                    );
                    return true;
                } else if (id == 3) {
                    PublicJavaCallCpp("book_statistics");
                    return true;
                }
                return false;
            });
            popup.show();
        });
        mBtnFavorite.setOnClickListener(v ->
            PublicJavaCallCpp("note_favorite")
        );
        // ========== 笔记菜单按钮 ==========
        mBtnNoteMenu.setOnClickListener(v -> {
            PopupMenu popup = new PopupMenu(NoteActivity.this, mBtnNoteMenu);
            boolean isZh = MyActivity.zh_cn;
            String imp = isZh ? "导入" : "Import";
            String exp = isZh ? "导出" : "Export";
            String del = isZh ? "删除" : "Delete";
            String pdfOut = isZh ? "输出到PDF" : "Export to PDF";
            String rename = isZh ? "重命名" : "Rename";
            String statItem = isZh ? "统计" : "Statistics";

            // 移除了move（移动到）条目
            popup.getMenu().add(0, 10, 0, imp);
            popup.getMenu().add(0, 11, 1, exp);
            popup.getMenu().add(0, 12, 2, del);
            popup.getMenu().add(0, 13, 3, pdfOut);
            popup.getMenu().add(0, 15, 4, rename);
            // 统计放到菜单最后一项
            popup.getMenu().add(0, 20, 5, statItem);

            popup.setOnMenuItemClickListener(item -> {
                int id = item.getItemId();
                if (id == 10) {
                    PublicJavaCallCpp("note_import");
                    finish();
                    return true;
                } else if (id == 11) {
                    int selectedNotePos = mNoteAdapter.getSelectedPosition();
                    if (selectedNotePos == -1) {
                        showTipDialog(
                            isZh
                                ? "请先选择一条笔记"
                                : "Please select a note first"
                        );
                        return true;
                    }
                    PublicJavaCallCpp("note_export|==|" + selectedNotePos);
                    return true;
                } else if (id == 12) {
                    // 删除笔记
                    int selectedNotePos = mNoteAdapter.getSelectedPosition();
                    if (selectedNotePos == -1) {
                        showTipDialog(
                            isZh
                                ? "请先选择一条笔记"
                                : "Please select a note first"
                        );
                        return true;
                    }
                    String noteTitle = mNoteAdapter.getItemAt(selectedNotePos);
                    showNoteDeleteConfirmDialog(
                        isZh,
                        selectedNotePos,
                        noteTitle
                    );
                    return true;
                } else if (id == 13) {
                    int selectedNotePos = mNoteAdapter.getSelectedPosition();
                    if (selectedNotePos == -1) {
                        showTipDialog(
                            isZh
                                ? "请先选择一条笔记"
                                : "Please select a note first"
                        );
                        return true;
                    }
                    PublicJavaCallCpp("note_export_pdf|==|" + selectedNotePos);
                    finish();
                    return true;
                } else if (id == 15) {
                    // 笔记重命名
                    int selectedNotePos = mNoteAdapter.getSelectedPosition();
                    if (selectedNotePos == -1) {
                        showTipDialog(
                            isZh
                                ? "请先选择一条笔记"
                                : "Please select a note first"
                        );
                        return true;
                    }
                    showNoteRenameDialog(isZh, selectedNotePos);
                    return true;
                } else if (id == 20) {
                    // 统计，沿用原来的book_statistics指令
                    PublicJavaCallCpp("notes_statistics");
                    return true;
                }
                return false;
            });
            popup.show();
        });
        mBtnNewNote.setOnClickListener(v -> {
            // 废弃笔记本，不再校验笔记本选中状态，直接新建笔记
            PublicJavaCallCpp("note_create_new");
        });
        mBtnSearch.setOnClickListener(v -> PublicJavaCallCpp("note_search"));
        mBtnView.setOnClickListener(v -> {
            int selectedNoteIndex = mNoteAdapter.getSelectedPosition();
            // 没有选中的笔记，直接返回，不调用C++
            if (selectedNoteIndex == -1) {
                return;
            }
            // 格式：note_view|==|索引，C++端split解析拿到索引
            String callArg = "note_view|==|" + selectedNoteIndex;
            PublicJavaCallCpp(callArg);
        });
        mBtnEdit.setOnClickListener(v -> {
            int selectedNoteIndex = mNoteAdapter.getSelectedPosition();
            if (selectedNoteIndex == -1) return;
            PublicJavaCallCpp("note_edit|==|" + selectedNoteIndex);
        });
        mBtnRecycle.setOnClickListener(v ->
            PublicJavaCallCpp("note_open_recycle")
        );
    }

    /**
     * 应用明暗主题，和MyEventActivity配色完全对齐
     */
    public void applyUiTheme() {
        int rootBg;
        int dividerColor;
        int iconTint;
        if (mIsDark) {
            rootBg = 0xFF1E1E1E;
            dividerColor = 0xFF444444;
            iconTint = 0xFFFFFFFF;
        } else {
            rootBg = 0xFFFFFFFF;
            dividerColor = 0xFFCCCCCC;
            iconTint = 0xFF000000;
        }
        findViewById(R.id.note_layout_root).setBackgroundColor(rootBg);
        findViewById(R.id.note_view_divider_center).setBackgroundColor(
            dividerColor
        );
        //统一给图标着色
        mBtnBookMenu.setColorFilter(iconTint);
        mBtnFavorite.setColorFilter(iconTint);
        mBtnNoteMenu.setColorFilter(iconTint);
        mBtnNewNote.setColorFilter(iconTint);
        mBtnSearch.setColorFilter(iconTint);
        mBtnView.setColorFilter(iconTint);
        mBtnEdit.setColorFilter(iconTint);
        mBtnRecycle.setColorFilter(iconTint);
        mBookAdapter.setDarkMode(mIsDark);
        mNoteAdapter.setDarkMode(mIsDark);
    }

    public void setDark(boolean dark) {
        mIsDark = dark;
        applyUiTheme();
    }

    //对外接口，供JNI推送数据
    public void setNoteBookList(ArrayList<String> bookList) {
        runOnUiThread(() -> {
            mBookAdapter.setData(bookList);
        });
    }

    public void setNoteEntryList(ArrayList<String> noteList) {
        runOnUiThread(() -> {
            mNoteAdapter.setData(noteList);
        });
    }

    public void setSelectedNotebook(int pos) {
        runOnUiThread(() -> {
            mBookAdapter.setSelectedPosition(pos);
        });
    }

    public void setSelectedNote(int pos) {
        runOnUiThread(() -> {
            mNoteAdapter.setSelectedPosition(pos);
        });
    }

    /**
     * JNI调用：获取当前选中笔记索引
     * @return 选中位置，无选中返回 -1
     */
    public int getSelectedNote() {
        return mNoteAdapter.getSelectedPosition();
    }

    /**
     * 弹出笔记本名称输入对话框
     * @param isZh 是否中文
     * @param isSub true=新建子笔记本；false=新建笔记本
     */
    private void showBookNameInputDialog(boolean isZh, boolean isSub) {
        boolean dark = ImmersiveUtil.applyRealImmersive(this);
        int textColor = dark ? 0xFFFFFFFF : 0xFF000000;

        EditText etInput = new EditText(this);
        etInput.setTextColor(textColor);

        String title, hint;
        if (isSub) {
            title = isZh ? "新建子笔记本" : "New Sub Notebook";
            hint = isZh ? "请输入子笔记本名称" : "Input sub notebook name";
        } else {
            title = isZh ? "新建笔记本" : "New Notebook";
            hint = isZh ? "请输入笔记本名称" : "Input notebook name";
        }
        etInput.setHint(hint);

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(title);
        builder.setView(etInput);
        String okText = isZh ? "确定" : "OK";
        String cancelText = isZh ? "取消" : "Cancel";
        builder.setPositiveButton(okText, (dialog, which) -> {
            String name = etInput.getText().toString().trim();
            if (name.isEmpty()) {
                return;
            }
            String cmd;
            if (isSub) {
                int parentIdx = mBookAdapter.getSelectedPosition();
                cmd = "book_create_sub|==|" + name + "|==|" + parentIdx;
            } else {
                cmd = "book_create_new|==|" + name;
            }
            PublicJavaCallCpp(cmd);
        });
        builder.setNegativeButton(cancelText, (dialog, which) -> {
            dialog.dismiss();
        });
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    /**
     * 简单提示弹窗，仅信息+确定按钮
     */
    private void showTipDialog(String msg) {
        boolean dark = ImmersiveUtil.applyRealImmersive(this);
        int textColor = dark ? 0xFFFFFFFF : 0xFF000000;
        boolean isZh = MyActivity.zh_cn;
        String okStr = isZh ? "确定" : "OK";

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage(msg);
        builder.setPositiveButton(okStr, (dialog, which) -> dialog.dismiss());
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    /**
     * 笔记本重命名弹窗
     * @param isZh 是否中文
     * @param selectedPos 当前选中笔记本索引
     */
    private void showBookRenameDialog(boolean isZh, int selectedPos) {
        boolean dark = ImmersiveUtil.applyRealImmersive(this);
        int textColor = dark ? 0xFFFFFFFF : 0xFF000000;

        EditText etInput = new EditText(this);
        etInput.setTextColor(textColor);

        // 获取当前笔记本原始名称
        String rawItem = mBookAdapter.getItemAt(selectedPos);
        String[] parts = rawItem.split("\\|==\\|");
        String currentName = parts.length > 0 ? parts[0] : "";
        etInput.setText(currentName);
        etInput.setSelection(etInput.getText().length()); //光标放末尾

        String title = isZh ? "重命名笔记本" : "Rename Notebook";
        String hint = isZh ? "输入笔记本新名称" : "Input new notebook name";
        etInput.setHint(hint);

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(title);
        builder.setView(etInput);
        String okText = isZh ? "确定" : "OK";
        String cancelText = isZh ? "取消" : "Cancel";

        builder.setPositiveButton(okText, (dialog, which) -> {
            String newName = etInput.getText().toString().trim();
            if (newName.isEmpty()) {
                return;
            }
            String cmd = "book_rename|==|" + selectedPos + "|==|" + newName;
            PublicJavaCallCpp(cmd);
        });
        builder.setNegativeButton(cancelText, (dialog, which) -> {
            dialog.dismiss();
        });
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    /**
     * 删除笔记确认弹窗
     * @param isZh 中英文标记
     * @param selectedPos 笔记选中索引
     * @param noteTitle 当前笔记标题
     */
    private void showNoteDeleteConfirmDialog(
        boolean isZh,
        int selectedPos,
        String noteTitle
    ) {
        boolean dark = ImmersiveUtil.applyRealImmersive(this);
        int textColor = dark ? 0xFFFFFFFF : 0xFF000000;
        String title = isZh ? "删除笔记" : "Delete Note";
        String msg = isZh
            ? "确定要删除笔记：\n" + noteTitle
            : "Confirm to delete note:\n" + noteTitle;
        String okText = isZh ? "确定" : "OK";
        String cancelText = isZh ? "取消" : "Cancel";

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(title);
        builder.setMessage(msg);
        builder.setPositiveButton(okText, (dialog, which) -> {
            String cmd = "note_delete|==|" + selectedPos;
            PublicJavaCallCpp(cmd);
        });
        builder.setNegativeButton(cancelText, (dialog, which) -> {
            dialog.dismiss();
        });
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    /**
     * 笔记重命名弹窗
     * @param isZh 是否中文
     * @param selectedPos 当前选中笔记索引
     */
    private void showNoteRenameDialog(boolean isZh, int selectedPos) {
        boolean dark = ImmersiveUtil.applyRealImmersive(this);
        int textColor = dark ? 0xFFFFFFFF : 0xFF000000;
        EditText etInput = new EditText(this);
        etInput.setTextColor(textColor);

        String currentTitle = mNoteAdapter.getItemAt(selectedPos);
        etInput.setText(currentTitle);
        etInput.setSelection(etInput.getText().length());

        String title = isZh ? "重命名笔记" : "Rename Note";
        String hint = isZh ? "输入笔记新名称" : "Input new note name";
        etInput.setHint(hint);

        String okText = isZh ? "确定" : "OK";
        String cancelText = isZh ? "取消" : "Cancel";
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(title);
        builder.setView(etInput);
        builder.setPositiveButton(okText, (dialog, which) -> {
            String newName = etInput.getText().toString().trim();
            if (newName.isEmpty()) {
                return;
            }
            String cmd = "note_rename|==|" + selectedPos + "|==|" + newName;
            PublicJavaCallCpp(cmd);
        });
        builder.setNegativeButton(cancelText, (dialog, which) -> {
            dialog.dismiss();
        });
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    /**
     * 删除笔记本确认弹窗
     * @param isZh 是否中文
     * @param selectedPos 笔记本索引
     * @param bookName 笔记本名称
     */
    private void showBookDeleteConfirmDialog(
        boolean isZh,
        int selectedPos,
        String bookName
    ) {
        boolean dark = ImmersiveUtil.applyRealImmersive(this);
        int textColor = dark ? 0xFFFFFFFF : 0xFF000000;
        String title = isZh ? "删除笔记本" : "Delete Notebook";
        String msg = isZh
            ? "确定要删除笔记本：\n" + bookName
            : "Confirm to delete notebook:\n" + bookName;
        String okText = isZh ? "确定" : "OK";
        String cancelText = isZh ? "取消" : "Cancel";

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(title);
        builder.setMessage(msg);
        builder.setPositiveButton(okText, (dialog, which) -> {
            String cmd = "book_delete|==|" + selectedPos;
            PublicJavaCallCpp(cmd);
        });
        builder.setNegativeButton(cancelText, (dialog, which) -> {
            dialog.dismiss();
        });
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    /**
     * 统计弹窗入口，复用已有的通用消息弹窗，不重复实现UI
     * @param msgList JNI传入数组，第0项为统计文本
     */
    public void showStatisticDialog(ArrayList<String> msgList) {
        if (msgList == null || msgList.isEmpty()) {
            return;
        }
        String statContent = msgList.get(0);
        String titleText = MyActivity.zh_cn ? "统计" : "Statistics";

        String finalText = titleText + "\n\n" + statContent;
        MyActivity.mInstance.showCommonMsgDialog(this, finalText);
    }

    /**
     * C++调用：打开AI等待弹窗，复用MyActivity公共showAiLoadingDialog
     */
    public void showLoadingDialog() {
        runOnUiThread(() -> {
            if (isFinishing() || isDestroyed()) return;
            if (mAiLoadingDialog == null || !mAiLoadingDialog.isShowing()) {
                mAiLoadingDialog = MyActivity.showAiLoadingDialog(this);
            }
        });
    }

    /**
     * C++调用：关闭AI等待弹窗
     */
    public void dismissLoadingDialog() {
        runOnUiThread(() -> {
            MyActivity.dismissAiLoadingDialog(mAiLoadingDialog);
            mAiLoadingDialog = null;
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mInstance = null;

        // 销毁页面时，强制关闭等待弹窗，防止泄漏
        runOnUiThread(() -> {
            MyActivity.dismissAiLoadingDialog(mAiLoadingDialog);
            mAiLoadingDialog = null;
        });
    }
}
