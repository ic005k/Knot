package com.x;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;

public class NoteActivity extends AppCompatActivity {

    private boolean mIsDark = false;
    //顶部按钮
    private ImageView mBtnBookMenu;
    private ImageView mBtnFavorite;
    private ImageView mBtnNoteMenu;
    private ImageView mBtnNewNote;
    //列表
    private RecyclerView mRvBookList;
    private RecyclerView mRvNoteList;
    //底部按钮
    private ImageView mBtnSearch;
    private ImageView mBtnView;
    private ImageView mBtnEdit;
    private ImageView mBtnRecycle;
    private NoteBookAdapter mBookAdapter;
    private NoteEntryAdapter mNoteAdapter;

    public static native void PublicJavaCallCpp(String type);

    private OnBackPressedCallback mBackCallback;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mBackCallback = new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                PublicJavaCallCpp("cancel_add_event_record");
                finish();
            }
        };
        getOnBackPressedDispatcher().addCallback(this, mBackCallback);

        setContentView(R.layout.activity_note);
        mIsDark = ImmersiveUtil.applyRealImmersive(this);
        bindView();
        initRecyclerView();

        // ===== 读取传入的笔记本列表，窗口创建自动加载左侧列表 =====
        ArrayList<String> notebookList = getIntent().getStringArrayListExtra(
            "notebook_list"
        );
        if (notebookList != null) {
            setNoteBookList(notebookList);
        }

        setupClickListeners();
        applyUiTheme();
    }

    private void bindView() {
        mBtnBookMenu = findViewById(R.id.note_btn_book_menu);
        mBtnFavorite = findViewById(R.id.note_btn_favorite);
        mBtnNoteMenu = findViewById(R.id.note_btn_note_menu);
        mBtnNewNote = findViewById(R.id.note_btn_new_note);
        mRvBookList = findViewById(R.id.note_rv_book_list);
        mRvNoteList = findViewById(R.id.note_rv_note_list);
        mBtnSearch = findViewById(R.id.note_btn_search);
        mBtnView = findViewById(R.id.note_btn_view);
        mBtnEdit = findViewById(R.id.note_btn_edit);
        mBtnRecycle = findViewById(R.id.note_btn_recycle);
    }

    private void initRecyclerView() {
        mBookAdapter = new NoteBookAdapter();
        mRvBookList.setLayoutManager(new LinearLayoutManager(this));
        mRvBookList.setAdapter(mBookAdapter);
        mNoteAdapter = new NoteEntryAdapter();
        mRvNoteList.setLayoutManager(new LinearLayoutManager(this));
        mRvNoteList.setAdapter(mNoteAdapter);
    }

    private void setupClickListeners() {
        mBtnBookMenu.setOnClickListener(v ->
            PublicJavaCallCpp("note_book_menu")
        );
        mBtnFavorite.setOnClickListener(v ->
            PublicJavaCallCpp("note_favorite")
        );
        mBtnNoteMenu.setOnClickListener(v -> PublicJavaCallCpp("note_menu"));
        mBtnNewNote.setOnClickListener(v ->
            PublicJavaCallCpp("note_create_new")
        );
        mBtnSearch.setOnClickListener(v -> PublicJavaCallCpp("note_search"));
        mBtnView.setOnClickListener(v -> PublicJavaCallCpp("note_view"));
        mBtnEdit.setOnClickListener(v -> PublicJavaCallCpp("note_edit"));
        mBtnRecycle.setOnClickListener(v ->
            PublicJavaCallCpp("note_open_recycle")
        );
    }

    /**
     * 应用明暗主题，和MyEventActivity配色完全对齐
     */
    public void applyUiTheme() {
        int rootBg;
        int dividerColor;
        int iconTint;
        if (mIsDark) {
            rootBg = 0xFF1E1E1E;
            dividerColor = 0xFF444444;
            iconTint = 0xFFFFFFFF;
        } else {
            rootBg = 0xFFFFFFFF;
            dividerColor = 0xFFCCCCCC;
            iconTint = 0xFF000000;
        }
        findViewById(R.id.note_layout_root).setBackgroundColor(rootBg);
        findViewById(R.id.note_view_divider_center).setBackgroundColor(
            dividerColor
        );
        //统一给图标着色
        mBtnBookMenu.setColorFilter(iconTint);
        mBtnFavorite.setColorFilter(iconTint);
        mBtnNoteMenu.setColorFilter(iconTint);
        mBtnNewNote.setColorFilter(iconTint);
        mBtnSearch.setColorFilter(iconTint);
        mBtnView.setColorFilter(iconTint);
        mBtnEdit.setColorFilter(iconTint);
        mBtnRecycle.setColorFilter(iconTint);
        mBookAdapter.setDarkMode(mIsDark);
        mNoteAdapter.setDarkMode(mIsDark);
    }

    public void setDark(boolean dark) {
        mIsDark = dark;
        applyUiTheme();
    }

    //对外接口，供JNI推送数据
    public void setNoteBookList(ArrayList<String> bookList) {
        mBookAdapter.setData(bookList);
    }

    public void setNoteEntryList(ArrayList<String> noteList) {
        mNoteAdapter.setData(noteList);
    }
}
