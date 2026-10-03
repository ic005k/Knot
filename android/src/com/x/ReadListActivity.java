package com.x;

import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.Toast;
import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class ReadListActivity extends AppCompatActivity {

    public static boolean isDark = false;
    private BookAdapter bookAdapter;
    private List<Book> bookList;

    private ImageButton btnOpen, btnRead, btnShare, btnRemove, btnClear;

    public static native void PublicJavaCallCpp(String type);

    private OnBackPressedCallback mBackCallback;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // 注册返回拦截回调
        mBackCallback = new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                PublicJavaCallCpp("back_read_list");
                finish();
            }
        };
        getOnBackPressedDispatcher().addCallback(this, mBackCallback);

        isDark = ImmersiveUtil.applyRealImmersive(this);
        setContentView(R.layout.activity_read_list);
        setTitle("Read List");

        // ========== ✅ 新增：动态暗黑模式适配 ==========
        int bgColorRoot = isDark ? 0xFF1E1E1E : 0xFFFFFFFF;
        int bgColorToolbar = isDark ? 0xFF2A2A2A : 0xFFF5F5F5;
        int iconColor = isDark ? 0xFFFFFFFF : 0xFF000000;

        // 根布局背景
        findViewById(R.id.rootLayout).setBackgroundColor(bgColorRoot);

        // 工具栏背景
        LinearLayout toolbarLayout = findViewById(R.id.toolbarLayout);
        toolbarLayout.setBackgroundColor(bgColorToolbar);
        // ================================================

        btnOpen = findViewById(R.id.btnOpen);
        btnRead = findViewById(R.id.btnRead);
        btnShare = findViewById(R.id.btnShare);
        btnRemove = findViewById(R.id.btnRemove);
        btnClear = findViewById(R.id.btnClear);
        btnClear.setVisibility(View.GONE);

        // ✅ 使用上面统一定义的 iconColor
        btnOpen.setColorFilter(iconColor);
        btnRead.setColorFilter(iconColor);
        btnShare.setColorFilter(iconColor);
        btnRemove.setColorFilter(iconColor);
        btnClear.setColorFilter(iconColor);

        RecyclerView recyclerView = findViewById(R.id.recyclerBookList);
        recyclerView.setLayoutManager(new LinearLayoutManager(this));
        // ✅ RecyclerView 也跟随根背景色
        recyclerView.setBackgroundColor(bgColorRoot);

        bookList = new ArrayList<>();

        // ===================== 加载书籍列表 =====================
        ArrayList<String> rawBookList = getIntent().getStringArrayListExtra(
            "book_list"
        );
        if (rawBookList != null && !rawBookList.isEmpty()) {
            for (String line : rawBookList) {
                String[] parts = line.split("\\|");
                if (parts.length >= 3) {
                    String showTitle = parts[0];
                    String path = parts[1];
                    String hiddenRawName = parts[2];
                    Book b = new Book(showTitle, path, hiddenRawName);
                    String ext = "";
                    int dotIndex = path.lastIndexOf('.');
                    if (dotIndex >= 0) {
                        ext = path.substring(dotIndex + 1).toLowerCase();
                    }
                    b.setExt(ext);
                    bookList.add(b);
                }
            }
        }

        if (bookList.isEmpty()) {
            Toast.makeText(this, "阅读列表为空", Toast.LENGTH_SHORT).show();
        }
        // ====================================================

        bookAdapter = new BookAdapter(bookList, isDark); // ✅ 传入 isDark
        recyclerView.setAdapter(bookAdapter);

        btnOpen.setOnClickListener(v -> {
            MyActivity.m_instance.openFilePicker();
            finish();
        });

        btnRead.setOnClickListener(v -> {
            Book sel = bookAdapter.getSelectedItem();
            if (sel == null) {
                showSelectBookTip();
                return;
            }
            String filePath = sel.getFilePath();
            if (filePath == null || filePath.isEmpty()) return;

            MyActivity.m_instance.setTempSwapStr(filePath);
            PublicJavaCallCpp("read_book_file");
            finish();
        });

        btnShare.setOnClickListener(v -> {
            Book sel = bookAdapter.getSelectedItem();
            if (sel == null) {
                showSelectBookTip();
                return;
            }
            MyActivity.m_instance.shareImage(
                sel.getTitle(),
                sel.getFilePath(),
                "*/*",
                MyActivity.m_instance
            );
        });

        btnRemove.setOnClickListener(v -> {
            Book sel = bookAdapter.getSelectedItem();
            if (sel == null) {
                showSelectBookTip();
                return;
            }
            int index = bookList.indexOf(sel);
            bookList.remove(index);
            bookAdapter.notifyItemRemoved(index);
            MyActivity.mInstance.PublicJavaCallCpp(
                "remove_read_list|==|" + index
            );
            if (bookAdapter.getSelectedPosition() == index) {
                bookAdapter.clearAllSelect();
            }
        });

        btnClear.setOnClickListener(v -> {
            Book sel = bookAdapter.getSelectedItem();
            if (sel == null) {
                showSelectBookTip();
                return;
            }
            String rawName = sel.getRawBookName();
            if (rawName == null || rawName.isEmpty()) return;

            String iniDir = "/storage/emulated/0/KnotData/";
            String file_ini = iniDir + "bookini/" + rawName + ".ini";
            File iniFile = new File(file_ini);
            if (!iniFile.exists() || !iniFile.isFile()) return;

            new AlertDialog.Builder(this)
                .setTitle("Knot")
                .setMessage(
                    MyActivity.zh_cn
                        ? "确定要清除本书阅读标记吗？\n\n " + file_ini
                        : "Clear reading marks for the current book?\n\n " +
                              file_ini
                )
                .setPositiveButton(
                    MyActivity.zh_cn ? "确定" : "OK",
                    (dialog, which) -> {
                        MyActivity.m_instance.setTempSwapStr(rawName);
                        PublicJavaCallCpp("clear_reader_records");
                    }
                )
                .setNegativeButton(MyActivity.zh_cn ? "取消" : "Cancel", null)
                .show();
        });
    }

    /**
     * 双语Toast：请首先选择一本书籍。
     */
    private void showSelectBookTip() {
        String msg = MyActivity.zh_cn
            ? "请首先选择一本书籍。"
            : "Please select a book first.";
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mBackCallback != null) {
            mBackCallback.remove();
            mBackCallback = null;
        }
    }
}
