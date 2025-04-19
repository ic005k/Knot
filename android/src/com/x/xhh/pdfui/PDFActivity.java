package com.xhh.pdfui;

import org.ini4j.Wini;

import com.x.MyActivity;
import com.x.MyPDF;
import com.x.MyProgBar;
import com.x.R;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;

import com.github.barteksc.pdfviewer.PDFView;
import com.github.barteksc.pdfviewer.PDFView.Configurator;
import com.github.barteksc.pdfviewer.listener.OnLoadCompleteListener;
import com.github.barteksc.pdfviewer.listener.OnPageChangeListener;
import com.github.barteksc.pdfviewer.listener.OnPageErrorListener;
import com.github.barteksc.pdfviewer.scroll.DefaultScrollHandle;
import com.github.barteksc.pdfviewer.util.FitPolicy;
import com.shockwave.pdfium.PdfDocument;
import com.xhh.pdfui.tree.TreeNodeData;

import java.io.File;
import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import android.view.WindowManager;
import android.view.Window;

import android.text.TextUtils;
import android.content.IntentFilter;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.net.Uri;

import android.view.MotionEvent;

/**
 * UI页面：PDF阅读
 * <p>
 * 主要功能：
 * 1、接收传递过来的pdf文件（包括assets中的文件名、文件uri）
 * 2、显示PDF文件
 * 3、接收目录页面、预览页面返回的PDF页码，跳转到指定的页面
 * <p>
 * 作者：齐行超
 * 日期：2019.08.07
 */
