package com.x;

import android.os.Bundle;
import android.widget.ImageButton;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
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
                    bookList.add(new Book(showTitle, path, hiddenRawName));
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
            MyActivity.m_instance.shareString(
                sel.getTitle(),
                sel.getFilePath(),
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

        // 清除选中，不删除数据
        btnClear.setOnClickListener(v -> {
            bookAdapter.clearAllSelect();
            Toast.makeText(this, "清除选中", Toast.LENGTH_SHORT).show();
        });
    }
}
