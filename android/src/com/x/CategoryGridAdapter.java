package com.x;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;

public class CategoryGridAdapter
    extends RecyclerView.Adapter<CategoryGridAdapter.CategoryViewHolder>
{

    public static class CategoryItem {

        public String title;
        public int index;
        public boolean selected;
        public boolean isTodayTab; // 是否为今日tab，用于特殊颜色绘制
    }

    private final ArrayList<CategoryItem> mItemList = new ArrayList<>();
    private OnItemClickListener mListener;

    public interface OnItemClickListener {
        void onItemClick(int position);
    }

    public void setOnItemClickListener(OnItemClickListener listener) {
        mListener = listener;
    }

    /**
     * 设置tab名称列表，来自Intent跳转传参
     * @param list tab名称字符串集合
     */
    public void setStringListData(ArrayList<String> list) {
        mItemList.clear();
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                CategoryItem item = new CategoryItem();
                item.title = list.get(i);
                item.index = i;
                item.selected = false;
                item.isTodayTab = false;
                mItemList.add(item);
            }
        }
        notifyDataSetChanged();
    }

    /**
     * 更新指定position选中状态
     */
    public void setItemSelected(int position, boolean selected) {
        if (position >= 0 && position < mItemList.size()) {
            mItemList.get(position).selected = selected;
            notifyItemChanged(position);
        }
    }

    /**
     * 设置哪一项是今日tab，会触发UI重绘
     */
    public void setTodayTabIndex(int position) {
        // 先清除全部今日标记
        for (CategoryItem item : mItemList) {
            item.isTodayTab = false;
        }
        if (position >= 0 && position < mItemList.size()) {
            mItemList.get(position).isTodayTab = true;
        }
        notifyDataSetChanged();
    }

    @NonNull
    @Override
    public CategoryViewHolder onCreateViewHolder(
        @NonNull ViewGroup parent,
        int viewType
    ) {
        View itemView = LayoutInflater.from(parent.getContext()).inflate(
            R.layout.item_maintab_card,
            parent,
            false
        );
        return new CategoryViewHolder(itemView);
    }

    @Override
    public void onBindViewHolder(
        @NonNull CategoryViewHolder holder,
        int position
    ) {
        CategoryItem item = mItemList.get(position);
        holder.tvMaintabTitle.setText(item.title);

        // 直接硬设置颜色，选中/未选中
        if (item.selected) {
            holder.itemView.setBackgroundColor(0xFF4285F4);
            holder.tvMaintabTitle.setTextColor(0xFFFFFFFF);
        } else {
            holder.itemView.setBackgroundColor(0xFFEEEEEE);
            holder.tvMaintabTitle.setTextColor(0xFF000000);
        }

        // 今日tab覆盖文字颜色
        if (item.isTodayTab) {
            holder.tvMaintabTitle.setTextColor(0xFFFF5722);
        }

        holder.itemView.setOnClickListener(v -> {
            android.util.Log.d("CARD_CLICK", "pos=" + position);
            if (mListener != null) {
                mListener.onItemClick(position);
            }
        });
    }

    @Override
    public int getItemCount() {
        return mItemList.size();
    }

    public static class CategoryViewHolder extends RecyclerView.ViewHolder {

        TextView tvMaintabTitle;

        // 可在这里增加 CardView 引用，方便修改背景颜色
        // CardView cardRoot;

        public CategoryViewHolder(@NonNull View itemView) {
            super(itemView);
            tvMaintabTitle = itemView.findViewById(R.id.tv_maintab_title);
            // cardRoot = itemView.findViewById(R.id.card_root);
        }
    }
}
