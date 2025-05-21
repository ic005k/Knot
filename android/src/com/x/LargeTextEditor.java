package com.x;

import android.content.Context;
import android.os.AsyncTask;
import android.text.Editable;
import android.text.SpannableStringBuilder;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.TextView;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

public class LargeTextEditor {
    private static final String TAG = "LargeTextEditor";
    private static final int CHUNK_SIZE = 1024 * 1024; // 1MB chunks

    private Context context;
    private File file;
    private int cursorPosition = 0;

    // 两种模式：EditText(旧) 或 RecyclerView(新)
    private EditText editText;
    private RecyclerView recyclerView;
    private boolean useRecyclerView = true; // 默认使用RecyclerView

    // 行数据和适配器
    private List<String> lines = new ArrayList<>();
    private TextLineAdapter lineAdapter;
    private Map<Integer, Integer> lineHeightCache = new HashMap<>(); // 缓存行高

    // Undo/Redo管理
    private Stack<TextChange> undoStack = new Stack<>();
    private Stack<TextChange> redoStack = new Stack<>();

    public LargeTextEditor(Context context, EditText editText) {
        this.context = context;
        this.editText = editText;
        setupTextWatcher();
    }

    // 重载构造函数，支持RecyclerView
    public LargeTextEditor(Context context, RecyclerView recyclerView) {
        this.context = context;
        this.recyclerView = recyclerView;
        this.useRecyclerView = true;
        initRecyclerView();
    }

    private void initRecyclerView() {
        lineAdapter = new TextLineAdapter(lines);
        recyclerView.setLayoutManager(new LinearLayoutManager(context));
        recyclerView.setAdapter(lineAdapter);
    }

    // 异步加载文件
    public void loadFile(File file) {
        this.file = file;
        if (useRecyclerView) {
            new LoadFileByLinesTask().execute(file);
        } else {
            new LoadFileTask().execute(file);
        }
    }

    // 异步保存文件
    public void saveFile() {
        if (useRecyclerView) {
            new SaveFileByLinesTask().execute();
        } else {
            new SaveFileTask().execute();
        }
    }

    // 保存光标位置
    public void saveCursorPosition() {
        if (useRecyclerView) {
            // 计算当前可见行和偏移
            LinearLayoutManager layoutManager = (LinearLayoutManager) recyclerView.getLayoutManager();
            int firstVisibleItemPosition = layoutManager.findFirstVisibleItemPosition();
            View firstVisibleView = layoutManager.findViewByPosition(firstVisibleItemPosition);
            if (firstVisibleView != null) {
                cursorPosition = firstVisibleItemPosition;
                // 记录更精确的光标位置...
            }
        } else {
            cursorPosition = editText.getSelectionStart();
        }
    }

    // 恢复光标位置
    public void restoreCursorPosition() {
        if (useRecyclerView) {
            recyclerView.scrollToPosition(cursorPosition);
        } else {
            if (editText.getText().length() >= cursorPosition) {
                editText.setSelection(cursorPosition);
            }
        }
    }

    // 查找功能
    public int findNext(String searchText, int startPos) {
        if (useRecyclerView) {
            // 在lines中查找
            for (int i = 0; i < lines.size(); i++) {
                String line = lines.get(i);
                int index = line.indexOf(searchText);
                if (index >= 0) {
                    recyclerView.scrollToPosition(i);
                    // 高亮逻辑...
                    return i;
                }
            }
            return -1;
        } else {
            Editable text = editText.getText();
            int index = text.toString().indexOf(searchText, startPos);
            if (index >= 0) {
                editText.setSelection(index, index + searchText.length());
            }
            return index;
        }
    }

    // 替换功能
    public void replace(String searchText, String replaceText) {
        if (useRecyclerView) {
            // 获取当前选中行
            LinearLayoutManager layoutManager = (LinearLayoutManager) recyclerView.getLayoutManager();
            int currentPosition = layoutManager.findFirstVisibleItemPosition();
            if (currentPosition >= 0 && currentPosition < lines.size()) {
                String line = lines.get(currentPosition);
                String newLine = line.replace(searchText, replaceText);
                lines.set(currentPosition, newLine);
                lineAdapter.notifyItemChanged(currentPosition);
            }
        } else {
            int selectionStart = editText.getSelectionStart();
            int selectionEnd = editText.getSelectionEnd();
            Editable text = editText.getText();

            if (selectionStart >= 0 && selectionEnd > selectionStart) {
                String selectedText = text.subSequence(selectionStart, selectionEnd).toString();
                if (selectedText.equals(searchText)) {
                    text.replace(selectionStart, selectionEnd, replaceText);
                }
            }
        }
    }

    // 撤销操作
    public void undo() {
        if (!undoStack.isEmpty()) {
            TextChange change = undoStack.pop();
            redoStack.push(change);

            if (useRecyclerView) {
                // 处理RecyclerView的撤销逻辑
                int lineIndex = getLineIndexForOffset(change.start);
                if (lineIndex >= 0 && lineIndex < lines.size()) {
                    String line = lines.get(lineIndex);
                    // 应用撤销...
                    lineAdapter.notifyItemChanged(lineIndex);
                }
            } else {
                Editable text = editText.getText();
                text.replace(change.start, change.end, change.before);
            }
        }
    }

    // 重做操作
    public void redo() {
        if (!redoStack.isEmpty()) {
            TextChange change = redoStack.pop();
            undoStack.push(change);

            if (useRecyclerView) {
                // 处理RecyclerView的重做逻辑
                int lineIndex = getLineIndexForOffset(change.start);
                if (lineIndex >= 0 && lineIndex < lines.size()) {
                    String line = lines.get(lineIndex);
                    // 应用重做...
                    lineAdapter.notifyItemChanged(lineIndex);
                }
            } else {
                Editable text = editText.getText();
                text.replace(change.start, change.start + change.before.length(), change.after);
            }
        }
    }

