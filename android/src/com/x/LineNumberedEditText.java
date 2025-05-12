package com.x;

import android.util.TypedValue;
import android.util.AttributeSet;
import android.graphics.*;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.graphics.Paint;
import android.view.ContextThemeWrapper;
import android.view.LayoutInflater;
import android.view.Gravity;
import android.content.IntentFilter;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.app.PendingIntent;
import android.text.TextUtils;
import android.app.AlertDialog;
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
import java.sql.Time;
import android.text.SpannableStringBuilder;
import android.text.style.BackgroundColorSpan;
import android.text.style.ForegroundColorSpan;
import android.text.Spannable;
import android.text.Spanned;
import android.view.MenuItem;
import android.widget.PopupMenu;
import android.text.method.ScrollingMovementMethod;
import android.view.inputmethod.EditorInfo;
import android.view.KeyEvent;
import android.widget.TextView.OnEditorActionListener;
import android.view.inputmethod.InputMethodManager;
import java.util.regex.*;
import android.widget.LinearLayout;
import android.view.LayoutInflater;
import android.widget.PopupWindow;

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

    public LineNumberedEditText(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        this.context = context;
        init();
    }

    private void init() {
        int lineCount = getLineCount();

        if (lineCount <= totalLines) {
            rect = new Rect();
            paint = new Paint(Paint.ANTI_ALIAS_FLAG);
            paint.setStyle(Paint.Style.FILL);
            paint.setColor(Color.GRAY);
            paint.setTextSize(32);
            paint.setTypeface(Typeface.MONOSPACE);
        }
    }

    @Override
    protected void onDraw(Canvas canvas) {

        int lineCount = getLineCount();

        if (lineCount <= totalLines) {
            int baseline;
            int lineNumber = 1;

            for (int i = 0; i < lineCount; ++i) {
                baseline = getLineBounds(i, null);
                if (i == 0) {
                    canvas.drawText("" + lineNumber, rect.left, baseline, paint);
                    ++lineNumber;
                } else if (getText().charAt(getLayout().getLineStart(i) - 1) == '\n') {
                    canvas.drawText("" + lineNumber, rect.left, baseline, paint);
                    ++lineNumber;
                }
            }

            // for setting edittext start padding
            if (lineCount < 100) {
                setPadding(40, getPaddingTop(), getPaddingRight(), getPaddingBottom());
            } else if (lineCount > 99 && lineCount < 1000) {
                setPadding(60, getPaddingTop(), getPaddingRight(), getPaddingBottom());
            } else if (lineCount > 999 && lineCount < 10000) {
                setPadding(80, getPaddingTop(), getPaddingRight(), getPaddingBottom());
            } else if (lineCount > 9999 && lineCount < 100000) {
                setPadding(100, getPaddingTop(), getPaddingRight(), getPaddingBottom());
            }
        }
        super.onDraw(canvas);

    }
}
