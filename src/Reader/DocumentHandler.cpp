/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "DocumentHandler.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QFileSelector>
#include <QMimeDatabase>
#include <QQmlFile>
#include <QQmlFileSelector>
#include <QStringConverter>
#include <QStringDecoder>
#include <QTextCharFormat>
#include <QTextDocument>

#include "src/LoadPic.h"
#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"

QString picfile;

DocumentHandler::DocumentHandler(QObject* parent)
    : QObject(parent),
      m_document(nullptr),
      m_cursorPosition(-1),
      m_selectionStart(0),
      m_selectionEnd(0) {}

QQuickTextDocument* DocumentHandler::document() const { return m_document; }

void DocumentHandler::setDocument(QQuickTextDocument* document) {
  if (document == m_document) return;

  if (m_document) m_document->textDocument()->disconnect(this);
  m_document = document;
  if (m_document)
    connect(m_document->textDocument(), &QTextDocument::modificationChanged,
            this, &DocumentHandler::modifiedChanged);
  emit documentChanged();
}

int DocumentHandler::cursorPosition() const { return m_cursorPosition; }

void DocumentHandler::setCursorPosition(int position) {
  if (position == m_cursorPosition) return;

  m_cursorPosition = position;
  reset();
  emit cursorPositionChanged();
}

int DocumentHandler::selectionStart() const { return m_selectionStart; }

void DocumentHandler::setSelectionStart(int position) {
  if (position == m_selectionStart) return;

  m_selectionStart = position;
  emit selectionStartChanged();
}

int DocumentHandler::selectionEnd() const { return m_selectionEnd; }

void DocumentHandler::setSelectionEnd(int position) {
  if (position == m_selectionEnd) return;

  m_selectionEnd = position;
  emit selectionEndChanged();
}

QString DocumentHandler::fontFamily() const {
  QTextCursor cursor = textCursor();
  if (cursor.isNull()) return QString();
  QTextCharFormat format = cursor.charFormat();
  return format.font().family();
}

void DocumentHandler::setFontFamily(const QString& family) {
  QTextCharFormat format;
  format.setFontFamily(family);
  mergeFormatOnWordOrSelection(format);
  emit fontFamilyChanged();
}

QColor DocumentHandler::textColor() const {
  QTextCursor cursor = textCursor();
  if (cursor.isNull()) return QColor(Qt::black);
  QTextCharFormat format = cursor.charFormat();
  return format.foreground().color();
}

void DocumentHandler::setTextColor(const QColor& color) {
  QTextCharFormat format;
  format.setForeground(QBrush(color));
  mergeFormatOnWordOrSelection(format);
  emit textColorChanged();
}

Qt::Alignment DocumentHandler::alignment() const {
  QTextCursor cursor = textCursor();
  if (cursor.isNull()) return Qt::AlignLeft;
  return textCursor().blockFormat().alignment();
}

void DocumentHandler::setAlignment(Qt::Alignment alignment) {
  QTextBlockFormat format;
  format.setAlignment(alignment);
  QTextCursor cursor = textCursor();
  cursor.mergeBlockFormat(format);
  emit alignmentChanged();
}

bool DocumentHandler::bold() const {
  QTextCursor cursor = textCursor();
  if (cursor.isNull()) return false;
  return textCursor().charFormat().fontWeight() == QFont::Bold;
}

void DocumentHandler::setBold(bool bold) {
  QTextCharFormat format;
  format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
  mergeFormatOnWordOrSelection(format);
  emit boldChanged();
}

bool DocumentHandler::italic() const {
  QTextCursor cursor = textCursor();
  if (cursor.isNull()) return false;
  return textCursor().charFormat().fontItalic();
}

void DocumentHandler::setItalic(bool italic) {
  QTextCharFormat format;
  format.setFontItalic(italic);
  mergeFormatOnWordOrSelection(format);
  emit italicChanged();
}

bool DocumentHandler::underline() const {
  QTextCursor cursor = textCursor();
  if (cursor.isNull()) return false;
  return textCursor().charFormat().fontUnderline();
}

void DocumentHandler::setUnderline(bool underline) {
  QTextCharFormat format;
  format.setFontUnderline(underline);
  mergeFormatOnWordOrSelection(format);
  emit underlineChanged();
}

int DocumentHandler::fontSize() const {
  QTextCursor cursor = textCursor();
  if (cursor.isNull()) return 0;
  QTextCharFormat format = cursor.charFormat();
  return format.font().pointSize();
}

