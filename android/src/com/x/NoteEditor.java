package com.x;

import com.x.LargeTextEditor;
import com.x.MyActivity;
import com.x.MDActivity;
import com.x.TextViewUndoRedo;
import com.x.PopupMenuCustomLayout;
import com.x.LineNumberedEditText;

// 读写ini文件的三方开源库
import org.ini4j.Wini;

import com.flask.colorpicker.ColorPickerView;
import com.flask.colorpicker.OnColorChangedListener;
import com.flask.colorpicker.OnColorSelectedListener;
import com.flask.colorpicker.builder.ColorPickerClickListener;
import com.flask.colorpicker.builder.ColorPickerDialogBuilder;

import io.noties.markwon.Markwon;
import io.noties.markwon.editor.MarkwonEditor;
import io.noties.markwon.editor.MarkwonEditorTextWatcher;

import androidx.appcompat.view.ContextThemeWrapper;
import androidx.core.content.ContextCompat;
import android.graphics.Typeface;

import androidx.appcompat.app.AlertDialog;

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
import android.widget.TextView;
import android.view.WindowManager;
import android.view.Window;
import android.widget.EditText;
import android.text.Editable;
import android.net.Uri;
import java.io.OutputStreamWriter;
import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStreamReader;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicInteger;
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
import java.nio.charset.Charset;
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
import android.widget.LinearLayout;
import android.view.LayoutInflater;
import android.widget.PopupWindow;
import android.graphics.drawable.ColorDrawable;
import java.lang.reflect.Field;
import android.annotation.SuppressLint;
import androidx.core.content.FileProvider;
import android.widget.PopupMenu;
import android.widget.ImageButton;
import android.util.TypedValue;
import android.widget.ProgressBar;
import android.content.SharedPreferences;

import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import android.widget.ScrollView;

import android.graphics.Bitmap;
import android.media.ExifInterface;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;

public class NoteEditor extends Activity implements View.OnClickListener, Application.ActivityLifecycleCallbacks {

    private MediaPlayer mediaPlayer;
    private static int curVol;
    private static String strInfo = "";
    private String strMute = "false";
    private boolean isRefreshAlarm = true;
    private AudioManager mAudioManager;
    private InternalConfigure internalConfigure;

    private Button btn_cancel;
    private Button btnUndo;
    private Button btnRedo;
    private Button btnMenu;

    private Button btnFind;
    private Button btnPrev;
    private Button btnNext;
    private ImageButton btnStartFind;

    private int myMethod = 1; /* 操控文件的方法 1.传统方法 2.新方法 */
    private LargeTextEditor largeTextEditor;
    private RecyclerView recyclerView;

    public static EditText editNote;
    public static EditText editFind;
    private EditText replaceEditText;
    public static TextView lblResult;
    private ArrayList<Integer> arrayFindResult = new ArrayList<Integer>();
    private int curIndexForResult = 0;
    private View mColorPreview;

    private boolean isAddImage = false;

    private String currentMDFile;
    private static Context context;
    public static NoteEditor m_instance;
    private static boolean isTextChanged = false;
    private TextViewUndoRedo helper;
    private String strBack1 = "#FFC1C1";
    private String strBack2 = "#CFCFCF";
    private String strFore = "#000000";
    private int start = 0;
    private int end = 0;
    private ArrayList<String> listMenuTitle = new ArrayList<>();

    // 用于存储拍照后的图片Uri
    private Uri photoUri;

    // 拍照请求码
    private static final int REQUEST_TAKE_PHOTO = 1;

    boolean isImageFile = true;

    public static Context getContext() {
        return context;
    }

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

    private static boolean isGoBackKnot = false;

    private ProgressBar progressBar;

    public static int setInfoText(String str) {
        strInfo = str;
        System.out.println("InfoText" + strInfo);
        return 1;
    }

