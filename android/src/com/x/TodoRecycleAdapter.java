package com.x;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;

public class TodoRecycleAdapter
    extends RecyclerView.Adapter<TodoRecycleAdapter.RecycleViewHolder>
{

    private ArrayList<String> mDataList = new ArrayList<>();
    private boolean mIsDark = false;
    public int mSelectedPos = -1;

    public void setData(ArrayList<String> list) {
        mDataList.clear();
        mDataList.addAll(list);
        mSelectedPos = -1;
        notifyDataSetChanged();
    }

    public void setDarkMode(boolean dark) {
        mIsDark = dark;
        notifyDataSetChanged();
    }

    public void clearSelection() {
        mSelectedPos = -1;
        notifyDataSetChanged();
    }

    @NonNull
    @Override
    public RecycleViewHolder onCreateViewHolder(
        @NonNull ViewGroup parent,
        int viewType
    ) {
        View item = LayoutInflater.from(parent.getContext()).inflate(
            R.layout.item_todo_recycle_card,
            parent,
            false
        );
        return new RecycleViewHolder(item);
    }

    @Override
    public void onBindViewHolder(
        @NonNull RecycleViewHolder holder,
        int position
    ) {
        String line = mDataList.get(position);
        String[] parts = line.split("\\|==\\|");

        String timeStr = "";
        String textStr = "";
        if (parts.length >= 2) {
            timeStr = parts[0];
            textStr = parts[1];
        }

        holder.tvTime.setText(timeStr);
        holder.tvText.setText(textStr);
        holder.viewStripe.setBackgroundColor(0xFF888888);

        boolean selected = position == mSelectedPos;
        if (mIsDark) {
            if (selected) {
                holder.itemView.setBackgroundColor(0xFF3A3A3A);
            } else {
                holder.itemView.setBackgroundColor(0xFF282828);
            }
            holder.tvTime.setTextColor(0xFFFFFFFF);
            holder.tvText.setTextColor(0xFFEFEFEF);
        } else {
            if (selected) {
                holder.itemView.setBackgroundColor(0xFFE8F0FE);
            } else {
                holder.itemView.setBackgroundColor(0xFFFFFFFF);
            }
            holder.tvTime.setTextColor(0xFF000000);
            holder.tvText.setTextColor(0xFF222222);
        }

        final int pos = position;
        holder.itemView.setOnClickListener(v -> {
            if (mSelectedPos == pos) {
                mSelectedPos = -1;
            } else {
                mSelectedPos = pos;
            }
            notifyDataSetChanged();
        });
    }

    @Override
    public int getItemCount() {
        return mDataList.size();
    }

    public static class RecycleViewHolder extends RecyclerView.ViewHolder {

        View viewStripe;
        TextView tvTime;
        TextView tvText;

        public RecycleViewHolder(@NonNull View itemView) {
            super(itemView);
            viewStripe = itemView.findViewById(R.id.viewStripe);
            tvTime = itemView.findViewById(R.id.tv_time);
            tvText = itemView.findViewById(R.id.tv_text);
        }
    }
}
