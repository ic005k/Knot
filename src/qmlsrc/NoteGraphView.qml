import QtQuick 2.15
import QtQuick.Controls 2.15
// --- 导入 C++ 注册的模块 ---
import NoteGraph 1.0

// --- 使用 Item 作为根 ---
Item {
    id: tableViewRoot

    // --- 使用 ScrollView 包含 Flickable 以获得更好的触控滚动体验 ---
    ScrollView {
        id: scrollView
        anchors.fill: parent
        padding: 5 // 减少内边距以适应小屏幕

        // --- 内部使用 Flickable 来承载内容 ---
        Flickable {
            id: flickableArea
            contentWidth: tableDisplay.paintedWidth
            contentHeight: tableDisplay.paintedHeight
            clip: true // 裁剪超出范围的内容

            // --- TextEdit 用于显示 HTML 表格 ---
            TextEdit {
                id: tableDisplay
                text: generateHtmlTable()
                textFormat: TextEdit.RichText
                readOnly: true
                selectByMouse: true // 保留选择功能
                font.family: "sans-serif" // 使用更通用的无衬线字体
                font.pixelSize: 18 // 增大字体大小（原为14）
                wrapMode: TextEdit.NoWrap

                onLinkActivated: (link) => {
                    console.log("Link activated (Android):", link);
                    if (typeof graphController !== "undefined" && graphController !== null) {
                        if (typeof graphController.handleNodeDoubleClick === "function") {
                            graphController.handleNodeDoubleClick(link);
                        }
                    }
                }
            }
        }
    }

    // --- 生成 HTML 表格的函数 ---
    function generateHtmlTable() {
        console.log("Generating HTML table for Android...");
        // 使用更简洁的 CSS，减少计算量
        let html = `
        <html>
        <head>
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <style>
                body { font-family: sans-serif; font-size: 18px; margin: 0; padding: 5px; background-color: #ffffff; color: #333333; } /* 增大字体大小（原为14px） */
                table { border-collapse: collapse; width: 100%; background-color: #ffffff; }
                th, td { border: 1px solid #dddddd; padding: 10px; text-align: left; vertical-align: top; } /* 增加内边距使内容不拥挤 */
                th { background-color: #e3f2fd; font-weight: bold; color: #1976d2; font-size: 19px; } /* 表头字体稍大 */
                tr:nth-child(even) { background-color: #fafafa; }
                a { color: #1E88E5; text-decoration: none; font-size: 17px; } /* 链接字体大小调整 */
                a:hover { text-decoration: underline; }
                .note-name { font-weight: bold; color: #212121; font-size: 18px; } /* 笔记名称字体大小 */
                .current-note { background-color: #bbdefb !important; }
                ul { margin: 0; padding-left: 20px; } /* 增加列表左内边距 */
                li { margin-bottom: 5px; font-size: 17px; } /* 列表项字体大小和间距 */
                h2 { color: #1976d2; margin-top: 0; margin-bottom: 15px; font-size: 22px; } /* 标题字体增大 */
            </style>
        </head>
        <body>
        `;

        if (typeof graphController === "undefined" || graphController === null || graphController.model === null) {
            html += "<p style='color: #d32f2f;'>错误：无法访问数据源。</p>";
            html += "</body></html>";
            return html;
        }

        const model = graphController.model;
        const rowCount = model.rowCount();

        if (rowCount === 0) {
            html += "<p>提示：暂无笔记数据。</p>";
            html += "</body></html>";
            return html;
        }

        let nodeMap = {};
        for (let i = 0; i < rowCount; i++) {
            const index = model.index(i, 0);
            const name = model.data(index, NoteGraphModel.NameRole);
            const filePath = model.data(index, NoteGraphModel.FilePathRole);
            const isCurrent = model.data(index, NoteGraphModel.IsCurrentNoteRole);
            const displayName = name && name.trim() !== "" ? name : (filePath ? Qt.resolvedUrl(filePath).split('/').pop().split('.')[0] || "Unnamed" : "Unnamed");
            nodeMap[i] = { name: displayName, filePath: filePath, isCurrent: isCurrent };
        }

        const relationsVariantList = model.getRelations();
        let referencesMap = {};
        let referencedByMap = {};

        for (let i = 0; i < relationsVariantList.length; i++) {
             const relMap = relationsVariantList[i];
             const sourceIndex = relMap.source;
             const targetIndex = relMap.target;

             if (sourceIndex !== undefined && targetIndex !== undefined) {
                 if (!referencesMap[sourceIndex]) {
                     referencesMap[sourceIndex] = [];
                 }
                 if (!referencedByMap[targetIndex]) {
                     referencedByMap[targetIndex] = [];
                 }
                 referencesMap[sourceIndex].push(targetIndex);
                 referencedByMap[targetIndex].push(sourceIndex);
             }
        }

        html += `<h2>笔记引用关系 (${rowCount} 项)</h2>`;
        html += `
        <table>
            <thead>
                <tr>
                    <th>笔记</th>
                    <th>引用</th>
                    <th>被引用</th>
                </tr>
            </thead>
            <tbody>
        `;

        for (let i = 0; i < rowCount; i++) {
            const node = nodeMap[i];
            if (!node) continue;

            const isCurrentClass = node.isCurrent ? ' class="current-note"' : '';
            html += `<tr${isCurrentClass}>`;

            const displayPath = node.filePath ? node.filePath : 'N/A';
            html += `<td><div class="note-name">${node.name}</div><div><a href="${node.filePath}">${displayPath}</a></div></td>`;

            html += "<td>";
            if (referencesMap[i] && referencesMap[i].length > 0) {
                html += "<ul>";
                const maxRefsToShow = 5;
                const refsToShow = referencesMap[i].slice(0, maxRefsToShow);
                for (let j = 0; j < refsToShow.length; j++) {
                    const targetIdx = refsToShow[j];
                    const targetNode = nodeMap[targetIdx];
                    if (targetNode) {
                         html += `<li><span class="note-name">${targetNode.name}</span></li>`;
                    }
                }
                if (referencesMap[i].length > maxRefsToShow) {
                    html += `<li style="color: #999;">... 及其他 ${referencesMap[i].length - maxRefsToShow} 项</li>`;
                }
                html += "</ul>";
            } else {
                html += "<span style='color: #999;'>-</span>";
            }
            html += "</td>";

            html += "<td>";
            if (referencedByMap[i] && referencedByMap[i].length > 0) {
                html += "<ul>";
                const maxRefsToShow = 5;
                const refsToShow = referencedByMap[i].slice(0, maxRefsToShow);
                for (let j = 0; j < refsToShow.length; j++) {
                    const sourceIdx = refsToShow[j];
                    const sourceNode = nodeMap[sourceIdx];
                    if (sourceNode) {
                         html += `<li><span class="note-name">${sourceNode.name}</span></li>`;
                    }
                }
                if (referencedByMap[i].length > maxRefsToShow) {
                    html += `<li style="color: #999;">... 及其他 ${referencedByMap[i].length - maxRefsToShow} 项</li>`;
                }
                html += "</ul>";
            } else {
                html += "<span style='color: #999;'>-</span>";
            }
            html += "</td>";

            html += "</tr>";
        }

        html += `
            </tbody>
        </table>
        </body>
        </html>
        `;

        console.log("HTML table for Android generated.");
        return html;
    }

    Connections {
        target: typeof graphController !== "undefined" && graphController !== null ? graphController : null
        function onModelChanged() {
             console.log("Model changed (Android). Updating table...");
             tableDisplay.text = generateHtmlTable();
        }
    }

    Connections {
        target: typeof graphController !== "undefined" && graphController !== null && graphController.model !== null ? graphController.model : null
        function onModelReset() { tableDisplay.text = generateHtmlTable(); }
        function onRowsInserted() { tableDisplay.text = generateHtmlTable(); }
        function onRowsRemoved() { tableDisplay.text = generateHtmlTable(); }
        function onDataChanged() { tableDisplay.text = generateHtmlTable(); }
    }

    Component.onCompleted: {
        console.log("Android Table View completed. Generating table...");
        tableDisplay.text = generateHtmlTable();
    }
}
