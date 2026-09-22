package com.x.artifex.mupdf.mini;

import android.Manifest;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Insets;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.graphics.drawable.ColorDrawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.FileUriExposedException;
import android.os.ParcelFileDescriptor;
import android.provider.OpenableColumns;
import android.text.Editable;
import android.text.Html;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.TextPaint;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.method.PasswordTransformationMethod;
import android.text.style.BackgroundColorSpan;
import android.text.style.ForegroundColorSpan;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.view.ActionMode;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AlertDialog;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import com.artifex.mupdf.fitz.*;
import com.artifex.mupdf.fitz.Quad;
import com.artifex.mupdf.fitz.android.*;
import com.x.ImmersiveUtil;
import com.x.MyActivity;
import com.x.MyService;
import com.x.R;
import com.x.TTSUtils;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Stack;

public class DocumentActivity extends Activity {

    private boolean isTxtFile = false;
    private android.app.ProgressDialog mConvertProgressDialog = null;

    // TTS /////////////////////////////////////////////////
    private ImageButton ttsButton;
    protected boolean mIsTtsReading = false;
    protected int mTtsReadingPage = -1;
    /** 缓存当前页提取的纯文本，用于末尾判断 */
    private String mCurrentPageText = "";
    /** 缓存当前页的变换矩阵，供高亮搜索使用 */
    private Matrix mCurrentPageCtm = null;

    // TTS睡眠定时//////////////////////////////////////////////
    private ImageButton sleepTimerButton;
    private boolean mSleepTimerEnabled = false;
    private final android.os.Handler mSleepTimerHandler =
        new android.os.Handler(android.os.Looper.getMainLooper());
    private final Runnable mSleepStopTtsRunnable = new Runnable() {
        @Override
        public void run() {
            // 2小时到，停止朗读
            stopTtsReading();
            mSleepTimerEnabled = false;
            updateSleepTimerButtonState();
            runOnUiThread(() -> {
                Toast.makeText(
                    DocumentActivity.this,
                    MyActivity.zh_cn
                        ? "睡眠定时已结束，朗读停止"
                        : "Sleep timer expired, reading stopped",
                    Toast.LENGTH_SHORT
                ).show();
            });
        }
    };

    //////////////////////////////////////////////////
    private ImageButton minimizeAppButton;
    private ImageButton readingNoteButton;

    protected boolean mInvertMode = false;

    public static DocumentActivity mPdfActivity = null;

    private final String APP = "MuPDF";

    public final int NAVIGATE_REQUEST = 1;

    protected final int MAXIMUM_OUTLINE_ITEMS = 1000;
    protected final int MAXIMUM_OUTLINE_DEPTH = 4;

    protected final float EXCLUSION_HEIGHT_FACTOR = 2.0f;

    protected Worker worker;
    protected SharedPreferences prefs;

    protected Document doc;

    protected String key;
    protected String mimetype;
    protected SeekableInputStream stream;
    protected byte[] buffer;

    protected boolean returnToLibraryActivity;
    protected boolean hasLoaded;
    protected boolean isReflowable;
    protected boolean fitPage;
    protected String title;
    protected ArrayList<OutlineActivity.Item> flatOutline;
    protected float layoutW, layoutH, layoutEm;
    protected float displayDPI;
    protected int canvasW, canvasH;
    protected float pageZoom;

    protected View currentBar;
    protected PageView pageView;
    protected View actionBar;
    protected TextView titleLabel;
    protected View searchButton;
    protected View searchBar;
    protected EditText searchText;
    protected View searchCloseButton;
    protected View searchBackwardButton;
    protected View searchForwardButton;
    protected View zoomButton;
    protected View layoutButton;
    protected PopupMenu layoutPopupMenu;
    protected View outlineButton;
    protected View bottomBar;
    protected View backgroundLayout;
    protected View topBar;
    protected TextView pageLabel;
    protected SeekBar pageSeekbar;

    protected boolean pageCountChanged;
    protected int pageCount;
    protected int currentPage;
    protected int searchHitPage;
    protected String searchNeedle;
    protected boolean stopSearch;
    protected Stack<Integer> history;
    protected boolean wentBack;
    protected boolean toggledUI;
    protected Insets systemInsets = Insets.NONE;
    protected boolean newSearchHitPage;

    // 用来存放C++返回的内存PDF byte[]
    private byte[] mConvertedPdfBuffer = null;

    public static native void CallJavaNotify_0();

    public static native void CallJavaNotify_1();

    public static native void CallJavaNotify_2();

    public static native void CallJavaNotify_3();

    public static native void CallJavaNotify_4();

    public static native void CallJavaNotify_5();

    public static native void CallJavaNotify_6();

    public static native void CallJavaNotify_7();

    public static native void CallJavaNotify_8();

    public static native void CallJavaNotify_9();

    public static native void CallJavaNotify_10();

    public static native void CallJavaNotify_11();

    public static native void CallJavaNotify_12();

    public static native void CallJavaNotify_13();

    public static native void CallJavaNotify_14();

    private String toHex(byte[] digest) {
        StringBuilder builder = new StringBuilder(2 * digest.length);
        for (byte b : digest) builder.append(String.format("%02x", b));
        return builder.toString();
    }

    private void openInput(Uri uri, long size, String mimetype)
        throws IOException {
        ContentResolver cr = getContentResolver();

        Log.i(APP, "Opening document " + uri);

        InputStream is = cr.openInputStream(uri);
        byte[] buf = null;
        int used = -1;
        try {
            final int limit = 8 * 1024 * 1024;
            if (size < 0) {
                // size is unknown
                buf = new byte[limit];
                used = is.read(buf);
                boolean atEOF = is.read() == -1;
                if (
                    used < 0 ||
                    (used == limit && !atEOF) // no or partial data
                ) buf = null;
            } else if (size <= limit) {
                // size is known and below limit
                buf = new byte[(int) size];
                used = is.read(buf);
                if (
                    used < 0 ||
                    used < size // no or partial data
                ) buf = null;
            }
            if (buf != null && buf.length != used) {
                byte[] newbuf = new byte[used];
                System.arraycopy(buf, 0, newbuf, 0, used);
                buf = newbuf;
            }
        } catch (OutOfMemoryError e) {
            buf = null;
        } finally {
            is.close();
        }

        if (buf != null) {
            Log.i(
                APP,
                "  Opening document from memory buffer of size " + buf.length
            );
            buffer = buf;
        } else {
            Log.i(APP, "  Opening document from stream");
            stream = new ContentInputStream(cr, uri, size);
        }
    }

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mPdfActivity = this;

        requestWindowFeature(Window.FEATURE_NO_TITLE);

        // 状态栏和导航栏 //////////////////////////////////////////////////////////

