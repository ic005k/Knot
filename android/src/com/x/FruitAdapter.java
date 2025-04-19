package com.x;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import java.util.List;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

//用于将上下文、listview 子项布局的 id 和数据都传递过来
public class FruitAdapter extends ArrayAdapter<Fruit> {
    public FruitAdapter(@NonNull Context context, int resource, @NonNull List<Fruit> objects) {
        super(context, resource, objects);
    }

    // 每个子项被滚动到屏幕内的时候会被调用
    @NonNull
    @Override
    public View getView(int position, @Nullable View convertView, @NonNull ViewGroup parent) {
        Fruit fruit = getItem(position);// 得到当前项的 Fruit 实例

        // 为每一个子项加载设定的布局
        View view;
        if (MyActivity.isDark)
            view = LayoutInflater.from(getContext()).inflate(R.layout.fruit_item_dark, parent, false);
        else
            view = LayoutInflater.from(getContext()).inflate(R.layout.fruit_item, parent, false);

        // 分别获取 image view 和 textview 的实例
        ImageView fruitimage = view.findViewById(R.id.fruit_image);
        TextView fruitname = view.findViewById(R.id.fruit_name);
        TextView fruitprice = view.findViewById(R.id.fruit_price);
        // 设置要显示的图片和文字
        fruitimage.setImageResource(fruit.getImageID());
        fruitname.setText(fruit.getName());
        fruitprice.setText(fruit.getPrice());
        return view;
    }
}
