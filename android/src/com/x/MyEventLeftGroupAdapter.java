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
    // -1 代表无选中
    private int mSelectedPosition = -1;

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
        // 更新数据源清空选中状态
        mSelectedPosition = -1;
        notifyDataSetChanged();
    }

    /**
     * 外部可调用：主动设置选中项
     * @param index 下标，传‑1取消选中
     */
    public void setSelectedPosition(int index) {
        int old = mSelectedPosition;
        mSelectedPosition = index;
        notifyItemChanged(old);
        notifyItemChanged(mSelectedPosition);
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

        // dateStr 判空隐藏
        if (item.dateStr == null || item.dateStr.trim().isEmpty()) {
            holder.tvDate.setVisibility(View.GONE);
        } else {
            holder.tvDate.setVisibility(View.VISIBLE);
            holder.tvDate.setText(item.dateStr);
        }

        // dayItemCount 字符串判空隐藏
        if (item.dayItemCount == null || item.dayItemCount.trim().isEmpty()) {
            holder.tvCount.setVisibility(View.GONE);
        } else {
            holder.tvCount.setVisibility(View.VISIBLE);
            holder.tvCount.setText(item.dayItemCount);
        }

        // dayTotalValue 字符串判空隐藏
        if (item.dayTotalValue == null || item.dayTotalValue.trim().isEmpty()) {
            holder.tvSum.setVisibility(View.GONE);
        } else {
            holder.tvSum.setVisibility(View.VISIBLE);
            holder.tvSum.setText(item.dayTotalValue);
        }

        boolean isSelected = position == mSelectedPosition;
        // 明暗+选中状态组合背景
        if (mIsDark) {
            if (isSelected) {
                holder.itemView.setBackgroundColor(0xFF3A3A3A);
            } else {
                holder.itemView.setBackgroundColor(0xFF1E1E1E);
            }
            holder.tvDate.setTextColor(0xFFFFFFFF);
            holder.tvCount.setTextColor(0xFFFFFFFF);
            holder.tvSum.setTextColor(0xFFFFFFFF);
        } else {
            if (isSelected) {
                holder.itemView.setBackgroundColor(0xFFE7F1FF);
            } else {
                holder.itemView.setBackgroundColor(0xFFFFFFFF);
            }
            holder.tvDate.setTextColor(0xFF000000);
            holder.tvCount.setTextColor(0xFF000000);
            holder.tvSum.setTextColor(0xFF000000);
        }

        final int pos = position;
        final MyEventDateGroup dataItem = item;
        holder.itemView.setOnClickListener(v -> {
            int oldSel = mSelectedPosition;
            mSelectedPosition = pos;
            // 局部刷新旧、新位置，避免全量刷新
            notifyItemChanged(oldSel);
            notifyItemChanged(mSelectedPosition);
            if (mClickListener != null) {
                mClickListener.onDateGroupItemClick(pos, dataItem);
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

    public int getSelectedPosition() {
        return mSelectedPosition;
    }
}
