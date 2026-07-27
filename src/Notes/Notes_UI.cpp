

#include "src/Notes/Notes.h"

// 按钮点击
void Notes::on_btnDone_clicked() { saveMDFile(); }
void Notes::on_btnInsertTable_clicked() {
#ifndef Q_OS_ANDROID
  QString table = "|Title1|Title2|\n|------|------|\n";
  m_EditSource->insert(table);
#endif
}

void Notes::on_btnS1_clicked() {
#ifndef Q_OS_ANDROID

  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Bold Italic");
  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("_**" + str + "**_");
  else
    m_EditSource->replaceSelectedText("_**" + str + "**_");

#endif
}

void Notes::on_btnS2_clicked() {
#ifndef Q_OS_ANDROID

  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Italic");
  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("_" + str + "_");
  else
    m_EditSource->replaceSelectedText("_" + str + "_");

#endif
}

void Notes::on_btnS3_clicked() {
#ifndef Q_OS_ANDROID

  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Underline");

  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("<u>" + str + "</u>");
  else
    m_EditSource->replaceSelectedText("<u>" + str + "</u>");

#endif
}

void Notes::on_btnS4_clicked() {
#ifndef Q_OS_ANDROID

  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Strickout");

  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("~~" + str + "~~");
  else
    m_EditSource->replaceSelectedText("~~" + str + "~~");

#endif
}
void Notes::on_btnS5_clicked() {
#ifndef Q_OS_ANDROID

  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Bold");
  if (!m_EditSource->hasSelectedText())
    m_EditSource->insert("**" + str + "**");
  else
    m_EditSource->replaceSelectedText("**" + str + "**");

#endif
}

void Notes::on_btnColor_clicked() {
#ifndef Q_OS_ANDROID

  QString strColor = m_Method->getCustomColor();
  if (strColor.isEmpty()) return;

  QString str = m_EditSource->selectedText();
  if (str == "") str = tr("Color");
  m_EditSource->insert("<font color=" + strColor + ">" + str + "</font>");

#endif
}

void Notes::on_btnPaste_clicked() {
#ifndef Q_OS_ANDROID

  const QClipboard* clipboard = QApplication::clipboard();
  const QMimeData* mimeData = clipboard->mimeData();
  if (mimeData->hasImage()) {
    QImage img = qvariant_cast<QImage>(mimeData->imageData());
    if (!img.isNull()) {
      QPixmap pix;
      QString strTar = privateDir + "temppic.png";
      pix = QPixmap::fromImage(img);
      pix = pix.scaled(img.width(), img.height(), Qt::KeepAspectRatio,
                       Qt::SmoothTransformation);
      pix.save(strTar);
      insertImage(strTar, false);
    }
  } else
    m_EditSource->paste();

#endif
}

