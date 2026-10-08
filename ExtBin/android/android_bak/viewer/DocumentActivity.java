package com.x.artifex.mupdf.viewer;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog; // ✅ NEW
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo; // ✅ 用于按文件类型控制屏幕方向
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.Insets;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.drawable.ShapeDrawable;
import android.graphics.drawable.shapes.RectShape;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.provider.OpenableColumns;
import android.text.Editable;
import android.text.TextWatcher;
import android.text.method.PasswordTransformationMethod;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MenuItem.OnMenuItemClickListener;
import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowManager;
import android.view.animation.Animation;
import android.view.animation.TranslateAnimation;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.PopupMenu;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ViewAnimator;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import com.artifex.mupdf.fitz.*;
import com.artifex.mupdf.fitz.Quad;
import com.artifex.mupdf.fitz.SeekableInputStream;
import com.artifex.mupdf.fitz.android.*;
import com.x.MyActivity; // ✅ NEW
import com.x.MyService;
import com.x.R;
import com.x.TTSUtils;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream; // ✅ NEW
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Locale;

public class DocumentActivity extends Activity {

    public static DocumentActivity mPdfActivity;

    private final String APP = "MuPDF";

    // ✅ NEW: TXT 转换相关字段
    private boolean isTxtFile = false;
    private ProgressDialog mConvertProgressDialog = null;

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
            //stopTtsReading();
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

    // 固定排版参数（不允许用户调整）
    private static final float FIXED_LETTER_SPACING = 0.08f; // 字间距：0.05~0.1em 最适宜，过大则散
    private static final float FIXED_LINE_HEIGHT = 1.65f; // 行高：1.5~1.8 为中文舒适区，1.65 兼顾紧凑与透气
    /////////////////////////////////////////////////

    private ImageButton minimizeAppButton;
    private ImageButton readingNoteButton;

    private ImageButton mDarkModeButton;
    protected boolean mInvertMode = false;

    /* The core rendering instance */
    enum TopBarMode {
        Main,
        Search,
        More,
    }

    private final float EXCLUSION_HEIGHT_FACTOR = 2.0f;

    private final int OUTLINE_REQUEST = 0;
    private MuPDFCore core;
    private String mDocTitle;
    private String mDocKey;
    private ReaderView mDocView;
    private View mButtonsView;
    private boolean mButtonsVisible;
    private EditText mPasswordView;
    private TextView mDocNameView;
    private SeekBar mPageSlider;
    private int mPageSliderRes;
    private TextView mPageNumberView;
    private ImageButton mSearchButton;
    private ImageButton mOutlineButton;
    private ViewAnimator mTopBarSwitcher;
    private LinearLayout mTopBar;
    private LinearLayout mActionBar;
    private LinearLayout mSearchBar;
    private LinearLayout mBottomBar;
    private ImageButton mLinkButton;
    private TopBarMode mTopBarMode = TopBarMode.Main;
    private ImageButton mSearchBack;
    private ImageButton mSearchFwd;
    private ImageButton mSearchClose;
    private EditText mSearchText;
    private SearchTask mSearchTask;
    private AlertDialog.Builder mAlertBuilder;
    private boolean mLinkHighlight = false;
    private final Handler mHandler = new Handler();
    private boolean mAlertsActive = false;
    private AlertDialog mAlertDialog;
    private ArrayList<OutlineActivity.Item> mFlatOutline;
    private boolean mReturnToLibraryActivity = false;

    protected int mDisplayDPI;
    private int mLayoutEM = 10;
    private int mLayoutW = 312;
    private int mLayoutH = 504;
    private static final int DEFAULT_LAYOUT_EM = 10;

    protected Insets systemInsets = Insets.NONE;

    protected View mLayoutButton;
    protected PopupMenu mLayoutPopupMenu;

    protected PageView pageView;
    protected String key;

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

    private MuPDFCore openBuffer(byte[] buffer, String magic) {
        try {
            core = new MuPDFCore(buffer, magic);
        } catch (Exception e) {
            Log.e(APP, "Error opening document buffer: " + e);
            return null;
        }
        return core;
    }

    private MuPDFCore openStream(SeekableInputStream stm, String magic) {
        try {
            core = new MuPDFCore(stm, magic);
        } catch (Exception e) {
            Log.e(APP, "Error opening document stream: " + e);
            return null;
        }
        return core;
    }

