package com.x;

import com.x.MyActivity;
import com.x.NoteEditor;
import com.x.ImageViewerActivity;
import com.x.DefaultGrammars;

import android.os.Bundle;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import androidx.annotation.NonNull;
import android.text.style.ClickableSpan;

import io.noties.markwon.Markwon;
import io.noties.markwon.ext.strikethrough.StrikethroughPlugin;
import io.noties.markwon.ext.tables.TablePlugin;
import io.noties.markwon.ext.tasklist.TaskListPlugin;
import io.noties.markwon.html.HtmlPlugin;
import io.noties.markwon.linkify.LinkifyPlugin;
import io.noties.markwon.simple.ext.SimpleExtPlugin;
import io.noties.markwon.image.glide.GlideImagesPlugin;
import io.noties.markwon.image.ImagesPlugin;
import io.noties.markwon.ext.latex.JLatexMathPlugin;
import io.noties.markwon.AbstractMarkwonPlugin;
import io.noties.markwon.inlineparser.MarkwonInlineParser;
import io.noties.markwon.inlineparser.InlineProcessor;

import io.noties.markwon.inlineparser.MarkwonInlineParserPlugin;

import io.noties.markwon.MarkwonConfiguration;
import io.noties.markwon.MarkwonSpansFactory;
import io.noties.markwon.MarkwonConfiguration;
import io.noties.markwon.core.MarkwonTheme;
import io.noties.markwon.image.AsyncDrawable;
import io.noties.markwon.image.AsyncDrawableLoader;
import io.noties.markwon.image.AsyncDrawableSpan;
import io.noties.markwon.image.ImageProps;
import io.noties.markwon.image.ImageSize;
import io.noties.markwon.image.ImageSizeResolver;
import io.noties.markwon.image.destination.ImageDestinationProcessor;
import io.noties.markwon.image.destination.ImageDestinationProcessorRelativeToAbsolute;
import io.noties.markwon.image.gif.GifMediaDecoder;
import io.noties.markwon.image.network.NetworkSchemeHandler;
import io.noties.markwon.image.svg.SvgMediaDecoder;
import io.noties.markwon.image.ImageSizeResolverDef;
import io.noties.markwon.image.ImageItem;
import io.noties.markwon.image.SchemeHandler;

import io.noties.markwon.Markwon;
import io.noties.markwon.MarkwonConfiguration;
import io.noties.markwon.image.AsyncDrawable;
import io.noties.markwon.image.ImagesPlugin;

import io.noties.markwon.image.ImageSizeResolver;
import io.noties.markwon.SpanFactory;

import io.noties.markwon.core.spans.LinkSpan;
import io.noties.markwon.LinkResolver;

import io.noties.markwon.syntax.Prism4jSyntaxHighlight;
import io.noties.markwon.syntax.Prism4jTheme;
import io.noties.prism4j.Prism4j;
import io.noties.markwon.syntax.Prism4jThemeDefault;
import io.noties.markwon.syntax.Prism4jThemeDarkula;
import io.noties.markwon.syntax.SyntaxHighlightPlugin;

import io.noties.prism4j.annotations.PrismBundle;

import org.commonmark.node.*;
import org.commonmark.parser.Parser;
import org.commonmark.renderer.html.HtmlRenderer;
import org.w3c.dom.Node;

import java.util.Iterator;
import java.util.List;
import java.io.BufferedReader;
import java.io.InputStreamReader;

import java.io.IOException;
import java.io.InputStream;

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
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileInputStream;

import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Properties;
import java.util.Set;
import java.io.IOException;
import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
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

import android.widget.LinearLayout;
import android.view.LayoutInflater;
import android.widget.PopupWindow;
import android.graphics.drawable.ColorDrawable;
import java.lang.reflect.Field;
import android.annotation.SuppressLint;
import androidx.core.content.FileProvider;
import android.widget.PopupMenu;
import android.widget.ImageButton;
import android.content.SharedPreferences;
import android.widget.ScrollView;
import android.text.method.LinkMovementMethod;

