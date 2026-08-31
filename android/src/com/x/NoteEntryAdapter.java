package com.x;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.NonNull;
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
        if (mIsDark) {
            holder.itemView.setBackgroundColor(sel ? 0xFF3A3A3A : 0xFF1E1E1E);
            holder.tvNoteTitle.setTextColor(0xFFFFFFFF);
        } else {
            holder.itemView.setBackgroundColor(sel ? 0xFFE7F1FF : 0xFFFFFFFF);
            holder.tvNoteTitle.setTextColor(0xFF000000);
        }

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

        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            tvNoteTitle = itemView.findViewById(R.id.note_tv_note_title);
        }
    }
}
