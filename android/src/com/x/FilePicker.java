package com.x;

import org.ini4j.Wini;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;
import android.app.Application;
import android.content.Context;
import android.content.ContentUris;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.widget.AdapterView;
import org.apache.http.entity.FileEntity;

import androidx.appcompat.view.ContextThemeWrapper;
import androidx.core.content.ContextCompat;
import android.graphics.Typeface;
import android.widget.ProgressBar;
import androidx.appcompat.app.AlertDialog;
import android.os.AsyncTask;
import android.view.LayoutInflater;
import android.view.Gravity;
import android.content.IntentFilter;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.app.PendingIntent;
import android.text.TextUtils;
import java.lang.CharSequence;
import android.app.Service;
import android.content.DialogInterface;
import android.graphics.Color;
import android.media.MediaPlayer;
import android.os.Build;
import android.os.Bundle;
import android.app.Activity;
import android.appwidget.AppWidgetManager;
import android.content.Context;
import android.appwidget.AppWidgetProvider;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.TextView;
import android.view.WindowManager;
import android.view.Window;
import android.widget.EditText;
import android.text.Editable;
import android.net.Uri;
import java.io.OutputStreamWriter;
import java.io.BufferedWriter;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Properties;
import java.io.IOException;
import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import android.text.TextWatcher;
import android.os.Handler;
import android.media.AudioManager;
import android.widget.TextView;
import android.content.DialogInterface;
import java.util.Locale;
import android.app.Application;
import android.os.Looper;
import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import android.provider.MediaStore;
import android.provider.DocumentsContract;
import android.database.Cursor;
import android.content.ContentUris;
import android.os.Environment;
import android.os.Message;
import android.view.Menu;
import android.widget.Toast;
import java.lang.reflect.Method;
import java.net.URI;
import java.net.URISyntaxException;
import java.sql.Time;
import android.text.SpannableStringBuilder;
import android.text.style.BackgroundColorSpan;
import android.text.style.ForegroundColorSpan;
import android.text.Spannable;
import android.text.Spanned;
import android.view.MenuItem;
import android.net.Uri;
import android.text.method.ScrollingMovementMethod;
import android.view.inputmethod.EditorInfo;
import android.view.KeyEvent;
import android.widget.TextView.OnEditorActionListener;
import android.view.inputmethod.InputMethodManager;
import java.util.regex.*;

import android.widget.ImageView;
import android.media.MediaScannerConnection;
import android.widget.LinearLayout;
import android.view.LayoutInflater;
import android.widget.PopupWindow;
import android.graphics.drawable.ColorDrawable;
import java.lang.reflect.Field;
import android.annotation.SuppressLint;
import android.content.ContentResolver;
import android.widget.ListView;
import java.io.File;
import java.net.URISyntaxException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

public class FilePicker extends Activity implements View.OnClickListener, Application.ActivityLifecycleCallbacks {

    public static Context MyContex;
    private ContentResolver mContentResolver;
    private List<String> files;
    private List<String> filesInfo;
    private List<Fruit> fruitlist;
    private ListView m_ListView;
    private TextView lblResult;
    private Button btnFind;
    private ImageButton btn_clear;
    private EditText editFind;
    public ProgressBar mProgressBar;
    private String filePath;
    public static FilePicker MyFilepicker;
    private boolean isDark = false;

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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        MyContex = FilePicker.this;
        MyFilepicker = this;
        mContentResolver = MyContex.getContentResolver();
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        isDark = MyActivity.isDark;
        if (isDark) {
            this.setStatusBarColor("#19232D"); // 深色
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
            setContentView(R.layout.myfilepicker_dark);
        } else {
            this.setStatusBarColor("#F3F3F3"); // 灰
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
            setContentView(R.layout.myfilepicker);
        }

        btn_clear = (ImageButton) findViewById(R.id.btn_clear);
        btn_clear.setOnClickListener(this);

        btnFind = (Button) findViewById(R.id.btnFind);
        btnFind.setOnClickListener(this);

        lblResult = (TextView) findViewById(R.id.lblResult);
        editFind = (EditText) findViewById(R.id.editFind);

        m_ListView = (ListView) findViewById(R.id.listView);

        mProgressBar = (ProgressBar) findViewById(R.id.progBar);
        mProgressBar.setVisibility(View.VISIBLE);

        btnFind.setVisibility(View.GONE);

        // HomeKey
        registerReceiver(mHomeKeyEvent, new IntentFilter(Intent.ACTION_CLOSE_SYSTEM_DIALOGS));
        initEditTextChangedListener();

