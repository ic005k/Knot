
package com.x;

import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.xhh.pdfui.PDFActivity;
import com.xhh.pdfui.UIUtils;

import android.view.WindowManager;
import android.view.Window;

import android.text.TextUtils;
import android.content.IntentFilter;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.content.Context;


public class MyPDF extends AppCompatActivity {

    private final static int REQUEST_CODE = 42;
    public static final int PERMISSION_CODE = 42042;
    public static final String READ_EXTERNAL_STORAGE = "android.permission.READ_EXTERNAL_STORAGE";

    Uri uri;

    Button btn_back, btn_file, btn_assets;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        setTheme(R.style.AppThemeprice);
        super.onCreate(savedInstanceState);
        // UIUtils.initWindowStyle(getWindow(), getSupportActionBar());
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.mypdf);

        initView();
        setEvent();

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

    private void initView() {
        btn_back = findViewById(R.id.btn_back);
        btn_file = findViewById(R.id.btn_file);
        btn_assets = findViewById(R.id.btn_assets);
    }

    private void setEvent() {
        btn_back.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                MyPDF.this.finish();
            }
        });
        btn_file.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                pickFile();
            }
        });
        btn_assets.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MyPDF.this, PDFActivity.class);
                intent.putExtra("AssetsPdf", "swift.pdf");
                MyPDF.this.startActivity(intent);
            }
        });
    }

    void pickFile() {
        int permissionCheck = ContextCompat.checkSelfPermission(this, READ_EXTERNAL_STORAGE);
        if (permissionCheck != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this,
                    new String[] { READ_EXTERNAL_STORAGE },
                    PERMISSION_CODE);
            return;
        }
        launchPicker();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String permissions[],
            @NonNull int[] grantResults) {
        if (requestCode == PERMISSION_CODE) {
            if (grantResults.length > 0
                    && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                launchPicker();
            }
        }
    }

    void launchPicker() {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("application/pdf");
        try {
            startActivityForResult(intent, REQUEST_CODE);
        } catch (ActivityNotFoundException e) {
            Toast.makeText(this, "Unable to pick file. Check status of file manager.", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        super.onActivityResult(requestCode, resultCode, intent);
        if (resultCode == RESULT_OK) {
            uri = intent.getData();
            Intent i = new Intent(this, PDFActivity.class);
            i.setData(uri);
            this.startActivity(i);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mHomeKeyEvent);

    }
}
