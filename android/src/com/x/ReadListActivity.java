package com.x;

import android.os.Bundle;
import android.widget.ImageButton;
import android.widget.Toast;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class ReadListActivity extends AppCompatActivity {

    private BookAdapter bookAdapter;
    private List<Book> bookList;

    private ImageButton btnOpen, btnRead, btnShare, btnRemove, btnClear;

    public static native void PublicJavaCallCpp(String type);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_read_list);
        setTitle("阅读列表");

        // 接收主Activity传递过来的暗黑模式，复用项目沉浸式工具
        boolean darkMode = getIntent().getBooleanExtra("isDarkMode", false);
        //ImmersiveUtil.applyRealImmersive(this, darkMode);
        ImmersiveUtil.applyRealImmersive(this);

        btnOpen = findViewById(R.id.btnOpen);
        btnRead = findViewById(R.id.btnRead);
        btnShare = findViewById(R.id.btnShare);
        btnRemove = findViewById(R.id.btnRemove);
        btnClear = findViewById(R.id.btnClear);

        RecyclerView recyclerView = findViewById(R.id.recyclerBookList);
        recyclerView.setLayoutManager(new LinearLayoutManager(this));

        bookList = new ArrayList<>();

        // ===================== 加载书籍列表 =====================
        ArrayList<String> rawBookList = getIntent().getStringArrayListExtra(
            "book_list"
        );
        if (rawBookList != null && !rawBookList.isEmpty()) {
            for (String line : rawBookList) {
                String[] parts = line.split("\\|");
                // 顺序：标题 | 文件路径 | 隐藏原始书名，至少3段
                if (parts.length >= 3) {
                    String showTitle = parts[0];
                    String path = parts[1];
                    String hiddenRawName = parts[2];
                    Book b = new Book(showTitle, path, hiddenRawName);
                    // 新增：计算扩展名 set到Book对象
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

        bookAdapter = new BookAdapter(bookList);
        recyclerView.setAdapter(bookAdapter);

        // 打开按钮
        btnOpen.setOnClickListener(v -> {
            MyActivity.m_instance.openFilePicker();
            onBackPressed();
        });

        // 阅读按钮
        btnRead.setOnClickListener(v -> {
            Book sel = bookAdapter.getSelectedItem();
            if (sel == null) {
                Toast.makeText(
                    this,
                    "Please select a book first.",
                    Toast.LENGTH_SHORT
                ).show();
                return;
            }
            String filePath = sel.getFilePath();
            if (filePath == null || filePath.isEmpty()) {
                return;
            }

            // 获取小写扩展名
            String ext = "";
            int dotIndex = filePath.lastIndexOf('.');
            if (dotIndex >= 0) {
                ext = filePath.substring(dotIndex + 1).toLowerCase();
            }

            /*if ("pdf".equals(ext) || "mobi".equals(ext)) {
                MyActivity.m_instance.openMyPDF(filePath);
            } else {
                MyActivity.m_instance.setTempSwapStr(filePath);
                PublicJavaCallCpp("open_book_file");
            }*/

            MyActivity.m_instance.setTempSwapStr(filePath);
            PublicJavaCallCpp("open_book_file");

            onBackPressed();
        });

        // 分享按钮：复用项目已有的分享工具
        btnShare.setOnClickListener(v -> {
            Book sel = bookAdapter.getSelectedItem();
            if (sel == null) {
                Toast.makeText(
                    this,
                    "Please select a book first.",
                    Toast.LENGTH_SHORT
                ).show();
                return;
            }
            MyActivity.m_instance.shareImage(
                sel.getTitle(),
                sel.getFilePath(),
                "Book",
                MyActivity.m_instance
            );
        });

        // 移除按钮
        btnRemove.setOnClickListener(v -> {
            Book sel = bookAdapter.getSelectedItem();
            if (sel == null) {
                Toast.makeText(
                    this,
                    "Please select a book first.",
                    Toast.LENGTH_SHORT
                ).show();
                return;
            }
            int index = bookList.indexOf(sel);
            bookList.remove(index);
            bookAdapter.notifyItemRemoved(index);

            // 如果删掉的就是当前选中，清空选择
            if (bookAdapter.getSelectedPosition() == index) {
                bookAdapter.clearAllSelect();
            }
        });

        // 清除阅读标记
        btnClear.setOnClickListener(v -> {
            Book sel = bookAdapter.getSelectedItem();
            if (sel == null) {
                Toast.makeText(
                    this,
                    "Please select a book first.",
                    Toast.LENGTH_SHORT
                ).show();
                return;
            }
            String rawName = sel.getRawBookName();
            if (rawName == null || rawName.isEmpty()) {
                return;
            }

            String iniDir = "/storage/emulated/0/KnotData/";
            String file_ini = iniDir + "bookini/" + rawName + ".ini";
            File iniFile = new File(file_ini);
            if (!iniFile.exists() || !iniFile.isFile()) {
                // 文件不存在，直接返回，不弹确认框
                return;
            }

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
}
