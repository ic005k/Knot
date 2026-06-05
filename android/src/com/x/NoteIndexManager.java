package com.x;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import org.json.JSONObject;

public class NoteIndexManager {

    // 对应你 C++ 的 m_metadataMap
    private final Map<String, NoteItem> noteIndexMap = new HashMap<>();

    // ====================
    // 1. 加载索引 JSON（核心）
    // ====================
    public void loadIndex(JSONObject root) {
        noteIndexMap.clear();

        if (root == null) return;
        JSONObject data = root.optJSONObject("data");
        if (data == null) return;

        Iterator<String> keys = data.keys();
        while (keys.hasNext()) {
            String filePath = keys.next();
            JSONObject obj = data.optJSONObject(filePath);

            if (obj != null) {
                NoteItem item = new NoteItem();
                item.title = obj.optString("title", "");
                item.noteIndex = obj.optInt("noteIndex", 0);
                item.notebookIndex = obj.optInt("notebookIndex", 0);
                noteIndexMap.put(filePath, item);
            }
        }
    }

    // ====================
    // 2. 搜索标题（模糊匹配）
    // ====================
    public List<String> searchTitles(String keyword) {
        List<String> result = new ArrayList<>();
        if (keyword == null) return result;

        for (Map.Entry<String, NoteItem> entry : noteIndexMap.entrySet()) {
            String title = entry.getValue().title;
            if (title.toLowerCase().contains(keyword.toLowerCase())) {
                result.add(title);
            }
        }
        return result;
    }

    // ====================
    // 3. 根据标题 → 获取完整路径
    // ====================
    public String getFilePathByTitle(String title) {
        for (Map.Entry<String, NoteItem> entry : noteIndexMap.entrySet()) {
            if (title.equals(entry.getValue().title)) {
                return entry.getKey();
            }
        }
        return "";
    }

    // ====================
    // 4. 获取 memo/xxx.md 格式路径（最终插入用）
    // ====================
    public String getMemoRelativePath(String fullPath) {
        if (fullPath == null || fullPath.isEmpty()) return "";

        int index = fullPath.lastIndexOf("memo/");
        if (index != -1) {
            return fullPath.substring(index);
        }
        return "";
    }

    // ====================
    // 实体类（对应你C++的 NoteMetadata）
    // ====================
    public static class NoteItem {

        public String title;
        public int noteIndex;
        public int notebookIndex;
    }
}
