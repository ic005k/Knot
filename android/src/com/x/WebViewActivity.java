package com.x;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.webkit.JavascriptInterface;
import android.webkit.WebResourceRequest;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Button;
import android.view.Window;
import androidx.core.app.ActivityCompat;
import android.Manifest;

public class WebViewActivity extends Activity {
    private static final String TAG = "WebViewActivity"; // 调试标签
    private WebView mWebView;
    private static final int REQUEST_EDIT = 1;
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static final int REQUEST_CALL_PERMISSION = 2;
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    // 电话权限（用于拨号界面操作）
    private static String[] PERMISSIONS_CALL = {
            Manifest.permission.CALL_PHONE
    };

    // 用于存储页面位置的SharedPreferences
    private SharedPreferences scrollPositionPrefs;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.activity_webview);

        // 初始化SharedPreferences
        scrollPositionPrefs = getSharedPreferences("WebViewScrollPositions", MODE_PRIVATE);

        // 检查存储权限
        verifyStoragePermissions(this);
        // 检查电话权限
        verifyCallPermissions(this);

        // 初始化WebView
        mWebView = findViewById(R.id.webview);

        // 配置WebView
        WebSettings webSettings = mWebView.getSettings();
        webSettings.setJavaScriptEnabled(true);
        webSettings.setAllowFileAccess(true);
        webSettings.setAllowFileAccessFromFileURLs(true);
        webSettings.setAllowUniversalAccessFromFileURLs(true);
        webSettings.setJavaScriptCanOpenWindowsAutomatically(true);
        // 启用DOM存储API
        webSettings.setDomStorageEnabled(true);

        // 添加JavaScript接口
        mWebView.addJavascriptInterface(new WebAppInterface(), "AndroidInterface");

        // 设置自定义WebViewClient
        mWebView.setWebViewClient(new CustomWebViewClient());

        // 加载本地HTML文件
        loadLocalHtmlFile();

        // 关闭按钮逻辑
        Button closeButton = findViewById(R.id.close_button);
        closeButton.setOnClickListener(v -> finish());

        Button editButton = findViewById(R.id.edit_button);
        editButton.setOnClickListener(v -> openEdit());

        // 后退按钮
        Button backButton = findViewById(R.id.back_button);
        backButton.setOnClickListener(v -> webGoBack());

        // 前进按钮
        Button forwardButton = findViewById(R.id.forward_button);
        forwardButton.setOnClickListener(v -> webGoForward());
    }

    // JavaScript接口
    public class WebAppInterface {
        // 打开图片
        @JavascriptInterface
        public void openImage(String url) {
            String localPath = url.replace("file://", "");
            MyActivity.strImageFile = localPath;
            Log.d(TAG, "打开图片路径: " + localPath);
            Intent intent = new Intent(WebViewActivity.this, ImageViewerActivity.class);
            startActivity(intent);
        }

        // 拨打电话（打开拨号准备界面）
        @JavascriptInterface
        public void makeCall(String phoneNumber) {
            String cleanNumber = phoneNumber.replaceAll("[^0-9+]", "");
            Log.d(TAG, "准备拨打号码: " + cleanNumber);
            
            // 使用ACTION_DIAL打开拨号界面（而非直接拨号）
            Intent intent = new Intent(Intent.ACTION_DIAL, Uri.parse("tel:" + cleanNumber));
            startActivity(intent);
        }

        // 发送邮件（恢复邮件处理功能）
        @JavascriptInterface
        public void sendEmail(String emailAddress) {
            Log.d(TAG, "准备发送邮件到: " + emailAddress);
            Intent intent = new Intent(Intent.ACTION_SENDTO, Uri.parse("mailto:" + emailAddress));
            startActivity(Intent.createChooser(intent, "选择邮件客户端"));
        }

        // 保存滚动位置到SharedPreferences
        @JavascriptInterface
        public void saveScrollPosition(int scrollY) {
            if (MyActivity.strMDFile != null) {
                SharedPreferences.Editor editor = scrollPositionPrefs.edit();
                editor.putInt(MyActivity.strMDFile, scrollY);
                editor.apply();
                Log.d(TAG, "保存滚动位置: " + scrollY + " 对于文件: " + MyActivity.strMDFile);
            }
        }
    }

    // 自定义WebViewClient（处理链接打开方式）
    private class CustomWebViewClient extends WebViewClient {
        @Override
        public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request) {
            String url = request.getUrl().toString();
            Log.d(TAG, "点击链接: " + url);

            // 处理电话号码（打开拨号界面）
            if (url.startsWith("tel:")) {
                String phoneNumber = url.substring(4);
                new WebAppInterface().makeCall(phoneNumber);
                return true;
            }
            // 恢复处理邮件地址链接
            else if (url.startsWith("mailto:")) {
                String email = url.substring(7);
                new WebAppInterface().sendEmail(email);
                return true;
            }
            // 处理图片
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

        // 判断是否为图片URL
        private boolean isImageUrl(String url) {
            String lowerUrl = url.toLowerCase();
            return lowerUrl.endsWith(".jpg") || lowerUrl.endsWith(".jpeg")
                    || lowerUrl.endsWith(".png") || lowerUrl.endsWith(".gif")
                    || lowerUrl.endsWith(".bmp") || lowerUrl.endsWith(".webp");
        }

        @Override
        public void onPageFinished(WebView view, String url) {
            super.onPageFinished(view, url);
            Log.d(TAG, "页面加载完成，开始处理内容");

            // 恢复滚动位置
            if (MyActivity.strMDFile != null) {
                int scrollY = scrollPositionPrefs.getInt(MyActivity.strMDFile, 0);
                if (scrollY > 0) {
                    String js = "window.scrollTo(0, " + scrollY + ");";
                    mWebView.evaluateJavascript(js, null);
                    Log.d(TAG, "恢复滚动位置: " + scrollY + " 对于文件: " + MyActivity.strMDFile);
                }
            }

            // 延迟执行JS处理（仅保留图片处理）
            mWebView.postDelayed(() -> processContent(), 500);

            // 添加滚动监听，实时保存位置
            String scrollListenerJs = "window.onscroll = function() {" +
                    "   AndroidInterface.saveScrollPosition(window.scrollY);" +
                    "};";
            mWebView.evaluateJavascript(scrollListenerJs, null);
        }
    }

    // 处理内容，仅保留图片处理逻辑
    private void processContent() {
        String js = "javascript:(function() {" +
                // 仅处理图片点击
                "var images = document.getElementsByTagName('img');" +
                "for (var i = 0; i < images.length; i++) {" +
                "    var img = images[i];" +
                "    img.style.cursor = 'pointer';" +
                "    img.style.maxWidth = '100%';" +
                "    if (!img.src.startsWith('http') && !img.src.startsWith('file://')) {" +
                "        var baseUrl = window.location.href;" +
                "        img.src = new URL(img.src, baseUrl).href;" +
                "    }" +
                "    img.onclick = function() {" +
                "        AndroidInterface.openImage(this.src);" +
                "    };" +
                "}" +
                "console.log('内容处理完成，图片数量: ' + images.length);" +
                "})()";

        mWebView.evaluateJavascript(js, result -> {
            Log.d(TAG, "JS执行结果: " + result);
        });
    }

    // 加载本地HTML文件的方法
    public void loadLocalHtmlFile() {
        String filePath = "file:///storage/emulated/0/KnotData/memo.html";
        Log.d(TAG, "加载HTML文件: " + filePath);
        mWebView.loadUrl(filePath);
    }

    // 验证存储权限
    public static void verifyStoragePermissions(Activity activity) {
        int permission = ActivityCompat.checkSelfPermission(activity, Manifest.permission.READ_EXTERNAL_STORAGE);
        if (permission != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                    activity,
                    PERMISSIONS_STORAGE,
                    REQUEST_EXTERNAL_STORAGE);
        }
    }

    // 验证电话权限
    public static void verifyCallPermissions(Activity activity) {
        int permission = ActivityCompat.checkSelfPermission(activity, Manifest.permission.CALL_PHONE);
        if (permission != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                    activity,
                    PERMISSIONS_CALL,
                    REQUEST_CALL_PERMISSION);
        }
    }

    public static void openLocalHtml(Activity parentActivity) {
        Intent intent = new Intent(parentActivity, WebViewActivity.class);
        parentActivity.startActivity(intent);
    }

    private void openEdit() {
        MyActivity.isEdit = true;
        Intent i = new Intent(this, NoteEditor.class);
        startActivityForResult(i, REQUEST_EDIT);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_CALL_PERMISSION) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                Log.d(TAG, "电话权限已授予");
            }
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_EDIT && resultCode == RESULT_OK) {
            loadLocalHtmlFile();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        // 在页面暂停时强制保存一次滚动位置
        mWebView.evaluateJavascript("AndroidInterface.saveScrollPosition(window.scrollY);", null);
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
}
