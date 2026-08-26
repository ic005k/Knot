package com.x;

import android.util.SparseBooleanArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.cardview.widget.CardView;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;

public class CategoryGridAdapter
    extends RecyclerView.Adapter<CategoryGridAdapter.CategoryViewHolder>
{

    private final ArrayList<String> mItemList = new ArrayList<>();
    private final SparseBooleanArray itemSelectedCache =
        new SparseBooleanArray();
    private OnItemClickListener mListener;
    private boolean mIsDarkMode;

    public interface OnItemClickListener {
        void onItemClick(int position);
    }

    public void setOnItemClickListener(OnItemClickListener listener) {
        mListener = listener;
    }

    public void setDarkMode(boolean isDark) {
        mIsDarkMode = isDark;
        notifyDataSetChanged();
    }

    public void setStringListData(ArrayList<String> list) {
        mItemList.clear();
        itemSelectedCache.clear();
        if (list != null) {
            mItemList.addAll(list);
        }
        notifyDataSetChanged();
    }

    public void setItemSelected(int position, boolean selected) {
        if (position >= 0 && position < mItemList.size()) {
            itemSelectedCache.put(position, selected);
            notifyItemChanged(position);
        }
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
    public int getItemCount() {
        return mItemList.size();
    }

    public static class CategoryViewHolder extends RecyclerView.ViewHolder {

        TextView tvMaintabTitle;
        CardView cardRoot;

        public CategoryViewHolder(@NonNull View itemView) {
            super(itemView);
            tvMaintabTitle = itemView.findViewById(R.id.tv_maintab_title);
            cardRoot = itemView.findViewById(R.id.card_root);
        }
    }

    @Override
    public void onBindViewHolder(
        @NonNull CategoryViewHolder holder,
        int position
    ) {
        String rawStr = mItemList.get(position);
        String title;
        boolean isTodayTab = false;

        String[] parts = rawStr.split("\\|==\\|");
        if (parts.length >= 2) {
            title = parts[0];
            try {
                int flag = Integer.parseInt(parts[1]);
                isTodayTab = flag == 1;
            } catch (NumberFormatException e) {
                isTodayTab = false;
            }
        } else {
            title = rawStr;
            isTodayTab = false;
        }
        holder.tvMaintabTitle.setText(title);
        boolean isSelected = itemSelectedCache.get(position, false);

        if (mIsDarkMode) {
            if (isSelected) {
                if (isTodayTab) {
                    holder.cardRoot.setCardBackgroundColor(0xFFE65100);
                    holder.tvMaintabTitle.setTextColor(0xFFFFFFFF);
                } else {
                    holder.cardRoot.setCardBackgroundColor(0xFF4285F4);
                    holder.tvMaintabTitle.setTextColor(0xFFFFFFFF);
                }
            } else {
                if (isTodayTab) {
                    holder.cardRoot.setCardBackgroundColor(0xFF482C20);
                    holder.tvMaintabTitle.setTextColor(0xFFFFAB91);
                } else {
                    holder.cardRoot.setCardBackgroundColor(0xFF2C2C2C);
                    holder.tvMaintabTitle.setTextColor(0xFFE0E0E0);
                }
            }
        } else {
            if (isSelected) {
                if (isTodayTab) {
                    holder.cardRoot.setCardBackgroundColor(0xFFFB8C00);
                    holder.tvMaintabTitle.setTextColor(0xFFFFFFFF);
                } else {
                    holder.cardRoot.setCardBackgroundColor(0xFF4285F4);
                    holder.tvMaintabTitle.setTextColor(0xFFFFFFFF);
                }
            } else {
                if (isTodayTab) {
                    holder.cardRoot.setCardBackgroundColor(0xFFFFF3E0);
                    holder.tvMaintabTitle.setTextColor(0xFFE64A19);
                } else {
                    holder.cardRoot.setCardBackgroundColor(0xFFEEEEEE);
                    holder.tvMaintabTitle.setTextColor(0xFF000000);
                }
            }
        }

        // 选中：放大阴影；未选中恢复默认1dp阴影
        if (isSelected) {
            holder.cardRoot.setElevation(8f);
        } else {
            holder.cardRoot.setElevation(1f);
        }

        final int pos = position;
        holder.itemView.setOnClickListener(v -> {
            if (mListener != null) {
                mListener.onItemClick(pos);
            }
        });
    }
}
