#ifndef SPLASHTIMER_H
#define SPLASHTIMER_H

#include <QApplication>
#include <QCloseEvent>
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
  // 新增 isDark 参数，默认为 false
  explicit SplashTimer(bool isAndroid = false, bool isDark = false,
                       int width = 600, int height = 300,
                       Qt::WindowFlags flags = Qt::WindowFlags())
      : QSplashScreen(QPixmap(), flags),
        m_isAndroid(isAndroid),
        m_isDark(isDark),  // 初始化暗黑模式标记
        m_width(width),
        m_height(height),
        m_animationTimer(nullptr),
        m_contextValid(true) {
    init();
  }

  ~SplashTimer() override {
    stopAnimation();
    if (m_animationTimer) {
      m_animationTimer->deleteLater();
      m_animationTimer = nullptr;
    }
  }

  void stopAnimation() {
    if (m_animationTimer && m_animationTimer->isActive()) {
      m_animationTimer->stop();
      disconnect(m_animationTimer, &QTimer::timeout, this, nullptr);
    }
    m_contextValid = false;
  }

 private:
  bool m_isAndroid;
  bool m_isDark;  // 新增：暗黑模式标记
  int m_width;
  int m_height;
  qreal m_dpr;
  QSize m_targetSize;
  QColor m_bgColor;
  QColor m_textColor;
  QColor m_highlightColor;
  QColor m_shadowColor;
  QColor m_gridColor;  // 新增：网格线颜色独立管理
  QPixmap m_basePixmap;
  qreal m_fontSize;
  const qreal m_maxFontRatio = 0.25;
  int m_animationFrame = 0;
  QTimer* m_animationTimer = nullptr;
  const qreal m_androidYOffset = 10.0;
  bool m_contextValid;

  void init() {
    initSizeAndDpi();
    initStyle();
    drawBaseBackground();
    setFixedSize(m_targetSize / m_dpr);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    if (m_isAndroid) {
      setWindowState(windowState() | Qt::WindowFullScreen);
      setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    }

    m_animationTimer = new QTimer(this);
    m_animationTimer->setInterval(100);
    connect(m_animationTimer, &QTimer::timeout, this, [this]() {
      if (!m_contextValid) return;
      m_animationFrame = (m_animationFrame + 1) % 60;
      updateDisplay();
    });
    m_animationTimer->start();
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

  // 核心修改：根据 isDark 分支设置颜色
  void initStyle() {
    if (m_isDark) {
      m_bgColor = QColor(30, 30, 35);
      m_textColor = QColor(200, 210, 225);
      m_highlightColor = QColor(100, 180, 240);
      m_shadowColor = QColor(0, 0, 0, 160);
      m_gridColor = QColor(255, 255, 255, 18);
    } else {
      m_bgColor = QColor(240, 242, 245);
      m_textColor = QColor(52, 73, 94);
      m_highlightColor = QColor(41, 128, 185);
      m_shadowColor = QColor(44, 62, 80, 120);
      m_gridColor = QColor(200, 205, 215, 30);
    }
  }

  // 核心修改：使用成员变量替代硬编码颜色
  void drawBaseBackground() {
    m_basePixmap = QPixmap(m_targetSize);
    m_basePixmap.fill(m_bgColor);

    QPainter painter(&m_basePixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient gradient(0, 0, 0, m_targetSize.height());
    if (m_isDark) {
      gradient.setColorAt(0, QColor(30, 30, 35));
      gradient.setColorAt(1, QColor(20, 20, 25));
    } else {
      gradient.setColorAt(0, QColor(240, 242, 245));
      gradient.setColorAt(1, QColor(220, 225, 235));
    }
    painter.fillRect(m_basePixmap.rect(), gradient);

    // 使用独立的网格颜色
    painter.setPen(QPen(m_gridColor, 1));
    int gridSize = 20 * m_dpr;
    for (int x = 0; x < m_targetSize.width(); x += gridSize) {
      painter.drawLine(x, 0, x, m_targetSize.height());
    }
    for (int y = 0; y < m_targetSize.height(); y += gridSize) {
      painter.drawLine(0, y, m_targetSize.width(), y);
    }
  }

  void updateDisplay() {
    if (!m_contextValid || m_basePixmap.isNull()) {
      return;
    }

    QPixmap displayPix = m_basePixmap.copy();
    QPainter painter(&displayPix);
    if (!painter.isActive()) {
      return;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    if (m_isAndroid) {
      drawVerticalKNOT(painter, displayPix);
    } else {
      drawHorizontalKNOT(painter, displayPix);
    }

    // drawGlowEffect(painter, displayPix.rect());
    painter.end();

    displayPix.setDevicePixelRatio(m_dpr);
    setPixmap(displayPix);
    update();
  }

  void drawGlowEffect(QPainter& painter, const QRect& rect) {
    QLinearGradient glowGradient(0, 0, 0, rect.height());
    // 暗黑模式下高光更柔和
    QColor glowColor =
        m_isDark ? QColor(100, 180, 240, 15) : QColor(41, 128, 185, 20);
    glowGradient.setColorAt(0, glowColor);
    glowGradient.setColorAt(0.5, glowColor.lighter(120));
    glowGradient.setColorAt(1, glowColor);

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
    if (m_animationTimer && m_animationTimer->isActive()) {
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

    // 阴影使用 m_shadowColor（已在 initStyle 中适配）
    painter.setPen(QPen(m_shadowColor, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawText(QPointF(charX + 2 * m_dpr, charBaseY + 2 * m_dpr), letter);

    // 文字渐变使用 m_highlightColor 和 m_textColor（已在 initStyle 中适配）
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

    qreal startX = canvasRect.center().x() - totalWidth / 2.0;
    qreal startY = canvasRect.center().y() - totalHeight / 2.0 -
                   (m_androidYOffset * m_dpr);

    for (int i = 0; i < letterCount; ++i) {
      QRectF letterRect(startX, startY + i * (letterHeight + spacing),
                        letterWidth, letterHeight);
      drawTextLetter(painter, text.at(i), letterRect);
    }
  }

  void closeEvent(QCloseEvent* event) override {
    stopAnimation();
    m_contextValid = false;
    QSplashScreen::closeEvent(event);
  }
};

#endif  // SPLASHTIMER_H