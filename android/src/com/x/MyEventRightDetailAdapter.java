package com.x;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;

public class MyEventRightDetailAdapter
    extends RecyclerView.Adapter<MyEventRightDetailAdapter.ViewHolder>
{

    private ArrayList<MyEventDetailItem> mDetailList = new ArrayList<>();
    private boolean mIsDark = false;
    // -1 代表无选中
    private int mSelectedPosition = -1;
    private OnDetailItemClickListener mClickListener;

    public interface OnDetailItemClickListener {
        void onDetailItemClick(int position, MyEventDetailItem item);
    }

    public void setClickListener(OnDetailItemClickListener listener) {
        mClickListener = listener;
    }

    public void setDarkMode(boolean dark) {
        mIsDark = dark;
        notifyDataSetChanged();
    }

    public void setDetailData(ArrayList<MyEventDetailItem> list) {
        mDetailList.clear();
        mDetailList.addAll(list);
        // 切换数据源清空选中
        mSelectedPosition = -1;
        notifyDataSetChanged();
    }

    /**
     * 外部主动设置选中下标，-1取消选中
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
            R.layout.item_myevent_detail,
            parent,
            false
        );
        return new ViewHolder(itemView);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        MyEventDetailItem item = mDetailList.get(position);

        // tvTime 日期 📅
        if (item.timeStr == null || item.timeStr.trim().isEmpty()) {
            holder.tvTime.setVisibility(View.GONE);
        } else {
            holder.tvTime.setVisibility(View.VISIBLE);
            holder.tvTime.setText(item.timeStr);
        }

        // tvValue 金额 💰
        if (item.eventValue == null || item.eventValue.trim().isEmpty()) {
            holder.tvValue.setVisibility(View.GONE);
        } else {
            holder.tvValue.setVisibility(View.VISIBLE);
            holder.tvValue.setText("💰 " + item.eventValue);
        }

        // tvCategory 类型 🛒
        if (item.category == null || item.category.trim().isEmpty()) {
            holder.tvCategory.setVisibility(View.GONE);
        } else {
            holder.tvCategory.setVisibility(View.VISIBLE);
            holder.tvCategory.setText("🛒 " + item.category);
        }

        // tvNote 备注 📝
        if (item.note == null || item.note.trim().isEmpty()) {
            holder.tvNote.setVisibility(View.GONE);
        } else {
            holder.tvNote.setVisibility(View.VISIBLE);
            holder.tvNote.setText("📝 " + item.note);
        }

        boolean isSelected = position == mSelectedPosition;
        if (mIsDark) {
            if (isSelected) {
                holder.itemView.setBackgroundColor(0xFF3A3A3A);
            } else {
                holder.itemView.setBackgroundColor(0xFF1E1E1E);
            }
            holder.tvTime.setTextColor(0xFFFFFFFF);
            holder.tvValue.setTextColor(0xFFFFFFFF);
            holder.tvCategory.setTextColor(0xFFFFFFFF);
            holder.tvNote.setTextColor(0xFFFFFFFF);
        } else {
            if (isSelected) {
                holder.itemView.setBackgroundColor(0xFFE7F1FF);
            } else {
                holder.itemView.setBackgroundColor(0xFFFFFFFF);
            }
            holder.tvTime.setTextColor(0xFF000000);
            holder.tvValue.setTextColor(0xFF000000);
            holder.tvCategory.setTextColor(0xFF000000);
            holder.tvNote.setTextColor(0xFF000000);
        }

        final int pos = position;
        final MyEventDetailItem dataItem = item;
        holder.itemView.setOnClickListener(v -> {
            int oldSel = mSelectedPosition;
            mSelectedPosition = pos;
            notifyItemChanged(oldSel);
            notifyItemChanged(mSelectedPosition);
            if (mClickListener != null) {
                mClickListener.onDetailItemClick(pos, dataItem);
            }
        });
    }

    @Override
    public int getItemCount() {
        return mDetailList.size();
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {

        TextView tvTime;
        TextView tvValue;
        TextView tvCategory;
        TextView tvNote;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            tvTime = itemView.findViewById(R.id.myevent_tv_detail_time);
            tvValue = itemView.findViewById(R.id.myevent_tv_detail_value);
            tvCategory = itemView.findViewById(R.id.myevent_tv_detail_category);
            tvNote = itemView.findViewById(R.id.myevent_tv_detail_note);
        }
    }

    /**
     * 获取当前选中下标，-1：无选中
     */
    public int getSelectedPosition() {
        return mSelectedPosition;
    }
}
