package com.xhh.pdfui;

import com.x.MyActivity;
import com.x.MyService;
import com.x.R;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.ActionBar;

import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import android.view.WindowManager;
import android.view.Window;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.github.barteksc.pdfviewer.util.FileUtils;
import com.shockwave.pdfium.PdfDocument;
import com.shockwave.pdfium.PdfiumCore;
import com.xhh.pdfui.preview.GridAdapter;
import com.xhh.pdfui.preview.PreviewUtils;

import java.io.File;

import android.text.TextUtils;
import android.content.IntentFilter;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.content.Context;

/**
 * UI页面：PDF预览缩略图（注意：此页面，需多关注内存管控）
 * <p>
 * 1、用于显示Pdf缩略图信息
 * 2、点击缩略图，带回Pdf页码到前一个页面
 * <p>
 * 作者：齐行超
 * 日期：2019.08.07
 */
public class PDFPreviewActivity extends AppCompatActivity implements GridAdapter.GridEvent {

    RecyclerView recyclerView;
    Button btn_back;
    TextView lblTitle;
    PdfiumCore pdfiumCore;
    PdfDocument pdfDocument;
    String assetsFileName;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        setTheme(R.style.AppThemeprice);
        super.onCreate(savedInstanceState);
        // UIUtils.initWindowStyle(getWindow(), getSupportActionBar());
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.activity_preview);

        initView();// 初始化控件
        setEvent();
        loadData();

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

                    onBackPressed();
                } else if (TextUtils.equals(reason, SYSTEM_HOME_KEY_LONG)) {
                    // 表示长按home键,显示最近使用的程序
                    System.out.println("NoteEditor 长按HOME键...");

                    onBackPressed();
                }
            }
        }
    };

    /**
     * 初始化控件
     */
    private void initView() {
        btn_back = findViewById(R.id.btn_back);
        recyclerView = findViewById(R.id.rv_grid);

        lblTitle = findViewById(R.id.lblTitle);
        if (MyService.zh_cn)
            lblTitle.setText("缩略图");
        else
            lblTitle.setText("Thumbnails");
    }

    /**
     * 设置事件
     */
    private void setEvent() {
        btn_back.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // 回收内存
                recycleMemory();

                PDFPreviewActivity.this.finish();
            }
        });

    }

    /**
     * 加载数据
     */
    private void loadData() {
        // 加载pdf文件
        loadPdfFile();

        // 获得pdf总页数
        int totalCount = pdfiumCore.getPageCount(pdfDocument);

        // 绑定列表数据
        GridAdapter adapter = new GridAdapter(this, pdfiumCore, pdfDocument, assetsFileName, totalCount);
        adapter.setGridEvent(this);
        recyclerView.setLayoutManager(new GridLayoutManager(this, 3));
        recyclerView.setAdapter(adapter);
    }

    /**
     * 加载pdf文件
     */
    private void loadPdfFile() {
        Intent intent = getIntent();
        if (intent != null) {
            assetsFileName = intent.getStringExtra("AssetsPdf");
            if (assetsFileName != null) {
                loadAssetsPdfFile(assetsFileName);
            } else {
                Uri uri = intent.getData();
                if (uri != null) {
                    loadUriPdfFile(uri);
                }
            }
        }
    }

    /**
     * 加载assets中的pdf文件
     */
    void loadAssetsPdfFile(String assetsFileName) {
        try {
            File f = FileUtils.fileFromAsset(this, assetsFileName);
            ParcelFileDescriptor pfd = ParcelFileDescriptor.open(f, ParcelFileDescriptor.MODE_READ_ONLY);
            pdfiumCore = new PdfiumCore(this);
            pdfDocument = pdfiumCore.newDocument(pfd);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    /**
     * 基于uri加载pdf文件
     */
    void loadUriPdfFile(Uri uri) {
        try {
            ParcelFileDescriptor pfd = getContentResolver().openFileDescriptor(uri, "r");
            pdfiumCore = new PdfiumCore(this);
            pdfDocument = pdfiumCore.newDocument(pfd);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    /**
     * 点击缩略图，带回Pdf页码到前一个页面
     *
     * @param position 页码
     */
    @Override
    public void onGridItemClick(int position) {
        // 回收内存
        recycleMemory();

        // 返回前一个页码
        Intent intent = new Intent();
        intent.putExtra("pageNum", position);
        setResult(Activity.RESULT_OK, intent);
        finish();
    }

    /**
     * 回收内存
     */
    private void recycleMemory() {
        // 关闭pdf对象
        if (pdfiumCore != null && pdfDocument != null) {
            pdfiumCore.closeDocument(pdfDocument);
            pdfiumCore = null;
        }
        // 清空图片缓存，释放内存空间
        PreviewUtils.getInstance().getImageCache().clearCache();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mHomeKeyEvent);

    }
}
