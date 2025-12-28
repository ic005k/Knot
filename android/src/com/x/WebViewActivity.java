package com.x;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.webkit.JavascriptInterface;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceRequest;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.Toast;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

public class WebViewActivity extends Activity {

    private static final String TAG = "WebViewActivity"; // 调试标签
    private static WebViewActivity instance;
    private WebView mWebView;
    private static final int REQUEST_EDIT = 100;
    private static final int REQUEST_EXTERNAL_STORAGE = 101;
    private static final int REQUEST_CALL_PERMISSION = 102;

    private ProgressBar mLoadingProgress; // 加载进度条

    private static String[] PERMISSIONS_STORAGE = {
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE,
    };

    // 电话权限（用于拨号界面操作）
    private static String[] PERMISSIONS_CALL = {
        Manifest.permission.CALL_PHONE,
    };

    // 用于存储页面位置的SharedPreferences
    private SharedPreferences scrollPositionPrefs;

    public static native void CallJavaNotify_17();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        instance = this;
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.activity_webview);

        // 初始化进度条（新增）
        mLoadingProgress = findViewById(R.id.web_loading_progress);

        if (MyActivity.isDark) {
            this.setStatusBarColor("#19232D"); // 深色
            getWindow()
                .getDecorView()
                .setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
        } else {
            this.setStatusBarColor("#F3F3F3"); // 灰
            getWindow()
                .getDecorView()
                .setSystemUiVisibility(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
        }

        // 初始化SharedPreferences
        scrollPositionPrefs = getSharedPreferences(
            "WebViewScrollPositions",
            MODE_PRIVATE
        );

        // 初始化WebView（仅配置，不加载）
        initWebView();

        // 检查存储权限，若已授予则直接加载
        if (
            ContextCompat.checkSelfPermission(
                this,
                Manifest.permission.READ_EXTERNAL_STORAGE
            ) ==
            PackageManager.PERMISSION_GRANTED
        ) {
            loadLocalHtmlFile();
        } else {
            verifyStoragePermissions(this); // 未授予则申请权限
        }

        // 关闭按钮逻辑
        Button closeButton = findViewById(R.id.close_button);
        if (MyActivity.zh_cn) closeButton.setText("关闭");
        else closeButton.setText("Close");
        closeButton.setOnClickListener(v -> finish());

        Button editButton = findViewById(R.id.edit_button);
        if (MyActivity.zh_cn) editButton.setText("编辑");
        else editButton.setText("Edit");
        editButton.setOnClickListener(v -> openEdit());

        // 后退按钮
        Button backButton = findViewById(R.id.back_button);
        if (MyActivity.zh_cn) backButton.setText("后退");
        else backButton.setText("Back");
        backButton.setOnClickListener(v -> webGoBack());

        // 前进按钮
        Button forwardButton = findViewById(R.id.forward_button);
        if (MyActivity.zh_cn) forwardButton.setText("前进");
        else forwardButton.setText("Forw");
        forwardButton.setOnClickListener(v -> webGoForward());
    }

    // JavaScript接口
    public class WebAppInterface {

        // 打开图片（保留响应功能）
        @JavascriptInterface
        public void openImage(String url) {
            String localPath = url.replace("file://", "");
            MyActivity.strImageFile = localPath;
            Log.d(TAG, "打开图片路径: " + localPath);
            Intent intent = new Intent(
                WebViewActivity.this,
                ImageViewerActivity.class
            );
            startActivity(intent);
        }

        // 拨打电话（打开拨号准备界面）
        @JavascriptInterface
        public void makeCall(String phoneNumber) {
            String cleanNumber = phoneNumber.replaceAll("[^0-9+]", "");
            Log.d(TAG, "准备拨打号码: " + cleanNumber);

            // 使用ACTION_DIAL打开拨号界面（而非直接拨号）
            Intent intent = new Intent(
                Intent.ACTION_DIAL,
                Uri.parse("tel:" + cleanNumber)
            );
            startActivity(intent);
        }

        // 发送邮件
        @JavascriptInterface
        public void sendEmail(String emailAddress) {
            Log.d(TAG, "准备发送邮件到: " + emailAddress);
            Intent intent = new Intent(
                Intent.ACTION_SENDTO,
                Uri.parse("mailto:" + emailAddress)
            );
            startActivity(Intent.createChooser(intent, "选择邮件客户端"));
        }

        // 保存滚动位置到SharedPreferences
        @JavascriptInterface
        public void saveScrollPosition(int scrollY) {
            if (MyActivity.strMDFile != null) {
                SharedPreferences.Editor editor = scrollPositionPrefs.edit();
                editor.putInt(MyActivity.strMDFile, scrollY);
                editor.apply();
                Log.d(
                    TAG,
                    "保存滚动位置: " +
                        scrollY +
                        " 对于文件: " +
                        MyActivity.strMDFile
                );
            }
        }
    }

    // 自定义WebViewClient（处理链接打开方式）
    private class CustomWebViewClient extends WebViewClient {

        @Override
        public boolean shouldOverrideUrlLoading(
            WebView view,
            WebResourceRequest request
        ) {
            String url = request.getUrl().toString();
            Log.d(TAG, "点击链接: " + url);

            // 处理电话号码（打开拨号界面）
            if (url.startsWith("tel:")) {
                String phoneNumber = url.substring(4);
                new WebAppInterface().makeCall(phoneNumber);
                return true;
            }
            // 处理邮件地址链接
            else if (url.startsWith("mailto:")) {
                String email = url.substring(7);
                new WebAppInterface().sendEmail(email);
                return true;
            }
            // 处理图片链接（仅响应，不解析）
            else if (isImageUrl(url)) {
                new WebAppInterface().openImage(url);
                return true;
            }
            // 普通链接（http/https）用外部浏览器打开
            else if (url.startsWith("http://") || url.startsWith("https://")) {
                Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
                startActivity(intent);
                return true;
            }

            // 其他链接默认用WebView打开
            return super.shouldOverrideUrlLoading(view, request);
        }

        // 判断是否为图片URL（仅用于响应已存在的图片链接）
        private boolean isImageUrl(String url) {
            String lowerUrl = url.toLowerCase();
            return (
                lowerUrl.endsWith(".jpg") ||
                lowerUrl.endsWith(".jpeg") ||
                lowerUrl.endsWith(".png") ||
                lowerUrl.endsWith(".gif") ||
                lowerUrl.endsWith(".bmp") ||
                lowerUrl.endsWith(".webp")
            );
        }

        @Override
        public void onPageFinished(WebView view, String url) {
            super.onPageFinished(view, url);
            Log.d(TAG, "页面加载完成，开始处理内容");

            // 恢复滚动位置
            if (MyActivity.strMDFile != null) {
                int scrollY = scrollPositionPrefs.getInt(
                    MyActivity.strMDFile,
                    0
                );
                if (scrollY > 0) {
                    String js = "window.scrollTo(0, " + scrollY + ");";
                    mWebView.evaluateJavascript(js, null);
                    Log.d(
                        TAG,
                        "恢复滚动位置: " +
                            scrollY +
                            " 对于文件: " +
                            MyActivity.strMDFile
                    );
                }
            }

            // 移除图片解析逻辑，仅保留滚动监听
            String scrollListenerJs =
                "window.onscroll = function() {" +
                "   AndroidInterface.saveScrollPosition(window.scrollY);" +
                "};";
            mWebView.evaluateJavascript(scrollListenerJs, null);
        }
    }

    // 加载本地HTML文件的方法
    public void loadLocalHtmlFile() {
        String filePath = "file:///storage/emulated/0/KnotData/memo.html";
        Log.d(TAG, "加载HTML文件: " + filePath);
        mWebView.loadUrl(filePath);
    }

    // 验证存储权限
    public static void verifyStoragePermissions(Activity activity) {
        int permission = ActivityCompat.checkSelfPermission(
            activity,
            Manifest.permission.READ_EXTERNAL_STORAGE
        );
        if (permission != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                activity,
                PERMISSIONS_STORAGE,
                REQUEST_EXTERNAL_STORAGE
            );
        }
    }

    // 验证电话权限
    public static void verifyCallPermissions(Activity activity) {
        int permission = ActivityCompat.checkSelfPermission(
            activity,
            Manifest.permission.CALL_PHONE
        );
        if (permission != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                activity,
                PERMISSIONS_CALL,
                REQUEST_CALL_PERMISSION
            );
        }
    }

    public static void openLocalHtml(Activity parentActivity) {
        // 启动前检查存储权限，确保状态最新
        if (
            ContextCompat.checkSelfPermission(
                parentActivity,
                Manifest.permission.READ_EXTERNAL_STORAGE
            ) !=
            PackageManager.PERMISSION_GRANTED
        ) {
            ActivityCompat.requestPermissions(
                parentActivity,
                PERMISSIONS_STORAGE,
                REQUEST_EXTERNAL_STORAGE
            );
            return;
        }

        Intent intent = new Intent(parentActivity, WebViewActivity.class);
        parentActivity.startActivity(intent);
    }

    private void openEdit() {
        MyActivity.isEdit = true;
        Intent i = new Intent(this, NoteEditor.class);
        startActivityForResult(i, REQUEST_EDIT);
    }

    public static void refreshWebViewContent() {
        // 防护1：获取当前WebViewActivity实例，判空
        final WebViewActivity instance = getInstance();
        if (instance == null) {
            Log.d(
                TAG,
                "refreshWebViewContent: WebViewActivity实例为空，无需刷新"
            );
            return;
        }

        // 防护2：判断页面是否已经销毁/正在销毁，避免空指针（核心必需防护）
        if (instance.isDestroyed() || instance.isFinishing()) {
            Log.d(
                TAG,
                "refreshWebViewContent: WebViewActivity已销毁，跳过刷新"
            );
            return;
        }

        // ========== 核心关键：强制切换到 Android 主线程执行WebView操作 【解决线程报错的核心代码】 ==========
        instance.runOnUiThread(
            new Runnable() {
                @Override
                public void run() {
                    // 防护3：只保留WebView实例判空即可，删除低版本不兼容的 isDestroyed()
                    if (instance.mWebView != null) {
                        Log.d(
                            TAG,
                            "refreshWebViewContent: 主线程执行刷新，调用loadLocalHtmlFile"
                        );
                        // 最终执行刷新，此时100%在Android主线程，无线程报错，刷新必生效
                        instance.loadLocalHtmlFile();
                    } else {
                        Log.e(
                            TAG,
                            "refreshWebViewContent: WebView实例为空，刷新失败"
                        );
                    }
                }
            }
        );
    }

    @Override
    public void onRequestPermissionsResult(
        int requestCode,
        String[] permissions,
        int[] grantResults
    ) {
        super.onRequestPermissionsResult(
            requestCode,
            permissions,
            grantResults
        );
        if (requestCode == REQUEST_CALL_PERMISSION) {
            if (
                grantResults.length > 0 &&
                grantResults[0] == PackageManager.PERMISSION_GRANTED
            ) {
                Log.d(TAG, "电话权限已授予");
            }
        }
        // 新增：处理存储权限
        else if (requestCode == REQUEST_EXTERNAL_STORAGE) {
            if (
                grantResults.length > 0 &&
                grantResults[0] == PackageManager.PERMISSION_GRANTED
            ) {
                Log.d(TAG, "存储权限已授予，重新加载HTML");
                loadLocalHtmlFile(); // 权限授予后主动加载
            } else {
                Log.e(TAG, "存储权限被拒绝，无法加载本地HTML");
                Toast.makeText(
                    this,
                    MyActivity.zh_cn
                        ? "需要存储权限才能加载本地网页"
                        : "Storage permission is required to load local web pages",
                    Toast.LENGTH_SHORT
                ).show();
            }
        }
    }

    @Override
    protected void onActivityResult(
        int requestCode,
        int resultCode,
        Intent data
    ) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_EDIT && resultCode == RESULT_OK) {
            loadLocalHtmlFile();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        // 在页面暂停时强制保存一次滚动位置
        if (mWebView != null) {
            mWebView.evaluateJavascript(
                "AndroidInterface.saveScrollPosition(window.scrollY);",
                null
            );
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        // 延迟请求电话权限，确保Activity上下文有效
        if (
            ContextCompat.checkSelfPermission(
                this,
                Manifest.permission.CALL_PHONE
            ) !=
            PackageManager.PERMISSION_GRANTED
        ) {
            verifyCallPermissions(this);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        CallJavaNotify_17();

        // 释放WebView，避免内存泄漏
        if (mWebView != null) {
            mWebView.removeAllViews();
            mWebView.destroy();
            mWebView = null;
        }
        instance = null; // 释放静态实例
    }

    /**
     * 网页后退功能
     */
    private void webGoBack() {
        if (mWebView != null && mWebView.canGoBack()) {
            mWebView.goBack();
            Log.d(TAG, "网页后退");
        } else {
            Log.d(TAG, "已无历史记录可后退");
        }
    }

    /**
     * 网页前进功能
     */
    private void webGoForward() {
        if (mWebView != null && mWebView.canGoForward()) {
            mWebView.goForward();
            Log.d(TAG, "网页前进");
        } else {
            Log.d(TAG, "已无历史记录可前进");
        }
    }

    public static WebViewActivity getInstance() {
        return instance;
    }

    private void setStatusBarColor(String color) {
        // 需要安卓版本大于5.0以上
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            getWindow().addFlags(
                WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS
            );
            getWindow().setStatusBarColor(Color.parseColor(color));
        }
    }

    private void initWebView() {
        mWebView = findViewById(R.id.webview);
        WebSettings webSettings = mWebView.getSettings();
        webSettings.setJavaScriptEnabled(true);
        webSettings.setAllowFileAccess(true);
        webSettings.setAllowFileAccessFromFileURLs(true);
        webSettings.setAllowUniversalAccessFromFileURLs(true);
        webSettings.setDomStorageEnabled(true);
        mWebView.addJavascriptInterface(
            new WebAppInterface(),
            "AndroidInterface"
        );
        mWebView.setWebViewClient(new CustomWebViewClient());

        // 新增：监听WebView加载进度（核心）
        mWebView.setWebChromeClient(
            new WebChromeClient() {
                @Override
                public void onProgressChanged(WebView view, int newProgress) {
                    super.onProgressChanged(view, newProgress);
                    if (newProgress == 100) {
                        // 加载完成，隐藏进度条
                        mLoadingProgress.setVisibility(View.GONE);
                    } else {
                        // 加载中，显示进度条并更新进度
                        mLoadingProgress.setVisibility(View.VISIBLE);
                        mLoadingProgress.setProgress(newProgress);
                    }
                }
            }
        );
    }
}
