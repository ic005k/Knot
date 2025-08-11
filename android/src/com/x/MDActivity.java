package com.x;

import com.stehno.mermaid.MermaidRenderer;
import com.stehno.mermaid.MermaidException;


import com.x.MyActivity;
import com.x.NoteEditor;
import com.x.ImageViewerActivity;
import com.x.MDActivity.MarkdownAdapter;
import com.x.DefaultGrammars;

import android.os.Bundle;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import androidx.annotation.NonNull;
import android.text.style.ClickableSpan;

import android.graphics.Bitmap;
import io.noties.markwon.MarkwonVisitor;
//import org.commonmark.node.CodeBlock;
// 替换原来的CodeBlock导入，使用0.13.0版本的代码块类
//import com.atlassian.commonmark.node.FencedCodeBlock;
import org.commonmark.node.FencedCodeBlock;
//import android.text.style.WidgetSpan;
import android.view.ViewGroup;
//import io.noties.markwon.widget.WidgetSpan;
import android.text.TextPaint;

import android.text.SpannableString;
import android.text.Spanned;
import android.graphics.Paint; 

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
import org.w3c.dom.Text;

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
import android.widget.ProgressBar;

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
import io.noties.markwon.core.CoreProps;
import io.noties.markwon.html.HtmlTag;
import io.noties.markwon.html.tag.SimpleTagHandler;
import io.noties.markwon.MarkwonConfiguration;

import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

public class MDActivity extends AppCompatActivity implements View.OnClickListener, Application.ActivityLifecycleCallbacks {

    private TextView markdownView;
    private TextView titleView;
    private Button btnEdit;

    private String strmdFileName;
    private static Markwon markwon;

    private RecyclerView recyclerView;
    private MarkdownAdapter adapter;
    private List<String> chunks = new ArrayList<>();
    private static final int CHUNK_SIZE = 100; // 每100行作为一个块

    private static final int REQUEST_EDIT = 1; // 可以是任意整数（建议从 1 开始）
    public static final int RESULT_SAVE = 1001;

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

        if (MyActivity.isDark) {
            this.setStatusBarColor("#19232D"); // 深色
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
            setContentView(R.layout.activity_md_dark);
        } else {
            this.setStatusBarColor("#F3F3F3"); // 灰
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
            setContentView(R.layout.activity_md);
        }

        initMarkdwon();

        initRecyclerView();

        titleView = findViewById(R.id.title);
        titleView.setText(MyActivity.strMDTitle);

        btnEdit = (Button) findViewById(R.id.btnEdit);
        if (MyActivity.zh_cn)
            btnEdit.setText("编辑");
        else
            btnEdit.setText("Edit");
        btnEdit.setOnClickListener(this);

        loadMD();

