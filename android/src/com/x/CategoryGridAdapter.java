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
        public boolean isTodayTab;
    }

    private final ArrayList<CategoryItem> mItemList = new ArrayList<>();
    private OnItemClickListener mListener;

    public interface OnItemClickListener {
        void onItemClick(int position);
    }

    public void setOnItemClickListener(OnItemClickListener listener) {
        mListener = listener;
    }

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

    public void setItemSelected(int position, boolean selected) {
        if (position >= 0 && position < mItemList.size()) {
            mItemList.get(position).selected = selected;
            notifyItemChanged(position);
        }
    }

    public void setTodayTabIndex(int position) {
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

        if (item.selected) {
            holder.itemView.setBackgroundColor(0xFF4285F4);
            holder.tvMaintabTitle.setTextColor(0xFFFFFFFF);
        } else {
            holder.itemView.setBackgroundColor(0xFFEEEEEE);
            holder.tvMaintabTitle.setTextColor(0xFF000000);
        }

        if (item.isTodayTab) {
            holder.tvMaintabTitle.setTextColor(0xFFFF5722);
        }

        final int pos = position;
        holder.itemView.setOnClickListener(v -> {
            if (mListener != null) {
                mListener.onItemClick(pos);
            }
        });
    }

    @Override
    public int getItemCount() {
        return mItemList.size();
    }

    public static class CategoryViewHolder extends RecyclerView.ViewHolder {

        TextView tvMaintabTitle;

        public CategoryViewHolder(@NonNull View itemView) {
            super(itemView);
            tvMaintabTitle = itemView.findViewById(R.id.tv_maintab_title);
        }
    }
}