        MyAsyncTask myAsyncTask = new MyAsyncTask();
        myAsyncTask.execute();

    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        CallJavaNotify_12();

    }

    @Override
    protected void onDestroy() {
        System.out.println("FilePicker onDestroy...");

        unregisterReceiver(mHomeKeyEvent);
        getApplication().unregisterActivityLifecycleCallbacks(this); // 注销回调
        super.onDestroy();
    }

    @Override
    public void onActivityCreated(Activity activity, Bundle savedInstanceState) {

    }

    @Override
    public void onActivityStarted(Activity activity) {

    }

    @Override
    public void onActivityResumed(Activity activity) {

    }

    @Override
    public void onActivityPaused(Activity activity) {

    }

    @Override
    public void onActivityStopped(Activity activity) {
        System.out.println("NoteEditor onActivityStopped...");
    }

    @Override
    public void onActivitySaveInstanceState(Activity activity, Bundle outState) {

    }

    @Override
    public void onActivityDestroyed(Activity activity) {

    }

    /**
     * 获取所有文件
     **/
    public List<String> getFilesByType() {
        files = new ArrayList<>();
        filesInfo = new ArrayList<>();
        // 扫描files文件库
        Cursor c = null;
        try {
            String select = "(" + MediaStore.Files.FileColumns.DATA + " LIKE '%.txt'" + " or "
                    + MediaStore.Files.FileColumns.DATA + " LIKE '%.epub'" + " or " + MediaStore.Files.FileColumns.DATA
                    + " LIKE '%.pdf'" + ")";
            c = mContentResolver.query(MediaStore.Files.getContentUri("external"), null, select, null, null);
            int columnIndexOrThrow_ID = c.getColumnIndexOrThrow(MediaStore.Files.FileColumns._ID);
            int columnIndexOrThrow_MIME_TYPE = c.getColumnIndexOrThrow(MediaStore.Files.FileColumns.MIME_TYPE);
            int columnIndexOrThrow_DATA = c.getColumnIndexOrThrow(MediaStore.Files.FileColumns.DATA);
            int columnIndexOrThrow_SIZE = c.getColumnIndexOrThrow(MediaStore.Files.FileColumns.SIZE);
            // 更改时间
            int columnIndexOrThrow_DATE_MODIFIED = c.getColumnIndexOrThrow(MediaStore.Files.FileColumns.DATE_MODIFIED);

            int tempId = 0;
            while (c.moveToNext()) {
                String path = c.getString(columnIndexOrThrow_DATA);
                String minType = c.getString(columnIndexOrThrow_MIME_TYPE);

                int position_do = path.lastIndexOf(".");
                if (position_do == -1) {
                    continue;
                }
                int position_x = path.lastIndexOf(File.separator);
                if (position_x == -1) {
                    continue;
                }
                String displayName = path.substring(position_x + 1, path.length());
                long size = c.getLong(columnIndexOrThrow_SIZE);
                long modified_date = c.getLong(columnIndexOrThrow_DATE_MODIFIED);
                File file = new File(path);
                String strFileSize = ShowLongFileSzie(size);
                String time = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date(file.lastModified()));
                String fileInfo = time + "  " + strFileSize;

                /*
                 * FileEntity info = new FileEntity(file);
                 * info.setName(displayName);
                 * info.setPath(path);
                 * info.setSize(ShowLongFileSzie(size));
                 * info.setId((tempId++) + "");
                 * info.setTime(time);
                 * files.add(info);
                 */

                System.out.println("FileManager" + " path:" + path + "  size:" + strFileSize);

                if (path.endsWith(".epub")) {
                    files.add(path);
                    filesInfo.add(fileInfo);
                } else if (path.endsWith(".txt") && size > 0) {
                    files.add(path);
                    filesInfo.add(fileInfo);
                } else if (path.endsWith(".pdf")) {
                    files.add(path);
                    filesInfo.add(fileInfo);
                }

            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (c != null) {
                c.close();
            }
        }
        System.out.println(files);
        return files;
    }

    private void setStatusBarColor(String color) {
        // 需要安卓版本大于5.0以上
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            getWindow().setStatusBarColor(Color.parseColor(color));
        }
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

                } else if (TextUtils.equals(reason, SYSTEM_HOME_KEY_LONG)) {
                    // 表示长按home键,显示最近使用的程序
                    System.out.println("NoteEditor 长按HOME键...");

                }
            }
        }
    };

    public class MyAsyncTask extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            // 在这里执行需要等待的操作
            // 刷新媒体库
            MediaScannerConnection.scanFile(
                    MyContex,
                    new String[] { "/storage/emulated/0/" },
                    new String[] { "text/plain", "application/epub+zip", "application/pdf" },
                    new MediaScannerConnection.OnScanCompletedListener() {
                        public void onMediaScannerConnected() {
                        }

                        public void onScanCompleted(String path, Uri uri) {
                            System.out.println("Scan done...");

                        }
                    });
            getFilesByType();

            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {

            // 在这里执行完成后的操作
            addToListView(files, filesInfo);

            editFind.requestFocus();
            hideKeyBoard(MyFilepicker);
        }
    }

    public String ShowLongFileSzie(Long length) {
        if (length >= 1048576) {
            return (length / 1048576) + "MB";
        } else if (length >= 1024) {
            return (length / 1024) + "KB";
        } else if (length < 1024) {
            return length + "B";
        } else {
            return "0KB";
        }
    }

    public static void closeFilePickerView() {
        if (MyFilepicker != null)
            MyFilepicker.finish();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {

            case R.id.btnFind:
                btnFind.setBackgroundColor(getResources().getColor(R.color.red));

                btnFind.setBackgroundColor(getResources().getColor(R.color.normal));

                break;

            case R.id.btn_clear:
                editFind.setText("");
                editFind.requestFocus();

                break;

        }
    }

    private void initEditTextChangedListener() {

        editFind.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

            }

            @Override
            public void afterTextChanged(Editable editable) {
                if (files.size() == 0)
                    return;

                List<String> findResult = new ArrayList<>();
                List<String> findResultInfo = new ArrayList<>();
                String strFind = editFind.getText().toString();
                for (int i = 0; i < files.size(); i++) {
                    String strItem = files.get(i);
                    String strInfo = filesInfo.get(i);
                    String a, b;
                    a = strItem.toLowerCase();
                    b = strFind.toLowerCase();
                    if (a.contains(b)) {
                        findResult.add(strItem);
                        findResultInfo.add(strInfo);

                    }

                }

                addToListView(findResult, findResultInfo);

                if (strFind == "") {
                    addToListView(files, filesInfo);
                }

            }
        });
    }

    private void addToListView(List<String> files, List<String> filesInfo) {
        fruitlist = new ArrayList<>();
        for (int i = 0; i < files.size(); i++) {
            String filePath = files.get(i);
            String fileInfo = filesInfo.get(i);
            if (filePath.endsWith(".epub")) {
                Fruit epubBook = new Fruit(R.drawable.epub, filePath, fileInfo);
                fruitlist.add(epubBook);
            }

            if (filePath.endsWith(".txt")) {
                Fruit txtBook = new Fruit(R.drawable.text, filePath, fileInfo);
                fruitlist.add(txtBook);
            }

            if (filePath.endsWith(".pdf")) {
                Fruit pdfBook = new Fruit(R.drawable.pdf, filePath, fileInfo);
                fruitlist.add(pdfBook);
            }
        }

        FruitAdapter adapter;
        if (isDark)
            adapter = new FruitAdapter(MyContex, R.layout.fruit_item_dark, fruitlist);
        else
            adapter = new FruitAdapter(MyContex, R.layout.fruit_item, fruitlist);
        m_ListView.setAdapter(adapter);

        mProgressBar.setVisibility(View.GONE);

        // listview 的点击事件
        m_ListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            // @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int position, long id) {
                Fruit fruit = fruitlist.get(position);

                filePath = fruit.getName();
                String strFileSize = fruit.getPrice();

                File file_path = new File(filePath);
                if (file_path.exists()) {
                    String filename = "/storage/emulated/0/.Knot/choice_book.ini";
                    try {
                        File file = new File(filename);
                        if (!file.exists())
                            file.createNewFile();
                        Wini ini = new Wini(file);
                        ini.put("book", "file", filePath);
                        ini.put("book", "type", "filepicker");
                        ini.store();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    Toast.makeText(MyContex, filePath + "   Size:" + strFileSize, Toast.LENGTH_LONG).show();
                    CallJavaNotify_9();
                }

                FilePicker.this.finish();

            }
        });

        lblResult.setText(String.valueOf(files.size()));

    }

    /**
     * 隐藏软键盘，要防止报空指针
     */
    public static void hideKeyBoard(Activity activity) {
        // 拿到InputMethodManager
        InputMethodManager imm = (InputMethodManager) activity.getSystemService(Context.INPUT_METHOD_SERVICE);
        // 如果window上view获取焦点 && view不为空
        if (imm.isActive() && activity.getCurrentFocus() != null) {
            // 拿到view的token 不为空
            if (activity.getCurrentFocus().getWindowToken() != null) {
                // 表示软键盘窗口总是隐藏，除非开始时以SHOW_FORCED显示。
                imm.hideSoftInputFromWindow(activity.getCurrentFocus().getWindowToken(),
                        InputMethodManager.HIDE_NOT_ALWAYS);
            }
        }
    }

}