void DocumentHandler::setFontSize(int size) {
  if (size <= 0) return;

  QTextCursor cursor = textCursor();
  if (cursor.isNull()) return;

  if (!cursor.hasSelection()) cursor.select(QTextCursor::WordUnderCursor);

  if (cursor.charFormat().property(QTextFormat::FontPointSize).toInt() == size)
    return;

  QTextCharFormat format;
  format.setFontPointSize(size);
  mergeFormatOnWordOrSelection(format);
  emit fontSizeChanged();
}

QString DocumentHandler::fileName() const {
  const QString filePath = QQmlFile::urlToLocalFileOrQrc(m_fileUrl);
  const QString fileName = QFileInfo(filePath).fileName();
  if (fileName.isEmpty()) return QStringLiteral("untitled.txt");
  return fileName;
}

QString DocumentHandler::fileType() const {
  return QFileInfo(fileName()).suffix();
}

QUrl DocumentHandler::fileUrl() const { return m_fileUrl; }

void DocumentHandler::parsingLink(QString linkFile, QString qwName) {
  mui->btnAutoStop->click();

  if (mw_one->curx != 0) return;

  // qDebug() << "link : " << linkFile;

  copyText = linkFile;
  if (linkFile.mid(0, 4) == "http" || linkFile.mid(0, 4) == "www.") {
    if (linkFile.mid(0, 4) != "http") {
      linkFile = "http://" + linkFile;
    }
    QUrl url = linkFile;

    bool ok = false;
    if (qwName == "reader") {
      auto msg = std::make_unique<ShowMessage>(mw_one);
      ok = msg->showMsg(appName,
                        tr("Open this URL?") + "\n\n" + copyText + "\n", 3);
    }

    if (qwName == "note") {
      auto msg = std::make_unique<ShowMessage>(mw_one);
      ok = msg->showMsg(appName,
                        tr("Open this URL?") + "\n\n" + copyText + "\n", 4);
    }

    if (ok) QDesktopServices::openUrl(url);

  }

  else if (linkFile.contains("@")) {
    QString str = linkFile;
    str.replace("mailto:", "");

    auto msg = std::make_unique<ShowMessage>(mw_one);
    copyText = str;
    bool ok = msg->showMsg(
        appName, tr("Writing an email?") + "\n\n" + linkFile + "\n", 3);
    if (ok) QDesktopServices::openUrl(QUrl(linkFile));

  }

  else if (linkFile.contains(".html") || linkFile.contains(".xhtml") ||
           linkFile.contains(".xml")) {
    bool is_sup = false;
    bool isOk = false;
    QString s1, s2, str_file, str_id;
    int current_i = 0;
    if (linkFile.contains("#")) {
      str_file = linkFile.split("#").at(0);
      str_id = linkFile.split("#").at(1);
      QString html_file;
      for (int i = 0; i < htmlFiles.count(); i++) {
        QString item = htmlFiles.at(i);
        if (item.contains(str_file)) {
          html_file = item;
          current_i = i;
          break;
        }
      }

      for (int n = current_i; n < htmlFiles.count(); n++) {
        QStringList buf_lists = mw_one->m_Reader->readText(htmlFiles.at(n));
        for (int i = 0; i < buf_lists.count(); i++) {
          QString item = buf_lists.at(i);
          if (item.contains(str_id)) {
            if (buf_lists.at(i + 1).mid(0, 4) == "<sup" ||
                buf_lists.at(i - 1).mid(0, 4) == "<sup") {
              is_sup = true;
              item = buf_lists.at(i + 1);
              i++;
            }
          }

          if (item.contains(str_id) && is_sup) {
            s1 = buf_lists.at(i + 1);
            s2 = buf_lists.at(i + 3);
            s1 = s1.trimmed();
            s2 = s2.trimmed();
            isOk = true;
            break;
          }
        }
        if (isOk) break;
      }
    }

    if (isOk) {
      auto msg = std::make_unique<ShowMessage>(mw_one);
      msg->showMsg(str_id, s1 + " " + s2, 1);
    }

    if (is_sup) return;

    mw_one->m_Reader->initLink(linkFile);
  } else if (linkFile.contains("data:image/")) {
    // open picture
    if (htmlIndex == 0 && !mw_one->m_Reader->isHidden()) {
      return;
    }

    mui->btnDelImage->hide();

    LoadPic* m_LoadPic = new LoadPic(mw_one);
    m_LoadPic->initMain(linkFile);
  }
}

