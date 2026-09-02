package com.x;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.cardview.widget.CardView;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;

public class NoteEntryAdapter
    extends RecyclerView.Adapter<NoteEntryAdapter.ViewHolder>
{

    private ArrayList<String> mData = new ArrayList<>();
    private boolean mIsDark;
    private int mSelectedPos = -1;

    public interface OnNoteItemClickListener {
        void onNoteClick(int pos, String title);
    }

    private OnNoteItemClickListener mListener;

    public void setListener(OnNoteItemClickListener l) {
        mListener = l;
    }

    public void setDarkMode(boolean dark) {
        mIsDark = dark;
        notifyDataSetChanged();
    }

    public void setData(ArrayList<String> list) {
        mData.clear();
        mData.addAll(list);
        mSelectedPos = -1;
        notifyDataSetChanged();
    }

    // JNI / Activity 外部调用设置选中
    public void setSelectedPosition(int pos) {
        int old = mSelectedPos;
        mSelectedPos = pos;
        notifyItemChanged(old);
        notifyItemChanged(mSelectedPos);
    }

    // ====获取当前选中笔记索引，-1代表没有选中=====
    public int getSelectedPosition() {
        return mSelectedPos;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(
        @NonNull ViewGroup parent,
        int viewType
    ) {
        View v = LayoutInflater.from(parent.getContext()).inflate(
            R.layout.item_note_entry,
            parent,
            false
        );
        return new ViewHolder(v);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        String title = mData.get(position);
        if (title == null || title.trim().isEmpty()) {
            holder.tvNoteTitle.setVisibility(View.GONE);
        } else {
            holder.tvNoteTitle.setVisibility(View.VISIBLE);
            holder.tvNoteTitle.setText(title);
        }

        boolean sel = position == mSelectedPos;
        int bgColor;
        int textColor;

        if (mIsDark) {
            if (sel) {
                bgColor = 0xFF3A3A3A;
                textColor = 0xFFFFFFFF;
            } else {
                bgColor = 0xFF1E1E1E;
                textColor = 0xFFBBBBBB;
            }
        } else {
            if (sel) {
                bgColor = 0xFFE7F1FF;
                textColor = 0xFF000000;
            } else {
                bgColor = 0xFFFFFFFF;
                textColor = 0xFF333333;
            }
        }

        // 使用CardView设置背景，保留圆角，禁止直接操作itemView背景
        holder.cardView.setCardBackgroundColor(bgColor);
        holder.tvNoteTitle.setTextColor(textColor);

        int pos = position;
        holder.itemView.setOnClickListener(v -> {
            int old = mSelectedPos;
            mSelectedPos = pos;
            notifyItemChanged(old);
            notifyItemChanged(mSelectedPos);
            if (mListener != null) mListener.onNoteClick(pos, mData.get(pos));
        });
    }

    @Override
    public int getItemCount() {
        return mData.size();
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {

        TextView tvNoteTitle;
        CardView cardView;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            cardView = itemView.findViewById(R.id.note_entry_card);
            tvNoteTitle = itemView.findViewById(R.id.note_tv_note_title);
        }
    }
}
