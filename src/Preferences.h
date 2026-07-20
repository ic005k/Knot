#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include <QFile>
#include <QFontDatabase>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStyleHints>
#include <QTimer>
#include <QToolButton>
#include <memory>

#include "src/Comm/ShowMessage.h"
#include "src/Comm/TextEditToolbar.h"

struct AiSingleRecord {
  QString endpoint;
  QString apiKey;
  QString modelId;
  double temperature;
  int timeoutSec;
  int maxTokens;

  // 下拉展示字符串，无中文厂商名
  QString displayText() const {
    QString keySuffix = apiKey.size() > 6 ? apiKey.right(6) : apiKey;
    return QString("%1 || %2 | %3").arg(endpoint, modelId, keySuffix);
  }

  // 三元唯一标识，用来判断当前整套配置是否已存在
  QString uniqueKey() const {
    return endpoint + "|||" + apiKey + "|||" + modelId;
  }
};

namespace Ui {
class Preferences;
}

class Preferences : public QDialog {
  Q_OBJECT

 public:
  explicit Preferences(QWidget* parent = nullptr);
  ~Preferences();
  Ui::Preferences* ui;

  bool isTestBtnClicked = false;
  void saveAIConfig();
  bool devMode = false;
  QString chkStyle;
  bool isFontChange = false;
  void saveOptions();

  void initOptions();

  void setBakStatus(bool status);
  bool getBakStatus();
  void setLatestAction(QString action);

  void appendBakFile(QString action, QString bakfile);

  QStringList getBakFilesList();

  void initCheckStatus();

  bool isOverUIFont();
  bool isOverReaderFont();
  void setDefaultFont(QString fontFamily);
  QString getDefaultFont();

  QString setFontDemoUI(QString customFontPath, QToolButton* btn, int fontSize);

  void setEncSyncStatusTip();

  void openPreferences();

  void on_btnShowPassword_pressed();

  void on_btnShowPassword_released();

  void on_btnShowValidate_pressed();

  void on_btnShowValidate_released();

  void on_editPassword_textChanged(const QString& arg1);

  void on_editValidate_textChanged(const QString& arg1);

  void on_chkZip_clicked();

  void initLocalModelList();

 protected:
  void keyReleaseEvent(QKeyEvent* event) override;

  bool eventFilter(QObject* watch, QEvent* evn) override;
  void closeEvent(QCloseEvent* event) override;

 public slots:

 private slots:
  void on_btnBack_clicked();

  void on_sliderFontSize_sliderMoved(int position);

  void on_btnCustomFont_clicked();

  void on_chkUIFont_clicked();

  void on_sliderFontSize_valueChanged(int value);

  void on_btnReStart_clicked();

  void on_chkDark_clicked(bool checked);

  void on_chkDark_clicked();

  void on_chkUIFont_clicked(bool checked);

  void on_btnAISelect_clicked();

  void on_btnAITest_clicked();

  void on_cboxEndpoint_currentIndexChanged(int index);

  void on_cboxEndpoint_activated(int index);

  void on_btnDownloadModel_clicked();

  void on_cboxModel_currentIndexChanged(int index);

  void on_cboxModel_currentTextChanged(const QString& arg1);

 private:
  bool isChanged;

  EditEventFilter* editFilter = nullptr;

  QString iniBakFiles = "BakFiles.ini";
  void getCheckStatusChange();
  QString orgCustomFontText;
  QList<int> listCheckStatus;
  int readerFontID = 0;
  int uiFontID = 0;

  QFont::Weight uiFontWeight;

  QByteArray aes_key0 = "MySuperSecretKey1234567890";  // 长度不足32会自动处理
  QByteArray aes_iv0 = "InitializationVe";

  void initAIConfig();
  const QString ai_config_json = "ai_config.json";
  QStringList m_endpointCache;
  QList<AiSingleRecord> m_aiAllRecords;
};

#endif  // PREFERENCES_H