        MyActivity.isEdit = false;

    }

    /*private void initMarkdwon() {
        float fixedTextSize = 16; // 单位：sp

        // 将 sp 转换为像素（JLatexMathPlugin 需要像素值）
        float textSizeInPx = TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_SP,
                fixedTextSize,
                getResources().getDisplayMetrics());
        final Prism4j prism4j = new Prism4j(new MyGrammarLocator());

        // 初始化 Markwon
        Context appContext = getApplicationContext();
        if (!MyActivity.isDark) {
            markwon = Markwon.builder(appContext)
                    .usePlugin(StrikethroughPlugin.create())
                    .usePlugin(TablePlugin.create(appContext))
                    .usePlugin(TaskListPlugin.create(appContext))
                    .usePlugin(LinkifyPlugin.create())
                    .usePlugin(SimpleExtPlugin.create())
                    .usePlugin(GlideImagesPlugin.create(appContext))
                    .usePlugin(MarkwonInlineParserPlugin.create())
                    .usePlugin(JLatexMathPlugin.create(textSizeInPx, builder -> {
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
                    .usePlugin(SyntaxHighlightPlugin.create(prism4j, Prism4jThemeDefault.create()))
                    .usePlugin(HtmlPlugin.create())
                    .build();
        } else {
            markwon = Markwon.builder(appContext)
                    .usePlugin(StrikethroughPlugin.create())
                    .usePlugin(TablePlugin.create(appContext))
                    .usePlugin(TaskListPlugin.create(appContext))
                    .usePlugin(LinkifyPlugin.create())
                    .usePlugin(SimpleExtPlugin.create())
                    .usePlugin(GlideImagesPlugin.create(appContext))
                    .usePlugin(MarkwonInlineParserPlugin.create())
                    .usePlugin(JLatexMathPlugin.create(textSizeInPx, builder -> {
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

        }
    }*/

    private void initMarkdwon() {
        float fixedTextSize = 16; // 单位：sp
        float textSizeInPx = TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_SP,
                fixedTextSize,
                getResources().getDisplayMetrics());
        final Prism4j prism4j = new Prism4j(new MyGrammarLocator());
    
        AbstractMarkwonPlugin mermaidPlugin = new AbstractMarkwonPlugin() {
            @Override
            public void configureVisitor(@NonNull MarkwonVisitor.Builder builder) {
                builder.on(FencedCodeBlock.class, (visitor, codeBlock) -> {
                    // 获取代码块语言标识
                    String language = codeBlock.getInfo();
                    if (language != null && language.trim().equalsIgnoreCase("mermaid")) {
                        // 【mermaid代码块处理逻辑不变】
                        final String chartSource = codeBlock.getLiteral();
                        SpannableString clickableText = new SpannableString("📊 点击查看图表");
                        clickableText.setSpan(new ClickableSpan() {
                            @Override
                            public void onClick(@NonNull View widget) {
                                showMermaidChart(chartSource);
                            }
        
                            @Override // 必须添加@Override注解，确保正确重写
                            public void updateDrawState(@NonNull TextPaint ds) {
                                super.updateDrawState(ds);
                                ds.setUnderlineText(false);
                                ds.setColor(ContextCompat.getColor(MDActivity.this,
                                        MyActivity.isDark ? R.color.light_gray : R.color.dark_gray));
                            }
                        }, 0, clickableText.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        
                        visitor.builder()
                                .append("\n")
                                .append(clickableText)
                                .append("\n");
                    } else {
                        // 【关键修改：移除visitor.visit(codeBlock)，避免递归】
                        // 非mermaid代码块会自动使用Markwon的默认渲染逻辑，无需手动调用visit
                    }
                });
            }
        };
    
        // 初始化Markwon实例（整合所有插件）
        Context appContext = getApplicationContext();
        if (!MyActivity.isDark) {
            markwon = Markwon.builder(appContext)
                    .usePlugin(StrikethroughPlugin.create())
                    .usePlugin(TablePlugin.create(appContext))
                    .usePlugin(TaskListPlugin.create(appContext))
                    .usePlugin(LinkifyPlugin.create())
                    .usePlugin(SimpleExtPlugin.create())
                    .usePlugin(GlideImagesPlugin.create(appContext))
                    .usePlugin(MarkwonInlineParserPlugin.create())
                    .usePlugin(JLatexMathPlugin.create(textSizeInPx, builder -> {
                        builder.inlinesEnabled(true);
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
                    .usePlugin(SyntaxHighlightPlugin.create(prism4j, Prism4jThemeDefault.create()))
                    .usePlugin(HtmlPlugin.create())
                    .usePlugin(mermaidPlugin) // 加入Mermaid处理插件
                    .build();
        } else {
            // 深色模式配置
            markwon = Markwon.builder(appContext)
                    .usePlugin(StrikethroughPlugin.create())
                    .usePlugin(TablePlugin.create(appContext))
                    .usePlugin(TaskListPlugin.create(appContext))
                    .usePlugin(LinkifyPlugin.create())
                    .usePlugin(SimpleExtPlugin.create())
                    .usePlugin(GlideImagesPlugin.create(appContext))
                    .usePlugin(MarkwonInlineParserPlugin.create())
                    .usePlugin(JLatexMathPlugin.create(textSizeInPx, builder -> {
                        builder.inlinesEnabled(true);
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
                    .usePlugin(mermaidPlugin) // 加入Mermaid处理插件
                    .build();
        }
    }
    
    private void showMermaidChart(String chartSource) {
        try {
            // 1. 渲染图表（已确认渲染成功）
            Bitmap chartBitmap = MermaidRenderer.render(chartSource);
            if (chartBitmap == null) {
                Toast.makeText(this, 
                        MyActivity.zh_cn ? "图表渲染为空" : "Chart is empty", 
                        Toast.LENGTH_SHORT).show();
                return;
            }
    
            // 2. 创建ImageView并强制设置布局参数
            ImageView chartImageView = new ImageView(this);
            chartImageView.setImageBitmap(chartBitmap);
            chartImageView.setScaleType(ImageView.ScaleType.FIT_CENTER);
            chartImageView.setAdjustViewBounds(true);
            
            // 手动设置图片最大尺寸（确保不超出屏幕）
            int maxWidth = (int) (getResources().getDisplayMetrics().widthPixels * 0.9f);
            int maxHeight = (int) (getResources().getDisplayMetrics().heightPixels * 0.8f);
            chartImageView.setMaxWidth(maxWidth);
            chartImageView.setMaxHeight(maxHeight);
    
            // 3. 使用最简化的对话框配置
            AlertDialog dialog = new AlertDialog.Builder(this)
                    .setTitle(MyActivity.zh_cn ? "图表预览" : "Chart Preview")
                    .setView(chartImageView)
                    .setNegativeButton(MyActivity.zh_cn ? "关闭" : "Close", 
                            (d, which) -> d.dismiss())
                    .create();
    
            // 4. 直接显示对话框（避免复杂的尺寸计算）
            dialog.show();
    
            // 5. 显示后调整对话框尺寸（确保图片可见）
            Window window = dialog.getWindow();
            if (window != null) {
                // 使用WRAP_CONTENT确保图片完整显示
                window.setLayout(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
                // 设置背景为白色（避免深色模式下图片可能被遮挡）
                window.setBackgroundDrawable(new ColorDrawable(
                        MyActivity.isDark ? Color.BLACK : Color.WHITE));
            }
    
        } catch (Exception e) {
            Log.e("Mermaid", "显示失败", e);
            Toast.makeText(this, 
                    MyActivity.zh_cn ? "图表显示失败" : "Failed to show chart", 
                    Toast.LENGTH_SHORT).show();
        }
    }
        
    
    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnEdit:

                MyActivity.isEdit = true;
                Intent i = new Intent(this, NoteEditor.class);
                // i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                // this.startActivity(i);
                startActivityForResult(i, REQUEST_EDIT);

                break;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_EDIT && resultCode == RESULT_SAVE) {
            // 1. 关闭当前
            finish();

            // 2. 启动新的实例
            MyActivity.openMDWindow();

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

        int position = ((LinearLayoutManager) recyclerView.getLayoutManager())
                .findFirstVisibleItemPosition();
        View view = recyclerView.getChildAt(0);
        int offset = (view != null) ? view.getTop() : 0;

        getSharedPreferences(strmdFileName + "scroll_position", MODE_PRIVATE)
                .edit()
                .putInt("scrollPosition", position)
                .putInt("scrollOffset", offset)
                .apply();

    }

    @Override
    protected void onStart() {
        super.onStart();

    }

    @Override
    public void onStop() {
        System.out.println("NoteEditor onStop...");

        super.onStop();

    }

    @Override
    public void onBackPressed() {
        MyActivity.closeAndroidProgressBar();
        super.onBackPressed();
        // AnimationWhenClosed();

    }

    @Override
    protected void onDestroy() {
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

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);

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
                    MyActivity.getMyAppContext(),
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

    private void setStatusBarColor(String color) {
        // 需要安卓版本大于5.0以上
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            getWindow().setStatusBarColor(Color.parseColor(color));
        }
    }

    private void initRecyclerView() {
        recyclerView = findViewById(R.id.recycler_view);
        recyclerView.setLayoutManager(new LinearLayoutManager(this));
        recyclerView.setItemAnimator(null); // 禁用动画提升性能
        recyclerView.setHasFixedSize(true); // 如果所有item高度固定可设置
    }

    // 滚动位置恢复/保存
    private void restoreScrollPosition() {
        SharedPreferences prefs = getSharedPreferences(strmdFileName + "scroll_position", MODE_PRIVATE);
        recyclerView.post(() -> {
            int position = prefs.getInt("scrollPosition", 0);
            int offset = prefs.getInt("scrollOffset", 0);
            ((LinearLayoutManager) recyclerView.getLayoutManager())
                    .scrollToPositionWithOffset(position, offset);
        });
    }

    // ViewHolder 类
    static class MarkdownViewHolder extends RecyclerView.ViewHolder {
        TextView textView;

        public MarkdownViewHolder(View itemView) {
            super(itemView);
            if (!MyActivity.isDark)
                textView = itemView.findViewById(R.id.markdown_view);
            else
                textView = itemView.findViewById(R.id.markdown_view_dark);
        }
    }

    // 自定义 RecyclerView 适配器
    class MarkdownAdapter extends RecyclerView.Adapter<MarkdownViewHolder> {
        private final List<String> chunks;
        private final float textSize; // 单位：sp

        public MarkdownAdapter(List<String> chunks, float textSize) {
            this.chunks = chunks;
            this.textSize = textSize;
        }

        @Override
        public MarkdownViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View view;
            if (!MyActivity.isDark)
                view = LayoutInflater.from(parent.getContext())
                        .inflate(R.layout.item_markdown, parent, false);
            else
                view = LayoutInflater.from(parent.getContext())
                        .inflate(R.layout.item_markdown_dark, parent, false);

            return new MarkdownViewHolder(view);
        }

        @Override
        public void onBindViewHolder(MarkdownViewHolder holder, int position) {
            // 应用固定字体大小
            holder.textView.setTextSize(textSize);

            // Markwon markwon= Markwon.create(MDActivity.this);

            markwon.setMarkdown(holder.textView, chunks.get(position));
        }

        @Override
        public int getItemCount() {
            return chunks.size();
        }
    }

    private void loadMD() {
        recyclerView = findViewById(R.id.recycler_view);
        recyclerView.setLayoutManager(new LinearLayoutManager(this));

        new Thread(() -> {
            try (BufferedReader reader = new BufferedReader(new FileReader(MyActivity.strMDFile))) {
                List<String> tempChunks = new ArrayList<>();
                StringBuilder chunk = new StringBuilder();
                int lineCount = 0;

                String line;
                while ((line = reader.readLine()) != null) {
                    chunk.append(line.replace("images/", "/storage/emulated/0/KnotData/memo/images/"))
                            .append("\n");
                    if (++lineCount % CHUNK_SIZE == 0) {
                        tempChunks.add(chunk.toString());
                        chunk.setLength(0);
                    }
                }
                if (chunk.length() > 0) {
                    tempChunks.add(chunk.toString());
                }

                runOnUiThread(() -> {
                    float fixedTextSize = 16; // 单位：sp
                    adapter = new MarkdownAdapter(tempChunks, fixedTextSize);
                    recyclerView.setAdapter(adapter);
                    restoreScrollPosition();
                });
            } catch (IOException e) {
                e.printStackTrace();
            }
        }).start();
    }

}