bool Notes::selectPDFFormat(QPrinter* printer) {
  QSettings settings;

  // 选择纸张尺寸
  QStringList pageSizeStrings;
  pageSizeStrings << QStringLiteral("A0") << QStringLiteral("A1")
                  << QStringLiteral("A2") << QStringLiteral("A3")
                  << QStringLiteral("A4") << QStringLiteral("A5")
                  << QStringLiteral("A6") << QStringLiteral("A7")
                  << QStringLiteral("A8") << QStringLiteral("A9")
                  << tr("Letter");
  QList<QPageSize::PageSizeId> pageSizes;
  pageSizes << QPageSize::A0 << QPageSize::A1 << QPageSize::A2 << QPageSize::A3
            << QPageSize::A4 << QPageSize::A5 << QPageSize::A6 << QPageSize::A7
            << QPageSize::A8 << QPageSize::A9 << QPageSize::Letter;

  // 关键：使用局部对象，执行完自动析构
  PrintPDF dlg1(this);
  QString pageSizeString =
      dlg1.getItem(tr("Page size"), tr("Page size"), pageSizeStrings, 4);

  if (pageSizeString.isEmpty()) {
    // 取消时主动清理遮罩层
    if (m_Method) {
      m_Method->closeGrayWindows();
    }
    return false;
  }

  int pageSizeIndex = pageSizeStrings.indexOf(pageSizeString);
  if (pageSizeIndex == -1) {
    if (m_Method) {
      m_Method->closeGrayWindows();
    }
    return false;
  }

  QPageSize pageSize(pageSizes.at(pageSizeIndex));
  settings.setValue(QStringLiteral("Printer/NotePDFExportPageSize"),
                    pageSizeIndex);
  printer->setPageSize(pageSize);

  // 选择打印方向
  QStringList orientationStrings;
  orientationStrings << tr("Portrait") << tr("Landscape");
  QList<QPageLayout::Orientation> orientations;
  orientations << QPageLayout::Portrait << QPageLayout::Landscape;

  PrintPDF dlg2(this);
  QString orientationString =
      dlg2.getItem(tr("Orientation"), tr("Orientation"), orientationStrings, 0);

  if (orientationString.isEmpty()) {
    if (m_Method) {
      m_Method->closeGrayWindows();
    }
    return false;
  }

  int orientationIndex = orientationStrings.indexOf(orientationString);
  if (orientationIndex == -1) {
    if (m_Method) {
      m_Method->closeGrayWindows();
    }
    return false;
  }

  printer->setPageOrientation(orientations.at(orientationIndex));
  settings.setValue(QStringLiteral("Printer/NotePDFExportOrientation"),
                    orientationIndex);

#ifdef Q_OS_ANDROID
  pdfFileName = "/storage/emulated/0/KnotBak/" + m_NotesList->noteTitle +
                QStringLiteral(".pdf");
#else
  QFileDialog dialog(NULL, QStringLiteral("NotePDFExport"));
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setNameFilter(tr("PDF files") + QStringLiteral(" (*.pdf)"));
  dialog.setWindowTitle(tr("Export current note as PDF"));
  dialog.selectFile(m_NotesList->noteTitle + QStringLiteral(".pdf"));
  int ret = dialog.exec();

  if (ret != QDialog::Accepted) {
    if (m_Method) {
      m_Method->closeGrayWindows();
    }
    return false;
  }

  pdfFileName = dialog.selectedFiles().at(0);
#endif

  if (pdfFileName.isEmpty()) {
    if (m_Method) {
      m_Method->closeGrayWindows();
    }
    return false;
  }

  if (QFileInfo(pdfFileName).suffix().isEmpty()) {
    pdfFileName.append(QLatin1String(".pdf"));
  }

  printer->setOutputFormat(QPrinter::PdfFormat);
  printer->setOutputFileName(pdfFileName);

  // 最终兜底：确保遮罩层被清理
  if (m_Method) {
    m_Method->closeGrayWindows();
  }

  return true;
}

void Notes::on_btnPDF_clicked() {
  MD2Html(currentMDFile);
  QString html = loadText(htmlFileName);
  html = html.replace("file://", "");
  auto doc = new QTextDocument(this);
  doc->setHtml(html);

  auto* printer = new QPrinter(QPrinter::HighResolution);

  if (selectPDFFormat(printer)) {
    doc->print(printer);

    if (isAndroid) {
      auto msg1 = std::make_unique<ShowMessage>(mw_one);
      msg1->ui->btnCancel->setText(tr("No"));
      msg1->ui->btnOk->setText(tr("Yes"));
      if (msg1->showMsg("PDF",
                        tr("The PDF file is successfully exported.") + "\n\n" +
                            tr("Want to share this PDF file?") + "\n\n" +
                            pdfFileName,
                        2)) {
        if (QFile::exists(pdfFileName)) {
          mw_one->m_ReceiveShare->shareImage(tr("Share to"), pdfFileName,
                                             "*/*");
        }
      }
    }
  }

  delete printer;
}

void Notes::on_btnView_clicked() {
  ui->btnDone->click();
  mui->btnOpenNote->click();
}

