package com.x;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.cardview.widget.CardView;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;

public class NoteBookAdapter
    extends RecyclerView.Adapter<NoteBookAdapter.BookViewHolder>
{

    private ArrayList<String> mRawList = new ArrayList<>();
    private boolean mDarkMode = false;
    public int mSelectedPos = -1;

    public void setData(ArrayList<String> list) {
        mRawList.clear();
        mRawList.addAll(list);
        mSelectedPos = -1;
        notifyDataSetChanged();
    }

    public void setDarkMode(boolean dark) {
        mDarkMode = dark;
        notifyDataSetChanged();
    }

    public void setSelectedPosition(int pos) {
        int old = mSelectedPos;
        mSelectedPos = pos;
        //局部刷新，不要全量notifyDataSetChanged
        notifyItemChanged(old);
        notifyItemChanged(mSelectedPos);
    }

    public int getSelectedPosition() {
        return mSelectedPos;
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

        // 层级缩进
        int dpPerLevel = 24;
        float density = holder.itemView
            .getResources()
            .getDisplayMetrics()
            .density;
        int baseLeftDp = 12;
        int leftPx = (int) ((baseLeftDp + level * dpPerLevel) * density);
        holder.llContent.setPadding(
            leftPx,
            (int) (12 * density),
            (int) (12 * density),
            (int) (12 * density)
        );

        // ========== 选中 + 暗黑配色 ==========
        boolean isSelected = position == mSelectedPos;
        int cardBg;
        int textColor;

        if (mDarkMode) {
            if (isSelected) {
                cardBg = 0xFF404858;
                textColor = 0xFFFFFFFF;
            } else {
                cardBg = 0xFF2C2C2C;
                textColor = 0xFFDDDDDD;
            }
        } else {
            if (isSelected) {
                cardBg = 0xFFDCE7F8;
                textColor = 0xFF000000;
            } else {
                cardBg = 0xFFFFFFFF;
                textColor = 0xFF000000;
            }
        }
        holder.cardView.setCardBackgroundColor(cardBg);
        holder.tvBookName.setTextColor(textColor);

        final int pos = position;
        holder.itemView.setOnClickListener(v -> {
            int old = mSelectedPos;
            mSelectedPos = pos;
            notifyItemChanged(old);
            notifyItemChanged(mSelectedPos);
            NoteActivity.PublicJavaCallCpp("note_book_click|==|" + pos);
        });
    }

    @Override
    public int getItemCount() {
        return mRawList.size();
    }

    public static class BookViewHolder extends RecyclerView.ViewHolder {

        CardView cardView;
        LinearLayout llContent;
        TextView tvBookName;

        public BookViewHolder(@NonNull View itemView) {
            super(itemView);
            cardView = itemView.findViewById(R.id.note_book_card);
            llContent = itemView.findViewById(R.id.note_book_ll_content);
            tvBookName = itemView.findViewById(R.id.note_tv_book_name);
        }
    }
}
