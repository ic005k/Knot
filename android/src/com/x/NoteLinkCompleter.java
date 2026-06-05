package com.x;

import java.util.List;

public class NoteLinkCompleter {

    private final NoteIndexManager indexManager;

    public NoteLinkCompleter(NoteIndexManager manager) {
        this.indexManager = manager;
    }

    // ====================
    // 搜索匹配的标题（给列表显示用）
    // ====================
    public List<String> complete(String keyword) {
        return indexManager.searchTitles(keyword);
    }

    // ====================
    // 根据标题 → 生成最终插入的 MD 链接
    // ====================
    public String buildMarkdownLink(String title) {
        String fullPath = indexManager.getFilePathByTitle(title);
        String relativePath = indexManager.getMemoRelativePath(fullPath);
        return "[" + title + "](" + relativePath + ")";
    }
}