// 搜索
void Notes::on_btnFind_clicked() {
  if (ui->editFind->text().trimmed() == "") return;
  show_findText();
}
void Notes::on_btnPrev_clicked() {
  ui->editFind->setFocus();
  searchPrevious();
}
void Notes::on_btnNext_clicked() {
  ui->editFind->setFocus();
  searchNext();
}
void Notes::on_editFind_returnPressed() { searchNext(); }
void Notes::on_editFind_textChanged(const QString& arg1) {
  searchText(arg1.trimmed(), true);
  m_lastSearchText = arg1.trimmed();
}

// 只替换当前项，不自动下一个
void Notes::on_btnReplace_clicked() {
#ifndef Q_OS_ANDROID
  QString search = ui->editFind->text().trimmed();
  QString replace = ui->editReplace->text();

  if (search.isEmpty() || !m_EditSource->hasSelectedText()) return;

  m_EditSource->replaceSelectedText(replace);
  isTextChange = true;
#endif
}

// 替换当前匹配项，并自动查找下一个
void Notes::on_btnFindReplace_clicked() {
#ifndef Q_OS_ANDROID
  QString search = ui->editFind->text().trimmed();
  // 替换输入框
  QString replace = ui->editReplace->text();

  if (search.isEmpty()) return;

  // 如果当前没有选中 = 先搜索一次
  if (!m_EditSource->hasSelectedText()) {
    searchNext();
    return;
  }

  // 执行替换
  m_EditSource->replaceSelectedText(replace);
  isTextChange = true;

  // 立即查找下一个（核心功能：替换后自动找下一个）
  searchNext();
#endif
}

// 替换所有匹配
void Notes::on_btnReplaceAll_clicked() {
#ifndef Q_OS_ANDROID
  QString search = ui->editFind->text().trimmed();
  QString replace = ui->editReplace->text();

  if (search.isEmpty()) return;

  // ==============================================
  // ✅ 正确保存：光标位置 + 锚点位置（两个都要存）
  // ==============================================
  int origCurPos =
      m_EditSource->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
  int origAnchor = m_EditSource->SendScintilla(QsciScintilla::SCI_GETANCHOR);

  // 回到文档开头
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETANCHOR, 0);
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETCURRENTPOS, 0);

  int count = 0;
  bool found =
      m_EditSource->findFirst(search, false, false, false, false, true);

  while (found) {
    m_EditSource->replaceSelectedText(replace);
    count++;
    isTextChange = true;
    found = m_EditSource->findNext();
  }

  // ==============================================
  // ✅ 正确恢复：两个位置一起恢复 → 完全无选中
  // ==============================================
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETANCHOR, origAnchor);
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETCURRENTPOS, origCurPos);

  qDebug() << "全部替换完成：" << count << " 处";
#endif
}

