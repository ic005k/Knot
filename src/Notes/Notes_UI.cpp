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
      auto msg1 = std::make_unique<ShowMessage>(this);
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

// 链接自动补全
void Notes::on_editNoteLink_textChanged(const QString& arg1) {
  QString keyword = arg1.trimmed();

  // 空内容隐藏
  if (keyword.isEmpty()) {
    m_popupList->hide();
    return;
  }

  m_popupList->setFixedWidth(ui->editNoteLink->width());

  // ==============================
  //  下拉列表完整样式（亮/暗双主题）
  //  垂直滚动条 + 水平滚动条 + 无箭头 + 圆角扁平现代风
  // ==============================
  if (isDark) {
    m_popupList->setStyleSheet(R"(
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
    m_popupList->setStyleSheet(R"(
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
  QStringList matches = m_NoteIndexManager->searchTitles(keyword);

  // 刷新列表
  m_popupList->clear();
  m_popupList->addItems(matches);

  // 显示在输入框正下方
  QPoint pos =
      ui->editNoteLink->mapToGlobal(QPoint(0, ui->editNoteLink->height()));
  m_popupList->move(pos);
  m_popupList->show();
}

void Notes::onPopupItemClicked(QListWidgetItem* item) {
  QString title = item->text();

  // 插入链接
  insertNoteLink(title);

  // 清空 + 关闭列表
  ui->editNoteLink->clear();
  m_popupList->hide();
}

void Notes::insertNoteLink(const QString& title) {
  // 1. 通过标题获取路径
  QString fullPath = m_NoteIndexManager->getFilePathByTitle(title);
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