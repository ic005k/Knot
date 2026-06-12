package com.x;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Application;
import android.app.PendingIntent;
import android.app.Service;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.BroadcastReceiver;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.graphics.*;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.text.Editable;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.method.ScrollingMovementMethod;
import android.text.style.BackgroundColorSpan;
import android.text.style.ForegroundColorSpan;
import android.util.AttributeSet;
import android.util.Log;
import android.util.TypedValue;
import android.view.ContextThemeWrapper;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.PopupMenu;
import android.widget.PopupWindow;
import android.widget.TextView;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;
import android.widget.Toast;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.RandomAccessFile;
import java.lang.reflect.Method;
import java.net.URI;
import java.sql.Time;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.Locale;
import java.util.Locale;
import java.util.Map;
import java.util.Properties;
import java.util.regex.*;

public class LineNumberedEditText extends EditText {

    private final Context context;
    private Rect rect;
    private Paint paint;
    private int totalLines = 2000;

    public LineNumberedEditText(Context context) {
        super(context);
        this.context = context;
        init();
    }

    public LineNumberedEditText(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        init();
    }

    public LineNumberedEditText(
        Context context,
        AttributeSet attrs,
        int defStyle
    ) {
        super(context, attrs, defStyle);
        this.context = context;
        init();
    }

    /*private void init() {
        int lineCount = getLineCount();

        if (lineCount <= totalLines) {
            rect = new Rect();
            paint = new Paint(Paint.ANTI_ALIAS_FLAG);
            paint.setStyle(Paint.Style.FILL);
            paint.setColor(Color.GRAY);
            paint.setTextSize(32);
            paint.setTypeface(Typeface.MONOSPACE);
        }
    }*/

    private void init() {
        rect = new Rect();
        paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        paint.setStyle(Paint.Style.FILL);

        // ✅ 1. 加大行号字体到 14sp（原来相当于约10sp）
        float textSizePx = TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_SP,
            14, // 加大到14sp
            getResources().getDisplayMetrics()
        );
        paint.setTextSize(textSizePx);

        paint.setColor(
            MyActivity.isDark ? Color.parseColor("#666666") : Color.GRAY
        );
        paint.setTypeface(Typeface.MONOSPACE);
    }

    /*@Override
    protected void onDraw(Canvas canvas) {
        int lineCount = getLineCount();

        if (lineCount <= totalLines) {
            int baseline;
            int lineNumber = 1;

            for (int i = 0; i < lineCount; ++i) {
                baseline = getLineBounds(i, null);
                if (i == 0) {
                    canvas.drawText(
                        "" + lineNumber,
                        rect.left,
                        baseline,
                        paint
                    );
                    ++lineNumber;
                } else if (
                    getText().charAt(getLayout().getLineStart(i) - 1) == '\n'
                ) {
                    canvas.drawText(
                        "" + lineNumber,
                        rect.left,
                        baseline,
                        paint
                    );
                    ++lineNumber;
                }
            }

            // for setting edittext start padding
            if (lineCount < 100) {
                setPadding(
                    40,
                    getPaddingTop(),
                    getPaddingRight(),
                    getPaddingBottom()
                );
            } else if (lineCount > 99 && lineCount < 1000) {
                setPadding(
                    60,
                    getPaddingTop(),
                    getPaddingRight(),
                    getPaddingBottom()
                );
            } else if (lineCount > 999 && lineCount < 10000) {
                setPadding(
                    80,
                    getPaddingTop(),
                    getPaddingRight(),
                    getPaddingBottom()
                );
            } else if (lineCount > 9999 && lineCount < 100000) {
                setPadding(
                    100,
                    getPaddingTop(),
                    getPaddingRight(),
                    getPaddingBottom()
                );
            }
        }
        super.onDraw(canvas);
        }*/

    @Override
    protected void onDraw(Canvas canvas) {
        // ✅ 关键：先执行 super，确保 Layout 初始化
        super.onDraw(canvas);

        // ✅ 空判断：Layout 未初始化时直接返回
        android.text.Layout layout = getLayout();
        if (layout == null) {
            return;
        }

        int lineCount = getLineCount();
        if (lineCount <= 0 || lineCount > totalLines) {
            return;
        }

        int baseline;
        int lineNumber = 1;

        // 动态计算当前最大行号需要的宽度
        String maxLineNumber = String.valueOf(lineCount);
        float lineNumberWidth = paint.measureText(maxLineNumber);

        // 行号和正文之间留 20px 间距
        int gap = 20;
        int totalPadding = (int) (lineNumberWidth + gap + 10);

        // 设置 padding
        setPadding(
            totalPadding,
            getPaddingTop(),
            getPaddingRight(),
            getPaddingBottom()
        );

        for (int i = 0; i < lineCount; ++i) {
            baseline = getLineBounds(i, null);

            if (i == 0) {
                // 第一行
                String num = String.valueOf(lineNumber);
                float numWidth = paint.measureText(num);
                float x = (lineNumberWidth - numWidth) + 5;
                canvas.drawText(num, x, baseline, paint);
                ++lineNumber;
            } else {
                // ✅ 这里也要加空判断
                int lineStart = layout.getLineStart(i);
                if (lineStart > 0 && lineStart < getText().length()) {
                    if (getText().charAt(lineStart - 1) == '\n') {
                        String num = String.valueOf(lineNumber);
                        float numWidth = paint.measureText(num);
                        float x = (lineNumberWidth - numWidth) + 5;
                        canvas.drawText(num, x, baseline, paint);
                        ++lineNumber;
                    }
                }
            }
        }
    }
}
