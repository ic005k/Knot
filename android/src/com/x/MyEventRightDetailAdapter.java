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

    public void setDarkMode(boolean dark) {
        mIsDark = dark;
        notifyDataSetChanged();
    }

    public void setDetailData(ArrayList<MyEventDetailItem> list) {
        mDetailList.clear();
        mDetailList.addAll(list);
        notifyDataSetChanged();
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
        holder.tvTime.setText(item.timeStr);
        holder.tvValue.setText(String.valueOf(item.eventValue));
        holder.tvCategory.setText(item.category);
        holder.tvNote.setText(item.note);

        if (mIsDark) {
            holder.itemView.setBackgroundColor(0xFF1E1E1E);
            holder.tvTime.setTextColor(0xFFFFFFFF);
            holder.tvValue.setTextColor(0xFFFFFFFF);
            holder.tvCategory.setTextColor(0xFFFFFFFF);
            holder.tvNote.setTextColor(0xFFFFFFFF);
        } else {
            holder.itemView.setBackgroundColor(0xFFFFFFFF);
            holder.tvTime.setTextColor(0xFF000000);
            holder.tvValue.setTextColor(0xFF000000);
            holder.tvCategory.setTextColor(0xFF000000);
            holder.tvNote.setTextColor(0xFF000000);
        }
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
}
