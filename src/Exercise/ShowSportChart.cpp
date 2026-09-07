#include "defines.h"
#include "src/Exercise/Steps.h"

/**
 * @brief 从 INI 文件中读取指定年月的运动统计数据
 * @param year  年份字符串，如 "2026"
 * @param month 月份 (1-12)，传 0 表示读取全年12个月数据
 * @return QVector<MonthData>
 *         - month > 0: 返回仅含1个元素的向量（对应当月）
 *         - month == 0: 返回含12个元素的向量（索引0-11对应1-12月）
 *         - 读取失败或无数据时返回空向量或零值向量
 */
QVector<MonthData> Steps::loadSportsData(const QString& year, int month) const {
  QVector<MonthData> result;

  // 参数校验
  if (year.isEmpty()) {
    qWarning() << "[loadSportsData] Year parameter is empty";
    return result;
  }

  const bool loadSingleMonth = (month >= 1 && month <= 12);
  const bool loadAllMonths = (month == 0);

  if (!loadSingleMonth && !loadAllMonths) {
    qWarning() << "[loadSportsData] Invalid month:" << month
               << "(expected 0 for all, or 1-12)";
    return result;
  }

  // 读取 INI 文件
  const QString iniPath = iniDir + year + "-gpslist.ini";
  QSettings reg(iniPath, QSettings::IniFormat);

  if (!QFile::exists(iniPath)) {
    qWarning() << "[loadSportsData] INI file not found:" << iniPath;
    return loadAllMonths ? QVector<MonthData>(12) : result;
  }

  // 确定要读取的月份范围
  const int startMonth = loadSingleMonth ? month : 1;
  const int endMonth = loadSingleMonth ? month : 12;

  // 预分配空间
  result.resize(endMonth - startMonth + 1);

  reg.beginGroup(year);
  for (int m = startMonth; m <= endMonth; ++m) {
    const QString value = reg.value(QString::number(m)).toString();
    if (value.isEmpty()) {
      continue;  // 保持默认零值
    }

    const QStringList parts = value.split("-=-");
    if (parts.size() != 8) {
      qWarning() << "[loadSportsData] Month" << m
                 << "data format invalid (expected 8 fields):" << value;
      continue;
    }

    // 计算结果向量中的索引
    const int idx = m - startMonth;
    MonthData& data = result[idx];
    data.cyclingDist = parts[2].toDouble();
    data.cyclingCount = parts[3].toInt();
    data.hikingDist = parts[4].toDouble();
    data.hikingCount = parts[5].toInt();
    data.runningDist = parts[6].toDouble();
    data.runningCount = parts[7].toInt();
  }
  reg.endGroup();

  return result;

  for (int i = 0; i < result.size(); ++i) {
    const auto& d = result[i];
    qDebug() << QString(
                    "Month %1: cycle=%2km/%3次, hike=%4km/%5次, run=%6km/%7次")
                    .arg(i + 1)
                    .arg(d.cyclingDist)
                    .arg(d.cyclingCount)
                    .arg(d.hikingDist)
                    .arg(d.hikingCount)
                    .arg(d.runningDist)
                    .arg(d.runningCount);
  }
}

QStringList Steps::getSportsDataSummary(const QVector<MonthData>& data,
                                        const QString& year) {
  QStringList list;
  if (data.isEmpty()) return list;

  // 1. 计算全年汇总
  double tCD = 0, tHD = 0, tRD = 0;
  int tCC = 0, tHC = 0, tRC = 0, activeMonths = 0;
  for (const auto& d : data) {
    tCD += d.cyclingDist;
    tCC += d.cyclingCount;
    tHD += d.hikingDist;
    tHC += d.hikingCount;
    tRD += d.runningDist;
    tRC += d.runningCount;
    if (d.totalDistance() > 0 || d.totalCount() > 0) ++activeMonths;
  }

  // 📅 全年总览
  list << QString("📅 %1 | ✅ %2/12").arg(year).arg(activeMonths, 2);
  list << QString("🏁 %1 km / %2")
              .arg(tCD + tHD + tRD, 8, 'f', 2)
              .arg(tCC + tHC + tRC, 4);

  // 🚴🥾🏃 分类汇总
  list << QString("🚴 %1 km / %2").arg(tCD, 8, 'f', 2).arg(tCC, 4);
  list << QString("🥾 %1 km / %2").arg(tHD, 8, 'f', 2).arg(tHC, 4);
  list << QString("🏃 %1 km / %2").arg(tRD, 8, 'f', 2).arg(tRC, 4);

  // 📆 各月明细 (每项独立成行，带 km 单位)
  for (int i = 0; i < data.size(); ++i) {
    const auto& d = data[i];
    if (d.totalDistance() == 0 && d.totalCount() == 0) continue;

    list << QString("%2M").arg(i + 1, 2);
    list << QString("  🚴 %1 km / %2")
                .arg(d.cyclingDist, 7, 'f', 1)
                .arg(d.cyclingCount, 3);
    list << QString("  🥾 %1 km / %2")
                .arg(d.hikingDist, 7, 'f', 1)
                .arg(d.hikingCount, 3);
    list << QString("  🏃 %1 km / %2")
                .arg(d.runningDist, 7, 'f', 1)
                .arg(d.runningCount, 3);
  }

  return list;
}

void Steps::getMySportData(QString strYear, int m) {
  QVector<MonthData> result = loadSportsData(strYear, m);
  QStringList list = getSportsDataSummary(result, strYear);
  m_Method->refreshJavaData("showSummaryDialog", "SportChartActivity", list);
}