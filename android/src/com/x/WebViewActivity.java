package com.x;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.widget.Button;
import android.Manifest;
import android.content.pm.PackageManager;
import android.content.Intent;
import androidx.core.app.ActivityCompat;

public class WebViewActivity extends Activity {
    private WebView mWebView;
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // 加载布局文件
        setContentView(R.layout.activity_webview);
        
        // 检查存储权限
        verifyStoragePermissions(this);
        
        // 初始化WebView
        mWebView = findViewById(R.id.webview);
        
        // 配置WebView
        WebSettings webSettings = mWebView.getSettings();
        webSettings.setJavaScriptEnabled(true);
        webSettings.setAllowFileAccess(true);
        webSettings.setAllowFileAccessFromFileURLs(true);
        webSettings.setAllowUniversalAccessFromFileURLs(true);
        
        // 加载本地HTML文件
        loadLocalHtmlFile();

        // 关闭按钮逻辑（返回主Activity）
        Button closeButton = findViewById(R.id.close_button);
        closeButton.setOnClickListener(v -> finish()); // 关闭当前Activity
    }
    
    // 加载本地HTML文件的方法
    public void loadLocalHtmlFile() {
        String filePath = "file:///storage/emulated/0/KnotData/memo.html";
        mWebView.loadUrl(filePath);
    }
    
    // 验证存储权限
    public static void verifyStoragePermissions(Activity activity) {
        int permission = ActivityCompat.checkSelfPermission(activity, Manifest.permission.READ_EXTERNAL_STORAGE);
        if (permission != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                activity,
                PERMISSIONS_STORAGE,
                REQUEST_EXTERNAL_STORAGE
            );
        }
    }
    
    // 供Qt或主Activity调用的静态方法：启动当前Activity
    public static void openLocalHtml(Activity parentActivity) {
        // 通过Intent启动WebViewActivity
        Intent intent = new Intent(parentActivity, WebViewActivity.class);
        parentActivity.startActivity(intent);
    }
}
    