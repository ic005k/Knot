#ifndef SPLASHTIMER_H
#define SPLASHTIMER_H

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QSplashScreen>
#include <QTimer>
#include <cmath>

class SplashTimer : public QSplashScreen {
  Q_OBJECT

 public:
  explicit SplashTimer(bool isAndroid = false, int width = 600,
                       int height = 300,
                       Qt::WindowFlags flags = Qt::WindowFlags())
      : QSplashScreen(QPixmap(), flags),
        m_isAndroid(isAndroid),
        m_width(width),
        m_height(height) {
    init();
  }

  ~SplashTimer() override {}

 private:
  bool m_isAndroid;
  int m_width;
  int m_height;
  qreal m_dpr;
  QSize m_targetSize;
  QColor m_bgColor;
  QColor m_textColor;
  QColor m_highlightColor;
  QColor m_shadowColor;
  QPixmap m_basePixmap;
  qreal m_fontSize;
  const qreal m_maxFontRatio = 0.25;
  int m_animationFrame = 0;
  QTimer* m_animationTimer = nullptr;
  // 安卓垂直偏移修正值（可根据设备微调）
  const qreal m_androidYOffset = 10.0;

  void init() {
    initSizeAndDpi();
    initStyle();
    drawBaseBackground();  // 恢复整体背景绘制（渐变+网格）
    setFixedSize(m_targetSize / m_dpr);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    if (m_isAndroid) {
      setWindowState(windowState() | Qt::WindowFullScreen);
      setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    }

    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, [this]() {
      m_animationFrame = (m_animationFrame + 1) % 60;
      updateDisplay();
    });
    m_animationTimer->start(100);
    updateDisplay();
  }

  void initSizeAndDpi() {
    m_dpr = qApp->primaryScreen()->devicePixelRatio();

    if (m_isAndroid) {
      QScreen* screen = QGuiApplication::primaryScreen();
      if (screen) {
        QRect screenGeometry = screen->geometry();
        m_targetSize = screenGeometry.size() * m_dpr;
      } else {
        m_targetSize = QSize(800 * m_dpr, 1280 * m_dpr);
      }
    } else {
      m_targetSize = QSize(m_width * m_dpr, m_height * m_dpr);
    }

    qreal windowMinSize = qMin(m_targetSize.width(), m_targetSize.height());
    m_fontSize = windowMinSize * m_maxFontRatio;
    m_fontSize = qMax(m_fontSize, 40.0 * m_dpr);
  }

  void initStyle() {
    // 恢复整体背景色
    m_bgColor = QColor(240, 242, 245);
    // 文字配色适配整体背景
    m_textColor = QColor(52, 73, 94);
    m_highlightColor = QColor(41, 128, 185);
    m_shadowColor = QColor(44, 62, 80, 120);
  }

  // 恢复整体渐变+网格背景绘制（仅去掉文字下的圆角背景）
  void drawBaseBackground() {
    m_basePixmap = QPixmap(m_targetSize);
    m_basePixmap.fill(m_bgColor);  // 填充整体背景色，避免黑条

    QPainter painter(&m_basePixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 渐变背景
    QLinearGradient gradient(0, 0, 0, m_targetSize.height());
    gradient.setColorAt(0, QColor(240, 242, 245));
    gradient.setColorAt(1, QColor(220, 225, 235));
    painter.fillRect(m_basePixmap.rect(), gradient);

    // 微妙网格背景
    painter.setPen(QPen(QColor(200, 205, 215, 30), 1));
    int gridSize = 20 * m_dpr;
    for (int x = 0; x < m_targetSize.width(); x += gridSize) {
      painter.drawLine(x, 0, x, m_targetSize.height());
    }
    for (int y = 0; y < m_targetSize.height(); y += gridSize) {
      painter.drawLine(0, y, m_targetSize.width(), y);
    }
  }

  void updateDisplay() {
    QPixmap displayPix = m_basePixmap.copy();
    QPainter painter(&displayPix);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    if (m_isAndroid) {
      drawVerticalKNOT(painter, displayPix);
    } else {
      drawHorizontalKNOT(painter, displayPix);
    }

    drawGlowEffect(painter, displayPix.rect());
    painter.end();

    displayPix.setDevicePixelRatio(m_dpr);
    setPixmap(displayPix);
    repaint();
  }

  void drawGlowEffect(QPainter& painter, const QRect& rect) {
    QLinearGradient glowGradient(0, 0, 0, rect.height());
    glowGradient.setColorAt(0, QColor(41, 128, 185, 20));
    glowGradient.setColorAt(0.5, QColor(41, 128, 185, 5));
    glowGradient.setColorAt(1, QColor(41, 128, 185, 20));

    painter.setPen(Qt::NoPen);
    painter.setBrush(glowGradient);
    qreal radiusX = rect.width() * 0.4;
    qreal radiusY = rect.height() * 0.1;
    painter.drawEllipse(QPointF(rect.center()), radiusX, radiusY);
  }

  void drawTextLetter(QPainter& painter, QChar letter,
                      const QRectF& letterRect) {
    QFont font;
    if (m_isAndroid) {
      font.setFamilies({"Roboto", "Arial", "Sans Serif"});
    } else {
      font.setFamilies({"Sans Serif", "Arial"});
    }
    font.setBold(true);
    font.setPixelSize(m_fontSize);
    painter.setFont(font);

    qreal scale = 1.0;
    qreal opacity = 1.0;
    if (m_animationTimer) {
      qreal phase = (m_animationFrame + letter.unicode() * 0.5) * 0.1;
      scale = 0.99 + 0.01 * std::sin(phase);
      opacity = 0.95 + 0.05 * std::cos(phase);
    }

    QFontMetrics fm(font);
    int charWidth = fm.horizontalAdvance(letter);
    int charAscent = fm.ascent();
    int charDescent = fm.descent();

    qreal charX = letterRect.center().x() - charWidth / 2.0;
    qreal charBaseY =
        letterRect.center().y() + (charAscent - charDescent) / 2.0;

    painter.save();
    painter.translate(letterRect.center());
    painter.scale(scale, scale);
    painter.translate(-letterRect.center());
    painter.setOpacity(opacity);

    // 文字阴影（适配整体背景，强度适中）
    painter.setPen(QPen(m_shadowColor, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawText(QPointF(charX + 2 * m_dpr, charBaseY + 2 * m_dpr), letter);

    // 渐变文字
    QLinearGradient textGradient(letterRect.topLeft(),
                                 letterRect.bottomRight());
    textGradient.setColorAt(0, m_highlightColor);
    textGradient.setColorAt(1, m_textColor);
    painter.setPen(QPen(textGradient, 1));
    painter.setBrush(textGradient);
    painter.drawText(QPointF(charX, charBaseY), letter);

    painter.restore();
  }

  void drawHorizontalKNOT(QPainter& painter, const QPixmap& displayPix) {
    const QString text = "KNOT";
    int letterCount = text.length();
    QRectF canvasRect = displayPix.rect();

    qreal letterWidth = m_fontSize * 2.0;
    qreal letterHeight = m_fontSize * 2.5;
    qreal spacing = letterWidth * 0.5;
    qreal totalWidth = letterCount * letterWidth + (letterCount - 1) * spacing;
    qreal totalHeight = letterHeight;

    if (totalWidth > canvasRect.width() * 0.9) {
      qreal scaleRatio = (canvasRect.width() * 0.9) / totalWidth;
      letterWidth *= scaleRatio;
      spacing *= scaleRatio;
      totalWidth = canvasRect.width() * 0.9;
    }

    qreal startX = canvasRect.center().x() - totalWidth / 2.0;
    qreal startY = canvasRect.center().y() - totalHeight / 2.0;

    // 移除文字下方的圆角背景绘制（仅删除这部分代码）
    // 直接绘制文字，无圆角背景
    for (int i = 0; i < letterCount; ++i) {
      QRectF letterRect(startX + i * (letterWidth + spacing), startY,
                        letterWidth, letterHeight);
      drawTextLetter(painter, text.at(i), letterRect);
    }
  }

  void drawVerticalKNOT(QPainter& painter, const QPixmap& displayPix) {
    const QString text = "KNOT";
    int letterCount = text.length();
    QRectF canvasRect = displayPix.rect();

    qreal letterWidth = m_fontSize * 2.5;
    qreal letterHeight = m_fontSize * 2.0;
    qreal spacing = letterHeight * 0.5;
    qreal totalHeight =
        letterCount * letterHeight + (letterCount - 1) * spacing;
    qreal totalWidth = letterWidth;

    if (totalHeight > canvasRect.height() * 0.9) {
      qreal scaleRatio = (canvasRect.height() * 0.9) / totalHeight;
      letterHeight *= scaleRatio;
      spacing *= scaleRatio;
      totalHeight = canvasRect.height() * 0.9;
    }

    // 安卓端垂直偏移修正，解决整体靠下问题
    qreal startX = canvasRect.center().x() - totalWidth / 2.0;
    qreal startY = canvasRect.center().y() - totalHeight / 2.0 -
                   (m_androidYOffset * m_dpr);

    // 移除文字下方的圆角背景绘制（仅删除这部分代码）
    // 直接绘制文字，无圆角背景
    for (int i = 0; i < letterCount; ++i) {
      QRectF letterRect(startX, startY + i * (letterHeight + spacing),
                        letterWidth, letterHeight);
      drawTextLetter(painter, text.at(i), letterRect);
    }
  }
};

#endif  // SPLASHTIMER_H
