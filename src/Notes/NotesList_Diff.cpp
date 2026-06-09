#include "NotesList.h"

void NotesList::newtextToOldtextFromDiffStr() {
  int targetIndex = getSelectedVersionIndex();
  if (targetIndex < 0) {
    qDebug() << "无效的版本索引";
    return;
  }

  // 检查补丁列表是否足够（需要 targetIndex 个补丁才能反推到该版本）
  if (targetIndex >= noteDiffPatch.size()) {
    qDebug() << "补丁数量不足，无法反推到指定版本";
    return;
  }

  // 从最新版本（index 0）开始
  QString currentText = loadText(currentMDFile);
  bool allSuccess = true;

  // 依次应用补丁反推：从 index 0 → 1 → ... → targetIndex
  for (int i = 0; i <= targetIndex; ++i) {
    QString patchStr = noteDiffPatch.at(i);
    QVector<bool> revertSuccess;

    // 调用反推函数，用当前文本和补丁得到上一版本文本
    currentText = m_Notes->m_NoteDiffManager.revertPatchToOldText(
        currentText, patchStr, revertSuccess);

    // 检查当前步骤是否成功
    if (revertSuccess.contains(false)) {
      allSuccess = false;
      qWarning() << "反推版本" << i << "时部分补丁应用失败";
      // 可以选择继续或中断，这里选择继续尝试
    }
  }

  // 处理最终结果
  QString html = markdownToHtmlWithMath(currentText);
  html = html.replace("images/", "file:///" + iniDir + "memo/images/");

  if (allSuccess) {
    qDebug() << "成功反推到版本" << targetIndex;
    setNoteDiffHtmlToQML(html);
  } else {
    qDebug() << "反推完成，但部分步骤存在问题，结果可能不准确";
    setNoteDiffHtmlToQML(html);  // 仍显示结果，但可能有误差
  }
}

void NotesList::closeNoteDiff() {
  noteDiffHtml.clear();
  noteDiffPatch.clear();
  noteDiffTime.clear();
  mui->frameDiff->hide();
  mui->frameNoteList->show();
}

void NotesList::getNoteDiffHtml() {
  int index = getSelectedVersionIndex();
  if (index < 0) return;

  QString html = noteDiffHtml.at(index);
  setNoteDiffHtmlToQML(html);
}

void NotesList::setNoteDiffHtmlToQML(const QString& html) {
  if (mui->qwNoteDiff->source().isEmpty()) {
    mui->qwNoteDiff->rootContext()->setContextProperty("m_NotesList",
                                                       m_NotesList);
    mui->qwNoteDiff->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/NoteDiffHtmlViewer.qml")));
    mui->qwNoteDiff->setResizeMode(
        QQuickWidget::SizeRootObjectToView);  // 自适应大小
  }
  QQuickItem* rootItem = mui->qwNoteDiff->rootObject();
  if (!rootItem) {
    qWarning() << "获取 QML 根对象失败，无法更新 HTML 内容";
    return;
  }

  // 设置 QML 中的 htmlContent 属性（名称必须与 QML 中完全一致）
  rootItem->setProperty("htmlContent", html);
}
