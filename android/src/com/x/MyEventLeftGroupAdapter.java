package com.x;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;

public class MyEventLeftGroupAdapter
    extends RecyclerView.Adapter<MyEventLeftGroupAdapter.ViewHolder>
{

    private ArrayList<MyEventDateGroup> mDataList = new ArrayList<>();
    private OnMyEventDateGroupClickListener mClickListener;
    private boolean mIsDark = false;

    public interface OnMyEventDateGroupClickListener {
        void onDateGroupItemClick(int position, MyEventDateGroup data);
    }

    public void setClickListener(OnMyEventDateGroupClickListener listener) {
        mClickListener = listener;
    }

    public void setDarkMode(boolean dark) {
        mIsDark = dark;
        notifyDataSetChanged();
    }

    public void setData(ArrayList<MyEventDateGroup> list) {
        mDataList.clear();
        mDataList.addAll(list);
        notifyDataSetChanged();
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(
        @NonNull ViewGroup parent,
        int viewType
    ) {
        View itemView = LayoutInflater.from(parent.getContext()).inflate(
            R.layout.item_myevent_date_group,
            parent,
            false
        );
        return new ViewHolder(itemView);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        MyEventDateGroup item = mDataList.get(position);
        holder.tvDate.setText(item.dateStr);
        holder.tvCount.setText(String.valueOf(item.dayItemCount));
        holder.tvSum.setText(String.valueOf(item.dayTotalValue));

        // 明暗模式：背景、文字颜色
        if (mIsDark) {
            holder.itemView.setBackgroundColor(0xFF1E1E1E);
            holder.tvDate.setTextColor(0xFFFFFFFF);
            holder.tvCount.setTextColor(0xFFFFFFFF);
            holder.tvSum.setTextColor(0xFFFFFFFF);
        } else {
            holder.itemView.setBackgroundColor(0xFFFFFFFF);
            holder.tvDate.setTextColor(0xFF000000);
            holder.tvCount.setTextColor(0xFF000000);
            holder.tvSum.setTextColor(0xFF000000);
        }

        holder.itemView.setOnClickListener(v -> {
            if (mClickListener != null) {
                mClickListener.onDateGroupItemClick(position, item);
            }
        });
    }

    @Override
    public int getItemCount() {
        return mDataList.size();
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {

        TextView tvDate;
        TextView tvCount;
        TextView tvSum;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            tvDate = itemView.findViewById(R.id.myevent_tv_date_text);
            tvCount = itemView.findViewById(R.id.myevent_tv_day_item_count);
            tvSum = itemView.findViewById(R.id.myevent_tv_day_sum_value);
        }
    }
}
