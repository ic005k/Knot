package com.x;

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

public class PopupMenuCustomLayout {
    private PopupMenuCustomOnClickListener onClickListener;
    private Context context;
    private PopupWindow popupWindow;
    private int rLayoutId;
    private View popupView;

    public PopupMenuCustomLayout(Context context, int rLayoutId, PopupMenuCustomOnClickListener onClickListener) {
        this.context = context;
        this.onClickListener = onClickListener;
        this.rLayoutId = rLayoutId;
        LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        popupView = inflater.inflate(rLayoutId, null);
        int width = LinearLayout.LayoutParams.WRAP_CONTENT;
        int height = LinearLayout.LayoutParams.WRAP_CONTENT;
        boolean focusable = true;
        popupWindow = new PopupWindow(popupView, width, height, focusable);
        popupWindow.setElevation(10);

        LinearLayout linearLayout = (LinearLayout) popupView;
        for (int i = 0; i < linearLayout.getChildCount(); i++) {
            View v = linearLayout.getChildAt(i);
            v.setOnClickListener(v1 -> {
                onClickListener.onClick(v1.getId());
                popupWindow.dismiss();
            });
        }
    }

    public void setAnimationStyle(int animationStyle) {
        popupWindow.setAnimationStyle(animationStyle);
    }

    public void show() {
        popupWindow.showAtLocation(popupView, Gravity.CENTER, 0, 0);
    }

    public void show(View anchorView, int gravity, int offsetX, int offsetY) {
        popupWindow.showAsDropDown(anchorView, 0, -2 * (anchorView.getHeight()));
    }

    public interface PopupMenuCustomOnClickListener {
        public void onClick(int menuItemId);
    }
}