void Notes::popupNoteLinkList(const QString& arg1) {
  QString keyword = arg1.trimmed();

  // 空内容隐藏
  if (keyword.isEmpty() && !isBtnAILinkClicked) {
    return;
  }

  popupLinkList->setFixedWidth(ui->editNoteLink->width() +
                               ui->btnAILink->width() +
                               ui->lblNoteLink->width());

  // ==============================
  //  下拉列表完整样式（亮/暗双主题）
  //  垂直滚动条 + 水平滚动条 + 无箭头 + 圆角扁平现代风
  // ==============================
  if (isDark) {
    ui->listNoteLink->setStyleSheet(R"(
    QListWidget {
        background-color: #2C2C2C;
        color: #E0E0E0;
        border: 1px solid #555555;
        border-top: none;
        padding: 4px;
        outline: none;
    }
    QListWidget::item:selected {
        background-color: #007ACC;
        color: white;
    }
    QListWidget::item:hover {
        background-color: #444444;
    }

    /* --- 垂直滚动条 --- */
    QScrollBar:vertical {
        background: #333333;
        width: 8px;
        margin: 0px;
        border:none;
    }
    QScrollBar::handle:vertical {
        background: #777777;
        border-radius: 4px;
        min-height: 25px;
        border:none;
    }
    QScrollBar::handle:vertical:hover {
        background: #999999;
    }
    QScrollBar::add-line:vertical,
    QScrollBar::sub-line:vertical {
        height: 0px;
    }
    QScrollBar::add-page:vertical,
    QScrollBar::sub-page:vertical {
        background: none;
    }

    /* --- 水平滚动条 --- */
    QScrollBar:horizontal {
        background: #333333;
        height: 8px;
        margin: 0px;
        border:none;
    }
    QScrollBar::handle:horizontal {
        background: #777777;
        border-radius: 4px;
        min-width: 25px;
        border:none;
    }
    QScrollBar::handle:horizontal:hover {
        background: #999999;
    }
    QScrollBar::add-line:horizontal,
    QScrollBar::sub-line:horizontal {
        width: 0px;
    }
    QScrollBar::add-page:horizontal,
    QScrollBar::sub-page:horizontal {
        background: none;
    }
  )");
  } else {
    ui->listNoteLink->setStyleSheet(R"(
    QListWidget {
        background-color: #FFFFFF;
        color: #2C2C2C;
        border: 1px solid #CCCCCC;
        border-top: none;
        padding: 4px;
        outline: none;
    }
    QListWidget::item:selected {
        background-color: #007ACC;
        color: white;
    }
    QListWidget::item:hover {
        background-color: #E5F1FF;
    }

    /* --- 垂直滚动条 --- */
    QScrollBar:vertical {
        background: #F5F5F5;
        width: 8px;
        margin: 0px;
        border:none;
    }
    QScrollBar::handle:vertical {
        background: #CCCCCC;
        border-radius: 4px;
        min-height: 25px;
        border:none;
    }
    QScrollBar::handle:vertical:hover {
        background: #AAAAAA;
    }
    QScrollBar::add-line:vertical,
    QScrollBar::sub-line:vertical {
        height: 0px;
    }
    QScrollBar::add-page:vertical,
    QScrollBar::sub-page:vertical {
        background: none;
    }

    /* --- 水平滚动条 --- */
    QScrollBar:horizontal {
        background: #F5F5F5;
        height: 8px;
        margin: 0px;
        border:none;
    }
    QScrollBar::handle:horizontal {
        background: #CCCCCC;
        border-radius: 4px;
        min-width: 25px;
        border:none;
    }
    QScrollBar::handle:horizontal:hover {
        background: #AAAAAA;
    }
    QScrollBar::add-line:horizontal,
    QScrollBar::sub-line:horizontal {
        width: 0px;
    }
    QScrollBar::add-page:horizontal,
    QScrollBar::sub-page:horizontal {
        background: none;
    }
  )");
  }

  // 搜索匹配的标题（真正使用用户输入！）
  QStringList matches;
  if (isBtnAILinkClicked) {
    for (int i = 0; i < m_NotesList->adaptedResults.size(); ++i) {
      const SearchResult& item = m_NotesList->adaptedResults[i];
      matches.append(item.preview);
    }

  } else {
    matches = m_NoteIndexManager->searchTitles(keyword);
  }

  ui->listNoteLink->clear();
  ui->listNoteLink->addItems(matches);
}

// 链接自动补全
void Notes::on_editNoteLink_textChanged(const QString& arg1) {
  isBtnAILinkClicked = false;
  popupNoteLinkList(arg1);
}

