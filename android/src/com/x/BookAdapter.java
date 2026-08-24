package com.x;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.util.List;

public class BookAdapter extends RecyclerView.Adapter<BookAdapter.ViewHolder> {

    private final List<Book> mBookList;
    // 单选：-1代表无选中
    private int mSelectedPos = -1;

    public BookAdapter(List<Book> list) {
        mBookList = list;
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
        // 设置selected状态，驱动selector背景变色
        holder.itemView.setSelected(mSelectedPos == position);

        holder.itemView.setOnClickListener(v -> {
            int old = mSelectedPos;
            mSelectedPos = holder.getAdapterPosition();
            // 刷新旧位置和新位置
            if (old != -1) notifyItemChanged(old);
            notifyItemChanged(mSelectedPos);
        });
    }

    @Override
    public int getItemCount() {
        return mBookList.size();
    }

    public Book getSelectedItem() {
        if (mSelectedPos < 0 || mSelectedPos >= mBookList.size()) {
            return null;
        }
        return mBookList.get(mSelectedPos);
    }

    public void clearAllSelect() {
        int old = mSelectedPos;
        mSelectedPos = -1;
        if (old != -1) {
            notifyItemChanged(old);
        }
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {

        TextView tvName;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            tvName = itemView.findViewById(R.id.tvBookName);
        }
    }

    public int getSelectedPosition() {
        return mSelectedPos;
    }
}
