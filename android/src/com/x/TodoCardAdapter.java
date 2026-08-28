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

    private ArrayList<String> mRawData = new ArrayList<>();
    private boolean mIsDark = false;
    private TodoActivity.OnTodoItemActionListener mListener;

    public void setStringListData(ArrayList<String> list) {
        mRawData.clear();
        mRawData.addAll(list);
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
        // C++约定格式： strTime |==| type(int) |==| strText
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

            // type映射为Android颜色int
            int stripeColor;
            switch (nType) {
                case 1: // red
                    stripeColor = 0xFFFF4444;
                    break;
                case 2: // orange
                    stripeColor = 0xFFFF9800;
                    break;
                case 3: // #3498DB blue
                    stripeColor = 0xFF3498DB;
                    break;
                default: // 0 gray
                    stripeColor = 0xFF888888;
                    break;
            }
            holder.viewStripe.setBackgroundColor(stripeColor);
            holder.tvTodoTag.setText(strTime);
            holder.tvTodoContent.setText(strText);
        }

        //明暗模式下卡片背景
        if (mIsDark) {
            holder.cardView.setCardBackgroundColor(0xFF282828);
            holder.tvTodoTag.setTextColor(0xFFFFFFFF);
            holder.tvTodoContent.setTextColor(0xFFEFEFEF);
        } else {
            holder.cardView.setCardBackgroundColor(0xFFFFFFFF);
            holder.tvTodoTag.setTextColor(0xFF000000);
            holder.tvTodoContent.setTextColor(0xFF222222);
        }
        //图标着色
        int iconTint = mIsDark ? 0xFFFFFFFF : 0xFF000000;
        holder.ivStar.setColorFilter(iconTint);
        holder.ivCopy.setColorFilter(iconTint);
        holder.ivEdit.setColorFilter(iconTint);
        holder.ivAlarm.setColorFilter(iconTint);
        holder.ivDelete.setColorFilter(iconTint);

        //各个操作按钮转发事件
        final int pos = position;
        holder.ivStar.setOnClickListener(v -> {
            if (mListener != null) mListener.onAction(pos, "star");
        });
        holder.ivCopy.setOnClickListener(v -> {
            if (mListener != null) mListener.onAction(pos, "copy");
        });
        holder.ivEdit.setOnClickListener(v -> {
            if (mListener != null) mListener.onAction(pos, "edit");
        });
        holder.ivAlarm.setOnClickListener(v -> {
            if (mListener != null) mListener.onAction(pos, "alarm");
        });
        holder.ivDelete.setOnClickListener(v -> {
            if (mListener != null) mListener.onAction(pos, "delete");
        });
        holder.tvDone.setOnClickListener(v -> {
            if (mListener != null) mListener.onAction(pos, "done");
        });
    }

    @Override
    public int getItemCount() {
        return mRawData.size();
    }

    public static class TodoViewHolder extends RecyclerView.ViewHolder {

        androidx.cardview.widget.CardView cardView;
        View viewStripe;
        TextView tvTodoTag;
        TextView tvTodoContent;
        ImageView ivStar;
        ImageView ivCopy;
        ImageView ivEdit;
        ImageView ivAlarm;
        ImageView ivDelete;
        TextView tvDone;

        public TodoViewHolder(@NonNull View itemView) {
            super(itemView);
            cardView = (androidx.cardview.widget.CardView) itemView;
            viewStripe = itemView.findViewById(R.id.view_stripe);
            tvTodoTag = itemView.findViewById(R.id.tv_todo_tag);
            tvTodoContent = itemView.findViewById(R.id.tv_todo_content);
            ivStar = itemView.findViewById(R.id.iv_action_star);
            ivCopy = itemView.findViewById(R.id.iv_action_copy);
            ivEdit = itemView.findViewById(R.id.iv_action_edit);
            ivAlarm = itemView.findViewById(R.id.iv_action_alarm);
            ivDelete = itemView.findViewById(R.id.iv_action_delete);
            tvDone = itemView.findViewById(R.id.tv_action_done);
        }
    }
}