void Notes::onPopupItemClicked(QListWidgetItem* item) {
#ifndef Q_OS_ANDROID

  QString title = item->text();

  // 插入链接
  QString fullPath;
  if (isBtnAILinkClicked) {
    int index = -1;
    index = m_popupList->currentRow();
    fullPath = m_NotesList->adaptedResults[index].filePath;
    title = takeFirstNTokens(title, 10);

    title = title.replace("\\", "\\\\")
                .replace("[", "\\[")
                .replace("]", "\\]")
                .replace("(", "\\(")
                .replace(")", "\\)")
                .replace("!", "\\!")
                .replace("#", "\\#")
                .replace("*", "\\*")
                .replace("_", "\\_");

  } else {
    fullPath = m_NoteIndexManager->getFilePathByTitle(title);
  }

  insertNoteLink(title, fullPath);

  // 清空 + 关闭列表
  ui->editNoteLink->clear();
  popupLinkList->close();

  isBtnAILinkClicked = false;
  ui->editNoteLink->setEnabled(true);

#endif
}

void Notes::insertNoteLink(const QString& title, const QString& path) {
  // 1. 通过标题获取路径
  QString fullPath = path;  // m_NoteIndexManager->getFilePathByTitle(title);
  if (fullPath.isEmpty()) return;

  // 2. 转成相对路径 memo/xxx.md
  QString basePath = iniDir;
  QString relativePath = QDir(basePath).relativeFilePath(fullPath);

  // 3. 生成最终格式
  QString link = QString("[%1](%2)").arg(title, relativePath);

  // 4. 插入编辑器
#ifndef Q_OS_ANDROID
  m_EditSource->insert(link);
#endif

  // 5. 清空输入框
  ui->editNoteLink->clear();
}

void Notes::on_btnInsertNoteLink_clicked() {
  onPopupItemClicked(ui->listNoteLink->currentItem());
}

void Notes::on_btnAILink_clicked() {
#ifndef Q_OS_ANDROID

  isBtnAILinkClicked = true;
  // ui->editNoteLink->setEnabled(false);

  // 前后各10个字词
  int cursorPos = getCursorCharOffset(m_EditSource);
  QString result = getCursorPosText(m_EditSource->text(), cursorPos, 10);
  qDebug() << "前后各取10各字词：" << result;

  m_NotesList->startVectorSerach(result);

  mw_one->showProgress();
  while (!isVectorSearchDone) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    QThread::msleep(1);
  }

  mw_one->safeCloseProgress();

  // 后面由NotesList::vectorSearchDone完成
  //  popupNoteLinkList("");

#endif
}

// 判断是否为 CJK 统一汉字/日文假名/韩文音节等
static inline bool isCjkChar(QChar ch) {
  uint u = ch.unicode();
  return (u >= 0x4E00 && u <= 0x9FFF) ||    // CJK Unified Ideographs
         (u >= 0x3400 && u <= 0x4DBF) ||    // CJK Extension A
         (u >= 0x20000 && u <= 0x2A6DF) ||  // CJK Extension B
         (u >= 0xF900 && u <= 0xFAFF) ||    // CJK Compatibility Ideographs
         (u >= 0x3040 && u <= 0x309F) ||    // Hiragana
         (u >= 0x30A0 && u <= 0x30FF) ||    // Katakana
         (u >= 0xAC00 && u <= 0xD7AF);      // Hangul Syllables
}

// 判断是否为空白字符（空格、制表符、换行等）
static inline bool isSpaceChar(QChar ch) {
  return ch.isSpace() || ch == '\n' || ch == '\r' || ch == '\t';
}

#ifndef Q_OS_ANDROID

/**
 * @brief 从字符串开头取前N个字词，保留原始格式（空格、标点等原样保留）
 * @param text  原始字符串
 * @param count 要取的字词数量（默认10）
 * @return 截取后的子串，若不足count个字词则返回原串
 */
