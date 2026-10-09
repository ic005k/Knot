package com.x.reader;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
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
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Insets;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.ShapeDrawable;
import android.graphics.drawable.shapes.RectShape;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.provider.OpenableColumns;
import android.text.Editable;
import android.text.Html;
import android.text.InputType;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.method.PasswordTransformationMethod;
import android.text.style.BackgroundColorSpan;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.view.ActionMode;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MenuItem.OnMenuItemClickListener;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowManager;
import android.view.animation.Animation;
import android.view.animation.TranslateAnimation;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.ScrollView;
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

    private androidx.appcompat.app.AlertDialog mAiLoadingDialog;

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

        Uri uri = getIntent().getData();
        if (uri != null) {
            // 使用完整 toString() 作为 key，与 Mini 完全一致
            // 这样无论 TXT/EPUB/PDF、无论哪种入口，key 都稳定
            mDocKey = uri.toString();
        }
        Log.i(APP, "STABLE DOC KEY: " + mDocKey);

        // ============ 屏幕方向控制（与旧版 Mini 实现一致）============
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

                // ✅ 增加双重保护：layoutReady + 尺寸真正变化 + 非首次布局
                if (layoutReady[0] && sizeChanged && oldw != 0 && oldh != 0) {
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

        // ✅ 恢复保存的状态（重构版）
        SharedPreferences prefs = getPreferences(Context.MODE_PRIVATE);

        // 1. 恢复字号（兼容旧版 Float 和新版 Int）
        int savedEm = DEFAULT_LAYOUT_EM;
        try {
            savedEm = prefs.getInt("layoutEm_" + mDocKey, -1);
        } catch (ClassCastException e) {
            try {
                float oldVal = prefs.getFloat("layoutEm_" + mDocKey, -1f);
                if (oldVal >= 6 && oldVal <= 16) savedEm = Math.round(oldVal);
            } catch (Exception ignored) {}
            prefs
                .edit()
                .putInt("layoutEm_" + mDocKey, savedEm)
                .apply();
        }
        if (savedEm < 6 || savedEm > 16) savedEm = DEFAULT_LAYOUT_EM;
        mLayoutEM = savedEm;

        // 2. 恢复页码
        int savedPage = prefs.getInt("page" + mDocKey, 0);

        // 3. 统一处理：先确保 core 使用正确的字号完成首次 layout
        //    再跳转页码，最后才允许 onSizeChanged 触发 relayout
        if (core.isReflowable()) {
            // ✅ 先用初始尺寸做一次 layout，仅用于获取 pageCount 等元数据
            core.layout(0, mLayoutW, mLayoutH, mLayoutEM);

            Log.i(
                APP,
                "Restore reflowable: em=" +
                    mLayoutEM +
                    ", savedPage=" +
                    savedPage +
                    ", pageCount=" +
                    core.countPages()
            );

            // ✅ 延迟到 View 布局完成后，用真实尺寸重新排版，然后直接跳转到 savedPage
            final int finalSavedPage = savedPage;
            mDocView.post(() -> {
                if (mDocView == null || core == null) return;

                // 此时 onSizeChanged 已更新 mLayoutW/H 为真实尺寸
                // ✅ 关键：anchor 传 0，不把 savedPage 作为 hint 传给 layout
                // 避免 MuPDF 的 anchor 映射导致页码偏移
                core.layout(0, mLayoutW, mLayoutH, mLayoutEM);

                // ✅ 直接跳转到保存的页码（clamp 到合法范围）
                int safePage = Math.max(
                    0,
                    Math.min(finalSavedPage, core.countPages() - 1)
                );

                Log.i(
                    APP,
                    "Real size restore: " +
                        mLayoutW +
                        "x" +
                        mLayoutH +
                        ", em=" +
                        mLayoutEM +
                        ", savedPage=" +
                        finalSavedPage +
                        ", safePage=" +
                        safePage +
                        ", pageCount=" +
                        core.countPages()
                );

                mDocView.setDisplayedViewIndex(safePage);

                // ✅ 现在才启用 onSizeChanged 响应
                layoutReady[0] = true;
                Log.i(APP, "layoutReady enabled after real-size restore");
            });
        } else {
            // PDF 等固定排版：直接跳转
            int safePage = Math.max(
                0,
                Math.min(savedPage, core.countPages() - 1)
            );
            Log.i(
                APP,
                "Restore fixed: page=" +
                    safePage +
                    ", pageCount=" +
                    core.countPages()
            );
            mDocView.setDisplayedViewIndex(safePage);

            mDocView.post(() -> {
                layoutReady[0] = true;
                Log.i(APP, "layoutReady enabled after fixed restore");
            });
        }

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

        // TTS UI

        ttsButton = (ImageButton) mButtonsView.findViewById(R.id.tts_button);
        if (ttsButton != null) ttsButton.setOnClickListener(v -> toggleTts());

        sleepTimerButton = (ImageButton) mButtonsView.findViewById(
            R.id.sleep_timer_button
        );
        if (sleepTimerButton != null) {
            sleepTimerButton.setOnClickListener(v -> {
                mSleepTimerEnabled = !mSleepTimerEnabled;
                updateSleepTimerButtonState();
                if (mIsTtsReading) {
                    mSleepTimerHandler.removeCallbacks(mSleepStopTtsRunnable);
                    if (mSleepTimerEnabled) mSleepTimerHandler.postDelayed(
                        mSleepStopTtsRunnable,
                        2 * 60 * 60 * 1000
                    );
                }
            });
        }

        // 恢复 TTS 状态
        prefs = getPreferences(Context.MODE_PRIVATE);
        mSleepTimerEnabled = prefs.getBoolean("sleep_timer_enabled", false);
        updateSleepTimerButtonState();

        if (MyService.isTextPlaying()) {
            String ttsKey = MyService.getCurrentTtsKey();
            if (key != null && key.equals(ttsKey)) {
                mDocView.post(this::syncTtsUiState);
            } else {
                MyService.stopTextPlay();
            }
        }
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
        // ✅ 不再在 onSaveInstanceState 中保存页码到 prefs
        // 页码持久化统一由 onPause 负责，避免重复写入和竞态
        if (mDocTitle != null) outState.putString("DocTitle", mDocTitle);
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

            // ✅ 关键：如果正在 TTS 朗读，保存 TTS 实际读到的页码
            // 而非用户手动翻到的页码，确保后台朗读进度不丢失
            int pageToSave =
                mIsTtsReading && mTtsReadingPage >= 0
                    ? mTtsReadingPage
                    : mDocView.getDisplayedViewIndex();

            edit.putInt("page" + mDocKey, pageToSave);
            edit.putInt("layoutEm_" + mDocKey, mLayoutEM);
            edit.putBoolean("sleep_timer_enabled", mSleepTimerEnabled);
            edit.apply();
        }

        // 保存暗黑模式状态
        if (mDocKey != null) {
            getPreferences(Context.MODE_PRIVATE)
                .edit()
                .putBoolean("invert_mode_" + mDocKey, mInvertMode)
                .apply();
        }
    }

    public void onDestroy() {
        dismissConvertDialog(); // ✅ 兜底关闭弹窗
        // ✅ 清理 AI 弹窗
        if (mAiLoadingDialog != null && mAiLoadingDialog.isShowing()) {
            try {
                mAiLoadingDialog.dismiss();
            } catch (Exception ignored) {}
            mAiLoadingDialog = null;
        }

        mSleepTimerHandler.removeCallbacks(mSleepStopTtsRunnable);
        // ✅ 不再调用 stopTtsReading()
        // 仅重置当前 Activity 的 UI 标记，让 MyService 继续在后台朗读
        mIsTtsReading = false;
        mTtsReadingPage = -1;

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
            final String finalKeyword = keyword;
            runOnUiThread(() -> {
                if (mDocView != null) {
                    mDocView.setDisplayedViewIndex(finalPage);

                    // ✅ 为非 PDF 文档也注入搜索高亮
                    new Thread(() -> {
                        Quad[][] hits = core.searchPage(
                            finalPage,
                            finalKeyword
                        );
                        runOnUiThread(() -> {
                            if (hits != null && hits.length > 0) {
                                // ✅ 直接使用带参构造器
                                SearchTaskResult result = new SearchTaskResult(
                                    finalKeyword,
                                    finalPage,
                                    hits
                                );
                                SearchTaskResult.set(result);
                                mDocView.resetupChildren();
                            }
                        });
                    }).start();
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

    //TTS////////////////////////////////////////////////////////////////////

    private void toggleTts() {
        if (mIsTtsReading) stopTtsReading();
        else startTtsReading();
    }

    public void startTtsReading() {
        if (mIsTtsReading || core == null || mDocView == null) return;

        mIsTtsReading = true;
        mTtsReadingPage = mDocView.getDisplayedViewIndex();
        updateTtsButtonState();

        if (mSleepTimerEnabled) {
            mSleepTimerHandler.removeCallbacks(mSleepStopTtsRunnable);
            mSleepTimerHandler.postDelayed(
                mSleepStopTtsRunnable,
                2 * 60 * 60 * 1000
            );
        }
        readCurrentPage();
    }

    public void stopTtsReading() {
        // ✅ 确保 UI 操作在主线程
        if (
            Thread.currentThread() !=
            android.os.Looper.getMainLooper().getThread()
        ) {
            runOnUiThread(this::stopTtsReading);
            return;
        }

        mIsTtsReading = false;
        if (mTtsReadingPage >= 0 && mDocView != null) {
            mDocView.setDisplayedViewIndex(mTtsReadingPage);
        }
        mTtsReadingPage = -1;
        MyService.stopTextPlay();

        View v = mDocView != null ? mDocView.getDisplayedView() : null;
        if (v instanceof PageView) ((PageView) v).clearTtsHighlight();

        updateTtsButtonState();
        mSleepTimerHandler.removeCallbacks(mSleepStopTtsRunnable);
    }

    private void readCurrentPage() {
        if (!mIsTtsReading || core == null) return;
        MyService.setCurrentTtsPosition(key, mTtsReadingPage, mDocTitle);

        new Thread(() -> {
            String text = core.getPageText(mTtsReadingPage);
            runOnUiThread(() -> {
                if (!mIsTtsReading) return;
                if (text == null || text.trim().isEmpty()) {
                    autoAdvanceTtsPage();
                    return;
                }
                mCurrentPageText = text.trim();
                MyService.playTextWithListener(
                    mCurrentPageText,
                    mTtsSentenceListener
                );
            });
        }).start();
    }

    protected void autoAdvanceTtsPage() {
        // ✅ 双重保险：确保始终在主线程执行
        if (
            Thread.currentThread() !=
            android.os.Looper.getMainLooper().getThread()
        ) {
            runOnUiThread(this::autoAdvanceTtsPage);
            return;
        }

        if (!mIsTtsReading || mDocView == null || core == null) return;

        if (mTtsReadingPage < core.countPages() - 1) {
            mTtsReadingPage++;

            // 清除旧高亮
            View v = mDocView.getDisplayedView();
            if (v instanceof PageView) ((PageView) v).clearTtsHighlight();

            // ✅ setDisplayedViewIndex 必须在主线程调用
            mDocView.setDisplayedViewIndex(mTtsReadingPage);

            // 等待新页面渲染完毕再朗读
            mDocView.postDelayed(this::readCurrentPage, 400);
        } else {
            stopTtsReading();
            Toast.makeText(
                this,
                MyActivity.zh_cn ? "全书朗读完毕。" : "Finished reading.",
                Toast.LENGTH_SHORT
            ).show();
        }
    }

    private final TTSUtils.OnSentenceProgressListener mTtsSentenceListener =
        new TTSUtils.OnSentenceProgressListener() {
            @Override
            public void onSentenceChanged(String sentence) {
                if (!mIsTtsReading) return;

                // ✅ TTS 回调在 Binder 线程，必须切回主线程
                if ("__TTS_PLAY_FINISHED__".equals(sentence)) {
                    runOnUiThread(() -> autoAdvanceTtsPage());
                    return;
                }

                // highlightCurrentSentence 内部已有 runOnUiThread，无需处理
                highlightCurrentSentence(sentence);
            }
        };

    protected void highlightCurrentSentence(String sentence) {
        if (
            sentence == null || sentence.trim().isEmpty() || core == null
        ) return;
        final String original = sentence.trim().replaceAll("\\n+", " ");
        final int pageNum = mTtsReadingPage;

        new Thread(() -> {
            Quad[][] hits = core.searchPage(pageNum, original);
            Quad[] targetQuads =
                hits != null && hits.length > 0 ? hits[0] : null;

            // 降级搜索
            if (targetQuads == null) {
                String compressed = original.replaceAll("\\s+", " ");
                if (!compressed.equals(original)) {
                    hits = core.searchPage(pageNum, compressed);
                    if (hits != null && hits.length > 0) targetQuads = hits[0];
                }
            }

            if (targetQuads != null) {
                final Quad[] quads = targetQuads;
                runOnUiThread(() -> {
                    if (!mIsTtsReading || mDocView == null) return;
                    View v = mDocView.getDisplayedView();
                    if (
                        v instanceof PageView &&
                        ((PageView) v).getPage() == pageNum
                    ) {
                        ((PageView) v).setTtsHighlight(quads);

                        // ✅ 触发 ReaderView 自动滚动到高亮位置
                        float centerY = 0;
                        for (Quad q : quads) centerY += (q.ul_y + q.lr_y) / 2f;
                        centerY /= quads.length;

                        mDocView.scrollToDocY(centerY);
                    }
                });
            }
        }).start();
    }

    private void updateTtsButtonState() {
        if (ttsButton == null) return;
        if (mIsTtsReading) {
            ttsButton.setImageResource(R.drawable.ic_stop_white_24dp);
        } else {
            ttsButton.setImageResource(R.drawable.ic_volume_up_white_24dp);
        }
    }

    private void updateSleepTimerButtonState() {
        if (sleepTimerButton == null) return;
        if (mSleepTimerEnabled) sleepTimerButton.setColorFilter(0xFF42A5F5);
        else sleepTimerButton.clearColorFilter();
    }

    private void syncTtsUiState() {
        if (!MyService.isTextPlaying()) {
            mIsTtsReading = false;
            updateTtsButtonState();
            return;
        }
        mIsTtsReading = true;
        mTtsReadingPage =
            mDocView != null ? mDocView.getDisplayedViewIndex() : 0;
        updateTtsButtonState();
        MyService service = MyService.getInstance();
        if (service != null) service.setTtsSentenceListener(
            mTtsSentenceListener
        );
    }

    /////////////////////////////////////////////////////////////////////////////////
    // ================= 笔记与 AI 弹窗相关方法开始 =================

    /**
     * 弹出独立文本窗口：显示当前页全文，高亮并定位到长按处的文字
     */
    public void showTextSelectionDialog(
        String fullText,
        String targetText,
        int pageNum
    ) {
        SpannableString spannable = new SpannableString(fullText);
        int highlightStart = -1;
        int highlightEnd = -1;

        if (targetText != null && !targetText.isEmpty()) {
            int idx = fullText.indexOf(targetText);
            if (idx < 0) {
                String compressed = targetText.replaceAll("\\s+", " ").trim();
                if (compressed.length() >= 3) idx = fullText.indexOf(
                    compressed
                );
                if (idx >= 0) targetText = compressed;
            }
            if (idx >= 0) {
                highlightStart = idx;
                highlightEnd = Math.min(
                    idx + targetText.length(),
                    fullText.length()
                );
                int hlColor = mInvertMode ? 0x66FFD54F : 0x88FFEB3B;
                spannable.setSpan(
                    new CenteredHighlightSpan(hlColor),
                    highlightStart,
                    highlightEnd,
                    Spanned.SPAN_EXCLUSIVE_EXCLUSIVE
                );
            }
        }

        final boolean dark = mInvertMode;
        final int bgColor = dark ? 0xFF000000 : 0xFFFAFAFA;
        final int textMain = dark ? 0xFFB8A890 : 0xFF333333;
        final int textSub = dark ? 0xFF706858 : 0xFF999999;

        int dp16 = dp2px(16);
        int dp12 = dp2px(12);
        int dp8 = dp2px(8);

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setBackgroundColor(bgColor);

        TextView headerLabel = new TextView(this);
        headerLabel.setText(
            (MyActivity.zh_cn ? "第 " : "Page ") +
                (pageNum + 1) +
                (MyActivity.zh_cn ? " 页" : "")
        );
        headerLabel.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12);
        headerLabel.setTextColor(textSub);
        headerLabel.setPadding(dp16, dp12, dp16, 0);
        root.addView(
            headerLabel,
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
        );

        final TextView textView = new TextView(this);
        textView.setText(spannable);
        textView.setTextIsSelectable(true);
        textView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 16);
        textView.setTextColor(textMain);
        textView.setLineSpacing(0, 1.4f);
        textView.setPadding(dp16, dp12, dp16, dp16);

        final ScrollView scrollView = new ScrollView(this);
        scrollView.setFillViewport(true);
        scrollView.setBackgroundColor(bgColor);
        scrollView.addView(
            textView,
            new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
        );
        root.addView(
            scrollView,
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                0,
                1f
            )
        );

        LinearLayout btnBar = new LinearLayout(this);
        btnBar.setOrientation(LinearLayout.HORIZONTAL);
        btnBar.setGravity(Gravity.END);
        btnBar.setPadding(dp8, dp8, dp8, dp8);
        btnBar.setBackgroundColor(bgColor);
        TextView btnClose = createButton(MyActivity.zh_cn ? "关闭" : "Close");
        btnBar.addView(
            btnClose,
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
        );
        root.addView(
            btnBar,
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
        );

        final Dialog dialog = new Dialog(this);
        dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        dialog.setContentView(root);
        dialog.setCancelable(true);
        if (dialog.getWindow() != null) {
            dialog.getWindow().setDimAmount(0.5f);
            dialog
                .getWindow()
                .setBackgroundDrawableResource(
                    dark
                        ? android.R.drawable.dialog_holo_dark_frame
                        : android.R.drawable.dialog_holo_light_frame
                );
        }

        textView.setCustomSelectionActionModeCallback(
            new TextSelectionActionModeCallback(textView, dialog)
        );
        btnClose.setOnClickListener(v -> dialog.dismiss());
        dialog.show();

        DisplayMetrics dm = getResources().getDisplayMetrics();
        int dialogW = Math.min(
            (int) (dm.widthPixels * 0.75f),
            (int) TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_DIP,
                1200,
                dm
            )
        );
        int dialogH = Math.min(
            (int) (dm.heightPixels * 0.85f),
            (int) TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_DIP,
                900,
                dm
            )
        );
        if (dialog.getWindow() != null) dialog
            .getWindow()
            .setLayout(dialogW, dialogH);

        if (highlightStart >= 0) {
            final int fStart = highlightStart;
            final int fEnd = highlightEnd;
            textView.post(() -> {
                android.text.Layout layout = textView.getLayout();
                if (layout == null) textView.post(() ->
                    scrollToHighlight(textView, scrollView, fStart, fEnd)
                );
                else scrollToHighlight(textView, scrollView, fStart, fEnd);
            });
        }
    }

    private class TextSelectionActionModeCallback
        implements ActionMode.Callback
    {

        private final TextView textView;
        private final Dialog parentDialog;
        private static final int ID_AI_SEARCH = 1001;
        private static final int ID_WEB_SEARCH = 1002;
        private static final int ID_ADD_NOTE = 1003;
        private static final int CONTEXT_RADIUS = 50;

        public TextSelectionActionModeCallback(
            TextView textView,
            Dialog parentDialog
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
                    showAiLoadingDialog();
                    MyActivity.mInstance.PublicJavaCallCpp(
                        "pdf_ai_search|==|" + selectedText
                    );
                    return true;
                case ID_WEB_SEARCH:
                    try {
                        Intent intent = new Intent(Intent.ACTION_WEB_SEARCH);
                        intent.putExtra(
                            android.app.SearchManager.QUERY,
                            selectedText
                        );
                        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        startActivity(intent);
                    } catch (Exception e) {
                        Toast.makeText(
                            DocumentActivity.this,
                            "Cannot launch web search",
                            Toast.LENGTH_SHORT
                        ).show();
                    }
                    mode.finish();
                    if (
                        parentDialog != null && parentDialog.isShowing()
                    ) parentDialog.dismiss();
                    return true;
                case ID_ADD_NOTE:
                    CharSequence fullText = textView.getText();
                    int totalLen = fullText.length();
                    int ctxStart = Math.max(0, start - CONTEXT_RADIUS);
                    int ctxEnd = Math.min(totalLen, end + CONTEXT_RADIUS);
                    String searchContext = fullText
                        .subSequence(ctxStart, ctxEnd)
                        .toString()
                        .replaceAll("\\s+", " ")
                        .trim();
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
        public void onDestroyActionMode(ActionMode mode) {}
    }

    private void scrollToHighlight(
        TextView textView,
        ScrollView scrollView,
        int start,
        int end
    ) {
        android.text.Layout layout = textView.getLayout();
        if (layout == null) return;
        int lineTop = layout.getLineTop(layout.getLineForOffset(start));
        int lineBottom = layout.getLineBottom(
            layout.getLineForOffset(
                Math.min(end, layout.getText().length() - 1)
            )
        );
        int highlightCenterY =
            (lineTop + lineBottom) / 2 + textView.getPaddingTop();
        int scrollTarget = Math.max(
            0,
            highlightCenterY - scrollView.getHeight() / 3
        );
        scrollView.smoothScrollTo(0, scrollTarget);
    }

    private static class CenteredHighlightSpan extends BackgroundColorSpan {

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
            Paint.FontMetrics fm = paint.getFontMetrics();
            float textHeight = fm.descent - fm.ascent;
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

    public void showNoteListDialog(ArrayList<String> noteList) {
        runOnUiThread(() -> {
            if (noteList == null || noteList.isEmpty()) return;
            final ArrayList<String> rawNoteData = new ArrayList<>(noteList);
            final int[] selectedPos = { -1 };
            final boolean dark = mInvertMode;
            int bgRoot = dark ? 0xFF202020 : 0xFFFFFFFF;
            int textColorMain = dark ? 0xFFEFEFEF : 0xFF222222;
            int textColorSub = dark ? 0xFFB0B0B0 : 0xFF555555;
            int itemSelectedBg = dark ? 0xFF354259 : 0xFFE0EDFF;
            int dividerColor = dark ? 0xFF444444 : 0xFFDDDDDD;

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
                        tvNote.setTypeface(
                            Typeface.create(tvNote.getTypeface(), Typeface.BOLD)
                        );
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
                    holder.tvPageTime.setTextColor(textColorSub);
                    holder.tvHtml.setTextColor(textColorMain);
                    holder.tvNote.setTextColor(textColorMain);
                    String fullStr = rawNoteData.get(position);
                    String[] parts = fullStr.split("===", 9);
                    if (parts.length < 8) return itemView;
                    String pageStr = parts[1].trim();
                    if (
                        !core.isReflowable() && !pageStr.equals("-1")
                    ) holder.tvPageTime.setText(
                        (MyActivity.zh_cn ? "页码：" : "Page:") +
                            pageStr +
                            " | " +
                            parts[2].trim()
                    );
                    else holder.tvPageTime.setText(parts[2].trim());
                    Spanned spannedHtml =
                        Build.VERSION.SDK_INT >= Build.VERSION_CODES.N
                            ? Html.fromHtml(
                                  parts[3].trim(),
                                  Html.FROM_HTML_MODE_LEGACY
                              )
                            : Html.fromHtml(parts[3].trim());
                    holder.tvHtml.setText(spannedHtml);
                    holder.tvNote.setText(parts[4].trim());
                    itemView.setBackgroundColor(
                        selectedPos[0] == position ? itemSelectedBg : 0x00000000
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
            listView.setDivider(new ColorDrawable(dividerColor));
            listView.setBackgroundColor(bgRoot);

            LinearLayout root = new LinearLayout(this);
            root.setOrientation(LinearLayout.VERTICAL);
            root.setBackgroundColor(bgRoot);
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
            btnBar.setBackgroundColor(bgRoot);
            TextView btnGo = createButton(MyActivity.zh_cn ? "转到" : "Go");
            TextView btnEdit = createButton(MyActivity.zh_cn ? "编辑" : "Edit");
            TextView btnDel = createButton(
                MyActivity.zh_cn ? "删除" : "Delete"
            );
            TextView btnClose = createButton(
                MyActivity.zh_cn ? "关闭" : "Close"
            );
            LinearLayout.LayoutParams btnLp = new LinearLayout.LayoutParams(
                0,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                1.0f
            );
            btnBar.addView(btnGo, btnLp);
            btnBar.addView(btnEdit, btnLp);
            btnBar.addView(btnDel, btnLp);
            btnBar.addView(btnClose, btnLp);
            root.addView(
                btnBar,
                new LinearLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT
                )
            );

            final Dialog dialog = new Dialog(this);
            dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
            dialog.setContentView(root);
            dialog.setCancelable(true);
            DisplayMetrics dm = getResources().getDisplayMetrics();
            int dialogW = (int) (dm.widthPixels * 0.95f);
            int dialogH = (int) (dm.heightPixels * 0.95f);

            listView.setOnItemClickListener((parent, view, position, id) -> {
                selectedPos[0] = position;
                adapter.notifyDataSetChanged();
            });

            btnGo.setOnClickListener(v -> {
                if (selectedPos[0] < 0) return;
                String[] parts = rawNoteData
                    .get(selectedPos[0])
                    .split("===", 9);
                final String keyword = parts[6].trim();
                final int savedPage = Integer.parseInt(parts[1].trim());
                final String savedBookmark =
                    parts.length > 8 ? parts[8].trim() : "";
                dialog.dismiss();
                if (keyword.isEmpty()) {
                    Toast.makeText(
                        DocumentActivity.this,
                        "Keyword empty",
                        Toast.LENGTH_SHORT
                    ).show();
                    return;
                }
                mDocView.pushHistory();
                if (core.isReflowable()) relayoutThenSearchNote(
                    savedPage,
                    keyword,
                    savedBookmark
                );
                else {
                    // ✅ 【PDF 策略】先跳页，再手动注入搜索结果以触发高亮
                    int targetPage = savedPage - 1;
                    if (targetPage < 0 || targetPage >= core.countPages()) {
                        targetPage = mDocView.getDisplayedViewIndex();
                    }

                    mDocView.pushHistory();
                    mDocView.setDisplayedViewIndex(targetPage);

                    // ✅ 使用 final 变量供 lambda/匿名类捕获
                    final int finalTargetPage = targetPage;
                    final String finalKeyword = keyword;

                    new Thread(() -> {
                        Quad[][] hits = core.searchPage(
                            finalTargetPage,
                            finalKeyword
                        );
                        runOnUiThread(() -> {
                            if (hits != null && hits.length > 0) {
                                // ✅ 直接使用带参构造器创建 SearchTaskResult
                                SearchTaskResult result = new SearchTaskResult(
                                    finalKeyword,
                                    finalTargetPage,
                                    hits
                                );
                                SearchTaskResult.set(result);

                                if (mDocView != null) {
                                    mDocView.resetupChildren();
                                }
                            } else {
                                Toast.makeText(
                                    DocumentActivity.this,
                                    MyActivity.zh_cn
                                        ? "未找到关键词高亮"
                                        : "Keyword highlight not found",
                                    Toast.LENGTH_SHORT
                                ).show();
                            }
                        });
                    }).start();
                }
            });

            btnEdit.setOnClickListener(v -> {
                if (selectedPos[0] < 0) return;
                String[] parts = rawNoteData
                    .get(selectedPos[0])
                    .split("===", 9);
                showNoteInputDialog(
                    parts[6].trim(),
                    parts[4].trim(),
                    parts[5].trim(),
                    parts[0].trim(),
                    dialog
                );
            });

            btnDel.setOnClickListener(v -> {
                if (selectedPos[0] < 0) return;
                String[] parts = rawNoteData
                    .get(selectedPos[0])
                    .split("===", 9);
                String id = parts[0].trim();
                new AlertDialog.Builder(DocumentActivity.this)
                    .setTitle(MyActivity.zh_cn ? "确认删除" : "Confirm Delete")
                    .setMessage(
                        MyActivity.zh_cn
                            ? "确定删除这条笔记？"
                            : "Are you sure?"
                    )
                    .setPositiveButton(
                        MyActivity.zh_cn ? "删除" : "Delete",
                        (d, w) -> {
                            MyActivity.mInstance.PublicJavaCallCpp(
                                "pdf_note_delete|==|" + id
                            );
                            rawNoteData.remove(selectedPos[0]);
                            selectedPos[0] = -1;
                            adapter.notifyDataSetChanged();
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
                dialog
                    .getWindow()
                    .setBackgroundDrawableResource(
                        mInvertMode
                            ? android.R.drawable.dialog_holo_dark_frame
                            : android.R.drawable.dialog_holo_light_frame
                    );
            }
        });
    }

    private void showNoteInputDialog(
        String keyword,
        String existingNote,
        String searchContext,
        String noteId,
        Dialog parentDialog
    ) {
        boolean isEditMode = noteId != null && !noteId.isEmpty();
        int dp16 = dp2px(16);
        int dp12 = dp2px(12);
        int dp8 = dp2px(8);
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(dp16, dp12, dp16, dp12);
        LinearLayout titleRow = new LinearLayout(this);
        titleRow.setOrientation(LinearLayout.HORIZONTAL);
        titleRow.setGravity(Gravity.CENTER_VERTICAL);
        titleRow.setPadding(0, 0, 0, dp8);
        TextView labelNote = new TextView(this);
        labelNote.setText(MyActivity.zh_cn ? "笔记" : "Note");
        labelNote.setTextSize(TypedValue.COMPLEX_UNIT_SP, 16);
        labelNote.setTypeface(Typeface.DEFAULT_BOLD);
        labelNote.setTextColor(mInvertMode ? 0xFFDDDDDD : 0xFF333333);
        titleRow.addView(
            labelNote,
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
        );
        TextView separator = new TextView(this);
        separator.setText(" · ");
        separator.setTextSize(TypedValue.COMPLEX_UNIT_SP, 16);
        separator.setTextColor(mInvertMode ? 0xFF888888 : 0xFF999999);
        titleRow.addView(
            separator,
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
        );
        String titleText =
            keyword.length() > 50 ? keyword.substring(0, 50) + "…" : keyword;
        TextView titleView = new TextView(this);
        titleView.setText(titleText);
        titleView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 14);
        titleView.setTextColor(mInvertMode ? 0xFFBBDEFB : 0xFF1565C0);
        titleView.setSingleLine(true);
        titleView.setEllipsize(TextUtils.TruncateAt.END);
        titleRow.addView(
            titleView,
            new LinearLayout.LayoutParams(
                0,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                1f
            )
        );
        root.addView(
            titleRow,
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
        );

        final EditText noteEdit = new EditText(this);
        noteEdit.setHint(
            MyActivity.zh_cn ? "输入笔记内容..." : "Enter note content..."
        );
        noteEdit.setGravity(Gravity.TOP | Gravity.START);
        noteEdit.setTextSize(TypedValue.COMPLEX_UNIT_SP, 15);
        noteEdit.setTextColor(mInvertMode ? 0xFFDDDDDD : 0xFF333333);
        noteEdit.setHintTextColor(mInvertMode ? 0xFF666666 : 0xFF999999);
        if (mInvertMode) noteEdit.setBackgroundColor(0xFF2D2D2D);
        noteEdit.setPadding(dp12, dp12, dp12, dp12);
        noteEdit.setInputType(
            InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_MULTI_LINE
        );
        if (isEditMode) {
            noteEdit.setText(existingNote);
            noteEdit.setSelection(existingNote.length());
        }
        LinearLayout.LayoutParams editParams = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            0,
            1.0f
        );
        editParams.topMargin = dp8;
        editParams.bottomMargin = dp8;
        root.addView(noteEdit, editParams);

        LinearLayout btnBar = new LinearLayout(this);
        btnBar.setOrientation(LinearLayout.HORIZONTAL);
        btnBar.setGravity(Gravity.END);
        btnBar.setWeightSum(2);
        TextView btnCancel = createButton(MyActivity.zh_cn ? "取消" : "Cancel");
        TextView btnSave = createButton(MyActivity.zh_cn ? "保存" : "Save");
        btnSave.setTypeface(Typeface.DEFAULT_BOLD);
        btnSave.setTextColor(mInvertMode ? 0xFF90CAF9 : 0xFF1565C0);
        btnBar.addView(
            btnCancel,
            new LinearLayout.LayoutParams(
                0,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                1f
            )
        );
        btnBar.addView(
            btnSave,
            new LinearLayout.LayoutParams(
                0,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                1f
            )
        );
        root.addView(
            btnBar,
            new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
        );

        final Dialog noteDialog = new Dialog(this);
        noteDialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        noteDialog.setContentView(root);
        noteDialog.setCancelable(true);
        if (noteDialog.getWindow() != null) {
            noteDialog.getWindow().setDimAmount(0.3f);
            noteDialog
                .getWindow()
                .setBackgroundDrawableResource(
                    mInvertMode
                        ? android.R.drawable.dialog_holo_dark_frame
                        : android.R.drawable.dialog_holo_light_frame
                );
        }

        btnCancel.setOnClickListener(v -> noteDialog.dismiss());
        btnSave.setOnClickListener(v -> {
            String noteContent = noteEdit.getText().toString().trim();
            if (noteContent.isEmpty()) {
                Toast.makeText(
                    DocumentActivity.this,
                    MyActivity.zh_cn
                        ? "笔记内容不能为空"
                        : "Note cannot be empty",
                    Toast.LENGTH_SHORT
                ).show();
                return;
            }
            String bookmark = "";
            int currentPage = mDocView.getDisplayedViewIndex();
            int pageForNote = currentPage + 1;
            if (core.isReflowable()) bookmark = String.valueOf(mLayoutEM);
            else bookmark = core.makeCurrentBookmark(currentPage);

            String payload;
            if (isEditMode) payload =
                "pdf_note_update|==|" +
                noteId +
                "|==|" +
                searchContext +
                "|==|" +
                keyword +
                "|==|" +
                noteContent;
            else payload =
                "pdf_save_note|==|" +
                searchContext +
                "|==|" +
                keyword +
                "|==|" +
                noteContent +
                "|==|" +
                pageForNote +
                "|==|" +
                bookmark;

            MyActivity.mInstance.PublicJavaCallCpp(payload);
            Toast.makeText(
                DocumentActivity.this,
                MyActivity.zh_cn ? "已保存" : "Saved",
                Toast.LENGTH_SHORT
            ).show();
            noteDialog.dismiss();
            if (
                parentDialog != null && parentDialog.isShowing()
            ) parentDialog.dismiss();
        });

        noteDialog.show();
        if (noteDialog.getWindow() != null) {
            DisplayMetrics dm = getResources().getDisplayMetrics();
            noteDialog
                .getWindow()
                .setLayout(
                    (int) (dm.widthPixels * 0.9),
                    (int) (dm.heightPixels * 0.9)
                );
            noteDialog
                .getWindow()
                .setSoftInputMode(
                    WindowManager.LayoutParams.SOFT_INPUT_ADJUST_PAN
                );
        }
        noteEdit.requestFocus();
    }

    // ================= 辅助方法 =================
    private TextView createButton(String text) {
        TextView tv = new TextView(this);
        tv.setText(text);
        tv.setPadding(dp2px(8), dp2px(8), dp2px(8), dp2px(8));
        tv.setGravity(Gravity.CENTER);
        tv.setTextSize(TypedValue.COMPLEX_UNIT_SP, 14);
        tv.setTextColor(mInvertMode ? 0xFFEFEFEF : 0xFF222222);
        return tv;
    }

    private int dp2px(int dpVal) {
        return (int) (dpVal * getResources().getDisplayMetrics().density +
            0.5f);
    }

    public void showAiLoadingDialog() {
        runOnUiThread(() -> {
            if (
                isFinishing() ||
                isDestroyed() ||
                (mAiLoadingDialog != null && mAiLoadingDialog.isShowing())
            ) return;
            LinearLayout layout = new LinearLayout(this);
            layout.setOrientation(LinearLayout.HORIZONTAL);
            layout.setPadding(dp2px(24), dp2px(16), dp2px(24), dp2px(16));
            layout.setGravity(Gravity.CENTER_VERTICAL);
            layout.addView(new ProgressBar(this));
            TextView tvMsg = new TextView(this);
            tvMsg.setText(MyActivity.zh_cn ? "处理中..." : "Processing...");
            tvMsg.setTextColor(mInvertMode ? 0xFFFFFFFF : 0xFF333333);
            layout.addView(tvMsg);
            androidx.appcompat.app.AlertDialog.Builder builder =
                new androidx.appcompat.app.AlertDialog.Builder(this);
            builder.setView(layout).setCancelable(false);
            mAiLoadingDialog = builder.create();
            mAiLoadingDialog.show();
        });
    }

    public void dismissAiLoadingDialog() {
        runOnUiThread(() -> {
            if (mAiLoadingDialog != null && mAiLoadingDialog.isShowing()) {
                try {
                    mAiLoadingDialog.dismiss();
                } catch (Exception ignored) {}
                mAiLoadingDialog = null;
            }
        });
    }

    public void showAiMarkdownDialog(ArrayList<String> mdList) {
        dismissAiLoadingDialog();
        MyActivity.showAiMarkdownDialog(this, mdList);
    }
    /////////////////////////////////////////////////////////////////////////////////
}