    private void bindViews() {
        ScrollView scrollView = findViewById(R.id.scrollView);
        recyclerView = findViewById(R.id.recyclerView);

        editNote = (EditText) findViewById(R.id.editNote);
        editNote.setTextSize(TypedValue.COMPLEX_UNIT_SP, MyActivity.myFontSize);

        if (myMethod == 2) {
            scrollView.setVisibility(View.GONE);
            editNote.setVisibility(View.GONE);
        }

        if (myMethod == 1)
            recyclerView.setVisibility(View.GONE);

        String str_file = MyActivity.strMDFile;
        File file = new File(str_file);
        if (getFileSizeInKB(file) < 200) {
            // 初始化 Markwon
            final Markwon markwon = Markwon.create(context);
            // 初始化 MarkwonEditor
            final MarkwonEditor editor = MarkwonEditor.create(markwon);
            // 添加 MarkwonEditorTextWatcher 到 EditText
            editNote.addTextChangedListener(MarkwonEditorTextWatcher.withPreRender(
                    editor,
                    Executors.newCachedThreadPool(),
                    editNote));
        }

        btn_cancel = (Button) findViewById(R.id.btn_cancel);
        btnFind = (Button) findViewById(R.id.btnFind);
        btnUndo = (Button) findViewById(R.id.btnUndo);
        btnRedo = (Button) findViewById(R.id.btnRedo);
        btnMenu = (Button) findViewById(R.id.btnMenu);
        mColorPreview = (View) findViewById(R.id.preview_selected_color);

        editFind = (EditText) findViewById(R.id.editFind);
        editFind.setOnEditorActionListener(new OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                boolean handled = false;
                if (actionId == EditorInfo.IME_ACTION_GO) {

                    btnNext.performClick();
                    handled = true;
                }
                return handled;
            }
        });

        lblResult = (TextView) findViewById(R.id.lblResult);
        lblResult.setText("0");
        btnPrev = (Button) findViewById(R.id.btnPrev);
        btnNext = (Button) findViewById(R.id.btnNext);
        btnStartFind = (ImageButton) findViewById(R.id.btnStartFind);

        if (MyActivity.zh_cn) {
            btn_cancel.setText("关闭");
            btnFind.setText("查找");
            btnUndo.setText("撤销");
            btnRedo.setText("恢复");
            btnMenu.setText("菜单");

            btnPrev.setText("<");
            btnNext.setText(">");
        } else {
            btn_cancel.setText("Close");
            btnFind.setText("Find");
            btnUndo.setText("Undo");
            btnRedo.setText("Redo");
            btnMenu.setText("Menu");

            btnPrev.setText("<");
            btnNext.setText(">");
        }

        mColorPreview.setVisibility(View.GONE);
        editFind.setVisibility(View.GONE);
        btnPrev.setVisibility(View.GONE);
        btnNext.setVisibility(View.GONE);
        btnStartFind.setVisibility(View.GONE);
        lblResult.setVisibility(View.GONE);

        btn_cancel.setOnClickListener(this);
        btnMenu.setOnClickListener(this);

        btnFind.setOnClickListener(this);
        btnPrev.setOnClickListener(this);
        btnNext.setOnClickListener(this);
        btnStartFind.setOnClickListener(this);

    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_cancel:
                btn_cancel.setBackgroundColor(getResources().getColor(R.color.red));
                hideKeyBoard(m_instance);
                onBackPressed();
                btn_cancel.setBackgroundColor(getResources().getColor(R.color.normal));

                break;

            case R.id.btnUndo:
                btnUndo.setBackgroundColor(getResources().getColor(R.color.red));
                helper.undo(); // perform undo
                btnUndo.setBackgroundColor(getResources().getColor(R.color.normal));
                break;

            case R.id.btnRedo:
                btnRedo.setBackgroundColor(getResources().getColor(R.color.red));
                helper.redo(); // perform redo
                btnRedo.setBackgroundColor(getResources().getColor(R.color.normal));

                break;

            case R.id.btnMenu:
                btnMenu.setBackgroundColor(getResources().getColor(R.color.red));

                start = editNote.getSelectionStart();
                end = editNote.getSelectionEnd();
                editNote.clearFocus();
                editNote.setSelection(start);
                editNote.setSelection(start, end);

                showPopupMenu(btnMenu);

                editNote.requestFocus();
                btnMenu.setBackgroundColor(getResources().getColor(R.color.normal));
                break;

            case R.id.btnFind:
                btnFind.setBackgroundColor(getResources().getColor(R.color.red));
                if (btnPrev.getVisibility() == View.VISIBLE) {
                    editFind.setVisibility(View.GONE);
                    btnPrev.setVisibility(View.GONE);
                    btnNext.setVisibility(View.GONE);
                    btnStartFind.setVisibility(View.GONE);
                    lblResult.setVisibility(View.GONE);

                } else {
                    editFind.setVisibility(View.VISIBLE);
                    btnPrev.setVisibility(View.VISIBLE);
                    btnNext.setVisibility(View.VISIBLE);
                    btnStartFind.setVisibility(View.VISIBLE);
                    lblResult.setVisibility(View.VISIBLE);
                    editFind.requestFocus();
                }
                btnFind.setBackgroundColor(getResources().getColor(R.color.normal));

                break;

            case R.id.btnPrev:
                btnPrev.setBackgroundColor(getResources().getColor(R.color.red));
                goFindResult(-1);
                int count = arrayFindResult.size();
                if (count > 0) {
                    String strInfo = String.valueOf(curIndexForResult + 1) + "/" + String.valueOf(count);
                    lblResult.setText(strInfo);

                    editNote.requestFocus();
                }

                btnPrev.setBackgroundColor(getResources().getColor(R.color.normal));

                break;

            case R.id.btnNext:
                btnNext.setBackgroundColor(getResources().getColor(R.color.red));
                goFindResult(1);
                count = arrayFindResult.size();
                if (count > 0) {
                    String strInfo = String.valueOf(curIndexForResult + 1) + "/" + String.valueOf(count);
                    lblResult.setText(strInfo);

                    editNote.requestFocus();
                }

                btnNext.setBackgroundColor(getResources().getColor(R.color.normal));

                break;

            case R.id.btnStartFind:
                btnStartFind.setBackgroundColor(getResources().getColor(R.color.red));
                on_editFindTextChanged();
                btnStartFind.setBackgroundColor(getResources().getColor(R.color.normal));

                break;
        }
    }

    private void AnimationWhenClosed() {
        // 淡出效果
        // overridePendingTransition(android.R.anim.fade_in, android.R.anim.fade_out);

        // 或者使用底部滑出效果(自定义文件exit_anim.xml)
        overridePendingTransition(0, R.anim.exit_anim);
    }

    private void AnimationWhenOpen() {
        overridePendingTransition(0, R.anim.enter_anim);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        setTheme(R.style.AppThemeprice);
        super.onCreate(savedInstanceState);

        context = NoteEditor.this;

        m_instance = this;

        Application application = this.getApplication();
        application.registerActivityLifecycleCallbacks(this);

        // 去除title(App Name)
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        if (MyActivity.isDark) {
            this.setStatusBarColor("#19232D"); // 深色
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
            setContentView(R.layout.noteeditor_dark);
        } else {
            this.setStatusBarColor("#F3F3F3"); // 灰
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);

            // if (myMethod == 1)
            setContentView(R.layout.noteeditor);
            // if (myMethod == 2)
            // setContentView(R.layout.noteeditor_large);
        }

        progressBar = findViewById(R.id.progressBar);

        bindViews();

        // 监听 UI 绘制完成
        findViewById(android.R.id.content).post(() -> {

            // 启动异步任务
            if (myMethod == 1)
                loadMDFileChunks();

            if (myMethod == 2) {
                openFile();

            }
        });
    }

    private void openFile() {
        currentMDFile = MyActivity.strMDFile;

        // largeTextEditor = new LargeTextEditor(this, editNote);

        largeTextEditor = new LargeTextEditor(this, recyclerView);

        File currentFile = new File(currentMDFile);
        largeTextEditor.loadFile(currentFile);

    }

    private void loadMDFile() {
        final String mdfile = MyActivity.strMDFile; // getIntent().getStringExtra("MD_FILE_PATH");
        // 启动子线程执行耗时操作
        Handler mHandler = new Handler(Looper.getMainLooper());

        new Thread(new Runnable() {
            @Override
            public void run() {

                // 子线程读取文件
                final String data = readTextFile(mdfile);

                // 文件读取完成后，通过 Handler 发送消息到主线程
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        editNote.setText(data);
                        setCursorPos();
                        openSearchResult();

                        isTextChanged = false;
                        initRedoUndo();
                        initEditTextChangedListener();
                        init_all();

                    }
                });
            }
        }).start();

    }

    private void loadMDFileChunks() {
        final String mdfile = MyActivity.strMDFile;
        // final Handler mHandler = new Handler(Looper.getMainLooper());

        // showAndroidProgressBar();
        // progressBar.setVisibility(View.VISIBLE);
        findViewById(R.id.progressContainer).setVisibility(View.VISIBLE);

        new Thread(new Runnable() {
            @Override
            public void run() {
                // 1. 子线程读取文件
                final String data = readTextFile(mdfile);

                // 2. 将数据分块
                final int totalChunks = 50; // 分块数量（根据数据量调整）
                final int chunkSize = data.length() / totalChunks;
                final AtomicInteger currentChunk = new AtomicInteger(0);

                // 3. 分批次更新UI
                for (int i = 0; i < totalChunks; i++) {
                    final int start = i * chunkSize;
                    final int end = (i == totalChunks - 1) ? data.length() : start + chunkSize;
                    final String chunk = data.substring(start, end);

                    // 主线程追加文本并更新进度
                    // mHandler.post(new Runnable() {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if (currentChunk.get() == 0) {
                                editNote.setText(chunk); // 首次设置文本
                            } else {
                                editNote.append(chunk); // 后续追加文本
                            }
                            currentChunk.incrementAndGet();

                            // 更新进度条（假设进度条支持进度百分比）
                            // int progress = (int) ((currentChunk.get() / (float) totalChunks) * 100);
                            // updateProgressBar(progress);
                        }
                    });

                    // 控制速度（避免主线程过载）
                    try {
                        int m_sleep = 50;
                        String str_file = MyActivity.strMDFile;
                        File file = new File(str_file);
                        if (getFileSizeInKB(file) < 200)
                            m_sleep = 2;
                        else
                            m_sleep = 50;
                        Thread.sleep(m_sleep); // 调整此值以平衡流畅性与速度
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

                // 4. 最终关闭进度条并执行其他操作
                // mHandler.post(new Runnable() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        setCursorPos();
                        openSearchResult();
                        isTextChanged = false;
                        initRedoUndo();
                        initEditTextChangedListener();
                        init_all();
                        // closeAndroidProgressBar();
                        // progressBar.setVisibility(View.GONE);
                        findViewById(R.id.progressContainer).setVisibility(View.GONE);
                    }
                });
            }

        }).start();
    }

    private void setCursorPos() {
        // set cursor pos
        String filename = "/storage/emulated/0/.Knot/note_text.ini";
        if (fileIsExists(filename)) {
            String s_cpos = "";

            try {
                Wini ini = new Wini(new File(filename));
                currentMDFile = ini.get("cpos", "currentMDFile", String.class);
                s_cpos = ini.get("cpos", currentMDFile);
                if (s_cpos == null || s_cpos.isEmpty()) {
                    s_cpos = ""; // 设置默认值
                }

            } catch (IOException e) {
                e.printStackTrace();
            }

            int cpos = 0;
            if (!s_cpos.equals(""))
                cpos = Integer.parseInt(s_cpos);
            int nLength = editNote.getText().length();
            if (cpos > nLength)
                cpos = nLength;

            editNote.requestFocus();
            editNote.setSelection(cpos);
        }

    }

    private void initRedoUndo() {
        // pass edittext object to TextViewUndoRedo class
        helper = new TextViewUndoRedo(editNote);
        btnUndo.setOnClickListener(this);
        btnRedo.setOnClickListener(this);

    }

    private void init_all() {

        initColorValue();

        writeReceiveData();

        initMenuTitle();

    }

    @Override
    public void onPause() {
        System.out.println("NoteEditor onPause...");
        super.onPause();

    }

    @Override
    public void onStop() {
        System.out.println("NoteEditor onStop...");

        super.onStop();

    }

    @Override
    public void onBackPressed() {
        if (isTextChanged)
            showNormalDialog();
        else {
            super.onBackPressed();
            // AnimationWhenClosed();
        }

    }

    @Override
    protected void onDestroy() {
        // save cursor pos
        String file2 = "/storage/emulated/0/.Knot/note_text.ini";
        int cpos = editNote.getSelectionStart();

        int index = editNote.getSelectionStart();
        String strLeft = getCursorPositionText(index, -5);
        String strRight = getCursorPositionText(index, 5);
        String cursorText = String.valueOf(cpos) + "   (" + "\"" + strLeft +
                "|" + strRight + "\"" + ")";

        String mPath = "/storage/emulated/0/.Knot/cursor_text.txt";
        writeTextFile(cursorText, mPath);

        try {
            File file = new File(file2);
            if (!file.exists())
                file.createNewFile();
            Wini ini = new Wini(file);
            ini.put("cpos", currentMDFile, String.valueOf(cpos));
            ini.store();
        } catch (IOException e) {
            e.printStackTrace();
        }

        if (isTextChanged) {
            // saveNote();

        } else {
            if (MyActivity.isEdit) {

            }
        }

        MyService.clearNotify();

        getApplication().unregisterActivityLifecycleCallbacks(this); // 注销回调

        super.onDestroy();

        if (myMethod == 2) {

        }

        System.out.println("NoteEditor onDestroy...");

    }

    public static void close() {

        if (m_instance != null) {
            m_instance.finish();
        }
    }

    public class InternalConfigure {
        private final Context context;
        private Properties properties;

        public InternalConfigure(Context context) {
            super();
            this.context = context;
        }

        /**
         * 保存文件filename为文件名，filecontent为存入的文件内容
         * 例:configureActivity.saveFiletoSD("text.ini","");
         */
        public void saveFile(String filename, Properties properties) throws Exception {
            // 设置Context.MODE_PRIVATE表示每次调用该方法会覆盖原来的文件数据
            FileOutputStream fileOutputStream;// = context.openFileOutput(filename, Context.MODE_PRIVATE);

            // 每次都会生成一个新文件并抹掉除本次key之外的其他key数据
            // File file = new File(filename);
            fileOutputStream = new FileOutputStream(filename);

            // 通过properties.stringPropertyNames()获得所有key的集合Set，里面是String对象
            for (String key : properties.stringPropertyNames()) {
                String s = key + " = " + properties.getProperty(key) + "\n";
                System.out.println(s);
                fileOutputStream.write(s.getBytes());
            }
            fileOutputStream.close();
        }

        /**
         * 读取文件
         */
        public void readFrom(String filename) throws Exception {
            properties = new Properties();

            FileInputStream fileInputStream;// = context.openFileInput(filename);

            File file = new File(filename);
            fileInputStream = new FileInputStream(file);

            InputStreamReader reader = new InputStreamReader(fileInputStream, "UTF-8");
            BufferedReader br = new BufferedReader(reader);

            properties.load(br);

            br.close();
            reader.close();
            fileInputStream.close();
        }

        /**
         * 返回指定key对应的value
         */
        public String getIniKey(String key) {
            if (properties.containsKey(key) == false) {
                return null;
            }
            return String.valueOf(properties.get(key));
        }
    }

    // fileName 为文件名称 返回true为存在
    public boolean fileIsExists(String fileName) {
        try {
            File f = new File(fileName);
            if (f.exists()) {
                Log.i("测试", "文件存在：" + fileName);
                return true;
            } else {
                Log.i("测试", "没有这个文件: " + fileName);
                return false;
            }
        } catch (Exception e) {
            Log.i("测试", "出现异常！");
            e.printStackTrace();
            return false;
        }
    }

    private void setStatusBarColor(String color) {
        // 需要安卓版本大于5.0以上
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            getWindow().setStatusBarColor(Color.parseColor(color));
        }
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

    /*
     * public String readTextFile(String filename) {
     * try {
     * File file = new File(filename);
     * StringBuffer strBuf = new StringBuffer();
     * BufferedReader bufferedReader = new BufferedReader(
     * new InputStreamReader(new FileInputStream(file), "UTF-8"));
     * int tempchar;
     * while ((tempchar = bufferedReader.read()) != -1) {
     * strBuf.append((char) tempchar);
     * }
     * bufferedReader.close();
     * return strBuf.toString();
     * } catch (Exception ex) {
     * ex.printStackTrace();
     * }
     * return "";
     * }
     */

    public String readTextFile(String filename) {
        BufferedReader reader = null;
        try {
            File file = new File(filename);
            reader = new BufferedReader(
                    new InputStreamReader(
                            new FileInputStream(file),
                            Charset.defaultCharset() // ✅ 自动适配系统编码
                    ));

            char[] buffer = new char[8192]; // 8KB 缓冲区
            StringBuilder sb = new StringBuilder();
            int charsRead;
            while ((charsRead = reader.read(buffer)) != -1) {
                sb.append(buffer, 0, charsRead);
            }
            return sb.toString();
        } catch (Exception ex) {
            ex.printStackTrace();
            return "";
        } finally {
            try {
                if (reader != null)
                    reader.close(); // ✅ 确保关闭
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public void writeTextFile(String content, String filename) {
        try {
            File file = new File(filename);

            if (file.exists()) {
                file.delete();
            }
            file.createNewFile();
            // 获取该文件的缓冲输出流
            BufferedWriter bufferedWriter = new BufferedWriter(
                    new OutputStreamWriter(new FileOutputStream(file), "UTF-8"));
            // 写入信息
            bufferedWriter.write(content);
            bufferedWriter.flush();// 清空缓冲区
            bufferedWriter.close();// 关闭输出流
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    // 生成文件
    public File makeFile(String filePath, String fileName) {
        File file = null;
        makeDirectory(filePath);
        try {
            file = new File(filePath + fileName);
            if (!file.exists()) {
                file.createNewFile();
            }
        } catch (IOException e) {
            System.out.println("生成文件错误: " + e.toString());
        }
        return file;
    }

    // 生成文件夹
    public void makeDirectory(String filePath) {
        File file = null;
        try {
            file = new File(filePath);
            if (!file.exists()) {
                file.mkdir();
            }
        } catch (Exception e) {
            System.out.println("生成文件夹错误: " + e.toString());
        }
    }

    public static void appendNote(String str) {
        Editable edit = editNote.getEditableText();
        edit.append("\n\n");
        edit.append(str);
        int cpos = editNote.getText().length();
        editNote.setSelection(cpos);

    }

    public static void insertNote(String str) {

        int index = editNote.getSelectionStart();// 获取光标所在位置
        Editable edit = editNote.getEditableText();// 获取EditText的文字
        if (index < 0 || index >= edit.length()) {
            edit.append(str);
        } else {
            edit.insert(index, str);// 光标所在位置插入文字
        }

    }

    private void saveNote() {
        final String mContent = editNote.getText().toString();
        final String filename = MyActivity.strMDFile;

        // showAndroidProgressBar();
        // progressBar.setVisibility(View.VISIBLE); // 显示进度条
        findViewById(R.id.progressContainer).setVisibility(View.VISIBLE);

        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    writeTextFile(mContent, filename);

                    CallJavaNotify_6();

                } finally {

                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            // closeAndroidProgressBar();
                            // progressBar.setVisibility(View.GONE);
                            findViewById(R.id.progressContainer).setVisibility(View.GONE);

                            if (MyActivity.isEdit) {
                                setResult(MDActivity.RESULT_SAVE);
                            }
                            finish();

                        }
                    });

                }
            }
        }).start();
    }

    private void openFilePicker() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.setType("image/*"); // "text/plain" 只显示 txt 文件
        // intent.setType("application/epub+zip");
        startActivityForResult(intent, 1);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == RESULT_OK && requestCode == 1) {
            Uri selectedFileUri;
            if (isImageFile) {
                selectedFileUri = data.getData();
            } else {
                String photoPath = photoUri.getPath(); // test
                selectedFileUri = photoUri;
            }

            String filePath = "/storage/emulated/0/.Knot/receive_share_pic.png";
            readFileFromUriToLocal(selectedFileUri, filePath);

            // 新增：校正图片方向
            correctImageOrientation(filePath);

            // 处理文件路径
            handleFilePath(filePath);
        }
    }

    // 新增方法：校正图片方向
    private void correctImageOrientation(String imagePath) {
        try {
            ExifInterface exif = new ExifInterface(imagePath);
            int orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_UNDEFINED);

            // 根据方向信息旋转图片
            Bitmap bitmap = BitmapFactory.decodeFile(imagePath);
            Bitmap rotatedBitmap = rotateBitmap(bitmap, orientation);

            // 保存校正后的图片
            saveBitmapToFile(rotatedBitmap, imagePath);

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    // 旋转位图的方法
    private Bitmap rotateBitmap(Bitmap bitmap, int orientation) {
        Matrix matrix = new Matrix();
        switch (orientation) {
            case ExifInterface.ORIENTATION_ROTATE_90:
                matrix.postRotate(90);
                break;
            case ExifInterface.ORIENTATION_ROTATE_180:
                matrix.postRotate(180);
                break;
            case ExifInterface.ORIENTATION_ROTATE_270:
                matrix.postRotate(270);
                break;
            case ExifInterface.ORIENTATION_FLIP_HORIZONTAL:
                matrix.postScale(-1, 1);
                break;
            case ExifInterface.ORIENTATION_FLIP_VERTICAL:
                matrix.postScale(1, -1);
                break;
            case ExifInterface.ORIENTATION_TRANSPOSE:
                matrix.postRotate(90);
                matrix.postScale(-1, 1);
                break;
            case ExifInterface.ORIENTATION_TRANSVERSE:
                matrix.postRotate(270);
                matrix.postScale(-1, 1);
                break;
            default:
                return bitmap;
        }

        try {
            return Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
        } catch (OutOfMemoryError e) {
            e.printStackTrace();
            return bitmap;
        }
    }

    // 保存位图到文件
    private void saveBitmapToFile(Bitmap bitmap, String filePath) {
        try (FileOutputStream out = new FileOutputStream(filePath)) {
            bitmap.compress(Bitmap.CompressFormat.JPEG, 100, out);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            bitmap.recycle();
        }
    }

    private void handleFilePath(String filePath) {
        // 处理文件路径的逻辑
        System.out.println("open image file=" + filePath);
        // "/storage/emulated/0/.Knot/receive_share_pic.png");
        if (fileIsExists(filePath)) {
            CallJavaNotify_7();

        } else
            Toast.makeText(this, filePath + "  The file does not exist.", 12000).show();
    }

    /**
     * Get a file path from a Uri. This will get the the path for Storage Access
     * Framework Documents, as well as the _data field for the MediaStore and
     * other file-based ContentProviders.
     *
     * @param context The context.
     * @param uri     The Uri to query.
     * @author paulburke
     */
    public static String getPath(final Context context, final Uri uri) {

        final boolean isKitKat = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT;
        ;

        // DocumentProvider
        if (isKitKat && DocumentsContract.isDocumentUri(context, uri)) {
            // ExternalStorageProvider
            if (isExternalStorageDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];

                if ("primary".equalsIgnoreCase(type)) {
                    return Environment.getExternalStorageDirectory() + "/" + split[1];
                }

                // TODO handle non-primary volumes
            }
            // DownloadsProvider
            else if (isDownloadsDocument(uri)) {

                final String id = DocumentsContract.getDocumentId(uri);
                final Uri contentUri = ContentUris.withAppendedId(
                        Uri.parse("content://downloads/public_downloads"), Long.valueOf(id));

                return getDataColumn(context, contentUri, null, null);
            }
            // MediaProvider
            else if (isMediaDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];

                Uri contentUri = null;
                if ("image".equals(type)) {
                    contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
                } else if ("video".equals(type)) {
                    contentUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
                } else if ("audio".equals(type)) {
                    contentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                }

                final String selection = "_id=?";
                final String[] selectionArgs = new String[] {
                        split[1]
                };

                return getDataColumn(context, contentUri, selection, selectionArgs);
            }
        }
        // MediaStore (and general)
        else if ("content".equalsIgnoreCase(uri.getScheme())) {

            // Return the remote address
            if (isGooglePhotosUri(uri))
                return uri.getLastPathSegment();

            return getDataColumn(context, uri, null, null);
        }
        // File
        else if ("file".equalsIgnoreCase(uri.getScheme())) {
            return uri.getPath();
        }

        return null;
    }

    /**
     **
     * Get the value of the data column for this Uri. This is useful for
     * MediaStore Uris, and other file-based ContentProviders.
     *
     * @param context       The context.
     * @param uri           The Uri to query.
     * @param selection     (Optional) Filter used in the query.
     * @param selectionArgs (Optional) Selection arguments used in the query.
     * @return The value of the _data column, which is typically a file path.
     */
    public static String getDataColumn(Context context, Uri uri, String selection,
            String[] selectionArgs) {

        Cursor cursor = null;
        final String column = "_data";
        final String[] projection = {
                column
        };

        try {
            cursor = context.getContentResolver().query(uri, projection, selection, selectionArgs,
                    null);
            if (cursor != null && cursor.moveToFirst()) {
                final int index = cursor.getColumnIndexOrThrow(column);
                return cursor.getString(index);
            }
        } finally {
            if (cursor != null)
                cursor.close();
        }
        return null;
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is ExternalStorageProvider.
     */
    public static boolean isExternalStorageDocument(Uri uri) {
        return "com.android.externalstorage.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is DownloadsProvider.
     */
    public static boolean isDownloadsDocument(Uri uri) {
        return "com.android.providers.downloads.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is MediaProvider.
     */
    public static boolean isMediaDocument(Uri uri) {
        return "com.android.providers.media.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is Google Photos.
     */
    public static boolean isGooglePhotosUri(Uri uri) {
        return "com.google.android.apps.photos.content".equals(uri.getAuthority());
    }

    private void initEditTextChangedListener() {

        editNote.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

            }

            @Override
            public void afterTextChanged(Editable editable) {
                isTextChanged = true;

                int undoCount = helper.getUndoCount();
                int redoCount = helper.getRedoCount();
                if (MyActivity.zh_cn) {
                    btnUndo.setText("撤销-" + String.valueOf(undoCount));
                    btnRedo.setText("恢复-" + String.valueOf(redoCount));
                } else {
                    btnUndo.setText("Undo-" + String.valueOf(undoCount));
                    btnRedo.setText("Redo-" + String.valueOf(redoCount));
                }

                if (isAddImage) {
                    isAddImage = false;
                    initTextFormat();
                }

            }
        });

        editFind.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

            }

            @Override
            public void afterTextChanged(Editable editable) {
                on_editFindTextChanged();
            }
        });
    }

    private void on_editFindTextChanged() {
        arrayFindResult.clear();

        String desString = editNote.getText().toString();
        String str = editFind.getText().toString();
        System.out.println("afterTextChanged=" + str);
        if (str.length() > 0) {

            arrayFindResult = findStr(desString, str);

            if (arrayFindResult.size() > 0) {
                goFindResult(0);
            }
        }

        lblResult.setText(String.valueOf(arrayFindResult.size()));
    }

    private void showNormalDialog() {
        final AlertDialog.Builder normalDialog = new AlertDialog.Builder(NoteEditor.this);
        normalDialog.setIcon(R.drawable.alarm);
        normalDialog.setTitle("Knot");
        String strYes, strNo;
        if (!MyActivity.zh_cn) {
            normalDialog.setMessage("The text has been modified. Do you want to save?");
            strYes = "Yes";
            strNo = "No";
        } else {
            normalDialog.setMessage("文本被修改，是否保存？");
            strYes = "是";
            strNo = "否";
        }
        normalDialog.setPositiveButton(strYes,
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // ...To-do
                        saveNote();

                    }
                });
        normalDialog.setNegativeButton(strNo,
                new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // ...To-do

                        finish();
                    }
                });
        // 显示
        normalDialog.show();

    }

    /* 创建选项菜单，目前暂不使用，它显示在菜单栏上 */
    // 该方法用于创建显示Menu

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_optionmenu, menu);
        return true;
    }

    // 在选项菜单打开以后会调用这个方法，设置menu图标显示（icon）

    @Override
    public boolean onMenuOpened(int featureId, Menu menu) {
        if (menu != null) {
            if (menu.getClass().getSimpleName().equalsIgnoreCase("MenuBuilder")) {
                try {
                    Method method = menu.getClass().getDeclaredMethod("setOptionalIconsVisible",
                            Boolean.TYPE);
                    method.setAccessible(true);
                    method.invoke(menu, true);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
        return super.onMenuOpened(featureId, menu);
    }

    // 该方法对菜单的item进行监听

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // mTextView.setText(item.getTitle());
        switch (item.getItemId()) {
            case R.id.menu1:
                Toast.makeText(this, "点击了第" + 1 + "个", Toast.LENGTH_SHORT).show();
                break;
            case R.id.menu2:
                Toast.makeText(this, "点击了第" + 2 + "个", Toast.LENGTH_SHORT).show();
                break;
            case R.id.menu3:
                Toast.makeText(this, "点击了第" + 3 + "个", Toast.LENGTH_SHORT).show();
                break;
            case R.id.menu4:
                Toast.makeText(this, "点击了第" + 4 + "个", Toast.LENGTH_SHORT).show();
                break;
            case R.id.menu5:
                Toast.makeText(this, "点击了第" + 5 + "个", Toast.LENGTH_SHORT).show();
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    private void showCustomPopupMenu() {
        PopupMenuCustomLayout popupMenu = new PopupMenuCustomLayout(
                this, R.layout.popup_menu_custom_layout,
                new PopupMenuCustomLayout.PopupMenuCustomOnClickListener() {
                    @Override
                    public void onClick(int itemId) {
                        // log statement: "Clicked on: " + itemId
                        switch (itemId) {
                            case R.id.format:
                                System.out.println("Item A was clicked!");
                                break;
                        }
                    }
                });

        popupMenu.show();

    }

    // 当前正在使用 main.xml main_cn.xml
    private void showPopupMenu(View view) {

        Context wrapper;
        if (MyActivity.isDark)
            wrapper = new ContextThemeWrapper(this, R.style.popup_menu_style_dark);
        else
            wrapper = new ContextThemeWrapper(this, R.style.popup_menu_style);

        PopupMenu popupMenu = new PopupMenu(wrapper, view);

        // menu布局
        if (MyActivity.zh_cn)
            popupMenu.getMenuInflater().inflate(R.menu.main_cn, popupMenu.getMenu());
        else
            popupMenu.getMenuInflater().inflate(R.menu.main, popupMenu.getMenu());

        // menu的item点击事件
        popupMenu.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
            @Override
            public boolean onMenuItemClick(MenuItem item) {

                // test
                switch (item.getItemId()) {
                    case R.id.s1:

                        break;

                }

                onClickMenuTitle(item.getTitle());

                // Toast.makeText(getApplicationContext(), item.getTitle(),
                // Toast.LENGTH_SHORT).show();
                return false;
            }
        });
        // PopupMenu关闭事件
        popupMenu.setOnDismissListener(new PopupMenu.OnDismissListener() {
            @Override
            public void onDismiss(PopupMenu menu) {
                btnMenu.setBackgroundColor(getResources().getColor(R.color.normal));

                // Toast.makeText(getApplicationContext(), "关闭PopupMenu",
                // Toast.LENGTH_SHORT).show();
            }
        });

        popupMenu.show();

    }

    private void onClickMenuTitle(CharSequence strTitle) {

        // Format
        if (strTitle.equals(listMenuTitle.get(0))) {
            initTextFormat();

        }

        // Image
        if (strTitle.equals(listMenuTitle.get(1))) {
            isImageFile = true;
            openFilePicker();
            isAddImage = true;

        }

        // Photo Shooting
        if (strTitle.equals(listMenuTitle.get(2))) {
            if (!MyActivity.checkCamera()) {
                String strInfo = "";
                if (MyActivity.zh_cn)
                    strInfo = "请开启摄像头权限！";
                else
                    strInfo = "Please enable camera permissions!";
                Toast.makeText(this, strInfo, Toast.LENGTH_SHORT).show();
                return;
            }

            isImageFile = false;
            dispatchTakePictureIntent();
            isAddImage = true;
        }

        // Table
        if (strTitle.equals(listMenuTitle.get(3))) {

            String str1 = "|Title1|Title2|Title3|\n";
            String str2 = "|------|------|----- |\n";
            String str3 = "|        |        |        |\n";
            String str4 = "|        |        |        |\n";

            insertNote(str1 + str2 + str3 + str4);
            initTextFormat();

        }

        // h1
        if (strTitle.equals(listMenuTitle.get(4))) {
            String sel = getEditSelectText();
            if (sel.length() > 0) {
                delEditSelectText();
                insertNote("# " + sel);
            } else
                insertNote("# ");

            initTextFormat();
        }

        // h2
        if (strTitle.equals(listMenuTitle.get(5))) {
            String sel = getEditSelectText();
            if (sel.length() > 0) {
                delEditSelectText();
                insertNote("## " + sel);
            } else
                insertNote("## ");

            initTextFormat();
        }

        // h3
        if (strTitle.equals(listMenuTitle.get(6))) {
            String sel = getEditSelectText();
            if (sel.length() > 0) {
                delEditSelectText();
                insertNote("### " + sel);
            } else
                insertNote("### ");

            initTextFormat();
        }

        // h4
        if (strTitle.equals(listMenuTitle.get(7))) {
            String sel = getEditSelectText();
            if (sel.length() > 0) {
                delEditSelectText();
                insertNote("#### " + sel);
            } else
                insertNote("#### ");

            initTextFormat();
        }

        // h5
        if (strTitle.equals(listMenuTitle.get(8))) {
            String sel = getEditSelectText();
            if (sel.length() > 0) {
                delEditSelectText();
                insertNote("##### " + sel);
            } else
                insertNote("##### ");

            initTextFormat();
        }

        // Font Color --> Separator
        if (strTitle.equals(listMenuTitle.get(9))) {
            insertNote("\n\n---\n\n");
            initTextFormat();
            if (3 > 2) {
                return;
            }

            String strChoose = "Choose Color";
            String strOk = "Ok";
            String strCancel = "Cancel";
            if (MyActivity.zh_cn) {
                strOk = "确定";
                strCancel = "取消";
                strChoose = "选择颜色";

            }

            ColorPickerDialogBuilder
                    .with(context)
                    .setTitle(strChoose)
                    .initialColor(Color.RED)
                    .wheelType(ColorPickerView.WHEEL_TYPE.FLOWER)
                    .density(12)
                    .setOnColorSelectedListener(new OnColorSelectedListener() {
                        @Override
                        public void onColorSelected(int selectedColor) {
                            // toast("onColorSelected: 0x" + Integer.toHexString(selectedColor));
                        }
                    })
                    .setPositiveButton(strOk, new ColorPickerClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int selectedColor, Integer[] allColors) {
                            String hexColor = "#"
                                    + Integer.toHexString(selectedColor).substring(2).toUpperCase();
                            String sel = getEditSelectText();
                            int len = sel.length();

                            if (len == 7) {
                                String subString = sel.substring(0, 1);
                                if (subString.equals("#") && isHexString(sel.substring(1))) {
                                    delEditSelectText();
                                    insertNote(hexColor);
                                } else {
                                    delEditSelectText();
                                    insertNote("<font color=" + hexColor + ">" + sel + "</font>");
                                }
                            } else if (len == 6) {
                                editNote.setSelection(start - 1, start);
                                if (getEditSelectText().equals("#") && isHexString(sel)) {
                                    editNote.setSelection(start - 1, end);
                                    delEditSelectText();
                                    insertNote(hexColor);

                                } else {
                                    editNote.setSelection(start, end);
                                    delEditSelectText();
                                    insertNote("<font color=" + hexColor + ">" + sel + "</font>");
                                }

                            } else if (len > 0) {
                                delEditSelectText();
                                insertNote("<font color=" + hexColor + ">" + sel + "</font>");
                            } else {
                                // old #E01B24
                                insertNote("<font color=" + hexColor + ">Color</font>");
                            }
                            initTextFormat();
                        }
                    })
                    .setNegativeButton(strCancel, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                        }
                    })
                    .build()
                    .show();

        }

        // Bold
        if (strTitle.equals(listMenuTitle.get(10))) {
            String sel = getEditSelectText();
            if (sel.length() > 0) {
                delEditSelectText();
                insertNote("**" + sel + "**");
            } else
                insertNote("**Bold**");

            initTextFormat();
        }

        // Italic
        if (strTitle.equals(listMenuTitle.get(11))) {
            String sel = getEditSelectText();
            if (sel.length() > 0) {
                delEditSelectText();
                insertNote("_" + sel + "_");
            } else
                insertNote("_Italic_");

            initTextFormat();

        }

        // Strickout
        if (strTitle.equals(listMenuTitle.get(12))) {
            String sel = getEditSelectText();
            if (sel.length() > 0) {
                delEditSelectText();
                insertNote("~~" + sel + "~~");
            } else
                insertNote("~~Strickout~~");

            initTextFormat();
        }

        // Underline
        if (strTitle.equals(listMenuTitle.get(13))) {
            String sel = getEditSelectText();
            if (sel.length() > 0) {
                delEditSelectText();
                insertNote("<u>" + sel + "</u>");
            } else
                insertNote("<u>Underline</u>");

            initTextFormat();

        }

        // Date
        if (strTitle.equals(listMenuTitle.get(14))) {

            SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd");
            String dateString = sdf.format(new Date());

            insertNote(dateString);
        }

        // Time
        if (strTitle.equals(listMenuTitle.get(15))) {
            SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss");
            String timeString = sdf.format(new Date());
            insertNote(timeString);
        }

        // Link
        if (strTitle.equals(listMenuTitle.get(16))) {
            insertNote("[]()");
        }

    }

    private ArrayList<String> initMenuTitle() {
        listMenuTitle.clear();
        if (MyActivity.zh_cn) {
            listMenuTitle.add("格式化");
            listMenuTitle.add("图片文件");
            listMenuTitle.add("拍摄照片");
            listMenuTitle.add("表格");
            listMenuTitle.add("h1 标题");
            listMenuTitle.add("h2 标题");
            listMenuTitle.add("h3 标题");
            listMenuTitle.add("h4 标题");
            listMenuTitle.add("h5 标题");
            listMenuTitle.add("分隔线");
            listMenuTitle.add("粗体");
            listMenuTitle.add("斜体");
            listMenuTitle.add("删除线");
            listMenuTitle.add("下划线");
            listMenuTitle.add("日期");
            listMenuTitle.add("时间");
            listMenuTitle.add("链接");
        } else {
            listMenuTitle.add("Format");
            listMenuTitle.add("Image File");
            listMenuTitle.add("Photo Shooting");
            listMenuTitle.add("Table");
            listMenuTitle.add("h1");
            listMenuTitle.add("h2");
            listMenuTitle.add("h3");
            listMenuTitle.add("h4");
            listMenuTitle.add("h5");
            listMenuTitle.add("Separator");
            listMenuTitle.add("Bold");
            listMenuTitle.add("Italic");
            listMenuTitle.add("Strickout");
            listMenuTitle.add("Underline");
            listMenuTitle.add("Date");
            listMenuTitle.add("Time");
            listMenuTitle.add("Link");
        }

        return listMenuTitle;

    }

    public boolean readFileFromUriToLocal(Uri uri, String localfile) {

        try {

            File outFile = new File(localfile);
            InputStream inputStream = getContentResolver().openInputStream(uri);
            FileOutputStream fos = new FileOutputStream(outFile);
            byte[] buf = new byte[1024];
            int readCount = 0;
            while ((readCount = inputStream.read(buf)) != -1) {
                fos.write(buf, 0, readCount);
            }
            fos.flush();
            inputStream.close();
            fos.close();

            return true;
        } catch (IOException e) {
            e.printStackTrace();
        }

        return false;
    }

    public String getEditSelectText() {
        Editable editable = editNote.getText();
        int beg = editNote.getSelectionStart();
        int end = editNote.getSelectionEnd();
        String sel = editable.toString().substring(beg, end);

        return sel;

    }

    public void delEditSelectText() {
        int beg = editNote.getSelectionStart();
        int end = editNote.getSelectionEnd();
        Editable editable = editNote.getText();
        editable.delete(beg, end);
    }

    public String getCursorPositionText(int index, int count) {
        int nLength = editNote.getText().length();
        int beg = 0;
        int end = 0;

        // left
        if (count < 0) {
            beg = index + count;
            end = index;
        } else {
            // right
            beg = index;
            end = index + count;
        }

        if (beg < 0)
            beg = 0;

        if (end < 0) {
            end = 0;
        }

        if (end > nLength)
            end = nLength;

        System.out.println("beg=" + beg + "  end=" + end);
        editNote.setSelection(beg, end);

        return getEditSelectText();

    }

    private void writeReceiveData() {

        String filename = "/storage/emulated/0/.Knot/myshare.ini";
        if (fileIsExists(filename)) {
            String strFlag = "";
            try {
                Wini ini = new Wini(new File(filename));
                strFlag = ini.get("share", "on_create");

            } catch (IOException e) {
                e.printStackTrace();
            }

            if (strFlag != null) {
                try {
                    Wini ini = new Wini(new File(filename));
                    ini.put("share", "on_create", "");
                    ini.store();
                } catch (IOException e) {
                    e.printStackTrace();
                }

                filename = "/storage/emulated/0/.Knot/share_text.txt";
                String str_receive = readTextFile(filename);
                if (strFlag.equals("insert")) {
                    insertNote(str_receive);
                }

                if (strFlag.equals("append")) {
                    appendNote(str_receive);
                }

            }
        }

    }

    private static int rabinKarpSearch(String desString, String subString) {
        if (desString == null || subString == null) {
            return -1;
        }
        // 1、主串长度
        int m = desString.length();
        // 2、模式串长度
        int n = subString.length();
        if (n > m) {
            throw new RuntimeException("主串长度不能小于模式串的长度");
        }
        // 3、获取模式串的hash值
        int patternCode = hash(subString);
        // 4、计算主串中与模式串长度相同第一个子串的hash值
        int strCode = hash(desString.substring(0, n));
        // 5、逐位比较hash值
        for (int i = 0; i < m - n + 1; i++) {
            // 如果hash值相等，为了防止hash碰撞产生的误差，故还需对字符串进行比较，以保证确实相等
            if (patternCode == strCode && compareString(i, desString, subString)) {
                return i;
            }
            // 判断是否为最后一轮，如果是最后一轮，则不再进行hash计算
            if (i < m - n) {
                strCode = nextHash(desString, strCode, i, n);
            }
        }
        return -1;
    }

    /**
     * 下一位的hash值计算
     *
     * @param desString 目标串
     * @param strCode   当前的hash值
     * @param index     当前hash值的所计算的字符串在目标串起始位置
     * @param offset    hash值计算的串长度
     * @return int
     * @Author muyi
     * @Date 14:03 2020/12/7
     */
    private static int nextHash(String desString, int strCode, int index, int offset) {
        /**
         * 比如 计算 ABCD 中 CD 的hash值，
         * 已知 BC的hash值为3，index = 1,offset = 2
         * 所以 CD的hash值等于 BC - B + D
         */
        // 1、减去起始位置字符的hash值
        strCode -= desString.charAt(index) - 'A';
        // 2、加上新增为字符的hash值
        strCode += desString.charAt(index + offset) - 'A';
        return strCode;
    }

    private static boolean compareString(int index, String desString, String subString) {
        String temp = desString.substring(index, index + subString.length());
        return temp.equals(subString);
    }

    /**
     * @param subString 字符串
     * @return int 约定计算模式的hash值
     * @Author muyi
     * @Date 14:00 2020/12/7
     */
    private static int hash(String subString) {
        int hashCode = 0;
        int length = subString.length();
        int i = 0;
        /**
         * 这里采用最简单的hashcode计算方式：
         * 把A当做0，把B当中1，把C当中2.....然后按位相加
         */
        while (i < length) {
            hashCode += subString.charAt(i) - 'A';
            i++;
        }
        return hashCode;
    }

    /**
     * 采用KMP算法，查找字符串子串的位置
     * 
     * @param source    源字符串
     * @param target    目标(子)字符串
     * @param pos       源字符串的起始索引
     * @param nextArray 函数式接口，传入获取next数组的方法(因为next数组上面定义了两种获取方式)
     * @return 目标字符串在源字符串中第一次出现的索引位置
     */
    public int indexOfToKMP(String source, String target, int pos, NextArray nextArray) {
        if (source == null || target == null) {
            return -1;
        }

        int i, j;
        // 调用接口中的方法，获取next数组
        int[] next = nextArray.getNext(target);
        // 初始化遍历
        i = pos;
        j = 0;
        // 判断i和j的索引都不能大于等于字符串长度，否则说明比较完成
        while (i < source.length() && j < target.length()) {
            // 如果j==-1，说明需要从头开始比较；或者由公共元素相等，继续比较
            if (j == -1 || source.charAt(i) == target.charAt(j)) {
                i++;
                j++;
            } else {
                // 需要按照数组回溯j
                j = next[j];
            }
        }
        // 判断子字符的索引是否大于等于子字符串，如果为true说明查找成功，返回源字符串当前位置-子字符串长度
        return j >= target.length() ? i - target.length() : -1;
    }

    // 函数式接口，获取next数组
    @FunctionalInterface
    public interface NextArray {
        int[] getNext(String target);
    }

    // 使用方法获取接口实例，接口中的getNext方法可以获取next数组
    public NextArray getNextInstance() {

        return target -> {
            // 初始化数组大小
            int[] next = new int[target.length()];
            // 0 位置默认为-1
            next[0] = -1;
            // 遍历字符串每一位，计算每个位置上的值
            for (int i = 0, k = -1; i < target.length() - 1;) {
                // k还不可比较说明是第一次进入或者回溯后可能产生的结果，k++，此位置的next[i]应为0
                // 如果前缀和后缀相等，说明可以计算权值
                if (k == -1 || target.charAt(i) == target.charAt(k)) {
                    i++;
                    k++;
                    next[i] = k;
                } else {
                    // 回溯
                    k = next[k];
                }
            }
            return next;
        };

    }

    private ArrayList<Integer> findStr(String desString, String subString) {
        ArrayList<Integer> arrayList = new ArrayList<Integer>();

        if (desString == null || subString == null) {
            return arrayList;
        }

        // editNote.setSelection(0);
        int index = 0;
        while (index != -1) {
            // index = rabinKarpSearch(desString, subString);
            index = indexOfToKMP(desString, subString, index, getNextInstance());
            System.out.println("find index=" + index);
            if (index >= 0) {
                arrayList.add(index);
                index++;
                int nLength = editNote.getText().length();
                if (index > nLength) {
                    index = -1;
                }
                // editNote.setSelection(index);
            }
        }

        return arrayList;

    }

    private void goFindResult(int nDirection) {
        String subString = editFind.getText().toString();
        if (arrayFindResult.size() > 0) {
            if (nDirection == -1) {
                curIndexForResult--;
            }
            if (nDirection == 0) {
                curIndexForResult = 0;
            }
            if (nDirection == 1) {
                curIndexForResult++;
            }
            if (curIndexForResult < 0)
                curIndexForResult = arrayFindResult.size() - 1;
            if (curIndexForResult >= arrayFindResult.size())
                curIndexForResult = 0;

            int pos = arrayFindResult.get(curIndexForResult);
            editNote.setSelection(pos, pos + subString.length());

        } else {
            Toast.makeText(this, "Not found.", 9000).show();
        }

    }

    /**
     * 设置文本关键字高亮处理
     * 
     * @param text     文本内容
     * @param keyword  关键字
     * @param textView 控件
     */
    private void setHighLineText(String text, String keyword, EditText textView) {
        if (!TextUtils.isEmpty(keyword) && !TextUtils.isEmpty(text)) {
            if (text.toLowerCase().contains(keyword.toLowerCase())) {
                int start = 0;
                if (text.contains(keyword)) {
                    start = text.indexOf(keyword);
                } else {
                    start = text.toLowerCase().indexOf(keyword.toLowerCase());
                }
                SpannableStringBuilder styled = new SpannableStringBuilder(text);
                styled.setSpan(new ForegroundColorSpan(Color.RED), start, start + keyword.length(),
                        Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
                textView.setText(styled);
            } else {
                textView.setText(text);
            }
        }
    }

    private SpannableStringBuilder style;

    private void hightKeyword(String strOrg, String strKey) {

        arrayFindResult.clear();
        arrayFindResult = findStr(strOrg, strKey);
        int count = arrayFindResult.size();
        if (count > 0) {

            for (int i = 0; i < count; i++) {
                int start = arrayFindResult.get(i);
                int end = start + strKey.length();
                System.out.print("start=" + start + "  end=" + end);

                if (strKey.equals("==Image==") || strKey.equals("<font ") || strKey.equals("</font>")
                        || strKey.equals("https://")
                        || strKey.equals("http://")) {
                    style.setSpan(new BackgroundColorSpan(Color.parseColor(strBack1)), start, end,
                            Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                    style.setSpan(new ForegroundColorSpan(Color.parseColor(strFore)), start, end,
                            Spannable.SPAN_EXCLUSIVE_INCLUSIVE);
                } else {
                    style.setSpan(new BackgroundColorSpan(Color.parseColor(strBack2)), start, end,
                            Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                    style.setSpan(new ForegroundColorSpan(Color.parseColor(strFore)), start, end,
                            Spannable.SPAN_EXCLUSIVE_INCLUSIVE);
                }

            }

        }

    }

    private void initTextFormat() {

        String strOrg = editNote.getText().toString();
        if (strOrg == null || strOrg.equals(""))
            return;

        int curIndex = editNote.getSelectionStart();

        for (int i = 0; i < 10; i++)
            strOrg = strOrg.replace("\n\n\n", "\n\n");

        // 在此处返回，目前只需要去除多余的空格即可
        if (strOrg != null) {
            editNote.setText(strOrg);
            int nLength = editNote.getText().length();
            if (curIndex > nLength)
                curIndex = nLength;

            editNote.setSelection(curIndex);
            return;
        }

        style = new SpannableStringBuilder(strOrg);

        hightKeyword(strOrg, "https://");
        hightKeyword(strOrg, "http://");
        hightKeyword(strOrg, "# ");
        hightKeyword(strOrg, "## ");
        hightKeyword(strOrg, "### ");
        hightKeyword(strOrg, "#### ");
        hightKeyword(strOrg, "##### ");
        hightKeyword(strOrg, "<font ");
        hightKeyword(strOrg, "</font>");
        hightKeyword(strOrg, "* ");
        hightKeyword(strOrg, "---");
        hightKeyword(strOrg, "**");
        hightKeyword(strOrg, "~~");
        hightKeyword(strOrg, "<u>");
        hightKeyword(strOrg, "</u>");
        hightKeyword(strOrg, "_**");
        hightKeyword(strOrg, "**_");

        // Get "![" and "]"
        Pattern patt = Pattern.compile("(?<=!\\[).*(?=\\])");
        Matcher matcher = patt.matcher(strOrg);
        while (matcher.find()) {
            // 返回匹配结果
            String result = matcher.group();

            // 返回起始位置
            int start = matcher.start();
            // 返回结束位置
            int end = matcher.end();

            style.setSpan(new BackgroundColorSpan(Color.parseColor(strBack1)), start, end,
                    Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            style.setSpan(new ForegroundColorSpan(Color.parseColor(strFore)), start, end,
                    Spannable.SPAN_EXCLUSIVE_INCLUSIVE);

            System.out.println("matcher result=" + result + "  start=" + start + "  end=" + end);

        }

        patt = Pattern.compile("(?<=\\[).*(?=\\]\\()");
        matcher = patt.matcher(strOrg);
        while (matcher.find()) {
            String result = matcher.group();
            int start = matcher.start();
            int end = matcher.end();
            style.setSpan(new BackgroundColorSpan(Color.parseColor(strBack1)), start, end,
                    Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            style.setSpan(new ForegroundColorSpan(Color.parseColor(strFore)), start, end,
                    Spannable.SPAN_EXCLUSIVE_INCLUSIVE);
            System.out.println("matcher result=" + result + "  start=" + start + "  end=" + end);

        }

        editNote.setText(style);

        int nLength = editNote.getText().length();
        if (curIndex > nLength)
            curIndex = nLength;

        editNote.setSelection(curIndex);
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

    private boolean isHexString(String hexString) {
        if (hexString.length() % 2 != 0) {
            return false;
        }

        String hexChars = "0123456789ABCDEF"; // 16进制字符集
        for (int i = 0; i < hexString.length(); i++) {
            char c = hexString.charAt(i);
            if (!hexChars.contains(Character.toUpperCase(c) + "")) {
                // 字符不是有效的16进制字符
                System.out.println("字符串包含非16进制字符");
                return false;
            }
        }
        return true;
    }

    public static void closeNoteEditorView() {
        if (m_instance != null)
            m_instance.finish();

    }

    private void hideInputMethod() {
        InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        boolean isOpen = imm.isActive();
        if (isOpen) {
            imm.toggleSoftInput(0, InputMethodManager.HIDE_NOT_ALWAYS);
        }
    }

    private void initColorValue() {
        if (MyActivity.isDark) {
            strBack1 = "#A52A2A";
            strBack2 = "#8B7E66";
            strFore = "#FFFFFF";

        } else {
            strBack1 = "#FFC1C1";
            strBack2 = "#CFCFCF";
            strFore = "#000000";
        }

    }

    // 触发拍照的方法
    private void dispatchTakePictureIntent() {
        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        // 确保有相机应用可以处理该Intent
        if (takePictureIntent.resolveActivity(getPackageManager()) != null) {
            // 创建临时文件用于存储拍摄的照片
            File photoFile = createImageFile();
            if (photoFile != null) {
                // 将临时文件的Uri传递给相机应用程序
                photoUri = FileProvider.getUriForFile(this,
                        "com.x",
                        photoFile);
                takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, photoUri);
                startActivityForResult(takePictureIntent, REQUEST_TAKE_PHOTO);
            }
        }
    }

    // 创建保存照片的临时文件
    private File createImageFile() {
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        String imageFileName = "JPEG_" + timeStamp + "_";
        File storageDir = getExternalFilesDir(Environment.DIRECTORY_PICTURES);
        File image = null;
        try {
            image = File.createTempFile(
                    imageFileName, /* prefix */
                    ".jpg", /* suffix */
                    storageDir /* directory */
            );
        } catch (IOException e) {
            // 错误处理
        }
        return image;
    }

    private void openSearchResult() {
        if (MyActivity.isOpenSearchResult) {
            if (!MyActivity.strSearchText.equals("")) {
                btnFind.performClick();
                editFind.setText(MyActivity.strSearchText);
                btnStartFind.performClick();
                btnNext.performClick();
                btnPrev.performClick();
            }
        }
        MyActivity.isOpenSearchResult = false;
    }

    public void showAndroidProgressBar() {
        Context context = NoteEditor.this;
        Intent i = new Intent(context, MyProgBar.class);
        i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(i);
    }

    public void closeAndroidProgressBar() {
        if (MyProgBar.m_MyProgBar != null)
            MyProgBar.m_MyProgBar.finish();
    }

    // 获取文件大小（单位：KB）
    public static long getFileSizeInKB(File file) {
        if (file == null || !file.exists())
            return 0;
        return file.length() / 1024; // 转换为 KB
    }

}