        // 刘海屏适配仍需保留（与沉浸模式无关）
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            getWindow().getAttributes().layoutInDisplayCutoutMode =
                WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }

        ///////////////////////////////////////////////////////////////////

        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        displayDPI = metrics.densityDpi;

        setContentView(R.layout.document_activity);

        //ImmersiveUtil.applyRealImmersive(this);

        actionBar = findViewById(R.id.action_bar);
        searchBar = findViewById(R.id.search_bar);
        bottomBar = findViewById(R.id.bottom_bar);
        backgroundLayout = findViewById(R.id.background_layout);
        topBar = findViewById(R.id.top_bar);

        ttsButton = findViewById(R.id.tts_button);
        ttsButton.setOnClickListener(v -> toggleTts());

        // 睡眠定时器按钮
        sleepTimerButton = findViewById(R.id.sleep_timer_button);
        sleepTimerButton.setOnClickListener(v -> {
            mSleepTimerEnabled = !mSleepTimerEnabled;
            updateSleepTimerButtonState();
            // 如果当前正在朗读，立刻重置倒计时
            if (mIsTtsReading) {
                mSleepTimerHandler.removeCallbacks(mSleepStopTtsRunnable);
                if (mSleepTimerEnabled) {
                    mSleepTimerHandler.postDelayed(
                        mSleepStopTtsRunnable,
                        2 * 60 * 60 * 1000
                    );
                }
            }
        });

        // ===== 最小化APP按钮 =====
        minimizeAppButton = findViewById(R.id.minimize_app_button);
        minimizeAppButton.setOnClickListener(v -> {
            // 将当前Activity任务退后台，最小化APP
            MyActivity.setMini();
        });

        currentBar = actionBar;

        ImageButton openButton = findViewById(R.id.open_button);
        openButton.setVisibility(View.GONE);
        openButton.setOnClickListener(v -> {
            finish();
            CallJavaNotify_10();
        });

        mInvertMode = MyActivity.mPdfInvertMode;
        ImageButton darkModeButton = findViewById(R.id.dark_mode_button);
        darkModeButton.setOnClickListener(v -> {
            //MyActivity.mPdfInvertMode = !MyActivity.mPdfInvertMode;
            //finish();
            //CallJavaNotify_13();

            // 原地刷新
            mInvertMode = !mInvertMode;
            MyActivity.mPdfInvertMode = mInvertMode;
            // 同步更新状态栏图标颜色
            if (bottomBar.getVisibility() == View.VISIBLE) {
                updateStatusBarIconMode(true);
            }
            loadPage(); // 仅重新渲染当前页，invertBitmap 会自动生效
        });

        // 读书笔记按钮
        readingNoteButton = findViewById(R.id.reading_note_button);
        readingNoteButton.setOnClickListener(v -> {
            // 调用C++，打开当前文档对应的读书笔记，`key` 就是当前文档 uri
            MyActivity.mInstance.PublicJavaCallCpp(
                "open_reading_notes|==|" + key
            );
        });

        // 初始化同步状态栏图标
        updateStatusBarIconMode(mInvertMode);

        Uri uri = getIntent().getData();
        mimetype = getIntent().getType();

        // 读取来自主Activity的暗黑标记
        mInvertMode = getIntent().getBooleanExtra("invert_mode", false);

        if (uri == null) {
            Toast.makeText(
                this,
                getString(R.string.toast_no_document_uri),
                Toast.LENGTH_SHORT
            ).show();
            return;
        }

        returnToLibraryActivity =
            getIntent().getIntExtra(
                getComponentName().getPackageName() +
                    ".ReturnToLibraryActivity",
                0
            ) != 0;

        key = uri.toString();

        Log.i(APP, "OPEN URI " + uri.toString());
        Log.i(APP, "  MAGIC (Intent) " + mimetype);

        title = "";
        long size = -1;
        Cursor cursor = null;

        try {
            cursor = getContentResolver().query(
                uri,
                null,
                null,
                null,
                null,
                null
            );
            if (cursor != null && cursor.moveToFirst()) {
                int idx;

                idx = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                if (
                    idx >= 0 && cursor.getType(idx) == Cursor.FIELD_TYPE_STRING
                ) title = cursor.getString(idx);

                idx = cursor.getColumnIndex(OpenableColumns.SIZE);
                if (
                    idx >= 0 && cursor.getType(idx) == Cursor.FIELD_TYPE_INTEGER
                ) size = cursor.getLong(idx);

                if (size == 0) size = -1;
            }
        } catch (Exception x) {
            // Ignore any exception and depend on default values for title
            // and size (unless one was decoded
        } finally {
            if (cursor != null) cursor.close();
        }

        // ========== 判断：如果是txt文件，走转换分支 ==========
        isTxtFile = title.toLowerCase().endsWith(".txt");
        if (isTxtFile) {
            Log.i(
                APP,
                "This is txt file, request convert to pdf. name=" + title
            );

            // 弹出等待转圈框
            runOnUiThread(() -> {
                if (isFinishing() || isDestroyed()) return;
                mConvertProgressDialog = new android.app.ProgressDialog(
                    DocumentActivity.this
                );
                mConvertProgressDialog.setMessage(
                    MyActivity.zh_cn
                        ? "正在转换，请稍后..."
                        : "Converting, please wait..."
                );
                mConvertProgressDialog.setCancelable(false);
                mConvertProgressDialog.show();
            });

            requestConvertTxtToPdf(uri);
        } else {
            // ========================================================

            Log.i(APP, "  NAME " + title);
            Log.i(APP, "  SIZE " + size);

            if (
                mimetype == null || mimetype.equals("application/octet-stream")
            ) {
                mimetype = getContentResolver().getType(uri);
                Log.i(APP, "  MAGIC (Resolver) " + mimetype);
            }
            if (
                mimetype == null || mimetype.equals("application/octet-stream")
            ) {
                mimetype = title;
                Log.i(APP, "  MAGIC (Filename) " + mimetype);
            }

            try {
                openInput(uri, size, mimetype);
            } catch (Exception x) {
                Log.e(APP, x.toString());
                String text = x.getMessage();
                if (text == null) text = x.getClass().getName();
                Toast.makeText(this, text, Toast.LENGTH_SHORT).show();
            }
        }

        titleLabel = (TextView) findViewById(R.id.title_label);
        titleLabel.setText(title);

        history = new Stack<Integer>();

        worker = new Worker(this);
        worker.start();

        prefs = getPreferences(Context.MODE_PRIVATE);
        layoutEm = prefs.getFloat("layoutEm", 8);
        fitPage = prefs.getBoolean("fitPage", false);
        currentPage = prefs.getInt(key, 0);
        searchHitPage = -1;
        hasLoaded = false;

        // 读取睡眠定时状态
        mSleepTimerEnabled = prefs.getBoolean("sleep_timer_enabled", false);
        updateSleepTimerButtonState();

        pageView = (PageView) findViewById(R.id.page_view);
        pageView.setActionListener(this);

        pageLabel = (TextView) findViewById(R.id.page_label);
        pageSeekbar = (SeekBar) findViewById(R.id.page_seekbar);
        pageSeekbar.setOnSeekBarChangeListener(
            new SeekBar.OnSeekBarChangeListener() {
                public int newProgress = -1;

                public void onProgressChanged(
                    SeekBar seekbar,
                    int progress,
                    boolean fromUser
                ) {
                    if (fromUser) {
                        newProgress = progress;
                        showPageNumber(progress + 1);
                    }
                }

                public void onStartTrackingTouch(SeekBar seekbar) {}

                public void onStopTrackingTouch(SeekBar seekbar) {
                    gotoPage(newProgress);
                }
            }
        );

        searchButton = findViewById(R.id.search_button);
        searchButton.setOnClickListener(
            new View.OnClickListener() {
                public void onClick(View v) {
                    showSearch();
                }
            }
        );
        searchText = (EditText) findViewById(R.id.search_text);
        searchText.setOnEditorActionListener(
            new TextView.OnEditorActionListener() {
                public boolean onEditorAction(
                    TextView v,
                    int actionId,
                    KeyEvent event
                ) {
                    if (
                        actionId == EditorInfo.IME_NULL &&
                        event.getAction() == KeyEvent.ACTION_DOWN
                    ) {
                        search(1);
                        return true;
                    }
                    if (actionId == EditorInfo.IME_ACTION_SEARCH) {
                        search(1);
                        return true;
                    }
                    return false;
                }
            }
        );
        searchText.addTextChangedListener(
            new TextWatcher() {
                public void afterTextChanged(Editable s) {}

                public void beforeTextChanged(
                    CharSequence s,
                    int start,
                    int count,
                    int after
                ) {}

                public void onTextChanged(
                    CharSequence s,
                    int start,
                    int before,
                    int count
                ) {
                    resetSearch();
                }
            }
        );
        searchCloseButton = findViewById(R.id.search_close_button);
        searchCloseButton.setOnClickListener(
            new View.OnClickListener() {
                public void onClick(View v) {
                    hideSearch();
                }
            }
        );
        searchBackwardButton = findViewById(R.id.search_backward_button);
        searchBackwardButton.setOnClickListener(
            new View.OnClickListener() {
                public void onClick(View v) {
                    search(-1);
                }
            }
        );
        searchForwardButton = findViewById(R.id.search_forward_button);
        searchForwardButton.setOnClickListener(
            new View.OnClickListener() {
                public void onClick(View v) {
                    search(1);
                }
            }
        );

        outlineButton = findViewById(R.id.outline_button);
        outlineButton.setOnClickListener(
            new View.OnClickListener() {
                public void onClick(View v) {
                    Intent intent = new Intent(
                        DocumentActivity.this,
                        OutlineActivity.class
                    );
                    Bundle bundle = new Bundle();
                    bundle.putInt("POSITION", currentPage);
                    bundle.putSerializable("OUTLINE", flatOutline);
                    intent.putExtras(bundle);
                    startActivityForResult(intent, NAVIGATE_REQUEST);
                }
            }
        );

        zoomButton = findViewById(R.id.zoom_button);
        zoomButton.setOnClickListener(
            new View.OnClickListener() {
                public void onClick(View v) {
                    fitPage = !fitPage;
                    loadPage();
                }
            }
        );

        layoutButton = findViewById(R.id.layout_button);
        layoutPopupMenu = new PopupMenu(this, layoutButton);
        layoutPopupMenu
            .getMenuInflater()
            .inflate(R.menu.layout_menu, layoutPopupMenu.getMenu());
        layoutPopupMenu.setOnMenuItemClickListener(
            new PopupMenu.OnMenuItemClickListener() {
                public boolean onMenuItemClick(MenuItem item) {
                    float oldLayoutEm = layoutEm;
                    int id = item.getItemId();
                    if (id == R.id.action_layout_6pt) layoutEm = 6;
                    else if (id == R.id.action_layout_7pt) layoutEm = 7;
                    else if (id == R.id.action_layout_8pt) layoutEm = 8;
                    else if (id == R.id.action_layout_9pt) layoutEm = 9;
                    else if (id == R.id.action_layout_10pt) layoutEm = 10;
                    else if (id == R.id.action_layout_11pt) layoutEm = 11;
                    else if (id == R.id.action_layout_12pt) layoutEm = 12;
                    else if (id == R.id.action_layout_13pt) layoutEm = 13;
                    else if (id == R.id.action_layout_14pt) layoutEm = 14;
                    else if (id == R.id.action_layout_15pt) layoutEm = 15;
                    else if (id == R.id.action_layout_16pt) layoutEm = 16;
                    if (oldLayoutEm != layoutEm) relayoutDocument();
                    return true;
                }
            }
        );
        layoutButton.setOnClickListener(
            new View.OnClickListener() {
                public void onClick(View v) {
                    layoutPopupMenu.show();
                }
            }
        );

        topBar.setOnApplyWindowInsetsListener(
            new View.OnApplyWindowInsetsListener() {
                public WindowInsets onApplyWindowInsets(
                    View v,
                    WindowInsets windowInsets
                ) {
                    applyInsets(windowInsets);
                    return WindowInsets.CONSUMED;
                }
            }
        );

        if (Build.VERSION.SDK_INT >= 29) bottomBar.addOnLayoutChangeListener(
            new View.OnLayoutChangeListener() {
                public void onLayoutChange(
                    View v,
                    int left,
                    int top,
                    int right,
                    int bottom,
                    int oldLeft,
                    int oldTop,
                    int oldRight,
                    int oldBottom
                ) {
                    View parent = (View) v.getParent();
                    android.graphics.Rect exclusion;

                    exclusion = new android.graphics.Rect(
                        0,
                        0,
                        v.getWidth(),
                        v.getHeight()
                    );
                    v.setSystemGestureExclusionRects(
                        Collections.singletonList(exclusion)
                    );

                    int extended_top =
                        parent.getHeight() -
                        (int) (EXCLUSION_HEIGHT_FACTOR * v.getHeight());
                    exclusion = new android.graphics.Rect(
                        0,
                        extended_top,
                        parent.getWidth(),
                        parent.getHeight()
                    );
                    parent.setSystemGestureExclusionRects(
                        Collections.singletonList(exclusion)
                    );
                }
            }
        );

        //enterSystemFullscreen();

        // 注册Home键广播
        int flags = Context.RECEIVER_NOT_EXPORTED;
        registerReceiver(
            mHomeKeyEvent,
            new IntentFilter(Intent.ACTION_CLOSE_SYSTEM_DIALOGS),
            flags
        );
    }

    protected void showPageNumber(int pageNumber) {
        if (pageCountChanged && bottomBar.getVisibility() == View.VISIBLE) {
            // set width of longest possible text as minimum width this
            // ensures that the size of pageLabel doesn't change whether
            // it renders 1 / pageCount or pageCount / pageCount,
            // which makes pageSeekbar to the left of pageLabel also have
            // a stable width.

            TextPaint paint = pageLabel.getPaint();
            float maxWidth =
                pageLabel.getPaddingLeft() +
                paint.measureText(pageCount + " / " + pageCount) + // longest possible text
                pageLabel.getPaddingRight();
            int minWidth = (int) Math.ceil(maxWidth);
            if (minWidth > 0) {
                pageLabel.setMinWidth(minWidth);
                pageCountChanged = false;
            }
        }

        pageLabel.setText(pageNumber + " / " + pageCount);
    }

    protected void applyInsets(WindowInsets windowInsets) {
        systemInsets = Insets.NONE;
        Insets systemBarInsets = windowInsets.getInsets(
            WindowInsets.Type.systemBars()
        );
        systemInsets = Insets.max(systemInsets, systemBarInsets);
        Insets cutoutInsets = windowInsets.getInsets(
            WindowInsets.Type.displayCutout()
        );
        systemInsets = Insets.max(systemInsets, cutoutInsets);
        topBar.setPadding(0, systemInsets.top, 0, 0);
        bottomBar.setPadding(0, 0, 0, systemInsets.bottom);
    }

    public boolean onKeyUp(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_PAGE_UP:
            case KeyEvent.KEYCODE_COMMA:
            case KeyEvent.KEYCODE_B:
                goBackward();
                return true;
            case KeyEvent.KEYCODE_PAGE_DOWN:
            case KeyEvent.KEYCODE_PERIOD:
            case KeyEvent.KEYCODE_SPACE:
                goForward();
                return true;
            case KeyEvent.KEYCODE_M:
                history.push(currentPage);
                return true;
            case KeyEvent.KEYCODE_T:
                if (!history.empty()) {
                    currentPage = history.pop();
                    loadPage();
                }
                return true;
        }
        return super.onKeyUp(keyCode, event);
    }

    public void onPageViewSizeChanged(int w, int h) {
        pageZoom = 1;
        canvasW = w;
        canvasH = h;
        layoutW = (canvasW * 72) / displayDPI;
        layoutH = (canvasH * 72) / displayDPI;
        if (!hasLoaded) {
            hasLoaded = true;
            if (!isTxtFile) {
                openDocument();
            }
        } else if (isReflowable) {
            relayoutDocument();
        } else {
            loadPage();
        }
    }

    public void onPageViewZoomChanged(float zoom) {
        if (zoom != pageZoom) {
            pageZoom = zoom;
            loadPage();
        }
    }

    protected void openDocument() {
        worker.add(
            new Worker.Task() {
                boolean needsPassword;

                public void work() {
                    Log.i(APP, "open document");
                    if (buffer != null) doc = Document.openDocument(
                        buffer,
                        mimetype
                    );
                    else doc = Document.openDocument(stream, mimetype);
                    needsPassword = doc.needsPassword();
                }

                public void run() {
                    if (needsPassword) askPassword(
                        R.string.dlog_password_message
                    );
                    else loadDocument();
                }
            }
        );
    }

    protected void askPassword(int message) {
        final EditText passwordView = new EditText(this);
        passwordView.setInputType(EditorInfo.TYPE_TEXT_VARIATION_PASSWORD);
        passwordView.setTransformationMethod(
            PasswordTransformationMethod.getInstance()
        );

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.dlog_password_title);
        builder.setMessage(message);
        builder.setView(passwordView);
        builder.setPositiveButton(
            android.R.string.ok,
            new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int id) {
                    checkPassword(passwordView.getText().toString());
                }
            }
        );
        builder.setNegativeButton(
            android.R.string.cancel,
            new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int id) {
                    finish();
                }
            }
        );
        builder.setOnCancelListener(
            new DialogInterface.OnCancelListener() {
                public void onCancel(DialogInterface dialog) {
                    finish();
                }
            }
        );
        builder.create().show();
    }

    protected void checkPassword(final String password) {
        worker.add(
            new Worker.Task() {
                boolean passwordOkay;

                public void work() {
                    Log.i(APP, "check password");
                    passwordOkay = doc.authenticatePassword(password);
                }

                public void run() {
                    if (passwordOkay) loadDocument();
                    else askPassword(R.string.dlog_password_retry);
                }
            }
        );
    }

    public void onPause() {
        super.onPause();
        if (prefs != null) {
            SharedPreferences.Editor editor = prefs.edit();
            editor.putFloat("layoutEm", layoutEm);
            editor.putBoolean("fitPage", fitPage);
            editor.putInt(key, currentPage);
            // 保存睡眠定时开关
            editor.putBoolean("sleep_timer_enabled", mSleepTimerEnabled);
            editor.apply();
        }
    }

    public void onBackPressed() {
        if (history.empty()) {
            super.onBackPressed();
            if (returnToLibraryActivity) {
                Intent intent = getPackageManager().getLaunchIntentForPackage(
                    getComponentName().getPackageName()
                );
                startActivity(intent);
            }
        } else {
            currentPage = history.pop();
            loadPage();
        }
    }

    @Override
    protected void onDestroy() {
        unregisterReceiver(mHomeKeyEvent);

        // 睡眠定时任务清理
        mSleepTimerHandler.removeCallbacks(mSleepStopTtsRunnable);

        // 兜底关闭转换等待弹窗，防止页面销毁弹窗残留
        if (
            mConvertProgressDialog != null && mConvertProgressDialog.isShowing()
        ) {
            mConvertProgressDialog.dismiss();
        }

        // 退出全屏模式
        CallJavaNotify_0();

        super.onDestroy();
        mPdfActivity = null;
    }

    public void onActivityResult(int request, int result, Intent data) {
        if (
            request == NAVIGATE_REQUEST && result >= RESULT_FIRST_USER
        ) gotoPage(result - RESULT_FIRST_USER);
    }

    protected void showKeyboard() {
        InputMethodManager imm = (InputMethodManager) getSystemService(
            Context.INPUT_METHOD_SERVICE
        );
        if (imm != null) imm.showSoftInput(searchText, 0);
    }

    protected void hideKeyboard() {
        InputMethodManager imm = (InputMethodManager) getSystemService(
            Context.INPUT_METHOD_SERVICE
        );
        if (imm != null) imm.hideSoftInputFromWindow(
            searchText.getWindowToken(),
            0
        );
    }

    protected void resetSearch() {
        stopSearch = true;
        searchHitPage = -1;
        searchNeedle = null;
        pageView.resetHits();
    }

    protected void runSearch(
        final int startPage,
        final int direction,
        final String needle
    ) {
        stopSearch = false;
        worker.add(
            new Worker.Task() {
                int searchPage = startPage;

                public void work() {
                    if (stopSearch || needle != searchNeedle) return;
                    for (int i = 0; i < 9; ++i) {
                        Log.i(APP, "search page " + searchPage);
                        Page page = doc.loadPage(searchPage);
                        Quad[][] hits = page.search(searchNeedle);
                        page.destroy();
                        if (hits != null && hits.length > 0) {
                            newSearchHitPage = true;
                            searchHitPage = searchPage;
                            break;
                        }
                        searchPage += direction;
                        if (searchPage < 0 || searchPage >= pageCount) break;
                    }
                }

                public void run() {
                    if (stopSearch || needle != searchNeedle) {
                        showPageNumber(currentPage + 1);
                    } else if (searchHitPage == currentPage) {
                        loadPage();
                    } else if (searchHitPage >= 0) {
                        history.push(currentPage);
                        currentPage = searchHitPage;
                        loadPage();
                    } else {
                        if (searchPage >= 0 && searchPage < pageCount) {
                            showPageNumber(searchPage + 1);
                            worker.add(this);
                        } else {
                            showPageNumber(currentPage + 1);
                            Log.i(APP, "search not found");
                            Toast.makeText(
                                DocumentActivity.this,
                                getString(R.string.toast_search_not_found),
                                Toast.LENGTH_SHORT
                            ).show();
                        }
                    }
                }
            }
        );
    }

    protected void search(int direction) {
        hideKeyboard();
        int startPage;
        if (searchHitPage == currentPage) startPage = currentPage + direction;
        else startPage = currentPage;
        searchHitPage = -1;
        searchNeedle = searchText.getText().toString();
        if (searchNeedle.length() == 0) searchNeedle = null;
        if (searchNeedle != null) if (
            startPage >= 0 && startPage < pageCount
        ) runSearch(startPage, direction, searchNeedle);
    }

    protected void loadDocument() {
        worker.add(
            new Worker.Task() {
                public void work() {
                    try {
                        Log.i(APP, "load document");
                        String metaTitle = doc.getMetaData(
                            Document.META_INFO_TITLE
                        );
                        if (metaTitle != null && !metaTitle.equals("")) title =
                            metaTitle;
                        isReflowable = doc.isReflowable();
                        if (isReflowable) {
                            Log.i(APP, "layout document");
                            doc.layout(layoutW, layoutH, layoutEm);
                        }
                        pageCount = doc.countPages();
                        pageCountChanged = true;
                    } catch (Throwable x) {
                        doc = null;
                        pageCount = 1;
                        pageCountChanged = true;
                        currentPage = 0;
                        throw x;
                    }
                }

                public void run() {
                    pageCountChanged = true;
                    if (
                        currentPage < 0 || currentPage >= pageCount
                    ) currentPage = 0;
                    titleLabel.setText(title);
                    if (isReflowable) layoutButton.setVisibility(View.VISIBLE);
                    else zoomButton.setVisibility(View.VISIBLE);
                    loadPage();
                    loadOutline();
                }
            }
        );
    }

    protected void relayoutDocument() {
        worker.add(
            new Worker.Task() {
                public void work() {
                    try {
                        long mark = doc.makeBookmark(
                            doc.locationFromPageNumber(currentPage)
                        );
                        Log.i(APP, "relayout document");
                        doc.layout(layoutW, layoutH, layoutEm);
                        pageCount = doc.countPages();
                        currentPage = doc.pageNumberFromLocation(
                            doc.findBookmark(mark)
                        );
                    } catch (Throwable x) {
                        pageCount = 1;
                        currentPage = 0;
                        throw x;
                    }
                }

                public void run() {
                    history.clear();
                    pageCountChanged = true;
                    loadPage();
                    loadOutline();
                }
            }
        );
    }

    private void loadOutline() {
        worker.add(
            new Worker.Task() {
                boolean outlineTruncated = false;

                private void flattenOutline(
                    Outline[] outline,
                    String indent,
                    int depth
                ) {
                    for (Outline node : outline) {
                        if (node.title != null) {
                            int outlinePage = doc.pageNumberFromLocation(
                                doc.resolveLink(node)
                            );
                            if (
                                flatOutline.size() >= MAXIMUM_OUTLINE_ITEMS
                            ) outlineTruncated = true;
                            else flatOutline.add(
                                new OutlineActivity.Item(
                                    indent + node.title,
                                    node.uri,
                                    outlinePage
                                )
                            );
                        }
                        if (node.down != null) {
                            if (
                                depth >= MAXIMUM_OUTLINE_DEPTH ||
                                flatOutline.size() >= MAXIMUM_OUTLINE_ITEMS
                            ) outlineTruncated = true;
                            else flattenOutline(
                                node.down,
                                indent + "    ",
                                depth + 1
                            );
                        }
                    }
                }

                public void work() {
                    Log.i(APP, "load outline");
                    Outline[] outline = doc.loadOutline();
                    if (outline != null) {
                        flatOutline = new ArrayList<OutlineActivity.Item>();
                        flattenOutline(outline, "", 0);
                    } else {
                        flatOutline = null;
                    }
                }

                public void run() {
                    if (flatOutline != null) outlineButton.setVisibility(
                        View.VISIBLE
                    );
                    if (outlineTruncated) Toast.makeText(
                        DocumentActivity.this,
                        getString(R.string.toast_outline_too_large),
                        Toast.LENGTH_SHORT
                    ).show();
                }
            }
        );
    }

    protected void loadPage() {
        final int pageNumber = currentPage;
        final float zoom = pageZoom;
        stopSearch = true;
        worker.add(
            new Worker.Task() {
                public Bitmap bitmap;
                public Rect[] linkBounds;
                public String[] linkURIs;
                public Quad[][] hits;
                public Matrix savedCtm; // ✅ 新增

                public void work() {
                    try {
                        Log.i(APP, "load page " + pageNumber);
                        Page page = doc.loadPage(pageNumber);
                        Log.i(APP, "draw page " + pageNumber + " zoom=" + zoom);
                        Matrix ctm;
                        if (fitPage) ctm = AndroidDrawDevice.fitPage(
                            page,
                            canvasW,
                            canvasH
                        );
                        else ctm = AndroidDrawDevice.fitPageWidth(
                            page,
                            canvasW
                        );

                        // ✅ 在 zoom 之前保存基础 ctm（与 linkBounds/hits 一致）
                        savedCtm = ctm;

                        Link[] links = page.getLinks();
                        if (links == null) {
                            linkBounds = new Rect[0];
                            linkURIs = new String[0];
                        } else {
                            linkBounds = new Rect[links.length];
                            linkURIs = new String[links.length];
                            for (int i = 0; i < links.length; i++) {
                                linkBounds[i] = links[i]
                                    .getBounds()
                                    .transform(ctm);
                                linkURIs[i] = links[i].getURI();
                            }
                        }
                        if (searchNeedle != null) {
                            hits = page.search(searchNeedle);
                            if (hits != null) for (Quad[] hit : hits)
                                for (Quad chr : hit) chr.transform(ctm);
                        }
                        if (zoom != 1) ctm.scale(zoom);

                        bitmap = AndroidDrawDevice.drawPage(page, ctm);
                    } catch (Throwable x) {
                        Log.e(APP, x.getMessage());
                    }
                }

                public void run() {
                    if (bitmap != null) {
                        // ✅ 核心：根据 mInvertMode 决定是否反色
                        if (mInvertMode) {
                            invertBitmap(bitmap);
                        }

                        mCurrentPageCtm = savedCtm; // ✅ 基础 ctm，不含 zoom

                        pageView.setBitmap(
                            bitmap,
                            zoom,
                            wentBack,
                            toggledUI,
                            newSearchHitPage,
                            linkBounds,
                            linkURIs,
                            hits
                        );
                    } else pageView.setError();
                    showPageNumber(currentPage + 1);
                    pageSeekbar.setMax(pageCount - 1);
                    pageSeekbar.setProgress(pageNumber);
                    wentBack = false;
                    toggledUI = false;
                    newSearchHitPage = false;
                }
            }
        );
    }

    protected void showSearch() {
        currentBar = searchBar;
        actionBar.setVisibility(View.GONE);
        searchBar.setVisibility(View.VISIBLE);
        searchBar.requestFocus();
        showKeyboard();
    }

    protected void hideSearch() {
        currentBar = actionBar;
        actionBar.setVisibility(View.VISIBLE);
        searchBar.setVisibility(View.GONE);
        hideKeyboard();
        resetSearch();
    }

    public void toggleUI() {
        toggledUI = true;
        if (bottomBar.getVisibility() == View.VISIBLE) {
            topBar.setVisibility(View.GONE);
            currentBar.setVisibility(View.GONE);
            bottomBar.setVisibility(View.GONE);
            if (currentBar == searchBar) hideKeyboard();

            updateStatusBarIconMode(mInvertMode);
        } else {
            topBar.setVisibility(View.VISIBLE);
            currentBar.setVisibility(View.VISIBLE);
            bottomBar.setVisibility(View.VISIBLE);
            showPageNumber(currentPage + 1);
            if (currentBar == searchBar) {
                searchBar.requestFocus();
                showKeyboard();
            }

            updateStatusBarIconMode(true);
        }
    }

    public void goBackward() {
        if (currentPage > 0) {
            wentBack = true;
            currentPage--;
            loadPage();
        }
    }

    public void goForward() {
        if (currentPage < pageCount - 1) {
            currentPage++;
            loadPage();
        }
    }

    public void gotoPage(int p) {
        if (p >= 0 && p < pageCount && p != currentPage) {
            history.push(currentPage);
            currentPage = p;
            loadPage();
        }
    }

    public void gotoPage(String uri) {
        gotoPage(doc.pageNumberFromLocation(doc.resolveLink(uri)));
    }

    public void gotoURI(String uri) {
        Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(uri));
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET); // FLAG_ACTIVITY_NEW_DOCUMENT in API>=21
        try {
            startActivity(intent);
        } catch (FileUriExposedException x) {
            Log.e(APP, x.toString());
            Toast.makeText(
                DocumentActivity.this,
                getString(R.string.toast_file_uris_not_allowed) + uri,
                Toast.LENGTH_LONG
            ).show();
        } catch (Throwable x) {
            Log.e(APP, x.getMessage());
            Toast.makeText(
                DocumentActivity.this,
                x.getMessage(),
                Toast.LENGTH_SHORT
            ).show();
        }
    }

    /**
     * 对 Bitmap 进行颜色反转（暗黑模式核心）
     * 保留 Alpha 通道，仅反转 RGB 分量
     */
    private void invertBitmap(Bitmap bitmap) {
        if (bitmap == null || bitmap.isRecycled()) return;

        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        int[] pixels = new int[width * height];

        bitmap.getPixels(pixels, 0, width, 0, 0, width, height);

        for (int i = 0; i < pixels.length; i++) {
            // 0xFF000000 保留 Alpha，0x00FFFFFF 取反 RGB
            pixels[i] = (pixels[i] & 0xFF000000) | (~pixels[i] & 0x00FFFFFF);
        }

        bitmap.setPixels(pixels, 0, width, 0, 0, width, height);
    }

    /**
     * 根据暗黑模式切换状态栏图标颜色
     * @param isDark true=暗黑模式，状态栏图标白色；false=亮色模式，状态栏图标黑色
     */
    private void updateStatusBarIconMode(boolean isDark) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            // Android6.0以下不支持状态栏图标变色，直接返回
            return;
        }
        Window window = getWindow();
        int vis = window.getDecorView().getSystemUiVisibility();
        // 基础flag保持不变：LAYOUT_STABLE | LAYOUT_FULLSCREEN
        int baseFlags =
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN;

        if (!isDark) {
            // 亮色模式：开启轻量状态栏，图标黑色
            vis = baseFlags | View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
        } else {
            // 暗黑模式：移除LIGHT_STATUS_BAR，图标白色
            vis = baseFlags;
        }
        window.getDecorView().setSystemUiVisibility(vis);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            enterSystemFullscreen();
        }
    }

    /**
     * 真正进入全屏：隐藏状态栏、导航栏
     */
    private void enterSystemFullscreen() {
        Window window = getWindow();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            WindowInsetsController controller = window.getInsetsController();
            if (controller != null) {
                controller.hide(
                    WindowInsets.Type.statusBars() |
                        WindowInsets.Type.navigationBars()
                );
                controller.setSystemBarsBehavior(
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
                );
            }
        } else {
            // API <30 旧版本
            int vis = window.getDecorView().getSystemUiVisibility();
            vis |= View.SYSTEM_UI_FLAG_FULLSCREEN;
            vis |= View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
            vis |= View.SYSTEM_UI_FLAG_IMMERSIVE;
            window.getDecorView().setSystemUiVisibility(vis);
        }
    }

    /**
     * 退出系统全屏，恢复状态栏、导航栏显示
     */
    private void exitSystemFullscreen() {
        Window window = getWindow();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            WindowInsetsController controller = window.getInsetsController();
            if (controller != null) {
                controller.show(
                    WindowInsets.Type.statusBars() |
                        WindowInsets.Type.navigationBars()
                );
            }
        } else {
            int vis = window.getDecorView().getSystemUiVisibility();
            // 清除全屏、隐藏导航栏标记，保留 LAYOUT_STABLE | LAYOUT_FULLSCREEN 布局延伸
            vis &= ~(
                View.SYSTEM_UI_FLAG_FULLSCREEN |
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_IMMERSIVE
            );
            window.getDecorView().setSystemUiVisibility(vis);
        }
    }

    // TTS///////////////////////////////////////////////////////////////////////
    /** TTS 句子进度监听器 */
    private final TTSUtils.OnSentenceProgressListener mTtsSentenceListener =
        new TTSUtils.OnSentenceProgressListener() {
            @Override
            public void onSentenceChanged(String sentence) {
                if (!mIsTtsReading) return;

                if ("__TTS_PLAY_FINISHED__".equals(sentence)) {
                    Log.i(APP, "TTS FINISHED on page " + mTtsReadingPage);
                    autoAdvanceTtsPage();
                    return;
                }

                highlightCurrentSentence(sentence);
            }
        };

    /** 开始朗读 */
    /*public void startTtsReading() {
        if (mIsTtsReading) {
            stopTtsReading();
            return;
        }

        mIsTtsReading = true;
        mTtsReadingPage = currentPage;
        updateTtsButtonState();
        updateTtsButtonState(); // ✅ 切换为停止图标
        readCurrentPage();
    }*/
    public void startTtsReading() {
        if (mIsTtsReading) {
            stopTtsReading();
            return;
        }
        mIsTtsReading = true;
        mTtsReadingPage = currentPage;
        updateTtsButtonState();
        // =========睡眠定时逻辑=========
        if (mSleepTimerEnabled) {
            // 先移除旧任务，重新开始2小时倒计时
            mSleepTimerHandler.removeCallbacks(mSleepStopTtsRunnable);
            mSleepTimerHandler.postDelayed(
                mSleepStopTtsRunnable,
                2 * 60 * 60 * 1000
            );
        }
        // =============================
        readCurrentPage();
    }

    /** 停止朗读 */
    /*public void stopTtsReading() {
        mIsTtsReading = false;
        mTtsReadingPage = -1;
        MyService.stopTextPlay();
        pageView.clearTtsHighlight();
        updateTtsButtonState();
        updateTtsButtonState(); // ✅ 切换为播放图标
    }*/
    public void stopTtsReading() {
        mIsTtsReading = false;
        mTtsReadingPage = -1;
        MyService.stopTextPlay();
        pageView.clearTtsHighlight();
        updateTtsButtonState();
        // =========睡眠定时：停止朗读就取消倒计时=========
        mSleepTimerHandler.removeCallbacks(mSleepStopTtsRunnable);
        // 不自动取消睡眠开关，用户可以继续保持开启，下次播放自动重新计时
    }

    /** 读取并播放当前页 */
    private void readCurrentPage() {
        worker.add(
            new Worker.Task() {
                String text;

                public void work() {
                    text = extractPageText(mTtsReadingPage);
                }

                public void run() {
                    if (!mIsTtsReading) return;

                    if (text == null || text.trim().isEmpty()) {
                        Log.i(
                            APP,
                            "Page " + mTtsReadingPage + " is empty, skipping..."
                        );
                        autoAdvanceTtsPage();
                        return;
                    }

                    mCurrentPageText = text.trim();

                    // ✅ 不再手动递增版本号！
                    // 版本号由 setBitmap 统一管理，这里只读取当前版本
                    final int currentVersion = pageView.getPageVersion();

                    Log.i(
                        APP,
                        "TTS start reading page " +
                            mTtsReadingPage +
                            ", version=" +
                            currentVersion +
                            ", text length=" +
                            mCurrentPageText.length()
                    );

                    MyService.playTextWithListener(
                        mCurrentPageText,
                        mTtsSentenceListener
                    );
                }
            }
        );
    }

    /** 自动翻到下一页继续朗读 */
    protected void autoAdvanceTtsPage() {
        if (!mIsTtsReading) return;

        if (mTtsReadingPage < pageCount - 1) {
            mTtsReadingPage++;
            currentPage = mTtsReadingPage;
            pageView.clearTtsHighlight();
            pageView.saveCurrentScrollX(); // ✅ TTS 翻页前快照水平位置
            loadPage();
            readCurrentPage();
        } else {
            stopTtsReading();
            runOnUiThread(() ->
                Toast.makeText(
                    this,
                    "Finished reading the entire book.",
                    Toast.LENGTH_SHORT
                ).show()
            );
        }
    }

    /** 高亮搜索 — 使用实时版本号 */
    protected void highlightCurrentSentence(String sentence) {
        if (sentence == null || sentence.trim().isEmpty()) return;

        // ✅ 关键：将 \n 替换为空格，使搜索字符串变为单行
        final String original = sentence.trim().replaceAll("\\n+", " ");
        final int capturedVersion = pageView.getPageVersion();
        final Matrix ctm = mCurrentPageCtm; // ✅ 捕获当前页的 ctm

        if (ctm == null) return; // 页面尚未渲染完成

        worker.add(
            new Worker.Task() {
                Quad[] quads;

                public void work() {
                    try {
                        Page page = doc.loadPage(mTtsReadingPage);

                        // 三级降级搜索
                        Quad[][] hits = page.search(original);

                        if (hits == null || hits.length == 0) {
                            String compressed = original.replaceAll(
                                "\\s+",
                                " "
                            );
                            if (!compressed.equals(original)) {
                                hits = page.search(compressed);
                            }
                        }

                        if (hits == null || hits.length == 0) {
                            String anchor =
                                original.length() > 10
                                    ? original
                                          .substring(0, 10)
                                          .replaceAll("\\s+", " ")
                                    : original.replaceAll("\\s+", " ");
                            if (anchor.length() >= 2) {
                                hits = page.search(anchor);
                            }
                        }

                        if (hits != null && hits.length > 0) {
                            quads = hits[0];
                            // ✅ 关键：对 Quad 应用和渲染页面相同的 ctm
                            for (Quad q : quads) {
                                q.transform(ctm);
                            }
                        } else {
                            Log.w(APP, "TTS search failed: [" + original + "]");
                        }
                        page.destroy();
                    } catch (Throwable x) {
                        Log.e(APP, "highlight error: " + x.getMessage());
                    }
                }

                public void run() {
                    if (quads != null && mIsTtsReading) {
                        pageView.setTtsHighlight(quads, capturedVersion);
                    }
                }
            }
        );
    }

    /////////////////////////////////////////////////////////////////////////////////////

    /** 切换 TTS 播放/停止 */
    private void toggleTts() {
        if (mIsTtsReading) {
            stopTtsReading();
        } else {
            startTtsReading();
        }
    }

    /** 更新按钮图标状态 */
    private void updateTtsButtonState() {
        if (ttsButton == null) return;

        if (mIsTtsReading) {
            ttsButton.setImageResource(R.drawable.ic_stop_white_24dp);
            ttsButton.setContentDescription("Stop");
        } else {
            ttsButton.setImageResource(R.drawable.ic_volume_up_white_24dp);
            ttsButton.setContentDescription("Play");
        }
    }

    /**
     * 提取指定页面的纯文本内容（用于 TTS 朗读）
     * MuPDF 1.28.0 兼容版 - 不使用 StructuredTextWalker
     */
    private String extractPageText(int pageNumber) {
        try {
            Page page = doc.loadPage(pageNumber);
            StructuredText stext = page.toStructuredText("preserve-whitespace");

            String text = null;

            // 方案1: 尝试 asText()（MuPDF 1.24+ 部分构建版本提供）
            try {
                java.lang.reflect.Method m = stext
                    .getClass()
                    .getMethod("asText");
                text = (String) m.invoke(stext);
            } catch (Exception ignored) {}

            // 方案2: 尝试 toPlainText()
            if (text == null) {
                try {
                    java.lang.reflect.Method m = stext
                        .getClass()
                        .getMethod("toPlainText");
                    text = (String) m.invoke(stext);
                } catch (Exception ignored) {}
            }

            // 方案3: 尝试 search("") 或 getText() 等其他可能的方法名
            if (text == null) {
                for (String methodName : new String[] {
                    "getText",
                    "toString",
                    "asString",
                }) {
                    try {
                        java.lang.reflect.Method m = stext
                            .getClass()
                            .getMethod(methodName);
                        Object result = m.invoke(stext);
                        if (
                            result instanceof String &&
                            !((String) result).isEmpty()
                        ) {
                            text = (String) result;
                            break;
                        }
                    } catch (Exception ignored) {}
                }
            }

            page.destroy();
            stext.destroy();

            if (text != null) {
                return text.trim();
            }

            // 方案4: 终极兜底 - 用 page.search 逐字符不可行，
            // 改为打印所有可用方法帮助调试
            Log.e(APP, "extractPageText: No text extraction method found.");
            Log.e(APP, "Available methods on StructuredText:");
            for (java.lang.reflect.Method m : stext.getClass().getMethods()) {
                Log.e(
                    APP,
                    "  " +
                        m.getName() +
                        "(" +
                        java.util.Arrays.toString(m.getParameterTypes()) +
                        ") -> " +
                        m.getReturnType().getSimpleName()
                );
            }
            return "";
        } catch (Exception e) {
            Log.e(APP, "extractPageText failed: " + e.getMessage());
            return "";
        }
    }

    ////////////////////////////////////////////////////////////////////////////
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
                    System.out.println("ClockActivity HOME键被按下...");
                } else if (TextUtils.equals(reason, SYSTEM_HOME_KEY_LONG)) {
                    System.out.println("ClockActivity 长按HOME键...");
                }
            }
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////

    // JNI回调：通知C++，需要把这个Uri对应的txt转为pdf
    private void requestConvertTxtToPdf(Uri txtUri) {
        runOnUiThread(() -> {
            // 调用C++方法，参数是txt的Uri字符串
            MyActivity.mInstance.PublicJavaCallCpp(
                "txt_convert_to_pdf|==|" + txtUri.toString()
            );
        });
    }

    /**
     * 【供C++ JNI调用】C++完成txt转pdf后，回传生成的pdf二进制数组
     */
    /*public void setConvertedPdfBuffer(byte[] pdfBytes) {
        runOnUiThread(() -> {
            if (isFinishing() || isDestroyed()) return;
            if (worker == null) return; // 增加保护，防止worker还没初始化完成

            // 关闭等待转圈弹窗
            if (
                mConvertProgressDialog != null &&
                mConvertProgressDialog.isShowing()
            ) {
                mConvertProgressDialog.dismiss();
                mConvertProgressDialog = null;
            }

            mConvertedPdfBuffer = pdfBytes;
            buffer = mConvertedPdfBuffer;
            stream = null;
            worker.add(
                new Worker.Task() {
                    boolean needsPassword;

                    public void work() {
                        Log.i(APP, "Loaded converted txt->pdf from buffer");
                        doc = Document.openDocument(buffer, "application/pdf");
                        needsPassword = doc.needsPassword();
                    }

                    public void run() {
                        if (needsPassword) askPassword(
                            R.string.dlog_password_message
                        );
                        else loadDocument();
                    }
                }
            );
        });
        }*/
    public void setConvertedPdfBuffer(byte[] pdfBytes) {
        // ✅ 在 Lambda 外面完成 Buffer 清理（此处不在 Lambda 内，可以随意修改）
        if (pdfBytes != null && pdfBytes.length > 0) {
            int start = 0;
            while (start < pdfBytes.length && pdfBytes[start] != '%') {
                start++;
            }
            if (start > 0) {
                byte[] cleaned = new byte[pdfBytes.length - start];
                System.arraycopy(pdfBytes, start, cleaned, 0, cleaned.length);
                pdfBytes = cleaned;
                Log.i(
                    APP,
                    "Trimmed " + start + " leading bytes from PDF buffer"
                );
            }

            // 验证 PDF 头
            if (
                pdfBytes.length < 5 ||
                pdfBytes[0] != '%' ||
                pdfBytes[1] != 'P' ||
                pdfBytes[2] != 'D' ||
                pdfBytes[3] != 'F'
            ) {
                Log.e(APP, "Invalid PDF buffer header after trimming!");
                return;
            }
        }

        // ✅ 用一个 final 变量捕获处理后的结果，供 Lambda 使用
        final byte[] finalPdfBytes = pdfBytes;

        runOnUiThread(() -> {
            if (isFinishing() || isDestroyed()) return;
            if (worker == null) return;

            // 关闭等待弹窗
            if (
                mConvertProgressDialog != null &&
                mConvertProgressDialog.isShowing()
            ) {
                mConvertProgressDialog.dismiss();
                mConvertProgressDialog = null;
            }

            // ✅ Lambda 内只读取 finalPdfBytes，不再修改任何外部变量
            mConvertedPdfBuffer = finalPdfBytes;
            buffer = mConvertedPdfBuffer;
            stream = null;

            worker.add(
                new Worker.Task() {
                    boolean needsPassword;

                    public void work() {
                        Log.i(
                            APP,
                            "Opening converted txt->pdf from cleaned buffer, size=" +
                                (finalPdfBytes != null
                                    ? finalPdfBytes.length
                                    : 0)
                        );
                        doc = Document.openDocument(
                            finalPdfBytes,
                            "application/pdf"
                        );
                        needsPassword = doc.needsPassword();
                    }

                    public void run() {
                        if (needsPassword) askPassword(
                            R.string.dlog_password_message
                        );
                        else loadDocument();
                    }
                }
            );
        });
    }

    public static String decodeGbk(byte[] data) {
        try {
            return new String(data, "GBK");
        } catch (java.io.UnsupportedEncodingException e) {
            return new String(data); // 兜底 UTF-8
        }
    }

    /** 更新睡眠按钮图标状态：开启时图标高亮 */
    private void updateSleepTimerButtonState() {
        if (sleepTimerButton == null) return;
        if (mSleepTimerEnabled) {
            sleepTimerButton.setColorFilter(0xFF42A5F5); //蓝色高亮，表示睡眠定时已启用
        } else {
            sleepTimerButton.clearColorFilter();
        }
    }

    /**
     * 长按选词入口（由 PageView 回调）
     * 提取当前页全文 + 定位长按处文字，弹出独立文本窗口
     */
    public void onLongPressSelectText(
        float touchX,
        float touchY,
        int scrollX,
        int scrollY,
        int bitmapW,
        int bitmapH,
        int canvasW,
        int canvasH,
        float viewScale,
        float pageScale,
        View anchorView
    ) {
        worker.add(
            new Worker.Task() {
                String fullText;
                String nearText;

                public void work() {
                    try {
                        Page page = doc.loadPage(currentPage);
                        StructuredText stext = page.toStructuredText(
                            "preserve-whitespace"
                        );

                        // ===== 1. 提取全页文字 =====
                        fullText = extractPageText(currentPage);

                        // ===== 2. 坐标转换：屏幕 → 文档 =====
                        Matrix baseCtm;
                        if (fitPage) {
                            baseCtm = AndroidDrawDevice.fitPage(
                                page,
                                canvasW,
                                canvasH
                            );
                        } else {
                            baseCtm = AndroidDrawDevice.fitPageWidth(
                                page,
                                canvasW
                            );
                        }
                        float totalScale = baseCtm.a * pageZoom;

                        float dx =
                            bitmapW <= canvasW
                                ? (bitmapW - canvasW) / 2f
                                : scrollX;
                        float dy =
                            bitmapH <= canvasH
                                ? (bitmapH - canvasH) / 2f
                                : scrollY;
                        float bmpX = touchX + dx;
                        float bmpY = touchY + dy;
                        float docX = bmpX / totalScale;
                        float docY = bmpY / totalScale;

                        Log.i(
                            APP,
                            "LongPress coord: touch=(" +
                                touchX +
                                "," +
                                touchY +
                                ") doc=(" +
                                docX +
                                "," +
                                docY +
                                ") totalScale=" +
                                totalScale
                        );

                        // ===== 3. 获取长按位置附近的文字片段 =====
                        nearText = null;
                        float r = 5f; // 稍大的搜索半径，提高命中率
                        Point p1 = new Point(docX - r, docY - r);
                        Point p2 = new Point(docX + r, docY + r);
                        Quad[] hlQuads = stext.highlight(p1, p2);

                        if (hlQuads != null && hlQuads.length > 0) {
                            // 取距离触摸点最近的 quad
                            Quad nearest = hlQuads[0];
                            float minDist = Float.MAX_VALUE;
                            for (Quad q : hlQuads) {
                                float cx = (q.ul_x + q.lr_x) / 2f;
                                float cy = (q.ul_y + q.lr_y) / 2f;
                                float dist =
                                    (cx - docX) * (cx - docX) +
                                    (cy - docY) * (cy - docY);
                                if (dist < minDist) {
                                    minDist = dist;
                                    nearest = q;
                                }
                            }

                            // 扩大复制范围：用 nearest quad 所在行的范围
                            // 这样能拿到完整的一行/一句话，而非单个字符
                            float lineTop = nearest.ul_y - 2f;
                            float lineBottom = nearest.lr_y + 2f;
                            // 向左扩展一些，尽量拿到所在行的更多文字
                            float lineLeft = Math.max(0, nearest.ul_x - 200f);
                            float lineRight = nearest.lr_x + 200f;

                            Point qa = new Point(lineLeft, lineTop);
                            Point qb = new Point(lineRight, lineBottom);
                            nearText = stext.copy(qa, qb);

                            Log.i(
                                APP,
                                "LongPress nearText raw: [" + nearText + "]"
                            );
                        }

                        stext.destroy();
                        page.destroy();
                    } catch (Throwable x) {
                        Log.e(
                            APP,
                            "onLongPressSelectText error: " + x.getMessage()
                        );
                    }
                }

                public void run() {
                    // ===== 防御性检查 =====
                    if (fullText == null || fullText.trim().isEmpty()) {
                        Toast.makeText(
                            DocumentActivity.this,
                            MyActivity.zh_cn
                                ? "此页无可提取文字"
                                : "No extractable text on this page",
                            Toast.LENGTH_SHORT
                        ).show();
                        return;
                    }

                    // 清理 nearText
                    String target = null;
                    if (nearText != null) {
                        target = nearText.trim();
                        // 去掉前后可能的空白行
                        target = target.replaceAll("^\\n+|\\n+$", "").trim();
                    }

                    showTextSelectionDialog(fullText.trim(), target);
                }
            }
        );
    }

    /**
     * 弹出独立文本窗口：显示当前页全文，高亮并定位到长按处的文字
     * 用户可在 TextView 上使用原生选词能力（长按、拖动、复制、分享）
     *
     * @param fullText   当前页全量纯文本
     * @param targetText 长按位置附近的文字片段（用于定位高亮），可为 null
     */
    private void showTextSelectionDialog(String fullText, String targetText) {
        // ===== 构建 SpannableString =====
        SpannableString spannable = new SpannableString(fullText);
        int highlightStart = -1;
        int highlightEnd = -1;

        if (targetText != null && !targetText.isEmpty()) {
            // 策略1: 完整匹配
            int idx = fullText.indexOf(targetText);

            // 策略2: 去掉多余空白后再匹配
            if (idx < 0) {
                String compressed = targetText.replaceAll("\\s+", " ").trim();
                if (compressed.length() >= 3) {
                    idx = fullText.indexOf(compressed);
                    if (idx >= 0) {
                        targetText = compressed;
                    }
                }
            }

            // 策略3: 取前 20 个字符作为锚点匹配
            if (idx < 0 && targetText.length() > 20) {
                String anchor = targetText.substring(0, 20).trim();
                if (anchor.length() >= 4) {
                    idx = fullText.indexOf(anchor);
                    if (idx >= 0) {
                        targetText = anchor;
                    }
                }
            }

            // 策略4: 取前 10 个字符
            if (idx < 0 && targetText.length() > 10) {
                String anchor = targetText.substring(0, 10).trim();
                if (anchor.length() >= 3) {
                    idx = fullText.indexOf(anchor);
                    if (idx >= 0) {
                        targetText = anchor;
                    }
                }
            }

            if (idx >= 0) {
                highlightStart = idx;
                highlightEnd = Math.min(
                    idx + targetText.length(),
                    fullText.length()
                );

                // 高亮背景色
                /*spannable.setSpan(
                    new BackgroundColorSpan(0x88FFEB3B), // 半透明黄色
                    highlightStart,
                    highlightEnd,
                    Spanned.SPAN_EXCLUSIVE_EXCLUSIVE
                );*/
                int hlColor = mInvertMode ? 0x66FFD54F : 0x88FFEB3B;
                spannable.setSpan(
                    new CenteredHighlightSpan(hlColor),
                    highlightStart,
                    highlightEnd,
                    Spanned.SPAN_EXCLUSIVE_EXCLUSIVE
                );
            } else {
                Log.w(
                    APP,
                    "showTextSelectionDialog: target not found in fullText, target=[" +
                        targetText +
                        "]"
                );
            }
        }

        // ===== 构建 UI 组件 =====
        int dp16 = (int) TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_DIP,
            16,
            getResources().getDisplayMetrics()
        );
        int dp12 = (int) TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_DIP,
            12,
            getResources().getDisplayMetrics()
        );

        // 页面标题标签
        TextView headerLabel = new TextView(this);
        headerLabel.setText(
            (MyActivity.zh_cn ? "第 " : "Page ") +
                (currentPage + 1) +
                (MyActivity.zh_cn ? " 页" : "")
        );
        headerLabel.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12);
        headerLabel.setTextColor(0xFF999999);
        headerLabel.setPadding(dp16, dp12, dp16, 0);

        // 核心 TextView：可选中、可复制
        final TextView textView = new TextView(this);
        textView.setText(spannable);
        textView.setTextIsSelectable(true);

        textView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 16);
        textView.setTextColor(mInvertMode ? 0xFFDDDDDD : 0xFF333333);
        textView.setLineSpacing(0, 1.4f);
        textView.setPadding(dp16, dp12, dp16, dp16);

        // ScrollView
        final ScrollView scrollView = new ScrollView(this);
        scrollView.addView(
            textView,
            new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
        );
        scrollView.setFillViewport(true);
        scrollView.setBackgroundColor(mInvertMode ? 0xFF1E1E1E : 0xFFFAFAFA);

        // 外层容器：header + scrollView
        LinearLayout container = new LinearLayout(this);
        container.setOrientation(LinearLayout.VERTICAL);
        container.addView(
            headerLabel,
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
        );
        container.addView(
            scrollView,
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                0,
                1f // weight=1，占满剩余空间
            )
        );

        // ===== 弹窗尺寸：占屏幕 85% 宽高 =====
        DisplayMetrics dm = getResources().getDisplayMetrics();

        // 宽度：85% 屏幕宽，但不超过 1200dp（平板友好）
        int maxW = (int) TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_DIP,
            1200,
            dm
        );
        int dialogW = Math.min((int) (dm.widthPixels * 0.75f), maxW);

        // 高度：85% 屏幕高，但不超过 900dp
        int maxH = (int) TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_DIP,
            900,
            dm
        );
        int dialogH = Math.min((int) (dm.heightPixels * 0.85f), maxH);

        // ===== 创建 AlertDialog =====
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setView(container);
        builder.setNegativeButton(MyActivity.zh_cn ? "关闭" : "Close", null);

        final AlertDialog dialog = builder.create();
        if (dialog.getWindow() != null) {
            dialog.getWindow().setDimAmount(0.5f);
        }

        // ✅ 在 show() 之前设置 Callback，此时 dialog 已创建
        textView.setCustomSelectionActionModeCallback(
            new TextSelectionActionModeCallback(textView, dialog)
        );

        dialog.show();

        // 必须在 show() 之后设置窗口尺寸
        if (dialog.getWindow() != null) {
            dialog.getWindow().setLayout(dialogW, dialogH);
        }

        // ===== 自动滚动到高亮位置 =====
        if (highlightStart >= 0) {
            final int fStart = highlightStart;
            final int fEnd = highlightEnd;

            textView.post(() -> {
                android.text.Layout layout = textView.getLayout();
                if (layout == null) {
                    // layout 尚未就绪，再等一帧
                    textView.post(() ->
                        scrollToHighlight(textView, scrollView, fStart, fEnd)
                    );
                } else {
                    scrollToHighlight(textView, scrollView, fStart, fEnd);
                }
            });
        }
    }

    /**
     * 自定义文本选择菜单回调：
     * 在保留系统默认菜单（复制/分享/全选）的基础上，追加 AI搜索、网页搜索、添加笔记。
     */
    private class TextSelectionActionModeCallback
        implements ActionMode.Callback
    {

        private final TextView textView;
        private final androidx.appcompat.app.AlertDialog parentDialog;

        private static final int ID_AI_SEARCH = 1001;
        private static final int ID_WEB_SEARCH = 1002;
        private static final int ID_ADD_NOTE = 1003;

        /** 上下文截取半径（字符数） */
        private static final int CONTEXT_RADIUS = 50;

        public TextSelectionActionModeCallback(
            TextView textView,
            AlertDialog parentDialog
        ) {
            this.textView = textView;
            this.parentDialog = parentDialog;
        }

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            int order = 100;
            menu.add(
                Menu.NONE,
                ID_AI_SEARCH,
                order++,
                MyActivity.zh_cn ? "AI 搜索" : "AI Search"
            ).setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);

            menu.add(
                Menu.NONE,
                ID_WEB_SEARCH,
                order++,
                MyActivity.zh_cn ? "网页搜索" : "Web Search"
            ).setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);

            menu.add(
                Menu.NONE,
                ID_ADD_NOTE,
                order++,
                MyActivity.zh_cn ? "添加笔记" : "Add Note"
            ).setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);

            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            return false;
        }

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            int start = textView.getSelectionStart();
            int end = textView.getSelectionEnd();

            if (start < 0 || end < 0 || start == end) return false;

            String selectedText = textView
                .getText()
                .subSequence(start, end)
                .toString()
                .trim();
            if (selectedText.isEmpty()) return false;

            switch (item.getItemId()) {
                case ID_AI_SEARCH:
                    MyActivity.mInstance.PublicJavaCallCpp(
                        "pdf_ai_search|==|" + selectedText
                    );
                    mode.finish();
                    if (
                        parentDialog != null && parentDialog.isShowing()
                    ) parentDialog.dismiss();
                    return true;
                case ID_WEB_SEARCH:
                    try {
                        Intent intent = new Intent(Intent.ACTION_WEB_SEARCH);
                        intent.putExtra(
                            android.app.SearchManager.QUERY,
                            selectedText
                        );
                        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        DocumentActivity.this.startActivity(intent);
                    } catch (Exception e) {
                        Log.e(APP, "Web search failed: " + e.getMessage());
                        Toast.makeText(
                            DocumentActivity.this,
                            MyActivity.zh_cn
                                ? "无法启动网页搜索"
                                : "Cannot launch web search",
                            Toast.LENGTH_SHORT
                        ).show();
                    }
                    mode.finish();
                    if (
                        parentDialog != null && parentDialog.isShowing()
                    ) parentDialog.dismiss();
                    return true;
                case ID_ADD_NOTE:
                    // ✅ 在此处提取上下文
                    CharSequence fullText = textView.getText();
                    int totalLen = fullText.length();
                    int ctxStart = Math.max(0, start - CONTEXT_RADIUS);
                    int ctxEnd = Math.min(totalLen, end + CONTEXT_RADIUS);
                    String searchContext = fullText
                        .subSequence(ctxStart, ctxEnd)
                        .toString()
                        .replaceAll("\\s+", " ")
                        .trim();

                    // ✅ 调用外部统一的 showNoteInputDialog（新增模式）
                    showNoteInputDialog(
                        selectedText,
                        null,
                        searchContext,
                        null,
                        parentDialog
                    );
                    return true;
                default:
                    return false;
            }
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            // 留空
        }
    } // 结束 TextSelectionActionModeCallback 类

    /**
     * 将 ScrollView 滚动到 TextView 中指定字符范围的位置，使其居中显示
     */
    private void scrollToHighlight(
        TextView textView,
        ScrollView scrollView,
        int start,
        int end
    ) {
        android.text.Layout layout = textView.getLayout();
        if (layout == null) return;

        // 获取高亮区域的像素 Y 坐标
        int lineTop = layout.getLineTop(layout.getLineForOffset(start));
        int lineBottom = layout.getLineBottom(
            layout.getLineForOffset(
                Math.min(end, layout.getText().length() - 1)
            )
        );
        int highlightCenterY = (lineTop + lineBottom) / 2;

        // 加上 textView 自身的 padding
        highlightCenterY += textView.getPaddingTop();

        // 计算滚动目标：让高亮位置出现在视口垂直中心偏上 1/3 处
        int scrollTarget = highlightCenterY - scrollView.getHeight() / 3;
        scrollTarget = Math.max(0, scrollTarget);

        scrollView.smoothScrollTo(0, scrollTarget);
    }

    /**
     * 垂直居中包裹文字的高亮 Span
     * 以文字视觉中心为基准，上下各扩展固定比例的行高，
     * 形成均匀包裹效果，而非撑满整行或仅贴附 Ascent-Descent。
     */
    private static class CenteredHighlightSpan extends BackgroundColorSpan {

        /** 上下各扩展的比例（相对于行高）。0.15 ≈ 视觉上均匀包裹中文 */
        private static final float VERTICAL_PADDING_RATIO = 0.15f;

        public CenteredHighlightSpan(int color) {
            super(color);
        }

        public void drawBackground(
            Canvas canvas,
            CharSequence text,
            int start,
            int end,
            float x,
            int top,
            int y,
            int bottom,
            Paint paint
        ) {
            int lineHeight = bottom - top;
            float padding = lineHeight * VERTICAL_PADDING_RATIO;

            // 文字视觉中心 = (top + bottom) / 2
            // 但更精确的做法是以 baseline(y) 为参考，
            // 因为 top/bottom 已含 LineSpacingExtra，可能不对称。
            // 这里用 ascent/descent 算出纯文字高度，再居中于 baseline：
            Paint.FontMetrics fm = paint.getFontMetrics();
            float textHeight = fm.descent - fm.ascent; // 纯正文字高度
            float visualCenter = y + (fm.ascent + fm.descent) / 2f;

            float halfBar = textHeight / 2f + padding;
            float drawTop = visualCenter - halfBar;
            float drawBottom = visualCenter + halfBar;

            float textWidth = paint.measureText(text, start, end);

            int savedColor = paint.getColor();
            paint.setColor(getBackgroundColor());
            canvas.drawRect(x, drawTop, x + textWidth, drawBottom, paint);
            paint.setColor(savedColor);
        }
    }

    /**
     * C++ JNI调用入口：PDF笔记列表弹窗
     * @param noteList 笔记数组，每条以 === 分割8个字段
     * 字段顺序：id === currentPage === time === contextHtml === noteContent === searchContext === keyword === color
     */
    /**
     * C++ JNI调用入口：PDF笔记列表弹窗
     */
    public void showNoteListDialog(ArrayList<String> noteList) {
        runOnUiThread(() -> {
            if (noteList == null || noteList.isEmpty()) return;
            final ArrayList<String> rawNoteData = new ArrayList<>(noteList);
            final int[] selectedPos = { -1 };

            // ... adapter 代码保持不变（ViewHolder 那一段）...
            ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                this,
                0,
                rawNoteData
            ) {
                @Override
                public View getView(
                    int position,
                    View convertView,
                    ViewGroup parent
                ) {
                    View itemView = convertView;
                    ViewHolder holder;
                    if (itemView == null) {
                        LinearLayout layout = new LinearLayout(
                            DocumentActivity.this
                        );
                        layout.setOrientation(LinearLayout.VERTICAL);
                        layout.setPadding(
                            dp2px(12),
                            dp2px(12),
                            dp2px(12),
                            dp2px(12)
                        );
                        TextView tvPageTime = new TextView(
                            DocumentActivity.this
                        );
                        tvPageTime.setTextSize(TypedValue.COMPLEX_UNIT_SP, 14);
                        TextView tvHtml = new TextView(DocumentActivity.this);
                        tvHtml.setTextSize(TypedValue.COMPLEX_UNIT_SP, 13);
                        tvHtml.setTypeface(
                            Typeface.create(
                                tvHtml.getTypeface(),
                                Typeface.ITALIC
                            )
                        );
                        TextView tvNote = new TextView(DocumentActivity.this);
                        tvNote.setTextSize(TypedValue.COMPLEX_UNIT_SP, 14);
                        layout.addView(tvPageTime);
                        layout.addView(tvHtml);
                        layout.addView(tvNote);
                        itemView = layout;
                        holder = new ViewHolder();
                        holder.tvPageTime = tvPageTime;
                        holder.tvHtml = tvHtml;
                        holder.tvNote = tvNote;
                        itemView.setTag(holder);
                    } else {
                        holder = (ViewHolder) itemView.getTag();
                    }
                    String fullStr = rawNoteData.get(position);
                    String[] parts = fullStr.split("===", 8);
                    if (parts.length < 8) return itemView;
                    holder.tvPageTime.setText(
                        (MyActivity.zh_cn ? "页码：" : "Page:") +
                            parts[1].trim() +
                            " | " +
                            parts[2].trim()
                    );
                    Spanned spannedHtml;
                    if (
                        android.os.Build.VERSION.SDK_INT >=
                        android.os.Build.VERSION_CODES.N
                    ) {
                        spannedHtml = Html.fromHtml(
                            parts[3].trim(),
                            Html.FROM_HTML_MODE_LEGACY
                        );
                    } else {
                        spannedHtml = Html.fromHtml(parts[3].trim());
                    }
                    holder.tvHtml.setText(spannedHtml);
                    holder.tvNote.setText(parts[4].trim());
                    itemView.setBackgroundColor(
                        selectedPos[0] == position ? 0xFFE0EDFF : 0x00000000
                    );
                    return itemView;
                }

                class ViewHolder {

                    TextView tvPageTime;
                    TextView tvHtml;
                    TextView tvNote;
                }
            };

            ListView listView = new ListView(this);
            listView.setAdapter(adapter);
            listView.setPadding(dp2px(8), dp2px(8), dp2px(8), dp2px(8));
            listView.setDividerHeight(dp2px(8));

            LinearLayout root = new LinearLayout(this);
            root.setOrientation(LinearLayout.VERTICAL);
            root.addView(
                listView,
                new LinearLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    0,
                    1.0f
                )
            );

            LinearLayout btnBar = new LinearLayout(this);
            btnBar.setPadding(dp2px(8), dp2px(8), dp2px(8), dp2px(8));
            btnBar.setWeightSum(4);
            btnBar.setOrientation(LinearLayout.HORIZONTAL);
            TextView btnGo = createButton(MyActivity.zh_cn ? "转到" : "Go");
            TextView btnEdit = createButton(MyActivity.zh_cn ? "编辑" : "Edit");
            TextView btnDel = createButton(
                MyActivity.zh_cn ? "删除" : "Delete"
            );
            TextView btnClose = createButton(
                MyActivity.zh_cn ? "关闭" : "Close"
            );
            btnBar.addView(btnGo);
            btnBar.addView(btnEdit);
            btnBar.addView(btnDel);
            btnBar.addView(btnClose);
            root.addView(btnBar);

            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setView(root);
            final AlertDialog dialog = builder.create();

            DisplayMetrics dm = getResources().getDisplayMetrics();
            int dialogW = (int) (dm.widthPixels * 0.95f);
            int dialogH = (int) (dm.heightPixels * 0.95f);

            listView.setOnItemClickListener((parent, view, position, id) -> {
                selectedPos[0] = position;
                adapter.notifyDataSetChanged();
            });

            // ✅ 转到：关闭弹窗 → 跳页 → 高亮关键词
            btnGo.setOnClickListener(v -> {
                if (selectedPos[0] < 0) return;
                String[] parts = rawNoteData
                    .get(selectedPos[0])
                    .split("===", 8);
                int targetPage = Integer.parseInt(parts[1].trim());
                String keyword = parts[6].trim();

                dialog.dismiss(); // 先关闭笔记列表

                // 跳转并高亮
                gotoPage(targetPage);
                searchNeedle = keyword;
                loadPage(); // loadPage 内部会用 searchNeedle 执行 page.search() 并渲染高亮
            });

            // ✅ 编辑：复用笔记输入弹窗，预填已有内容
            btnEdit.setOnClickListener(v -> {
                if (selectedPos[0] < 0) return;
                String[] parts = rawNoteData
                    .get(selectedPos[0])
                    .split("===", 8);
                String id = parts[0].trim();
                String keyword = parts[6].trim();
                String existingNote = parts[4].trim();
                String searchContext = parts[5].trim();

                // 弹出编辑弹窗（不关闭笔记列表，编辑完成后自动刷新）
                showNoteInputDialog(
                    keyword,
                    existingNote,
                    searchContext,
                    id,
                    dialog
                );
            });

            // 删除（保持不变）
            btnDel.setOnClickListener(v -> {
                if (selectedPos[0] < 0) return;
                String[] parts = rawNoteData
                    .get(selectedPos[0])
                    .split("===", 8);
                String id = parts[0].trim();
                new AlertDialog.Builder(DocumentActivity.this)
                    .setTitle(MyActivity.zh_cn ? "确认删除" : "Confirm Delete")
                    .setMessage(
                        MyActivity.zh_cn
                            ? "确定删除这条笔记？"
                            : "Are you sure to delete this note?"
                    )
                    .setPositiveButton(
                        MyActivity.zh_cn ? "删除" : "Delete",
                        (d, w) -> {
                            MyActivity.mInstance.PublicJavaCallCpp(
                                "pdf_note_delete|==|" + id
                            );
                            dialog.dismiss();
                        }
                    )
                    .setNegativeButton(
                        MyActivity.zh_cn ? "取消" : "Cancel",
                        null
                    )
                    .show();
            });

            btnClose.setOnClickListener(v -> dialog.dismiss());

            dialog.show();
            if (dialog.getWindow() != null) {
                dialog.getWindow().setLayout(dialogW, dialogH);
                dialog.getWindow().setDimAmount(0.5f);
            }
        });
    }

    // 辅助：创建按钮
    private TextView createButton(String text) {
        TextView tv = new TextView(this);
        tv.setText(text);
        tv.setPadding(dp2px(12), dp2px(8), dp2px(12), dp2px(8));
        tv.setLayoutParams(
            new LinearLayout.LayoutParams(
                0,
                LinearLayout.LayoutParams.WRAP_CONTENT,
                1
            )
        );
        tv.setTextSize(15);
        return tv;
    }

    private int dp2px(int dpVal) {
        return (int) (dpVal * getResources().getDisplayMetrics().density +
            0.5f);
    }

    /**
     * 统一笔记输入弹窗：自动判断新增/编辑，用户只需点「保存」
     * @param keyword       关键词
     * @param existingNote  已有内容（编辑时传入，新增传 null）
     * @param searchContext 上下文
     * @param noteId        笔记ID（编辑时传入，新增传 null）
     * @param parentDialog  父弹窗引用（可选）
     */
    private void showNoteInputDialog(
        String keyword,
        String existingNote,
        String searchContext,
        String noteId,
        AlertDialog parentDialog
    ) {
        boolean isEditMode = noteId != null && !noteId.isEmpty();

        int dp16 = dp2px(16);
        int dp12 = dp2px(12);

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(dp16, dp12, dp16, dp12);

        // 标题：显示关键词
        String titleText =
            keyword.length() > 40 ? keyword.substring(0, 40) + "..." : keyword;
        TextView titleView = new TextView(this);
        titleView.setText(titleText);
        titleView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 16);
        titleView.setTextColor(mInvertMode ? 0xFFBBDEFB : 0xFF1565C0);
        titleView.setMaxLines(2);
        titleView.setEllipsize(TextUtils.TruncateAt.END);
        titleView.setPadding(0, 0, 0, dp12);
        layout.addView(titleView);

        // 输入框
        final EditText noteEdit = new EditText(this);
        noteEdit.setHint(
            MyActivity.zh_cn ? "输入笔记内容..." : "Enter note content..."
        );
        noteEdit.setMinLines(3);
        noteEdit.setGravity(Gravity.TOP | Gravity.START);
        noteEdit.setTextSize(TypedValue.COMPLEX_UNIT_SP, 15);
        noteEdit.setTextColor(mInvertMode ? 0xFFDDDDDD : 0xFF333333);
        noteEdit.setHintTextColor(mInvertMode ? 0xFF666666 : 0xFF999999);
        if (mInvertMode) noteEdit.setBackgroundColor(0xFF2D2D2D);
        noteEdit.setPadding(dp12, dp12, dp12, dp12);
        if (isEditMode) {
            noteEdit.setText(existingNote);
            noteEdit.setSelection(existingNote.length());
        }
        layout.addView(noteEdit);

        // ✅ 按钮文案统一为「保存」，不区分新增/编辑
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setView(layout);
        builder.setTitle(MyActivity.zh_cn ? "笔记" : "Note");
        builder.setPositiveButton(MyActivity.zh_cn ? "保存" : "Save", null);
        builder.setNegativeButton(MyActivity.zh_cn ? "取消" : "Cancel", null);

        final AlertDialog noteDialog = builder.create();
        if (noteDialog.getWindow() != null) noteDialog
            .getWindow()
            .setDimAmount(0.3f);

        noteDialog.setOnShowListener(dlg -> {
            android.widget.Button positiveBtn = noteDialog.getButton(
                AlertDialog.BUTTON_POSITIVE
            );
            positiveBtn.setOnClickListener(v -> {
                String noteContent = noteEdit.getText().toString().trim();
                if (noteContent.isEmpty()) {
                    Toast.makeText(
                        DocumentActivity.this,
                        MyActivity.zh_cn
                            ? "笔记内容不能为空"
                            : "Note content cannot be empty",
                        Toast.LENGTH_SHORT
                    ).show();
                    return;
                }

                // ✅ 自动判断：有 ID 就更新，没 ID 就新增
                String payload;
                if (isEditMode) {
                    payload =
                        "pdf_note_update|==|" +
                        noteId +
                        "|==|" +
                        searchContext +
                        "|==|" +
                        keyword +
                        "|==|" +
                        noteContent;
                } else {
                    payload =
                        "pdf_save_note|==|" +
                        searchContext +
                        "|==|" +
                        keyword +
                        "|==|" +
                        noteContent +
                        "|==|" +
                        currentPage;
                }
                MyActivity.mInstance.PublicJavaCallCpp(payload);

                Toast.makeText(
                    DocumentActivity.this,
                    MyActivity.zh_cn ? "已保存" : "Saved",
                    Toast.LENGTH_SHORT
                ).show();

                noteDialog.dismiss();
                if (parentDialog != null && parentDialog.isShowing()) {
                    parentDialog.dismiss();
                }
            });
        });

        noteDialog.show();
        noteEdit.requestFocus();
        noteEdit.post(() -> {
            InputMethodManager imm = (InputMethodManager) getSystemService(
                Context.INPUT_METHOD_SERVICE
            );
            if (imm != null) imm.showSoftInput(
                noteEdit,
                InputMethodManager.SHOW_IMPLICIT
            );
        });
    }
}
