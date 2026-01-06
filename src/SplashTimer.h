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
    m_bgColor = QColor(240, 242, 245);
    m_textColor = QColor(52, 73, 94);
    m_highlightColor = QColor(41, 128, 185);
    m_shadowColor = QColor(44, 62, 80, 120);
  }

  void drawBaseBackground() {
    m_basePixmap = QPixmap(m_targetSize);
    m_basePixmap.fill(m_bgColor);

    QPainter painter(&m_basePixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient gradient(0, 0, 0, m_targetSize.height());
    gradient.setColorAt(0, QColor(240, 242, 245));
    gradient.setColorAt(1, QColor(220, 225, 235));
    painter.fillRect(m_basePixmap.rect(), gradient);

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

  // 核心修复1：单字母精准居中（水平+垂直）
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

    QFontMetrics fm(font);
    // 1. 水平居中：使用字符实际宽度，而非字体默认宽度
    int charWidth = fm.horizontalAdvance(letter);
    qreal charX = letterRect.center().x() - charWidth / 2.0;

    // 2. 垂直居中：基于基线精准计算，避免截断+保证居中
    int charAscent = fm.ascent();
    int charDescent = fm.descent();
    // 基线Y坐标 = 字母区域中心Y + (上伸高度 - 下伸高度)/2 → 完美垂直居中
    qreal charBaseY =
        letterRect.center().y() + (charAscent - charDescent) / 2.0;

    // 动画参数（幅度缩小，避免干扰居中视觉）
    qreal scale = 1.0;
    qreal opacity = 1.0;
    if (m_animationTimer) {
      qreal phase = (m_animationFrame + letter.unicode() * 0.5) * 0.1;
      scale = 0.99 + 0.01 * std::sin(phase);  // 极小幅度缩放
      opacity = 0.98 + 0.02 * std::cos(phase);
    }

    painter.save();
    // 变换中心为字母区域中心，避免缩放导致偏移
    painter.translate(letterRect.center());
    painter.scale(scale, scale);
    painter.translate(-letterRect.center());
    painter.setOpacity(opacity);

    // 绘制阴影
    painter.setPen(QPen(m_shadowColor, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawText(QPointF(charX + 2 * m_dpr, charBaseY + 2 * m_dpr), letter);

    // 绘制主文字
    QLinearGradient textGradient(letterRect.topLeft(),
                                 letterRect.bottomRight());
    textGradient.setColorAt(0, m_highlightColor);
    textGradient.setColorAt(1, m_textColor);
    painter.setPen(QPen(textGradient, 1));
    painter.setBrush(textGradient);
    painter.drawText(QPointF(charX, charBaseY), letter);

    painter.restore();
  }

  // 核心修复2：水平排列 KNOT 整体居中
  void drawHorizontalKNOT(QPainter& painter, const QPixmap& displayPix) {
    const QString text = "KNOT";
    int letterCount = text.length();
    QRectF canvasRect = displayPix.rect();

    // 单个字母区域尺寸（留足余量，避免挤压）
    qreal letterWidth = m_fontSize * 2.0;
    qreal letterHeight = m_fontSize * 2.5;
    qreal spacing = letterWidth * 0.5;  // 字母间距

    // 计算文字组总宽度（精准无误差）
    qreal totalWidth = letterCount * letterWidth + (letterCount - 1) * spacing;
    qreal totalHeight = letterHeight;

    // 边界校验：文字组不超出画布 90% 区域
    if (totalWidth > canvasRect.width() * 0.9) {
      qreal scaleRatio = (canvasRect.width() * 0.9) / totalWidth;
      letterWidth *= scaleRatio;
      spacing *= scaleRatio;
      totalWidth = canvasRect.width() * 0.9;
    }

    // 整体居中：起始坐标基于画布中心计算
    qreal startX = canvasRect.center().x() - totalWidth / 2.0;
    qreal startY = canvasRect.center().y() - totalHeight / 2.0;

    // 背景矩形与文字组完全对齐
    painter.setPen(QPen(QColor(255, 255, 255, 100), 2 * m_dpr));
    painter.setBrush(QBrush(QColor(255, 255, 255, 30)));
    painter.drawRoundedRect(startX - 20 * m_dpr, startY - 20 * m_dpr,
                            totalWidth + 40 * m_dpr, totalHeight + 40 * m_dpr,
                            20 * m_dpr, 20 * m_dpr);

    // 逐个绘制字母
    for (int i = 0; i < letterCount; ++i) {
      QRectF letterRect(startX + i * (letterWidth + spacing), startY,
                        letterWidth, letterHeight);
      drawTextLetter(painter, text.at(i), letterRect);
    }
  }

  // 核心修复3：垂直排列 KNOT 整体居中
  void drawVerticalKNOT(QPainter& painter, const QPixmap& displayPix) {
    const QString text = "KNOT";
    int letterCount = text.length();
    QRectF canvasRect = displayPix.rect();

    // 单个字母区域尺寸
    qreal letterWidth = m_fontSize * 2.5;
    qreal letterHeight = m_fontSize * 2.0;
    qreal spacing = letterHeight * 0.5;

    // 计算文字组总高度（精准无误差）
    qreal totalHeight =
        letterCount * letterHeight + (letterCount - 1) * spacing;
    qreal totalWidth = letterWidth;

    // 边界校验
    if (totalHeight > canvasRect.height() * 0.9) {
      qreal scaleRatio = (canvasRect.height() * 0.9) / totalHeight;
      letterHeight *= scaleRatio;
      spacing *= scaleRatio;
      totalHeight = canvasRect.height() * 0.9;
    }

    // 整体居中：起始坐标基于画布中心计算
    qreal startX = canvasRect.center().x() - totalWidth / 2.0;
    qreal startY = canvasRect.center().y() - totalHeight / 2.0;

    // 背景矩形与文字组完全对齐
    painter.setPen(QPen(QColor(255, 255, 255, 100), 2 * m_dpr));
    painter.setBrush(QBrush(QColor(255, 255, 255, 30)));
    painter.drawRoundedRect(startX - 20 * m_dpr, startY - 20 * m_dpr,
                            totalWidth + 40 * m_dpr, totalHeight + 40 * m_dpr,
                            20 * m_dpr, 20 * m_dpr);

    // 逐个绘制字母
    for (int i = 0; i < letterCount; ++i) {
      QRectF letterRect(startX, startY + i * (letterHeight + spacing),
                        letterWidth, letterHeight);
      drawTextLetter(painter, text.at(i), letterRect);
    }
  }
};

#endif  // SPLASHTIMER_H