import android.text.method.MovementMethod;
import android.text.method.Touch;
import android.view.MotionEvent;
import android.view.ViewTreeObserver;
import android.widget.ImageView;
import android.util.DisplayMetrics;
import android.view.ViewGroup;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import android.text.util.Linkify;
import android.util.TypedValue;

import io.noties.markwon.syntax.Prism4jSyntaxHighlight;
import io.noties.markwon.syntax.Prism4jTheme;
import io.noties.markwon.syntax.Prism4jThemeBase;
import io.noties.markwon.syntax.Prism4jThemeDarkula;
import io.noties.markwon.syntax.Prism4jThemeDefault;
import io.noties.markwon.syntax.SyntaxHighlightPlugin;
import io.noties.prism4j.GrammarLocator;

import static java.util.regex.Pattern.CASE_INSENSITIVE;
import static java.util.regex.Pattern.compile;
import static io.noties.prism4j.Prism4j.grammar;
import static io.noties.prism4j.Prism4j.pattern;
import static io.noties.prism4j.Prism4j.token;

import android.graphics.Color;
import android.text.style.ForegroundColorSpan;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import org.commonmark.node.Emphasis;

import io.noties.markwon.core.CoreProps;
import io.noties.markwon.html.HtmlTag;
import io.noties.markwon.html.tag.SimpleTagHandler;
import io.noties.markwon.MarkwonConfiguration;

public class MDActivity extends Activity implements View.OnClickListener, Application.ActivityLifecycleCallbacks {

    private TextView markdownView;
    private TextView titleView;
    private Button btnEdit;
    private ScrollView scrollView;
    private String strmdFileName;

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

        String filePath = MyActivity.strMDFile;
        File md_file = new File(filePath);
        strmdFileName = md_file.getName();

        Application application = this.getApplication();
        application.registerActivityLifecycleCallbacks(this);

        // 去除title(App Name)
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        setContentView(R.layout.activity_md);
        markdownView = findViewById(R.id.markdownView);
        markdownView.setTextSize(TypedValue.COMPLEX_UNIT_SP, MyActivity.myFontSize);

        markdownView.setText("Hello");

        titleView = findViewById(R.id.title);
        titleView.setText(MyActivity.strMDTitle);

        btnEdit = (Button) findViewById(R.id.btnEdit);
        if (MyActivity.zh_cn)
            btnEdit.setText("编辑");
        else
            btnEdit.setText("Edit");
        btnEdit.setOnClickListener(this);

        scrollView = findViewById(R.id.scroll_view);

        final Prism4j prism4j = new Prism4j(new MyGrammarLocator());

        // 初始化 Markwon
        // .usePlugin(ImagesPlugin.create())
        final Markwon markwon = Markwon.builder(this)
                .usePlugin(StrikethroughPlugin.create())
                .usePlugin(TablePlugin.create(this))
                .usePlugin(TaskListPlugin.create(this))
                .usePlugin(LinkifyPlugin.create())
                .usePlugin(SimpleExtPlugin.create())
                .usePlugin(GlideImagesPlugin.create(this))
                .usePlugin(MarkwonInlineParserPlugin.create())
                .usePlugin(JLatexMathPlugin.create(markdownView.getTextSize(), builder -> {
                    builder.inlinesEnabled(true); // 启用行内公式
                }))
                .usePlugin(new AbstractMarkwonPlugin() {
                    @Override
                    public void configureSpansFactory(MarkwonSpansFactory.Builder builder) {
                        builder.appendFactory(Image.class, (configuration, props) -> {
                            String url = ImageProps.DESTINATION.require(props);
                            return new LinkSpan(configuration.theme(), url, new ImageLinkResolver());
                        });
                    }
                })
                .usePlugin(SyntaxHighlightPlugin.create(prism4j, Prism4jThemeDarkula.create()))
                .usePlugin(HtmlPlugin.create())
                .build();

