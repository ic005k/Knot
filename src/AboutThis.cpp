#include "src/AboutThis.h".h "

#include "MainWindow.h"
#include "ui_AboutThis.h"
#include "ui_MainWindow.h"

extern MainWindow *mw_one;
extern Method *m_Method;
extern bool loading, zh_cn, isDark;
extern QString noteText, appName, ver;
extern int curPos;

AboutThis::AboutThis(QWidget *parent) : QDialog(parent), ui(new Ui::AboutThis) {
  ui->setupUi(this);
  setWindowFlag(Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  if (isDark)
    ui->frame->setStyleSheet(
        "#frame{background-color:#19232D;border-radius:10px; "
        "border:1px solid gray;}");
  else
    ui->frame->setStyleSheet(
        "#frame{background-color:rgb(250,250,250);border-radius:10px; "
        "border:1px solid gray;}");
  this->layout()->setContentsMargins(5, 5, 5, 5);

  mw_one->set_ToolButtonStyle(this);

  QFont font = this->font();
  font.setPointSize(13);
  ui->lblTip->setFont(font);

  setModal(true);

  this->installEventFilter(this);
  ui->lblLogo->installEventFilter(this);

  ui->btnDownloadUP->hide();
  ui->lblTip->hide();

  ui->lblAbout->adjustSize();
  ui->lblAbout->setWordWrap(true);
  ui->lblLogo->adjustSize();
  ui->lblLogo->setText("");
  ui->lblLogo->setFixedHeight(185);
  ui->lblLogo->setFixedWidth(185);
  ui->lblLogo->setStyleSheet(
      "QLabel{"
      "border-image:url(:/res/apk.png) 4 4 4 4 stretch stretch;"
      "}");

  manager = new QNetworkAccessManager(this);
  connect(manager, SIGNAL(finished(QNetworkReply *)), this,
          SLOT(replyFinished(QNetworkReply *)));
}

AboutThis::~AboutThis() { delete ui; }

bool AboutThis::eventFilter(QObject *obj, QEvent *evn) {
  QMouseEvent *event = static_cast<QMouseEvent *>(evn);
  if (obj == ui->lblLogo) {
    if (event->type() == QEvent::MouseButtonDblClick) {
      if (s_link == "") return true;
      QClipboard *pClip = QApplication::clipboard();
      pClip->setText(s_link);
      ShowMessage *msg = new ShowMessage(this);
      msg->showMsg("Knot", tr("Download link copied.") + "\n\n" + s_link, 1);
    }
  }

  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnBack_About_clicked();
      return true;
    }
  }

  return QWidget::eventFilter(obj, evn);
}

void AboutThis::keyReleaseEvent(QKeyEvent *event) { Q_UNUSED(event); }

void AboutThis::resizeEvent(QResizeEvent *event) { Q_UNUSED(event); }

void AboutThis::on_btnHomePage_clicked() {
  QString str;
  str = "https://github.com/ic005k/Knot/issues";
  QUrl url(str);
  QDesktopServices::openUrl(url);
}

void AboutThis::CheckUpdate() {
  QNetworkRequest quest;
  quest.setUrl(QUrl("https://api.github.com/repos/ic005k/" + appName +
                    "/releases/latest"));
  quest.setHeader(QNetworkRequest::UserAgentHeader, "RT-Thread ART");
  manager->get(quest);
}

void AboutThis::replyFinished(QNetworkReply *reply) {
  QString str = reply->readAll();
  parse_UpdateJSON(str);
  reply->deleteLater();
}

QString AboutThis::getUrl(QVariantList list) {
  QString androidUrl, macUrl, winUrl, linuxUrl;
  for (int i = 0; i < list.count(); i++) {
    QVariantMap map = list[i].toMap();
    QString fName = map["name"].toString();

    if (fName.contains("android"))
      androidUrl = map["browser_download_url"].toString();
    if (fName.contains("Mac")) macUrl = map["browser_download_url"].toString();
    if (fName.contains("Win")) winUrl = map["browser_download_url"].toString();
    if (fName.contains("Linux"))
      linuxUrl = map["browser_download_url"].toString();
  }

#ifdef Q_OS_ANDROID
  return androidUrl;
#endif

#ifdef Q_OS_MAC
  return macUrl;
#endif

#ifdef Q_OS_WIN
  return winUrl;
#endif

  return linuxUrl;
}