public class PDFActivity extends AppCompatActivity implements
        OnPageChangeListener,
        OnLoadCompleteListener,
        OnPageErrorListener {
    // PDF控件
    PDFView pdfView;
    // 按钮控件：返回、目录、缩略图
    static ImageButton btn_back, btn_dark, btn_catalogue, btn_preview, btn_open, btn_books;

    // 页码
    Integer pageNumber = 0;
    // PDF目录集合
    List<TreeNodeData> catelogues;

    // pdf文件名（限：assets里的文件）
    String assetsFileName;
    // pdf文件uri
    Uri uri;

    public native static void CallJavaNotify_0();

    public native static void CallJavaNotify_1();

    public native static void CallJavaNotify_2();

    public native static void CallJavaNotify_3();

    public native static void CallJavaNotify_4();

    public native static void CallJavaNotify_5();

    public native static void CallJavaNotify_6();

    public native static void CallJavaNotify_7();

    public native static void CallJavaNotify_8();

    public native static void CallJavaNotify_9();

    public native static void CallJavaNotify_10();

    public native static void CallJavaNotify_11();

    public native static void CallJavaNotify_12();

    public native static void CallJavaNotify_13();

    public native static void CallJavaNotify_14();

    public static PDFActivity mPdfActivity;
    public static Context context;
    private static float f_zoom = 1.25f;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        setTheme(R.style.AppThemeprice);
        super.onCreate(savedInstanceState);
        // UIUtils.initWindowStyle(getWindow(), getSupportActionBar());//设置沉浸式
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);
        mPdfActivity = this;
        context = PDFActivity.this;
        setContentView(R.layout.activity_pdf);

        initView();// 初始化view
        setEvent();// 设置事件
        loadPdf();// 加载PDF文件

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

    @Override
    public void onBackPressed() {
        super.onBackPressed();

    }

    /**
     * 初始化view
     */
    private void initView() {
        pdfView = findViewById(R.id.pdfView);
        btn_back = findViewById(R.id.btn_back);
        btn_dark = findViewById(R.id.btn_dark);
        btn_catalogue = findViewById(R.id.btn_catalogue);
        btn_preview = findViewById(R.id.btn_preview);
        btn_open = findViewById(R.id.btn_open);
        btn_books = findViewById(R.id.btn_books);
    }

    /**
     * 设置事件
     */
    private void setEvent() {
        // 返回
        btn_back.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                PDFActivity.this.finish();
            }
        });
        btn_dark.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (PDFView.nightMode == false) {
                    PDFView.nightMode = true;
                    Configurator.nightMode = true;
                } else {
                    PDFView.nightMode = false;
                    Configurator.nightMode = false;
                }

                savePDFInfo();
                PDFActivity.this.finish();
                CallJavaNotify_13();

            }
        });
        // 跳转目录页面
        btn_catalogue.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(PDFActivity.this, PDFCatelogueActivity.class);
                intent.putExtra("catelogues", (Serializable) catelogues);
                PDFActivity.this.startActivityForResult(intent, 200);
            }
        });
        // 跳转缩略图页面
        btn_preview.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(PDFActivity.this, PDFPreviewActivity.class);
                intent.putExtra("AssetsPdf", assetsFileName);
                intent.setData(uri);
                PDFActivity.this.startActivityForResult(intent, 201);
            }
        });

        btn_open.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                PDFActivity.this.finish();
                CallJavaNotify_10();

            }
        });

        btn_books.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                PDFActivity.this.finish();
                CallJavaNotify_11();

            }
        });

    }

    /**
     * 加载PDF文件
     */
    private void loadPdf() {
        Intent intent = getIntent();
        if (intent != null) {
            assetsFileName = intent.getStringExtra("AssetsPdf");
            if (assetsFileName != null) {
                displayFromAssets(assetsFileName);
            } else {
                uri = intent.getData();
                System.out.println("PDFActivity uri:" + uri);

                if (uri != null) {
                    String filename = "/storage/emulated/0/.Knot/mypdf.ini";
                    try {
                        File file = new File(filename);
                        if (file.exists()) {
                            Wini ini = new Wini(file);
                            String name = uri.toString();
                            name = name.replace(":", "");
                            name = name.replace(".", "");
                            name = name.replace("/", "");
                            String strPage = ini.get("pdf", name);
                            String strZoom = ini.get("zoom", name);
                            String strNight = ini.get("night", name);
                            System.out.print("strPage:" + strPage);

                            if (strPage != null)
                                pageNumber = Integer.parseInt(strPage);
                            else
                                pageNumber = 0;

                            if (strZoom != null)
                                f_zoom = Float.parseFloat(strZoom);
                            else
                                f_zoom = 1.0f;

                            boolean isNight;
                            if (strNight != null)
                                isNight = Boolean.parseBoolean(strNight);
                            else
                                isNight = false;
                            pdfView.nightMode = isNight;
                            Configurator.nightMode = isNight;
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }

                    displayFromUri(uri);

                }
            }
        }
    }

    /**
     * 基于assets显示 PDF 文件
     *
     * @param fileName 文件名称
     */
    private void displayFromAssets(String fileName) {
        pdfView.fromAsset(fileName)
                .defaultPage(pageNumber)
                .onPageChange(this)
                .enableAnnotationRendering(true)
                .onLoad(this)
                .scrollHandle(new DefaultScrollHandle(this))
                .spacing(10) // 单位 dp
                .onPageError(this)
                .pageFitPolicy(FitPolicy.BOTH)
                .load();
    }

    /**
     * 基于uri显示 PDF 文件
     *
     * @param uri 文件路径
     */
    private void displayFromUri(Uri uri) {
        pdfView.fromUri(uri)
                .defaultPage(pageNumber)
                .onPageChange(this)
                .enableAnnotationRendering(true)
                .onLoad(this)
                .scrollHandle(new DefaultScrollHandle(this))
                .spacing(10) // 单位 dp
                .onPageError(this)
                .load();

    }

    /**
     * 当成功加载PDF：
     * 1、可获取PDF的目录信息
     *
     * @param nbPages the number of pages in this PDF file
     */
    @Override
    public void loadComplete(int nbPages) {
        // 获得文档书签信息
        List<PdfDocument.Bookmark> bookmarks = pdfView.getTableOfContents();
        if (catelogues != null) {
            catelogues.clear();
        } else {
            catelogues = new ArrayList<>();
        }
        // 将bookmark转为目录数据集合
        bookmarkToCatelogues(catelogues, bookmarks, 1);
    }

    /**
     * 将bookmark转为目录数据集合（递归）
     *
     * @param catelogues 目录数据集合
     * @param bookmarks  书签数据
     * @param level      目录树级别（用于控制树节点位置偏移）
     */
    private void bookmarkToCatelogues(List<TreeNodeData> catelogues, List<PdfDocument.Bookmark> bookmarks, int level) {
        for (PdfDocument.Bookmark bookmark : bookmarks) {
            TreeNodeData nodeData = new TreeNodeData();
            nodeData.setName(bookmark.getTitle());
            nodeData.setPageNum((int) bookmark.getPageIdx());
            nodeData.setTreeLevel(level);
            nodeData.setExpanded(false);
            catelogues.add(nodeData);
            if (bookmark.getChildren() != null && bookmark.getChildren().size() > 0) {
                List<TreeNodeData> treeNodeDatas = new ArrayList<>();
                nodeData.setSubset(treeNodeDatas);
                bookmarkToCatelogues(treeNodeDatas, bookmark.getChildren(), level + 1);
            }
        }
    }

    @Override
    public void onPageChanged(int page, int pageCount) {
        pageNumber = page;
    }

    @Override
    public void onPageError(int page, Throwable t) {
    }

    /**
     * 从缩略图、目录页面带回页码，跳转到指定PDF页面
     *
     * @param requestCode
     * @param resultCode
     * @param data
     */
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == RESULT_OK) {
            int pageNum = data.getIntExtra("pageNum", 0);
            if (pageNum > 0) {
                pdfView.jumpTo(pageNum);
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mHomeKeyEvent);
        savePDFInfo();
        MyProgBar.closeAndroidProgressBar();

        // 是否内存
        if (pdfView != null) {
            pdfView.recycle();
        }
    }

    public static void closeMyPDF() {
        if (mPdfActivity != null)
            mPdfActivity.finish();
    }

    public static void hideOrShowToolBar() {
        if (btn_back.getVisibility() == View.VISIBLE) {
            btn_back.setVisibility(View.GONE);
            btn_dark.setVisibility(View.GONE);
            btn_open.setVisibility(View.GONE);
            btn_books.setVisibility(View.GONE);
            btn_catalogue.setVisibility(View.GONE);
            btn_preview.setVisibility(View.GONE);
        } else {
            btn_back.setVisibility(View.VISIBLE);
            btn_dark.setVisibility(View.VISIBLE);
            btn_open.setVisibility(View.VISIBLE);
            btn_books.setVisibility(View.VISIBLE);
            btn_catalogue.setVisibility(View.VISIBLE);
            btn_preview.setVisibility(View.VISIBLE);
        }
    }

    private void savePDFInfo() {
        String filename = "/storage/emulated/0/.Knot/mypdf.ini";
        try {
            File file = new File(filename);
            if (!file.exists())
                file.createNewFile();
            Wini ini = new Wini(file);
            String name = uri.toString();
            name = name.replace(":", "");
            name = name.replace(".", "");
            name = name.replace("/", "");
            ini.put("pdf", name, String.valueOf(pageNumber));
            ini.put("zoom", name, String.valueOf(pdfView.getZoom()));
            ini.put("night", name, String.valueOf(pdfView.nightMode));
            ini.store();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}
