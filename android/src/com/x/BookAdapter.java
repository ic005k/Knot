package com.x;

import android.graphics.drawable.ColorDrawable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.util.List;

public class BookAdapter extends RecyclerView.Adapter<BookAdapter.ViewHolder> {

    private final List<Book> mBookList;
    private int mSelectedPos = -1;
    private boolean isDark; // ✅ 可变的暗黑状态

    // ✅ 构造函数接收 isDark
    public BookAdapter(List<Book> list, boolean isDark) {
        mBookList = list;
        this.isDark = isDark;
    }

    // ✅ 新增：外部通知暗黑模式切换，实时刷新所有条目
    public void updateDarkMode(boolean newIsDark) {
        if (this.isDark != newIsDark) {
            this.isDark = newIsDark;
            notifyDataSetChanged();
        }
    }

    // 根据扩展名获取标记颜色 ARGB（与暗黑模式无关，保持不变）
    private int getTagColor(String ext) {
        if (ext == null) return 0xFF444444;
        switch (ext) {
            case "pdf":
                return 0xFFE53935;
            case "mobi":
                return 0xFFFF8800;
            case "epub":
                return 0xFF27A844;
            case "txt":
                return 0xFF777777;
            default:
                return 0xFF444444;
        }
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(
        @NonNull ViewGroup parent,
        int viewType
    ) {
        View v = LayoutInflater.from(parent.getContext()).inflate(
            R.layout.item_book,
            parent,
            false
        );
        return new ViewHolder(v);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        Book book = mBookList.get(position);
        holder.tvName.setText(book.getTitle());

        // ✅ 左侧类型色块颜色（与暗黑无关，保持原逻辑）
        holder.vFileTypeTag.setBackgroundColor(getTagColor(book.getExt()));

        // ========== ✅ 动态暗黑模式适配 ==========
        boolean selected = mSelectedPos == position;

        // 条目背景色
        int itemBgColor;
        if (selected) {
            itemBgColor = isDark ? 0xFF3A3A3A : 0xFFE3F2FD; // 选中态
        } else {
            itemBgColor = isDark ? 0xFF2A2A2A : 0xFFFFFFFF; // 默认态
        }
        holder.itemView.setBackground(new ColorDrawable(itemBgColor));

        // 文字颜色
        holder.tvName.setTextColor(isDark ? 0xFFE0E0E0 : 0xFF333333);
        // ==========================================

        // 点击事件保持不变
        holder.itemView.setOnClickListener(v -> {
            int old = mSelectedPos;
            mSelectedPos = holder.getAdapterPosition();
            if (old != -1) notifyItemChanged(old);
            notifyItemChanged(mSelectedPos);
        });
    }

    @Override
    public int getItemCount() {
        return mBookList.size();
    }

    public Book getSelectedItem() {
        if (mSelectedPos < 0 || mSelectedPos >= mBookList.size()) return null;
        return mBookList.get(mSelectedPos);
    }

    public void clearAllSelect() {
        int old = mSelectedPos;
        mSelectedPos = -1;
        if (old != -1) notifyItemChanged(old);
    }

    public int getSelectedPosition() {
        return mSelectedPos;
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {

        TextView tvName;
        View vFileTypeTag;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            tvName = itemView.findViewById(R.id.tvBookName);
            vFileTypeTag = itemView.findViewById(R.id.v_file_type_tag);
        }
    }
}