    // 监听文本变化用于Undo/Redo
    private void setupTextWatcher() {
        if (!useRecyclerView && editText != null) {
            editText.addTextChangedListener(new TextWatcher() {
                private int start;
                private int count;
                private CharSequence before;

                @Override
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                    this.start = start;
                    this.count = count;
                    this.before = s.subSequence(start, start + count);
                }

                @Override
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                    // 清空redo栈
                    if (count > 0 || before > 0) {
                        redoStack.clear();
                        CharSequence after = s.subSequence(start, start + count);
                        undoStack.push(new TextChange(start, this.start + this.count, this.before, after));
                    }
                }

                @Override
                public void afterTextChanged(Editable s) {
                }
            });
        }
    }

    // 文本变化记录类
    private static class TextChange {
        int start;
        int end;
        CharSequence before;
        CharSequence after;

        TextChange(int start, int end, CharSequence before, CharSequence after) {
            this.start = start;
            this.end = end;
            this.before = before;
            this.after = after;
        }
    }

    // 根据偏移量获取行号
    private int getLineIndexForOffset(int offset) {
        int currentOffset = 0;
        for (int i = 0; i < lines.size(); i++) {
            currentOffset += lines.get(i).length() + 1; // +1 为换行符
            if (currentOffset > offset) {
                return i;
            }
        }
        return lines.size() - 1;
    }

    // 异步按行加载文件任务
    private class LoadFileByLinesTask extends AsyncTask<File, Integer, Boolean> {
        @Override
        protected Boolean doInBackground(File... files) {
            lines.clear();
            try (BufferedReader reader = new BufferedReader(new FileReader(files[0]))) {
                String line;
                int lineCount = 0;
                while ((line = reader.readLine()) != null) {
                    lines.add(line);
                    lineCount++;
                    if (lineCount % 100 == 0) {
                        publishProgress(lineCount);
                    }
                }
                return true;
            } catch (IOException e) {
                Log.e(TAG, "Error loading file: " + e.getMessage());
                return false;
            }
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            // 更新进度
            Log.d(TAG, "Loaded " + values[0] + " lines");
        }

        @Override
        protected void onPostExecute(Boolean success) {
            if (success) {
                lineAdapter.notifyDataSetChanged();
                restoreCursorPosition();
            }
        }
    }

    // 异步按行保存文件任务
    private class SaveFileByLinesTask extends AsyncTask<Void, Void, Boolean> {
        @Override
        protected Boolean doInBackground(Void... voids) {
            try (BufferedWriter writer = new BufferedWriter(new FileWriter(file))) {
                for (String line : lines) {
                    writer.write(line);
                    writer.newLine();
                }
                return true;
            } catch (IOException e) {
                Log.e(TAG, "Error saving file: " + e.getMessage());
                return false;
            }
        }

        @Override
        protected void onPostExecute(Boolean success) {
            // 处理保存结果
        }
    }

    // 异步加载文件任务（旧版，保留用于兼容性）
    private class LoadFileTask extends AsyncTask<File, Void, SpannableStringBuilder> {
        @Override
        protected SpannableStringBuilder doInBackground(File... files) {
            SpannableStringBuilder builder = new SpannableStringBuilder();
            try (BufferedReader reader = new BufferedReader(new FileReader(files[0]))) {
                char[] buffer = new char[CHUNK_SIZE];
                int charsRead;
                while ((charsRead = reader.read(buffer)) != -1) {
                    builder.append(new String(buffer, 0, charsRead));
                }
            } catch (IOException e) {
                Log.e(TAG, "Error loading file: " + e.getMessage());
            }
            return builder;
        }

        @Override
        protected void onPostExecute(SpannableStringBuilder result) {
            editText.setText(result);
            restoreCursorPosition();
        }
    }

    // 异步保存文件任务（旧版，保留用于兼容性）
    private class SaveFileTask extends AsyncTask<Void, Void, Boolean> {
        @Override
        protected Boolean doInBackground(Void... voids) {
            try (BufferedWriter writer = new BufferedWriter(new FileWriter(file))) {
                writer.write(editText.getText().toString());
                return true;
            } catch (IOException e) {
                Log.e(TAG, "Error saving file: " + e.getMessage());
                return false;
            }
        }

        @Override
        protected void onPostExecute(Boolean success) {
            // 处理保存结果
        }
    }

    // RecyclerView适配器
    private class TextLineAdapter extends RecyclerView.Adapter<TextLineAdapter.LineViewHolder> {
        private List<String> lines;

        public TextLineAdapter(List<String> lines) {
            this.lines = lines;
        }

        @Override
        public LineViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View view = LayoutInflater.from(parent.getContext())
                    .inflate(android.R.layout.simple_list_item_1, parent, false);
            return new LineViewHolder(view);
        }

        @Override
        public void onBindViewHolder(LineViewHolder holder, int position) {
            String line = lines.get(position);
            holder.textView.setText(line);

            // 缓存行高以优化滚动性能
            holder.textView.post(() -> {
                if (!lineHeightCache.containsKey(position)) {
                    lineHeightCache.put(position, holder.textView.getHeight());
                }
            });
        }

        @Override
        public int getItemCount() {
            return lines.size();
        }

        public class LineViewHolder extends RecyclerView.ViewHolder {
            TextView textView;

            public LineViewHolder(View itemView) {
                super(itemView);
                textView = itemView.findViewById(android.R.id.text1);
            }
        }
    }
}