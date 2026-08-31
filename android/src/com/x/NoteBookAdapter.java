package com.x;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;

public class NoteBookAdapter
    extends RecyclerView.Adapter<NoteBookAdapter.BookViewHolder>
{

    private ArrayList<String> mRawList = new ArrayList<>();
    private boolean mDarkMode = false;

    public void setData(ArrayList<String> list) {
        mRawList.clear();
        mRawList.addAll(list);
        notifyDataSetChanged();
    }

    public void setDarkMode(boolean dark) {
        mDarkMode = dark;
        notifyDataSetChanged();
    }

    @NonNull
    @Override
    public BookViewHolder onCreateViewHolder(
        @NonNull ViewGroup parent,
        int viewType
    ) {
        View v = LayoutInflater.from(parent.getContext()).inflate(
            R.layout.item_note_book,
            parent,
            false
        );
        return new BookViewHolder(v);
    }

    @Override
    public void onBindViewHolder(@NonNull BookViewHolder holder, int position) {
        String itemStr = mRawList.get(position);
        String[] parts = itemStr.split("\\|==\\|");

        String bookName;
        int level = 0;
        if (parts.length >= 2) {
            bookName = parts[0];
            try {
                level = Integer.parseInt(parts[1]);
            } catch (NumberFormatException e) {
                level = 0;
            }
        } else {
            bookName = itemStr;
            level = 0;
        }

        holder.tvBookName.setText(bookName);

        // 每一级24dp缩进，叠加基础左边12dp
        int dpPerLevel = 24;
        float density = holder.itemView
            .getResources()
            .getDisplayMetrics()
            .density;
        int baseLeftDp = 12;
        int leftPx = (int) ((baseLeftDp + level * dpPerLevel) * density);

        // 修改内部LinearLayout的left padding，top/bottom/right固定不变
        holder.llContent.setPadding(
            leftPx,
            (int) (12 * density),
            (int) (12 * density),
            (int) (12 * density)
        );

        // 文字颜色
        if (mDarkMode) {
            holder.tvBookName.setTextColor(0xFFFFFFFF);
        } else {
            holder.tvBookName.setTextColor(0xFF000000);
        }

        final int pos = position;
        holder.itemView.setOnClickListener(v -> {
            NoteActivity.PublicJavaCallCpp("note_book_click|==|" + pos);
        });
    }

    @Override
    public int getItemCount() {
        return mRawList.size();
    }

    public static class BookViewHolder extends RecyclerView.ViewHolder {

        LinearLayout llContent;
        TextView tvBookName;

        public BookViewHolder(@NonNull View itemView) {
            super(itemView);
            llContent = itemView.findViewById(R.id.note_book_ll_content);
            tvBookName = itemView.findViewById(R.id.note_tv_book_name);
        }
    }
}