int AboutThis::parse_UpdateJSON(QString str) {
  QJsonParseError err_rpt;
  QJsonDocument root_Doc = QJsonDocument::fromJson(str.toUtf8(), &err_rpt);

  if (err_rpt.error != QJsonParseError::NoError) {
    if (!blAutoCheckUpdate) {
      ShowMessage *m_ShowMsg = new ShowMessage(this);
      m_ShowMsg->showMsg(appName, tr("Network error!"), 1);
    }
    blAutoCheckUpdate = false;
    return -1;
  }
  if (root_Doc.isObject()) {
    QJsonObject root_Obj = root_Doc.object();

    QVariantList list = root_Obj.value("assets").toArray().toVariantList();
    QString Url = getUrl(list);
    if (zh_cn) {
#ifdef Q_OS_ANDROID
      // gitee
      s_link =
          "https://gitee.com/ic005k/Knot/releases/download/Latest/"
          "android-build-release-signed.apk";
#else
      // github
      QString mirror0 = "";
      QString mirror1 = "https://ghproxy.com/";
      QString mirror2 = "https://gh.flyinbug.top/gh/";

      s_link = mirror0 + Url;
#endif
    } else
      s_link = Url;

    qDebug() << "s_link = " << s_link << Url;
    ui->btnDownloadUP->show();
    ui->lblTip->show();

    QString Verison = root_Obj.value("tag_name").toString();
    QString UpdateTime = root_Obj.value("published_at").toString();
    QString ReleaseNote = root_Obj.value("body").toString();

    if (Verison > ver && Url != "") {
      QString warningStr = tr("New version detected!") + "\n" +
                           tr("Version: ") + "V" + Verison + "\n" +
                           tr("Published at: ") + UpdateTime + "\n" +
                           tr("Release Notes: ") + "\n" + ReleaseNote;

      // int ret = QMessageBox::warning(this, "", warningStr, tr("Download"),
      //                                tr("Cancel"));
      // for Android ret = 3 Mac ret = 0 or 1(Cancel)

      ShowMessage *m_ShowMsg = new ShowMessage(this);
      int ret = m_ShowMsg->showMsg("Knot", warningStr, 1);

      if (ret >= 0) {
#ifdef Q_OS_ANDROID
        show_download();
#else
        // const QUrl url("https://github.com/ic005k/" + appName +
        //                "/releases/latest");

        QDesktopServices::openUrl(QUrl(s_link));

        // QApplication::exit(0);
#endif
      }
    } else {
      if (!blAutoCheckUpdate) {
        ShowMessage *m_ShowMsg = new ShowMessage(this);
        m_ShowMsg->showMsg(tr("Upgrade Check"),
                           tr("You are currently using the latest version!"),
                           1);
      }
    }
  }
  blAutoCheckUpdate = false;
  return 0;
}

void AboutThis::show_download() {
  int aver = getAndroidVer();
  // Android7.0及以上
  if (aver >= 24) {
    m_AutoUpdate = new AutoUpdate(this);
    int y = (mw_one->height() - m_AutoUpdate->height()) / 2;
    m_AutoUpdate->setGeometry(mw_one->geometry().x(), y, mw_one->width(),
                              m_AutoUpdate->height());

    m_Method->showGrayWindows();
    m_AutoUpdate->show();
    m_AutoUpdate->startDownload(s_link);
    qDebug() << "start dl..... " << s_link;
    this->close();
  } else {
    // Android6.0及以下通过浏览器下载
    QUrl url(s_link);
    QDesktopServices::openUrl(url);
  }
}

void AboutThis::on_btnCheckUpdate_clicked() { CheckUpdate(); }

void AboutThis::on_btnDownloadUP_clicked() {
  mw_one->m_Reader->setPdfViewVisible(false);
  if (s_link == "") return;

#ifdef Q_OS_ANDROID
  show_download();
#else
  // const QUrl url("https://github.com/ic005k/" + appName +
  // "/releases/latest");

  QDesktopServices::openUrl(QUrl(s_link));
#endif
}

int AboutThis::getAndroidVer() {
  int a = 0;
#ifdef Q_OS_UNIX
  a = 24;
#endif

#ifdef Q_OS_WIN
  a = 24;
#endif

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject jo = QAndroidJniObject::fromString("ver");
  a = jo.callStaticMethod<int>("com.x/MyActivity", "getAndroidVer", "()I");
#else
  QJniObject jo = QJniObject::fromString("ver");
  a = jo.callStaticMethod<int>("com.x/MyActivity", "getAndroidVer", "()I");
#endif

#endif
  return a;
}

void AboutThis::on_btnBack_About_clicked() { close(); }
