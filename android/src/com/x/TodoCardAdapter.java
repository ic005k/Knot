package com.x;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;

public class TodoCardAdapter
    extends RecyclerView.Adapter<TodoCardAdapter.TodoViewHolder>
{

    public static native void PublicJavaCallCpp(String type);

    private ArrayList<String> mRawData = new ArrayList<>();
    private boolean mIsDark = false;
    private TodoActivity.OnTodoItemActionListener mListener;
    private int mSelectedPos = -1;

    public void setStringListData(ArrayList<String> list) {
        mRawData.clear();
        mRawData.addAll(list);
        mSelectedPos = -1;
        notifyDataSetChanged();
    }

    public void setDarkMode(boolean dark) {
        mIsDark = dark;
        notifyDataSetChanged();
    }

    public void setOnItemActionListener(
        TodoActivity.OnTodoItemActionListener l
    ) {
        mListener = l;
    }

    public void clearSelection() {
        mSelectedPos = -1;
        notifyDataSetChanged();
    }

    @NonNull
    @Override
    public TodoViewHolder onCreateViewHolder(
        @NonNull ViewGroup parent,
        int viewType
    ) {
        View v = LayoutInflater.from(parent.getContext()).inflate(
            R.layout.item_todo_card,
            parent,
            false
        );
        return new TodoViewHolder(v);
    }

    @Override
    public void onBindViewHolder(@NonNull TodoViewHolder holder, int position) {
        String item = mRawData.get(position);
        String[] parts = item.split("\\|==\\|");
        if (parts.length >= 3) {
            String strTime = parts[0];
            int nType;
            try {
                nType = Integer.parseInt(parts[1]);
            } catch (NumberFormatException e) {
                nType = 0;
            }
            String strText = parts[2];

            int stripeColor;
            switch (nType) {
                case 1:
                    stripeColor = 0xFFFF4444;
                    break;
                case 2:
                    stripeColor = 0xFFFF9800;
                    break;
                case 3:
                    stripeColor = 0xFF3498DB;
                    break;
                default:
                    stripeColor = 0xFF888888;
                    break;
            }
            holder.viewStripe.setBackgroundColor(stripeColor);
            holder.tvTodoTag.setText(strTime);
            holder.tvTodoContent.setText(strText);
        }

        boolean isSelected = position == mSelectedPos;
        // 明暗模式 + 选中背景
        if (mIsDark) {
            if (isSelected) {
                holder.itemView.setBackgroundColor(0xFF3A3A3A);
            } else {
                holder.itemView.setBackgroundColor(0xFF282828);
            }
            holder.tvTodoTag.setTextColor(0xFFFFFFFF);
            holder.tvTodoContent.setTextColor(0xFFEFEFEF);
        } else {
            if (isSelected) {
                holder.itemView.setBackgroundColor(0xFFE8F0FE);
            } else {
                holder.itemView.setBackgroundColor(0xFFFFFFFF);
            }
            holder.tvTodoTag.setTextColor(0xFF000000);
            holder.tvTodoContent.setTextColor(0xFF222222);
        }

        int iconTint = mIsDark ? 0xFFFFFFFF : 0xFF000000;
        holder.ivStar.setColorFilter(iconTint);
        holder.ivCopy.setColorFilter(iconTint);
        holder.ivEdit.setColorFilter(iconTint);
        holder.ivAlarm.setColorFilter(iconTint);
        holder.ivDelete.setColorFilter(iconTint);

        final int pos = position;
        holder.ivStar.setOnClickListener(v -> {
            if (mListener != null) mListener.onAction(pos, "high");
        });
        holder.ivCopy.setOnClickListener(v -> {
            if (mListener != null) mListener.onAction(pos, "low");
        });
        holder.ivEdit.setOnClickListener(v -> {
            if (mListener != null) mListener.onAction(pos, "edit");
        });
        holder.ivAlarm.setOnClickListener(v -> {
            if (mListener != null) mListener.onAction(pos, "alarm");
            PublicJavaCallCpp("todo_alarm|==|" + pos);
        });
        holder.ivDelete.setOnClickListener(v -> {
            if (mListener != null) mListener.onAction(pos, "recycle");
            PublicJavaCallCpp("todo_recycle");
        });
        holder.tvDone.setOnClickListener(v -> {
            if (mListener != null) mListener.onAction(pos, "done");
        });

        if (isSelected) {
            holder.actionContainer.setVisibility(View.VISIBLE);
        } else {
            holder.actionContainer.setVisibility(View.GONE);
        }

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
        return mRawData.size();
    }

    public static class TodoViewHolder extends RecyclerView.ViewHolder {

        View viewStripe;
        TextView tvTodoTag;
        TextView tvTodoContent;
        ImageView ivStar;
        ImageView ivCopy;
        ImageView ivEdit;
        ImageView ivAlarm;
        ImageView ivDelete;
        TextView tvDone;
        View actionContainer;

        public TodoViewHolder(@NonNull View itemView) {
            super(itemView);
            viewStripe = itemView.findViewById(R.id.viewStripe);
            tvTodoTag = itemView.findViewById(R.id.tv_todo_tag);
            tvTodoContent = itemView.findViewById(R.id.tv_todo_content);
            ivStar = itemView.findViewById(R.id.iv_action_star);
            ivCopy = itemView.findViewById(R.id.iv_action_copy);
            ivEdit = itemView.findViewById(R.id.iv_action_edit);
            ivAlarm = itemView.findViewById(R.id.iv_action_alarm);
            ivDelete = itemView.findViewById(R.id.iv_action_delete);
            tvDone = itemView.findViewById(R.id.tv_action_done);
            actionContainer = itemView.findViewById(R.id.action_container);
        }
    }
}
