package com.x;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.util.List;

public class BookAdapter
    extends RecyclerView.Adapter<BookAdapter.BookViewHolder>
{

    private final List<Book> bookList;

    public BookAdapter(List<Book> bookList) {
        this.bookList = bookList;
    }

    @NonNull
    @Override
    public BookViewHolder onCreateViewHolder(
        @NonNull ViewGroup parent,
        int viewType
    ) {
        View view = LayoutInflater.from(parent.getContext()).inflate(
            R.layout.item_book,
            parent,
            false
        );
        return new BookViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull BookViewHolder holder, int position) {
        Book book = bookList.get(position);
        holder.tvBookName.setText(book.getTitle());
        holder.itemView.setSelected(book.isSelected());

        holder.itemView.setOnClickListener(v -> {
            book.setSelected(!book.isSelected());
            notifyItemChanged(position);
        });
    }

    @Override
    public int getItemCount() {
        return bookList.size();
    }

    public Book getSelectedItem() {
        for (Book b : bookList) {
            if (b.isSelected()) {
                return b;
            }
        }
        return null;
    }

    public void clearAllSelect() {
        for (Book b : bookList) {
            b.setSelected(false);
        }
        notifyDataSetChanged();
    }

    public static class BookViewHolder extends RecyclerView.ViewHolder {

        TextView tvBookName;

        public BookViewHolder(@NonNull View itemView) {
            super(itemView);
            tvBookName = itemView.findViewById(R.id.tvBookName);
        }
    }
}