void DocumentHandler::loadBuffer(QString str) {
  QByteArray data = str.toUtf8();

  // QTextCodec *codec = QTextCodec::codecForName("utf-8");
  // emit loaded(codec->toUnicode(data), Qt::AutoText);

  QStringDecoder decoder(QStringDecoder::Utf8);
  QString text = decoder.decode(data);  // 自动处理编码
  emit loaded(text, Qt::AutoText);

  reset();

  emit fileUrlChanged();
}

void DocumentHandler::load(const QUrl& fileUrl) {
  if (fileUrl == m_fileUrl) return;

  const QUrl path = fileUrl;
  const QString fileName = QQmlFile::urlToLocalFileOrQrc(path);

  if (QFile::exists(fileName)) {
    QMimeType mime = QMimeDatabase().mimeTypeForFile(fileName);
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
      QByteArray data = file.readAll();
      if (QTextDocument* doc = textDocument()) {
        doc->setBaseUrl(path.adjusted(QUrl::RemoveFilename));
        if (mime.inherits("text/markdown")) {
          emit loaded(QString::fromUtf8(data), Qt::MarkdownText);
        } else {
          // QTextCodec *codec =
          // QTextCodec::codecForName("utf-8");  // 解决中文乱码
          // emit loaded(codec->toUnicode(data), Qt::AutoText);

          QStringDecoder decoder(QStringDecoder::Utf8);
          QString text = decoder.decode(data);  // 自动处理编码
          emit loaded(text, Qt::AutoText);
        }
        doc->setModified(false);
      }

      reset();
    }
  }

  m_fileUrl = fileUrl;

  emit fileUrlChanged();
}

void DocumentHandler::saveAs(const QUrl& fileUrl) {
  QTextDocument* doc = textDocument();
  if (!doc) return;

  const QString filePath = fileUrl.toLocalFile();
  const bool isHtml =
      QFileInfo(filePath).suffix().contains(QLatin1String("htm"));
  QFile file(filePath);
  if (!file.open(QFile::WriteOnly | QFile::Truncate |
                 (isHtml ? QFile::NotOpen : QFile::Text))) {
    emit error(tr("Cannot save: ") + file.errorString());
    return;
  }
  file.write((isHtml ? doc->toHtml() : doc->toPlainText()).toUtf8());
  file.close();

  if (fileUrl == m_fileUrl) return;

  m_fileUrl = fileUrl;
  emit fileUrlChanged();
}

void DocumentHandler::reset() {
  emit fontFamilyChanged();
  emit alignmentChanged();
  emit boldChanged();
  emit italicChanged();
  emit underlineChanged();
  emit fontSizeChanged();
  emit textColorChanged();
}

QTextCursor DocumentHandler::textCursor() const {
  QTextDocument* doc = textDocument();
  if (!doc) return QTextCursor();

  QTextCursor cursor = QTextCursor(doc);
  if (m_selectionStart != m_selectionEnd) {
    cursor.setPosition(m_selectionStart);
    cursor.setPosition(m_selectionEnd, QTextCursor::KeepAnchor);
  } else {
    cursor.setPosition(m_cursorPosition);
  }
  return cursor;
}

QTextDocument* DocumentHandler::textDocument() const {
  if (!m_document) return nullptr;

  return m_document->textDocument();
}

void DocumentHandler::mergeFormatOnWordOrSelection(
    const QTextCharFormat& format) {
  QTextCursor cursor = textCursor();
  if (!cursor.hasSelection()) cursor.select(QTextCursor::WordUnderCursor);
  cursor.mergeCharFormat(format);
}

bool DocumentHandler::modified() const {
  return m_document && m_document->textDocument()->isModified();
}

void DocumentHandler::setModified(bool m) {
  if (m_document) m_document->textDocument()->setModified(m);
}

void DocumentHandler::setBackDir(QString link) {
  if (link.contains(".html") || link.contains(".xhtml")) {
    if (catalogueFile != mw_one->m_Reader->currentHtmlFile &&
        !link.contains("#")) {
      mw_one->m_Reader->mainDirIndex = htmlIndex;

      mw_one->repaint();
      qDebug() << "mainDirIndex: " << mw_one->m_Reader->mainDirIndex;
    }
  }
}