        StringBuilder markdownContent = new StringBuilder();
        try {
            // "/storage/emulated/0/.Knot/mymd.md"
            File file = new File(MyActivity.strMDFile);
            BufferedReader reader = new BufferedReader(new FileReader(file));
            String line;
            while ((line = reader.readLine()) != null) {
                markdownContent.append(line).append("\n");
            }
            reader.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        String strMD = markdownContent.toString();
        strMD = strMD.replace("images/", "/storage/emulated/0/KnotData/memo/images/");
        markwon.setMarkdown(markdownView, strMD);

        // HomeKey
        registerReceiver(mHomeKeyEvent, new IntentFilter(Intent.ACTION_CLOSE_SYSTEM_DIALOGS));

        MyActivity.isEdit = false;

        markdownView.getViewTreeObserver().addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {

            public boolean onPreDraw() {
                // 确保只执行一次
                markdownView.getViewTreeObserver().removeOnPreDrawListener(this);

                // 恢复滚动条位置
                SharedPreferences sharedPreferences = getSharedPreferences(strmdFileName + "scroll_position",
                        MODE_PRIVATE);
                int savedScrollX = sharedPreferences.getInt("scrollX", 0);
                int savedScrollY = sharedPreferences.getInt("scrollY", 0);
                scrollView.scrollTo(savedScrollX, savedScrollY);
                // Toast.makeText(MDActivity.this, savedScrollX + " " + savedScrollY,
                // Toast.LENGTH_LONG).show();

                return true; // 返回 true 表示继续绘制
            }
        });

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

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnEdit:

                MyActivity.isEdit = true;
                Intent i = new Intent(this, NoteEditor.class);
                i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                this.startActivity(i);
                onBackPressed();
                break;
        }
    }

    @Override
    public void onResume() {

        super.onResume();

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
        // 保存滚动条位置
        int scrollX = scrollView.getScrollX();
        int scrollY = scrollView.getScrollY();
        // 可以将scrollX和scrollY存储起来，例如保存在SharedPreferences中
        SharedPreferences sharedPreferences = getSharedPreferences(strmdFileName + "scroll_position",
                MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putInt("scrollX", scrollX);
        editor.putInt("scrollY", scrollY);
        editor.apply();

        super.onBackPressed();
        // AnimationWhenClosed();

    }

    @Override
    protected void onDestroy() {
        unregisterReceiver(mHomeKeyEvent);

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

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putInt("scrollY", scrollView.getScrollY());
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        if (savedInstanceState != null) {
            int scrollY = savedInstanceState.getInt("scrollY");
            scrollView.post(() -> scrollView.scrollTo(0, scrollY));
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

    // 使用系统图片查看器打开图片
    private void openImageWithSystemViewer(String imageUrl) {
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setDataAndType(Uri.parse(imageUrl), "image/*");
        startActivity(intent);
    }

    private class ImageLinkResolver implements LinkResolver {

        @Override
        public void resolve(android.view.View view, String link) {

            // Toast.makeText(MDActivity.this, link,
            // Toast.LENGTH_LONG).show();

            callImageView(link);
            // callThirdPartyImageViewer(link);

            onBackPressed();

        }
    }

    private void callImageView(String link) {

        MyActivity.strImageFile = link;
        Intent i = new Intent(this, ImageViewerActivity.class);
        i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        this.startActivity(i);

    }

    private void callThirdPartyImageViewer(String link) {

        Uri photoUri;
        if (Build.VERSION.SDK_INT >= 24) {
            photoUri = FileProvider.getUriForFile(
                    MyActivity.context,
                    "com.x",
                    new File(link));
        } else {
            photoUri = Uri.fromFile(new File(link));
        }

        // 用第三方的图片查看器打开， 创建一个Intent，用于打开图片
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setDataAndType(photoUri, "image/*");

        // 检查是否有应用可以处理该Intent
        if (intent.resolveActivity(getPackageManager()) != null) {
            startActivity(intent);
        }

    }

}