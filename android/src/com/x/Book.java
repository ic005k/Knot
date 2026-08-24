package com.x;

public class Book {

    private String title; // UI展示标题
    private String filePath; // 文件路径
    private String rawBookName; // 隐藏原始书名
    private boolean selected; // 选中标记

    private String ext;

    //新构造：标题 | 文件路径 | 隐藏原始书名
    public Book(String title, String filePath, String rawBookName) {
        this.title = title;
        this.filePath = filePath;
        this.rawBookName = rawBookName;
        this.selected = false;
    }

    //兼容旧代码构造
    public Book(String bookName, String filePath) {
        this.title = bookName;
        this.filePath = filePath;
        this.rawBookName = "";
        this.selected = false;
    }

    public String getTitle() {
        return title;
    }

    public String getFilePath() {
        return filePath;
    }

    public String getRawBookName() {
        return rawBookName;
    }

    public boolean isSelected() {
        return selected;
    }

    public void setSelected(boolean selected) {
        this.selected = selected;
    }

    public String getExt() {
        return ext;
    }

    public void setExt(String ext) {
        this.ext = ext;
    }
}
