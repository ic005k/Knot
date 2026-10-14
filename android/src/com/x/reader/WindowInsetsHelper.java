// WindowInsetsHelper.java
package com.x.reader;

import android.graphics.Insets;
import android.view.WindowInsets;

public class WindowInsetsHelper {

    // 仅Android R+调用
    public static Insets getInsetsApi30(WindowInsets ins) {
        return ins.getInsets(WindowInsets.Type.systemBars());
    }

    public static Insets getCutoutApi30(WindowInsets ins) {
        return ins.getInsets(WindowInsets.Type.displayCutout());
    }
}
