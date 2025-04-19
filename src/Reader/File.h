#ifndef QT_HUB_FILE_H
#define QT_HUB_FILE_H

#include <QObject>

class File : public QObject {
  Q_OBJECT
 public:
  File();
  QString m_text;
  qreal m_textPos;
  qreal m_textHeight;
  qreal m_curX;
  QString m_strUri;
  QString m_prog;

  Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
  Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
  Q_PROPERTY(qreal textPos READ textPos WRITE setTextPos NOTIFY textPosChanged)
  Q_PROPERTY(qreal textHeight READ textHeight WRITE setTextHeight NOTIFY
                 textHeightChanged)
  Q_PROPERTY(qreal curX READ curX WRITE setCurX NOTIFY curXChanged)

  Q_PROPERTY(
      QString webEnd READ webEnd WRITE setWebEnd NOTIFY loadWebEndChanged)
  Q_PROPERTY(QString prog READ prog WRITE setProg NOTIFY loadWebEndChanged)

  QString source() const;
  void setSource(const QString &source);

  QString text() const;
  void setText(const QString &text);

  qreal textPos();
  qreal textHeight();
  qreal curX();
  void setTextPos(qreal &textPos);
  void setTextHeight(qreal &textHeight);
  void setCurX(qreal &curX);

  void setWebEnd(QString &strUri);
  QString webEnd();

  QString prog() const;
  void setProg(const QString &prog);

  void setStr(QString);
 signals:
  void sourceChanged();
  void textChanged();
  void textPosChanged();
  void textHeightChanged();
  void curXChanged();
  void loadWebEndChanged();
  void progChanged();

 public slots:
  void readFile();

 private slots:

 private:
  QString m_source;
};

#endif  // FILE_H