    private MuPDFCore openCore(Uri uri, long size, String mimetype)
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
            return openBuffer(buf, mimetype);
        } else {
            Log.i(APP, "  Opening document from stream");
            return openStream(new ContentInputStream(cr, uri, size), mimetype);
        }
    }

    private void showCannotOpenDialog(String reason) {
        Resources res = getResources();
        AlertDialog alert = mAlertBuilder.create();
        setTitle(
            String.format(
                Locale.ROOT,
                res.getString(R.string.cannot_open_document_Reason),
                reason
            )
        );
        alert.setButton(
            AlertDialog.BUTTON_POSITIVE,
            getString(R.string.dismiss),
            new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    finish();
                }
            }
        );
        alert.show();
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // ============ 屏幕方向控制（与旧版 Mini 实现一致）============
        Uri uri = getIntent().getData();
        String fileName = "";
        if (uri != null) {
            Cursor cursor = getContentResolver().query(
                uri,
                null,
                null,
                null,
                null
            );
            if (cursor != null && cursor.moveToFirst()) {
                int nameIdx = cursor.getColumnIndex(
                    OpenableColumns.DISPLAY_NAME
                );
                if (nameIdx >= 0) fileName = cursor.getString(nameIdx);
                cursor.close();
            }
        }
        // 仅 PDF 强制横屏，TXT/EPUB 等流式文档保持系统默认方向
        boolean isPdf = fileName.toLowerCase().endsWith(".pdf");
        if (isPdf) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }
        // =========================================================

        mPdfActivity = this;

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        mDisplayDPI = (int) metrics.densityDpi;

        mAlertBuilder = new AlertDialog.Builder(this);

        if (core == null) {
            if (
                savedInstanceState != null &&
                savedInstanceState.containsKey("DocTitle")
            ) {
                mDocTitle = savedInstanceState.getString("DocTitle");
            }
        }
        if (core == null) {
            Intent intent = getIntent();
            SeekableInputStream file;

            mReturnToLibraryActivity =
                intent.getIntExtra(
                    getComponentName().getPackageName() +
                        ".ReturnToLibraryActivity",
                    0
                ) != 0;

            if (Intent.ACTION_VIEW.equals(intent.getAction())) {
                uri = intent.getData();
                String mimetype = getIntent().getType();

                if (uri == null) {
                    showCannotOpenDialog("No document uri to open");
                    return;
                }

                mDocKey = uri.toString();

                Log.i(APP, "OPEN URI " + uri.toString());
                Log.i(APP, "  MAGIC (Intent) " + mimetype);

                mDocTitle = null;
                long size = -1;
                Cursor cursor = null;

                try {
                    cursor = getContentResolver().query(
                        uri,
                        null,
                        null,
                        null,
                        null
                    );
                    if (cursor != null && cursor.moveToFirst()) {
                        int idx;

                        idx = cursor.getColumnIndex(
                            OpenableColumns.DISPLAY_NAME
                        );
                        if (
                            idx >= 0 &&
                            cursor.getType(idx) == Cursor.FIELD_TYPE_STRING
                        ) mDocTitle = cursor.getString(idx);

                        idx = cursor.getColumnIndex(OpenableColumns.SIZE);
                        if (
                            idx >= 0 &&
                            cursor.getType(idx) == Cursor.FIELD_TYPE_INTEGER
                        ) size = cursor.getLong(idx);

                        if (size == 0) size = -1;
                    }
                } catch (Exception x) {
                    // Ignore any exception and depend on default values for title
                    // and size (unless one was decoded
                } finally {
                    if (cursor != null) cursor.close();
                }

                Log.i(APP, "  NAME " + mDocTitle);
                Log.i(APP, "  SIZE " + size);

                // ✅ NEW: TXT 文件走 EPUB 转换分支
                isTxtFile =
                    mDocTitle != null &&
                    mDocTitle.toLowerCase().endsWith(".txt");

                if (isTxtFile) {
                    Log.i(
                        APP,
                        "TXT file detected, requesting EPUB conversion. name=" +
                            mDocTitle
                    );

                    runOnUiThread(() -> {
                        if (isFinishing() || isDestroyed()) return;
                        mConvertProgressDialog = new ProgressDialog(this);
                        mConvertProgressDialog.setMessage(
                            MyActivity.zh_cn
                                ? "正在转换，请稍后..."
                                : "Converting, please wait..."
                        );
                        mConvertProgressDialog.setCancelable(false);
                        mConvertProgressDialog.show();
                    });

                    requestConvertTxtToEpub(uri);
                    return; // ✅ 提前返回，等待 JNI 回调
                }

                if (
                    mimetype == null ||
                    mimetype.equals("application/octet-stream")
                ) {
                    mimetype = getContentResolver().getType(uri);
                    Log.i(APP, "  MAGIC (Resolved) " + mimetype);
                }
                if (
                    mimetype == null ||
                    mimetype.equals("application/octet-stream")
                ) {
                    mimetype = mDocTitle;
                    Log.i(APP, "  MAGIC (Filename) " + mimetype);
                }

                try {
                    core = openCore(uri, size, mimetype);
                    SearchTaskResult.set(null);
                } catch (Exception x) {
                    showCannotOpenDialog(x.toString());
                    return;
                }
            }
            if (core != null && core.needsPassword()) {
                requestPassword(savedInstanceState);
                return;
            }
            if (core != null && core.countPages() == 0) {
                core = null;
            }
        }
        if (core == null) {
            AlertDialog alert = mAlertBuilder.create();
            alert.setTitle(R.string.cannot_open_document);
            alert.setButton(
                AlertDialog.BUTTON_POSITIVE,
                getString(R.string.dismiss),
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        finish();
                    }
                }
            );
            alert.setOnCancelListener(
                new OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        finish();
                    }
                }
            );
            alert.show();
            return;
        }

        createUI(savedInstanceState);
    }

    public void requestPassword(final Bundle savedInstanceState) {
        mPasswordView = new EditText(this);
        mPasswordView.setInputType(EditorInfo.TYPE_TEXT_VARIATION_PASSWORD);
        mPasswordView.setTransformationMethod(
            new PasswordTransformationMethod()
        );

        AlertDialog alert = mAlertBuilder.create();
        alert.setTitle(R.string.enter_password);
        alert.setView(mPasswordView);
        alert.setButton(
            AlertDialog.BUTTON_POSITIVE,
            getString(R.string.okay),
            new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    if (
                        core.authenticatePassword(
                            mPasswordView.getText().toString()
                        )
                    ) {
                        createUI(savedInstanceState);
                    } else {
                        requestPassword(savedInstanceState);
                    }
                }
            }
        );
        alert.setButton(
            AlertDialog.BUTTON_NEGATIVE,
            getString(R.string.cancel),
            new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    finish();
                }
            }
        );
        alert.show();
    }

    public void relayoutDocument() {
        if (core == null || mDocView == null) return;
        int current = mDocView.getDisplayedViewIndex();
        int loc = core.layout(current, mLayoutW, mLayoutH, mLayoutEM);
        // 边界保护：防止映射后页码越界
        if (loc < 0 || loc >= core.countPages()) {
            loc = Math.max(0, Math.min(current, core.countPages() - 1));
        }
        mFlatOutline = null;
        mDocView.mHistory.clear();
        mDocView.refresh();
        mDocView.setDisplayedViewIndex(loc);
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
        mTopBar.setPadding(0, systemInsets.top, 0, 0);
        mBottomBar.setPadding(0, 0, 0, systemInsets.bottom);
    }

    public void createUI(Bundle savedInstanceState) {
        if (core == null) return;

        // ✅ 同步 key 与 mDocKey（读书笔记等功能依赖 key）
        key = mDocKey;

        // Now create the UI.
        // First create the document view
        // ✅ 标记：createUI 中页码恢复完成前，禁止 onSizeChanged 触发 relayout
        final boolean[] layoutReady = { false };

        mDocView = new ReaderView(this) {
            @Override
            protected void onMoveToChild(int i) {
                if (core == null) return;
                mPageNumberView.setText(
                    String.format(
                        Locale.ROOT,
                        "%d / %d",
                        i + 1,
                        core.countPages()
                    )
                );
                mPageSlider.setMax((core.countPages() - 1) * mPageSliderRes);
                mPageSlider.setProgress(i * mPageSliderRes);
                super.onMoveToChild(i);
            }

            @Override
            protected void onTapMainDocArea() {
                if (!mButtonsVisible) {
                    showButtons();
                } else {
                    if (mTopBarMode == TopBarMode.Main) hideButtons();
                }
            }

            @Override
            protected void onDocMotion() {
                hideButtons();
            }

            @Override
            public void onSizeChanged(int w, int h, int oldw, int oldh) {
                int newW = (w * 72) / mDisplayDPI;
                int newH = (h * 72) / mDisplayDPI;

                // 仅当尺寸真正变化时才更新（避免首次布局的虚假触发）
                boolean sizeChanged = newW != mLayoutW || newH != mLayoutH;
                mLayoutW = newW;
                mLayoutH = newH;

                if (layoutReady[0] && sizeChanged) {
                    if (core.isReflowable()) {
                        relayoutDocument();
                    } else {
                        refresh();
                    }
                }
            }
        };
        mDocView.setAdapter(new PageAdapter(this, core));

        mSearchTask = new SearchTask(this, core) {
            @Override
            protected void onTextFound(SearchTaskResult result) {
                SearchTaskResult.set(result);
                // Ask the ReaderView to move to the resulting page
                mDocView.setDisplayedViewIndex(result.pageNumber);
                // Make the ReaderView act on the change to SearchTaskResult
                // via overridden onChildSetup method.
                mDocView.resetupChildren();
            }
        };

        // Make the buttons overlay, and store all its
        // controls in variables
        makeButtonsView();

        // ✅ 暗黑模式切换按钮
        mDarkModeButton = (ImageButton) mButtonsView.findViewById(
            R.id.dark_mode_button
        );
        if (mDarkModeButton != null) {
            // 从 SharedPreferences 恢复状态
            SharedPreferences prefs = getPreferences(Context.MODE_PRIVATE);
            mInvertMode = prefs.getBoolean("invert_mode_" + mDocKey, false);

            mDarkModeButton.setOnClickListener(v -> {
                mInvertMode = !mInvertMode;

                // 持久化当前文档的暗黑模式状态
                getPreferences(Context.MODE_PRIVATE)
                    .edit()
                    .putBoolean("invert_mode_" + mDocKey, mInvertMode)
                    .apply();

                // 同步更新状态栏图标颜色
                updateStatusBarIconMode(mInvertMode);

                // ✅ 通知 MuPDFCore 更新反色状态
                if (core != null) {
                    core.setInvertMode(mInvertMode);
                }

                // ✅ 刷新当前显示的页面（触发重新绘制）
                if (mDocView != null) {
                    mDocView.applyToChildren(view -> {
                        if (view instanceof PageView) {
                            ((PageView) view).setInvertMode(mInvertMode);
                            ((PageView) view).invalidate();
                        }
                    });
                }
            });

            // 初始化状态栏图标
            updateStatusBarIconMode(mInvertMode);
        }

        // Set up the page slider
        int smax = Math.max(core.countPages() - 1, 1);
        mPageSliderRes = ((10 + smax - 1) / smax) * 2;

        // Set the file-name text
        String docTitle = core.getTitle();
        if (docTitle != null) mDocNameView.setText(docTitle);
        else mDocNameView.setText(mDocTitle);

        // Activate the seekbar
        mPageSlider.setOnSeekBarChangeListener(
            new SeekBar.OnSeekBarChangeListener() {
                public void onStopTrackingTouch(SeekBar seekBar) {
                    mDocView.pushHistory();
                    mDocView.setDisplayedViewIndex(
                        (seekBar.getProgress() + mPageSliderRes / 2) /
                            mPageSliderRes
                    );
                }

                public void onStartTrackingTouch(SeekBar seekBar) {}

                public void onProgressChanged(
                    SeekBar seekBar,
                    int progress,
                    boolean fromUser
                ) {
                    updatePageNumView(
                        (progress + mPageSliderRes / 2) / mPageSliderRes
                    );
                }
            }
        );

        // Activate the search-preparing button
        mSearchButton.setOnClickListener(
            new View.OnClickListener() {
                public void onClick(View v) {
                    searchModeOn();
                }
            }
        );

        mSearchClose.setOnClickListener(
            new View.OnClickListener() {
                public void onClick(View v) {
                    searchModeOff();
                }
            }
        );

        // Search invoking buttons are disabled while there is no text specified
        mSearchBack.setEnabled(false);
        mSearchFwd.setEnabled(false);
        mSearchBack.setColorFilter(Color.argb(255, 128, 128, 128));
        mSearchFwd.setColorFilter(Color.argb(255, 128, 128, 128));

        // React to interaction with the text widget
        mSearchText.addTextChangedListener(
            new TextWatcher() {
                public void afterTextChanged(Editable s) {
                    boolean haveText = s.toString().length() > 0;
                    setButtonEnabled(mSearchBack, haveText);
                    setButtonEnabled(mSearchFwd, haveText);

                    // Remove any previous search results
                    if (
                        SearchTaskResult.get() != null &&
                        !mSearchText
                            .getText()
                            .toString()
                            .equals(SearchTaskResult.get().txt)
                    ) {
                        SearchTaskResult.set(null);
                        mDocView.resetupChildren();
                    }
                }

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
                ) {}
            }
        );

        //React to Done button on keyboard
        mSearchText.setOnEditorActionListener(
            new TextView.OnEditorActionListener() {
                public boolean onEditorAction(
                    TextView v,
                    int actionId,
                    KeyEvent event
                ) {
                    if (actionId == EditorInfo.IME_ACTION_DONE) search(1);
                    return false;
                }
            }
        );

        mSearchText.setOnKeyListener(
            new View.OnKeyListener() {
                public boolean onKey(View v, int keyCode, KeyEvent event) {
                    if (
                        event.getAction() == KeyEvent.ACTION_DOWN &&
                        keyCode == KeyEvent.KEYCODE_ENTER
                    ) search(1);
                    return false;
                }
            }
        );

        // Activate search invoking buttons
        mSearchBack.setOnClickListener(
            new View.OnClickListener() {
                public void onClick(View v) {
                    search(-1);
                }
            }
        );
        mSearchFwd.setOnClickListener(
            new View.OnClickListener() {
                public void onClick(View v) {
                    search(1);
                }
            }
        );

        mLinkButton.setOnClickListener(
            new View.OnClickListener() {
                public void onClick(View v) {
                    setLinkHighlight(!mLinkHighlight);
                }
            }
        );

        if (core.isReflowable()) {
            mLayoutButton.setVisibility(View.VISIBLE);
            mLayoutPopupMenu = new PopupMenu(this, mLayoutButton);
            mLayoutPopupMenu
                .getMenuInflater()
                .inflate(R.menu.layout_menu, mLayoutPopupMenu.getMenu());
            mLayoutPopupMenu.setOnMenuItemClickListener(
                new PopupMenu.OnMenuItemClickListener() {
                    public boolean onMenuItemClick(MenuItem item) {
                        int oldLayoutEM = mLayoutEM;
                        int id = item.getItemId();
                        if (id == R.id.action_layout_6pt) mLayoutEM = 6;
                        else if (id == R.id.action_layout_7pt) mLayoutEM = 7;
                        else if (id == R.id.action_layout_8pt) mLayoutEM = 8;
                        else if (id == R.id.action_layout_9pt) mLayoutEM = 9;
                        else if (id == R.id.action_layout_10pt) mLayoutEM = 10;
                        else if (id == R.id.action_layout_11pt) mLayoutEM = 11;
                        else if (id == R.id.action_layout_12pt) mLayoutEM = 12;
                        else if (id == R.id.action_layout_13pt) mLayoutEM = 13;
                        else if (id == R.id.action_layout_14pt) mLayoutEM = 14;
                        else if (id == R.id.action_layout_15pt) mLayoutEM = 15;
                        else if (id == R.id.action_layout_16pt) mLayoutEM = 16;
                        if (oldLayoutEM != mLayoutEM) {
                            // ✅ 持久化字号
                            getPreferences(Context.MODE_PRIVATE)
                                .edit()
                                .putInt("layoutEm_" + mDocKey, mLayoutEM)
                                .apply();
                            relayoutDocument();
                        }
                        return true;
                    }
                }
            );
            mLayoutButton.setOnClickListener(
                new View.OnClickListener() {
                    public void onClick(View v) {
                        // ✅ 每次弹出前重置菜单标题（去掉旧 ✓ 标记）
                        Menu menu = mLayoutPopupMenu.getMenu();
                        for (int i = 0; i < menu.size(); i++) {
                            MenuItem item = menu.getItem(i);
                            CharSequence title = item.getTitle();
                            if (title != null) {
                                String t = title.toString();
                                if (t.startsWith("✓ ")) {
                                    item.setTitle(t.substring(2));
                                }
                            }
                        }
                        // ✅ 给当前字号加 ✓ 标记
                        int currentId = getMenuIdForEm(mLayoutEM);
                        if (currentId != -1) {
                            MenuItem currentItem = menu.findItem(currentId);
                            if (currentItem != null) {
                                currentItem.setTitle(
                                    "✓ " + currentItem.getTitle()
                                );
                            }
                        }
                        mLayoutPopupMenu.show();
                    }
                }
            );
        }

        if (core.hasOutline()) {
            mOutlineButton.setOnClickListener(
                new View.OnClickListener() {
                    public void onClick(View v) {
                        boolean outlineTruncated = false;
                        if (mFlatOutline == null) {
                            mFlatOutline = core.getOutline();
                            outlineTruncated = core.wasOutlineTruncated();
                        }
                        if (mFlatOutline != null) {
                            Intent intent = new Intent(
                                DocumentActivity.this,
                                OutlineActivity.class
                            );
                            Bundle bundle = new Bundle();
                            bundle.putInt(
                                "POSITION",
                                mDocView.getDisplayedViewIndex()
                            );
                            bundle.putSerializable("OUTLINE", mFlatOutline);
                            intent.putExtra(
                                "PALLETBUNDLE",
                                Pallet.sendBundle(bundle)
                            );
                            startActivityForResult(intent, OUTLINE_REQUEST);
                            if (outlineTruncated) Toast.makeText(
                                DocumentActivity.this,
                                "Outline too large, truncated",
                                Toast.LENGTH_SHORT
                            ).show();
                        }
                    }
                }
            );
        } else {
            mOutlineButton.setVisibility(View.GONE);
        }

        // Reenstate last state if it was recorded
        /////////////////////////////////////////////////////////////////////
        // ✅ 恢复保存的状态
        SharedPreferences prefs = getPreferences(Context.MODE_PRIVATE);

        // 1. 恢复字号（兼容旧版 Float 和新版 Int）
        int savedEm = DEFAULT_LAYOUT_EM;
        try {
            savedEm = prefs.getInt("layoutEm_" + mDocKey, -1);
        } catch (ClassCastException e) {
            // 旧版遗留 Float，读取并迁移
            try {
                float oldVal = prefs.getFloat("layoutEm_" + mDocKey, -1f);
                if (oldVal >= 6 && oldVal <= 16) {
                    savedEm = Math.round(oldVal);
                }
            } catch (Exception ignored) {}
            // 立即迁移为 Int，防止 onPause 再次写 Float 前就崩溃
            prefs
                .edit()
                .putInt("layoutEm_" + mDocKey, savedEm)
                .apply();
        }
        if (savedEm < 6 || savedEm > 16) savedEm = DEFAULT_LAYOUT_EM;
        mLayoutEM = savedEm;

        // 2. 恢复页码
        int savedPage = prefs.getInt("page" + mDocKey, 0);

        // 3. 对 reflowable 文档：先用恢复的字号 layout，再跳转
        if (core.isReflowable()) {
            // core.layout 返回映射后的安全页码
            int mappedPage = core.layout(
                savedPage,
                mLayoutW,
                mLayoutH,
                mLayoutEM
            );
            // 边界保护
            if (mappedPage < 0 || mappedPage >= core.countPages()) {
                mappedPage = Math.max(
                    0,
                    Math.min(savedPage, core.countPages() - 1)
                );
            }
            mDocView.setDisplayedViewIndex(mappedPage);
            Log.i(
                APP,
                "Restored: em=" +
                    mLayoutEM +
                    ", savedPage=" +
                    savedPage +
                    ", mappedPage=" +
                    mappedPage +
                    ", pageCount=" +
                    core.countPages()
            );
        } else {
            // PDF 等固定排版文档：直接跳转，做边界保护
            int safePage = Math.max(
                0,
                Math.min(savedPage, core.countPages() - 1)
            );
            mDocView.setDisplayedViewIndex(safePage);
            Log.i(
                APP,
                "Restored: page=" +
                    safePage +
                    ", pageCount=" +
                    core.countPages()
            );
        }

        // ✅ 页码恢复完成，允许后续 onSizeChanged 触发 relayout
        layoutReady[0] = true;

        /////////////////////////////////////////////////////////

        if (
            savedInstanceState == null ||
            !savedInstanceState.getBoolean("ButtonsHidden", false)
        ) showButtons();

        if (
            savedInstanceState == null ||
            !savedInstanceState.getBoolean("ButtonsHidden", false)
        ) showButtons();

        if (
            savedInstanceState != null &&
            savedInstanceState.getBoolean("SearchMode", false)
        ) searchModeOn();

        mTopBar.setOnApplyWindowInsetsListener(
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

        if (Build.VERSION.SDK_INT >= 29) mBottomBar.addOnLayoutChangeListener(
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

        // Stick the document view and the buttons overlay into a parent view
        RelativeLayout layout = new RelativeLayout(this);
        layout.setBackgroundColor(Color.DKGRAY);
        layout.addView(mDocView);
        layout.addView(mButtonsView);
        setContentView(layout);
    }

    @Override
    protected void onActivityResult(
        int requestCode,
        int resultCode,
        Intent data
    ) {
        switch (requestCode) {
            case OUTLINE_REQUEST:
                if (resultCode >= RESULT_FIRST_USER && mDocView != null) {
                    mDocView.pushHistory();
                    mDocView.setDisplayedViewIndex(
                        resultCode - RESULT_FIRST_USER
                    );
                }
                break;
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        if (mDocKey != null && mDocView != null) {
            if (mDocTitle != null) outState.putString("DocTitle", mDocTitle);

            // Store current page in the prefs against the file name,
            // so that we can pick it up each time the file is loaded
            // Other info is needed only for screen-orientation change,
            // so it can go in the bundle
            SharedPreferences prefs = getPreferences(Context.MODE_PRIVATE);
            SharedPreferences.Editor edit = prefs.edit();
            edit.putInt("page" + mDocKey, mDocView.getDisplayedViewIndex());
            edit.apply();
        }

        if (!mButtonsVisible) outState.putBoolean("ButtonsHidden", true);

        if (mTopBarMode == TopBarMode.Search) outState.putBoolean(
            "SearchMode",
            true
        );
    }

    @Override
    protected void onPause() {
        super.onPause();

        if (mSearchTask != null) mSearchTask.stop();

        if (mDocKey != null && mDocView != null) {
            SharedPreferences prefs = getPreferences(Context.MODE_PRIVATE);
            SharedPreferences.Editor edit = prefs.edit();
            edit.putInt("page" + mDocKey, mDocView.getDisplayedViewIndex());
            // ✅ 新增：按文档 key 保存字号
            edit.putInt("layoutEm_" + mDocKey, mLayoutEM);
            edit.apply();
        }

        // ✅ 保存暗黑模式状态
        if (mDocKey != null) {
            SharedPreferences prefs = getPreferences(Context.MODE_PRIVATE);
            prefs
                .edit()
                .putBoolean("invert_mode_" + mDocKey, mInvertMode)
                .apply();
        }
    }

    public void onDestroy() {
        dismissConvertDialog(); // ✅ 兜底关闭弹窗

        if (mDocView != null) {
            mDocView.applyToChildren(
                new ReaderView.ViewMapper() {
                    @Override
                    public void applyToView(View view) {
                        ((PageView) view).releaseBitmaps();
                    }
                }
            );
        }
        if (core != null) core.onDestroy();
        core = null;

        // 退出全屏模式
        CallJavaNotify_0();

        super.onDestroy();
        mPdfActivity = null;
    }

    private void setButtonEnabled(ImageButton button, boolean enabled) {
        button.setEnabled(enabled);
        button.setColorFilter(
            enabled
                ? Color.argb(255, 255, 255, 255)
                : Color.argb(255, 128, 128, 128)
        );
    }

    private void setLinkHighlight(boolean highlight) {
        mLinkHighlight = highlight;
        // LINK_COLOR tint
        mLinkButton.setColorFilter(
            highlight
                ? Color.argb(0xFF, 0x00, 0x66, 0xCC)
                : Color.argb(0xFF, 255, 255, 255)
        );
        // Inform pages of the change.
        mDocView.setLinksEnabled(highlight);
    }

    private void showButtons() {
        if (core == null) return;
        if (!mButtonsVisible) {
            mButtonsVisible = true;
            // Update page number text and slider
            int index = mDocView.getDisplayedViewIndex();
            updatePageNumView(index);
            mPageSlider.setMax((core.countPages() - 1) * mPageSliderRes);
            mPageSlider.setProgress(index * mPageSliderRes);
            if (mTopBarMode == TopBarMode.Search) {
                mSearchText.requestFocus();
                showKeyboard();
            }

            Animation anim = new TranslateAnimation(
                0,
                0,
                -(mTopBarSwitcher.getHeight() + systemInsets.top),
                0
            );
            anim.setDuration(200);
            anim.setAnimationListener(
                new Animation.AnimationListener() {
                    public void onAnimationStart(Animation animation) {
                        mTopBarSwitcher.setVisibility(View.VISIBLE);
                    }

                    public void onAnimationRepeat(Animation animation) {}

                    public void onAnimationEnd(Animation animation) {}
                }
            );
            mTopBarSwitcher.startAnimation(anim);

            anim = new TranslateAnimation(
                0,
                0,
                mBottomBar.getHeight() + systemInsets.bottom,
                0
            );
            anim.setDuration(200);
            anim.setAnimationListener(
                new Animation.AnimationListener() {
                    public void onAnimationStart(Animation animation) {
                        mBottomBar.setVisibility(View.VISIBLE);
                    }

                    public void onAnimationRepeat(Animation animation) {}

                    public void onAnimationEnd(Animation animation) {
                        mPageNumberView.setVisibility(View.VISIBLE);
                    }
                }
            );
            mBottomBar.startAnimation(anim);
        }
    }

    private void hideButtons() {
        if (mButtonsVisible) {
            mButtonsVisible = false;
            hideKeyboard();

            Animation anim = new TranslateAnimation(
                0,
                0,
                0,
                -(mTopBarSwitcher.getHeight() + systemInsets.top)
            );
            anim.setDuration(200);
            anim.setAnimationListener(
                new Animation.AnimationListener() {
                    public void onAnimationStart(Animation animation) {}

                    public void onAnimationRepeat(Animation animation) {}

                    public void onAnimationEnd(Animation animation) {
                        mTopBarSwitcher.setVisibility(View.INVISIBLE);
                    }
                }
            );
            mTopBarSwitcher.startAnimation(anim);

            anim = new TranslateAnimation(
                0,
                0,
                0,
                mBottomBar.getHeight() + systemInsets.bottom
            );
            anim.setDuration(200);
            anim.setAnimationListener(
                new Animation.AnimationListener() {
                    public void onAnimationStart(Animation animation) {
                        mPageNumberView.setVisibility(View.INVISIBLE);
                    }

                    public void onAnimationRepeat(Animation animation) {}

                    public void onAnimationEnd(Animation animation) {
                        mBottomBar.setVisibility(View.INVISIBLE);
                    }
                }
            );
            mBottomBar.startAnimation(anim);
        }
    }

    private void searchModeOn() {
        if (mTopBarMode != TopBarMode.Search) {
            mTopBarMode = TopBarMode.Search;
            //Focus on EditTextWidget
            mSearchText.requestFocus();
            showKeyboard();
            mActionBar.setVisibility(View.GONE);
            mSearchBar.setVisibility(View.VISIBLE);
        }
    }

    private void searchModeOff() {
        if (mTopBarMode == TopBarMode.Search) {
            mTopBarMode = TopBarMode.Main;
            hideKeyboard();
            mActionBar.setVisibility(View.VISIBLE);
            mSearchBar.setVisibility(View.GONE);
            SearchTaskResult.set(null);
            // Make the ReaderView act on the change to mSearchTaskResult
            // via overridden onChildSetup method.
            mDocView.resetupChildren();
        }
    }

    private void updatePageNumView(int index) {
        if (core == null) return;
        mPageNumberView.setText(
            String.format(Locale.ROOT, "%d / %d", index + 1, core.countPages())
        );
    }

    private void makeButtonsView() {
        mButtonsView = getLayoutInflater().inflate(
            R.layout.document_activity,
            null
        );
        mDocNameView = (TextView) mButtonsView.findViewById(R.id.docNameText);
        mPageSlider = (SeekBar) mButtonsView.findViewById(R.id.pageSlider);
        mPageNumberView = (TextView) mButtonsView.findViewById(R.id.pageNumber);
        mSearchButton = (ImageButton) mButtonsView.findViewById(
            R.id.searchButton
        );
        mOutlineButton = (ImageButton) mButtonsView.findViewById(
            R.id.outlineButton
        );
        mTopBarSwitcher = (ViewAnimator) mButtonsView.findViewById(
            R.id.switcher
        );
        mTopBar = (LinearLayout) mButtonsView.findViewById(R.id.topBar);
        mActionBar = (LinearLayout) mButtonsView.findViewById(R.id.actionBar);
        mSearchBar = (LinearLayout) mButtonsView.findViewById(R.id.searchBar);
        mBottomBar = (LinearLayout) mButtonsView.findViewById(R.id.bottomBar);
        mSearchBack = (ImageButton) mButtonsView.findViewById(R.id.searchBack);
        mSearchFwd = (ImageButton) mButtonsView.findViewById(
            R.id.searchForward
        );
        mSearchClose = (ImageButton) mButtonsView.findViewById(
            R.id.searchClose
        );
        mSearchText = (EditText) mButtonsView.findViewById(R.id.searchText);
        mLinkButton = (ImageButton) mButtonsView.findViewById(R.id.linkButton);
        mLayoutButton = mButtonsView.findViewById(R.id.layoutButton);
        mTopBarSwitcher.setVisibility(View.INVISIBLE);
        mPageNumberView.setVisibility(View.INVISIBLE);
        mActionBar.setVisibility(View.VISIBLE);
        mTopBar.setVisibility(View.VISIBLE);
        mSearchBar.setVisibility(View.GONE);
        mBottomBar.setVisibility(View.INVISIBLE);

        // 读书笔记按钮
        readingNoteButton = (ImageButton) mButtonsView.findViewById(
            R.id.reading_note_button
        );
        readingNoteButton.setOnClickListener(v -> {
            MyActivity.mInstance.PublicJavaCallCpp(
                "open_reading_notes|==|" + key
            );
        });

        // 睡眠定时器按钮
        sleepTimerButton = (ImageButton) mButtonsView.findViewById(
            R.id.sleep_timer_button
        );
        sleepTimerButton.setOnClickListener(v -> {
            mSleepTimerEnabled = !mSleepTimerEnabled;
            updateSleepTimerButtonState();
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
        minimizeAppButton = (ImageButton) mButtonsView.findViewById(
            R.id.minimize_app_button
        );
        minimizeAppButton.setOnClickListener(v -> {
            MyActivity.setMini();
        });
    }

    private void showKeyboard() {
        InputMethodManager imm = (InputMethodManager) getSystemService(
            Context.INPUT_METHOD_SERVICE
        );
        if (imm != null) imm.showSoftInput(mSearchText, 0);
    }

    private void hideKeyboard() {
        InputMethodManager imm = (InputMethodManager) getSystemService(
            Context.INPUT_METHOD_SERVICE
        );
        if (imm != null) imm.hideSoftInputFromWindow(
            mSearchText.getWindowToken(),
            0
        );
    }

    private void search(int direction) {
        hideKeyboard();
        int displayPage = mDocView.getDisplayedViewIndex();
        SearchTaskResult r = SearchTaskResult.get();
        int searchPage = r != null ? r.pageNumber : -1;
        mSearchTask.go(
            mSearchText.getText().toString(),
            direction,
            displayPage,
            searchPage
        );
    }

    @Override
    public boolean onSearchRequested() {
        if (mButtonsVisible && mTopBarMode == TopBarMode.Search) {
            hideButtons();
        } else {
            showButtons();
            searchModeOn();
        }
        return super.onSearchRequested();
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        if (mButtonsVisible && mTopBarMode != TopBarMode.Search) {
            hideButtons();
        } else {
            showButtons();
            searchModeOff();
        }
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    public void onBackPressed() {
        if (mDocView == null || (mDocView != null && !mDocView.popHistory())) {
            super.onBackPressed();
            if (mReturnToLibraryActivity) {
                Intent intent = getPackageManager().getLaunchIntentForPackage(
                    getComponentName().getPackageName()
                );
                startActivity(intent);
            }
        }
    }

    // ✅ NEW: 请求 C++ 将 TXT 转为 EPUB
    private void requestConvertTxtToEpub(Uri txtUri) {
        runOnUiThread(() -> {
            MyActivity.mInstance.PublicJavaCallCpp(
                "txt_convert_to_epub|==|" + txtUri.toString()
            );
        });
    }

    /**
     * 【供 C++ JNI 调用】TXT→EPUB 转换完成后回传字节数组
     * 直接使用 openBuffer 打开 EPUB，无需写临时文件
     */
    public void setConvertedEpubBuffer(byte[] epubBytes) {
        if (epubBytes == null || epubBytes.length == 0) {
            Log.e(APP, "Empty EPUB buffer!");
            runOnUiThread(() -> {
                dismissConvertDialog();
                showCannotOpenDialog("TXT conversion produced empty EPUB");
            });
            return;
        }

        final byte[] finalEpubBytes = epubBytes;

        runOnUiThread(() -> {
            if (isFinishing() || isDestroyed()) return;

            dismissConvertDialog();

            try {
                // ✅ 直接用 buffer 打开 EPUB，与 openCore 中 openBuffer 路径一致
                // MIME 使用 application/epub+zip（MuPDF 识别 EPUB 的标准类型）
                core = openBuffer(finalEpubBytes, "application/epub+zip");
                SearchTaskResult.set(null);

                if (core == null) {
                    showCannotOpenDialog(
                        "Failed to open converted EPUB buffer"
                    );
                    return;
                }
                if (core.needsPassword()) {
                    requestPassword(null);
                    return;
                }
                if (core.countPages() == 0) {
                    core = null;
                    showCannotOpenDialog("Converted EPUB has 0 pages");
                    return;
                }

                createUI(null);
            } catch (Exception x) {
                Log.e(APP, "Error opening converted EPUB: " + x);
                showCannotOpenDialog(x.toString());
            }
        });
    }

    // ✅ NEW: 安全关闭转换弹窗
    private void dismissConvertDialog() {
        if (
            mConvertProgressDialog != null && mConvertProgressDialog.isShowing()
        ) {
            try {
                mConvertProgressDialog.dismiss();
            } catch (Exception ignored) {}
            mConvertProgressDialog = null;
        }
    }

    /**
     * GBK 编码解码（与旧版 Mini 实现一致）
     * 用于处理中文 TXT 文件的编码兼容
     */
    public static String decodeGbk(byte[] data) {
        if (data == null || data.length == 0) return "";
        try {
            return new String(data, "GBK");
        } catch (java.io.UnsupportedEncodingException e) {
            return new String(data); // 兜底使用平台默认编码（通常 UTF-8）
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
     * 根据暗黑模式切换状态栏图标颜色
     */
    private void updateStatusBarIconMode(boolean isDark) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) return;

        Window window = getWindow();
        int vis = window.getDecorView().getSystemUiVisibility();
        int baseFlags =
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN;

        if (!isDark) {
            vis = baseFlags | View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
        } else {
            vis = baseFlags;
        }
        window.getDecorView().setSystemUiVisibility(vis);
    }

    /** 根据 MenuItem ID 返回对应的字号值 */
    private int getEmForMenuId(int id) {
        if (id == R.id.action_layout_6pt) return 6;
        else if (id == R.id.action_layout_7pt) return 7;
        else if (id == R.id.action_layout_8pt) return 8;
        else if (id == R.id.action_layout_9pt) return 9;
        else if (id == R.id.action_layout_10pt) return 10;
        else if (id == R.id.action_layout_11pt) return 11;
        else if (id == R.id.action_layout_12pt) return 12;
        else if (id == R.id.action_layout_13pt) return 13;
        else if (id == R.id.action_layout_14pt) return 14;
        else if (id == R.id.action_layout_15pt) return 15;
        else if (id == R.id.action_layout_16pt) return 16;
        return -1;
    }

    /** 获取指定字号对应的 MenuItem ID */
    private int getMenuIdForEm(int em) {
        switch (em) {
            case 6:
                return R.id.action_layout_6pt;
            case 7:
                return R.id.action_layout_7pt;
            case 8:
                return R.id.action_layout_8pt;
            case 9:
                return R.id.action_layout_9pt;
            case 10:
                return R.id.action_layout_10pt;
            case 11:
                return R.id.action_layout_11pt;
            case 12:
                return R.id.action_layout_12pt;
            case 13:
                return R.id.action_layout_13pt;
            case 14:
                return R.id.action_layout_14pt;
            case 15:
                return R.id.action_layout_15pt;
            case 16:
                return R.id.action_layout_16pt;
            default:
                return -1;
        }
    }

    /**
     * 非 PDF 笔记跳转：恢复字号 relayout 后，以保存页码为中心进行螺旋搜索
     * @param savedPage 1-based 保存页码
     * @param keyword   关键词
     * @param savedBookmark 保存的字号字符串（如 "10"）
     */
    private void relayoutThenSearchNote(
        final int savedPage,
        final String keyword,
        final String savedBookmark
    ) {
        // 1. 恢复字号
        try {
            int savedEm = Integer.parseInt(savedBookmark);
            if (savedEm >= 6 && savedEm <= 16 && savedEm != mLayoutEM) {
                mLayoutEM = savedEm;
                getPreferences(Context.MODE_PRIVATE)
                    .edit()
                    .putInt("layoutEm_" + mDocKey, mLayoutEM)
                    .apply();
            }
        } catch (NumberFormatException ignored) {}

        // 2. 重新排版 + 螺旋搜索
        new Thread(() -> {
            int targetPage = -1;
            try {
                int centerPage = savedPage - 1; // 1-based → 0-based
                int pageCount = core.countPages();
                if (centerPage < 0) centerPage = 0;
                if (centerPage >= pageCount) centerPage = pageCount - 1;

                // 螺旋搜索：以 centerPage 为中心，向外扩展最多 50 页
                int maxRadius = 50;
                for (int radius = 0; radius <= maxRadius; radius++) {
                    int pageToCheck = centerPage + radius;
                    if (pageToCheck < pageCount) {
                        if (checkPageForKeyword(pageToCheck, keyword)) {
                            targetPage = pageToCheck;
                            break;
                        }
                    }
                    if (radius > 0) {
                        pageToCheck = centerPage - radius;
                        if (pageToCheck >= 0) {
                            if (checkPageForKeyword(pageToCheck, keyword)) {
                                targetPage = pageToCheck;
                                break;
                            }
                        }
                    }
                    if (
                        centerPage + radius >= pageCount &&
                        centerPage - radius < 0
                    ) {
                        break;
                    }
                }

                if (targetPage < 0) targetPage = centerPage;
            } catch (Exception e) {
                targetPage = 0;
            }

            final int finalPage = targetPage;
            runOnUiThread(() -> {
                if (mDocView != null) {
                    mDocView.setDisplayedViewIndex(finalPage);
                }
            });
        }).start();
    }

    /** 检查指定页面是否包含关键词 */
    private boolean checkPageForKeyword(int pageNum, String keyword) {
        try {
            Quad[][] hits = core.searchPage(pageNum, keyword);
            return hits != null && hits.length > 0;
        } catch (Exception e) {
            return false;
        }
    }
}