QString Notes::takeFirstNTokens(const QString& text, int count) {
  if (text.isEmpty() || count <= 0) return text;

  int len = text.length();
  int i = 0;
  int tokenCount = 0;

  while (i < len && tokenCount < count) {
    QChar ch = text[i];

    // 跳过空白（空白不计为token，但会被保留在最终截取范围内）
    if (ch.isSpace()) {
      ++i;
      continue;
    }

    // 遇到一个非空白字符 = 开始一个新token
    ++tokenCount;

    if (isCjkChar(ch)) {
      // CJK：单字即一个token，前进1个字符
      ++i;
    } else {
      // 西文：连续的非空白、非CJK字符构成一个词
      while (i < len && !text[i].isSpace() && !isCjkChar(text[i])) {
        ++i;
      }
    }
  }

  // tokenCount < count 说明不够10个，返回原串
  if (tokenCount < count) return text;

  // 截取从开头到第10个token结束位置的子串
  // 注意：此时 i 恰好停在第10个token之后的第一个字符
  // 如果后面紧跟空白，这些空白不属于第10个token，不应包含
  // 但如果用户希望保留token后的尾随空格，可去掉下面的trim逻辑
  return text.left(i);
}

/**
 * @brief 获取 QScintilla 编辑器中光标的字符偏移量（UTF-16 索引）
 * @return 从文档开头到光标位置的字符数，可直接传给 getCursorPosText()
 */
int Notes::getCursorCharOffset(QsciScintilla* editor) {
  if (!editor) return 0;

  int line, index;
  editor->getCursorPosition(&line, &index);

  // positionFromLineIndex 返回的是字节偏移（取决于编码）
  // 对于 QString 操作，我们需要 UTF-16 字符偏移
  long pos = editor->positionFromLineIndex(line, index);

  // QScintilla 内部使用 UTF-8，而 QString 是 UTF-16
  // 必须将 UTF-8 字节偏移转换为 UTF-16 字符偏移
  QByteArray utf8 = editor->text().toUtf8();
  QString text = QString::fromUtf8(utf8.left(pos));
  return text.length();
}

#endif

/**
 * @brief 获取光标前后各 textCount 个字词的上下文（纯字符串操作，跨平台）
 * @param fullText   编辑器全量文本
 * @param cursorPos  光标在 fullText 中的字符偏移量 [0, fullText.length()]
 * @param textCount  光标每侧需要获取的字词数量
 * @return 拼接后的上下文字符串，保留原始格式；不足则返回实际可用部分
 */
QString Notes::getCursorPosText(const QString& fullText, int cursorPos,
                                int textCount) {
  if (fullText.isEmpty() || textCount <= 0) return {};

  // 安全钳制光标位置
  cursorPos = qBound(0, cursorPos, fullText.length());

  // ---------- 向前扫描：找第 textCount 个 token 的起始位置 ----------
  int leftStart = cursorPos;
  int leftCount = 0;
  while (leftStart > 0 && leftCount < textCount) {
    --leftStart;
    // 跳过空白（空白不计数，但会被包含在截取范围内）
    if (fullText[leftStart].isSpace()) continue;

    // 遇到非空白 = 一个 token 的末尾（从右往左看）
    ++leftCount;

    // 如果是 CJK 字符，单字即一个 token，leftStart 已指向它
    if (isCjkChar(fullText[leftStart])) continue;

    // 西文词：继续向左直到遇到空白或 CJK 边界
    while (leftStart > 0 && !fullText[leftStart - 1].isSpace() &&
           !isCjkChar(fullText[leftStart - 1])) {
      --leftStart;
    }
  }

  // ---------- 向后扫描：找第 textCount 个 token 的结束位置 ----------
  int rightEnd = cursorPos;
  int rightCount = 0;
  int len = fullText.length();
  while (rightEnd < len && rightCount < textCount) {
    // 跳过空白
    if (fullText[rightEnd].isSpace()) {
      ++rightEnd;
      continue;
    }

    ++rightCount;

    if (isCjkChar(fullText[rightEnd])) {
      ++rightEnd;
    } else {
      // 西文词：向右吞掉整个词
      while (rightEnd < len && !fullText[rightEnd].isSpace() &&
             !isCjkChar(fullText[rightEnd])) {
        ++rightEnd;
      }
    }
  }

  // 直接截取原始子串，完美保留空格、标点等原始格式
  return fullText.mid(leftStart, rightEnd - leftStart);
}
