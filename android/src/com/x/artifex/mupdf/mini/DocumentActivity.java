package com.x.artifex.mupdf.mini;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Insets;
import android.graphics.drawable.ColorDrawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.FileUriExposedException;
import android.os.ParcelFileDescriptor;
import android.provider.OpenableColumns;
import android.text.Editable;
import android.text.TextPaint;
import android.text.TextWatcher;
import android.text.method.PasswordTransformationMethod;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.PopupMenu;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;
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
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Stack;

public class DocumentActivity extends Activity {

    // TTS //////////////////////////////////////////
    private ImageButton ttsButton;
    protected boolean mIsTtsReading = false;
    protected int mTtsReadingPage = -1;
    /** 缓存当前页提取的纯文本，用于末尾判断 */
    private String mCurrentPageText = "";
    /** 缓存当前页的变换矩阵，供高亮搜索使用 */
    private Matrix mCurrentPageCtm = null;

    //////////////////////////////////////////////////

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
        //getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

        /*Window window = getWindow();
        window
            .getDecorView()
            .setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
                    View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
            );
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            window.setStatusBarColor(0x00000000);
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            window.getAttributes().layoutInDisplayCutoutMode =
                WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }*/

        // 刘海屏适配仍需保留（与沉浸模式无关）
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            getWindow().getAttributes().layoutInDisplayCutoutMode =
                WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }

        ///////////////////////////////////////////////////////////////////

        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        displayDPI = metrics.densityDpi;

        //if (MyActivity.mPdfInvertMode) {
        setContentView(R.layout.document_activity_dark);
        //} else {
        //setContentView(R.layout.document_activity);
        //}

        //ImmersiveUtil.applyRealImmersive(this);

        actionBar = findViewById(R.id.action_bar);
        searchBar = findViewById(R.id.search_bar);
        bottomBar = findViewById(R.id.bottom_bar);
        backgroundLayout = findViewById(R.id.background_layout);
        topBar = findViewById(R.id.top_bar);

        ttsButton = findViewById(R.id.tts_button);
        ttsButton.setOnClickListener(v -> toggleTts());

        currentBar = actionBar;

        ImageButton openButton = findViewById(R.id.open_button);
        openButton.setOnClickListener(v -> {
            finish();
            CallJavaNotify_10(); // 后续需要JNI回调再打开注释
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

        Log.i(APP, "  NAME " + title);
        Log.i(APP, "  SIZE " + size);

        if (mimetype == null || mimetype.equals("application/octet-stream")) {
            mimetype = getContentResolver().getType(uri);
            Log.i(APP, "  MAGIC (Resolver) " + mimetype);
        }
        if (mimetype == null || mimetype.equals("application/octet-stream")) {
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
            openDocument();
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
        // 离开此Activity强制恢复系统UI，避免返回上一个Activity状态栏丢失
        exitSystemFullscreen();

        super.onPause();
        if (prefs != null) {
            SharedPreferences.Editor editor = prefs.edit();
            editor.putFloat("layoutEm", layoutEm);
            editor.putBoolean("fitPage", fitPage);
            editor.putInt(key, currentPage);
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
        // 离开此Activity强制恢复系统UI，避免返回上一个Activity状态栏丢失
        exitSystemFullscreen();

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
    public void startTtsReading() {
        if (mIsTtsReading) {
            stopTtsReading();
            return;
        }

        mIsTtsReading = true;
        mTtsReadingPage = currentPage;
        updateTtsButtonState();
        updateTtsButtonState(); // ✅ 切换为停止图标
        readCurrentPage();
    }

    /** 停止朗读 */
    public void stopTtsReading() {
        mIsTtsReading = false;
        mTtsReadingPage = -1;
        MyService.stopTextPlay();
        pageView.clearTtsHighlight();
        updateTtsButtonState();
        updateTtsButtonState(); // ✅ 切换为播放图标
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

    /** 获取文本中最后一个有意义的句子 */
    private String getLastSentence(String text) {
        if (text == null || text.isEmpty()) return "";
        // 按句号、问号、感叹号、换行分割，取最后一个非空片段
        String[] parts = text.split("[。！？!?\\n]+");
        for (int i = parts.length - 1; i >= 0; i--) {
            String s = parts[i].trim();
            if (!s.isEmpty()) return s;
        }
        return text.trim();
    }

    /** 归一化字符串用于比对（去标点、去空白、转小写） */
    private String normalizeForCompare(String s) {
        if (s == null) return "";
        return s.replaceAll("[\\p{Punct}\\s]", "").toLowerCase();
    }

    ////////////////////////////////////////////////////////////////////////////
}
