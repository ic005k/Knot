import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Fusion

Rectangle {
    id: root

    color: isDark ? "#19232D" : "white"

    property int itemCount: 0
    property bool isHighPriority: false
    property string strGpsTime: ""
    property string strTitleColor: "lightgray"
    property bool isShowRoute: true // 补充缺失的属性

    // 新增：获取设备像素比（安卓/iOS关键）
    property real pixelRatio: Screen.pixelRatio > 0 ? Screen.pixelRatio : 1
    property real baseFontSize: (Qt.platform.os
                                 === "android") ? (20 * pixelRatio) : (9 * pixelRatio)

    function showRouteDialog() {
        routeDialog.visible = true
    }

    function closeRouteDialog() {
        routeDialog.visible = false
    }

    function setItemHeight(h) {}

    function getGpsList() {
        return strGpsTime
    }

    function gotoEnd() {
        view.positionViewAtEnd()
    }

    function gotoBeginning() {
        view.positionViewAtBeginning()
    }

    function gotoIndex(index) {
        view.positionViewAtIndex(index, ListView.Center) // 改为ListView.Center
    }

    function setHighPriority(isFalse) {
        isHighPriority = isFalse
    }

    function setCurrentItem(currentIndex) {
        view.currentIndex = currentIndex
    }

    function getCurrentIndex() {
        return view.currentIndex
    }

    function getItemCount() {
        itemCount = view.count
        return itemCount
    }

    function getItemText(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.time + "|=|" + data.dototext
    }

    function getText0(itemIndex) {
        var existingItem = view.model.get(itemIndex)
        return existingItem.text0
    }

    function getText1(itemIndex) {
        var existingItem = view.model.get(itemIndex)
        return existingItem.text1
    }

    function getText2(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.text2
    }

    function getText3(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.text3
    }

    function getTop(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.text_top
    }

    function getType(itemIndex) {
        var data = view.model.get(itemIndex)
        return data.type
    }

    // 绘制绿-黄-橙-红色带（相对映射算法）
    function drawColorRibbon(ctx, speedData, canvasWidth, canvasHeight) {
        const vMin = Math.min(...speedData)
        const vMax = Math.max(...speedData)
        const vRange = vMax - vMin

        if (vRange <= 0) {
            ctx.fillStyle = "#FFFF00"
            ctx.fillRect(0, 0, canvasWidth, canvasHeight)
            return
        }

        const pointCount = speedData.length
        const segmentWidth = canvasWidth / (pointCount - 1)

        speedData.forEach((speed, index) => {
                              const ratio = (speed - vMin) / vRange
                              const hue = 120 - (ratio * 120)
                              const rgb = hsvToRgb(hue, 0.8, 0.9)
                              ctx.fillStyle = `rgb(${rgb.r}, ${rgb.g}, ${rgb.b})`
                              const x = index * segmentWidth // 修复：补充x变量定义
                              ctx.fillRect(x, 0, segmentWidth, canvasHeight)
                          })
    }

    function drawSpeedSpectrum_old(ctx, speedData, canvasWidth, canvasHeight) {
        if (speedData.length < 2) {
            ctx.fillStyle = "rgba(150, 150, 150, 0.3)"
            ctx.fillRect(0, 0, canvasWidth, canvasHeight)
            return
        }

        ctx.clearRect(0, 0, canvasWidth, canvasHeight)

        const vMin = Math.min(...speedData)
        const vMax = Math.max(...speedData)
        const vRange = vMax - vMin
        const isUniform = vRange <= 0.1

        const pointCount = speedData.length
        const points = []
        const segmentWidth = canvasWidth / (pointCount - 1)

        speedData.forEach((speed, index) => {
                              const ratio = isUniform ? 0.5 : (speed - vMin) / vRange
                              const x = index * segmentWidth
                              const y = canvasHeight - (ratio * canvasHeight)
                              points.push({
                                              "x": x,
                                              "y": y,
                                              "ratio": ratio
                                          })
                          })

        ctx.beginPath()
        ctx.moveTo(0, canvasHeight)
        for (var i = 0; i < points.length; i++) {
            const p = points[i]
            if (i === 0) {
                ctx.lineTo(p.x, p.y)
            } else {
                const prev = points[i - 1]
                const controlX = (prev.x + p.x) / 2
                ctx.quadraticCurveTo(controlX, (prev.y + p.y) / 2, p.x, p.y)
            }
        }
        ctx.lineTo(canvasWidth, canvasHeight)
        ctx.closePath()

        const gradient = ctx.createLinearGradient(0, 0, canvasWidth, 0)
        points.forEach((p, i) => {
                           const pos = i / (points.length - 1)
                           const hue = 240 - (p.ratio * 240)
                           const rgb = hsvToRgb(hue, 0.85, 0.85)
                           gradient.addColorStop(
                               pos, `rgb(${rgb.r}, ${rgb.g}, ${rgb.b})`)
                       })
        ctx.fillStyle = gradient
        ctx.fill()

        ctx.strokeStyle = "rgba(255, 255, 255, 0.6)"
        ctx.lineWidth = 1.2
        ctx.stroke()
    }

    // 融合降采样的速度曲线绘制函数
    function drawSpeedSpectrum(ctx, speedData, canvasWidth, canvasHeight) {
        if (speedData.length < 2) {
            ctx.fillStyle = "rgba(150, 150, 150, 0.3)"
            ctx.fillRect(0, 0, canvasWidth, canvasHeight)
            return
        }

        ctx.clearRect(0, 0, canvasWidth, canvasHeight)

        // 1. 基于原始数据计算极值（必须用原始数据，避免降采样导致isUniform判断错误）
        const vMin = Math.min(...speedData)
        const vMax = Math.max(...speedData)
        const vRange = vMax - vMin
        const isUniform = vRange <= 0.1

        const pointCount = speedData.length
        const segmentWidth = canvasWidth / (pointCount - 1)

        // 2. 生成原始坐标点（保留speed/ratio，供降采样用）
        const originalPoints = []
        speedData.forEach((speed, index) => {
                              const ratio = isUniform ? 0.5 : (speed - vMin) / vRange
                              const x = index * segmentWidth
                              const y = canvasHeight - (ratio * canvasHeight)
                              originalPoints.push({
                                                      "x": x,
                                                      "y": y,
                                                      "ratio": ratio,
                                                      "speed": speed // 保留原始速度值，供后续扩展（比如标注极值）
                                                  })
                          })

        // 3. 核心：降采样逻辑（复用已定义的算法，和海拔曲线保持一致）
        const MAX_DRAW_POINTS = 200
        let drawPoints = originalPoints

        if (originalPoints.length > MAX_DRAW_POINTS) {
            const epsilon = canvasWidth / 200
            drawPoints = douglasPeucker(originalPoints, epsilon)
            if (drawPoints.length > MAX_DRAW_POINTS) {
                drawPoints = intervalSample(drawPoints, MAX_DRAW_POINTS)
            }
        }

        // 4. 绘制速度曲线（全部改用drawPoints）
        ctx.beginPath()
        ctx.moveTo(0, canvasHeight)
        for (var i = 0; i < drawPoints.length; i++) {
            const p = drawPoints[i]
            if (i === 0) {
                ctx.lineTo(p.x, p.y)
            } else {
                const prev = drawPoints[i - 1]
                const controlX = (prev.x + p.x) / 2
                ctx.quadraticCurveTo(controlX, (prev.y + p.y) / 2, p.x, p.y)
            }
        }
        ctx.lineTo(canvasWidth, canvasHeight)
        ctx.closePath()

        // 5. 生成渐变（基于降采样后的点，保证渐变和曲线匹配）
        const gradient = ctx.createLinearGradient(0, 0, canvasWidth, 0)
        drawPoints.forEach((p, i) => {
                               const pos = i / (drawPoints.length - 1)
                               const hue = 240 - (p.ratio * 240)
                               const rgb = hsvToRgb(hue, 0.85, 0.85)
                               gradient.addColorStop(
                                   pos, `rgb(${rgb.r}, ${rgb.g}, ${rgb.b})`)
                           })
        ctx.fillStyle = gradient
        ctx.fill()

        // 6. 绘制轮廓线
        ctx.strokeStyle = "rgba(255, 255, 255, 0.6)"
        ctx.lineWidth = 1.2
        ctx.stroke()

        // 可选：标注最大/最小速度
        let maxSpeed = speedData[0], maxSpeedIdx = 0
        let minSpeed = speedData[0], minSpeedIdx = 0
        speedData.forEach((speed, idx) => {
                              if (speed > maxSpeed) {
                                  maxSpeed = speed
                                  maxSpeedIdx = idx
                              }
                              if (speed < minSpeed) {
                                  minSpeed = speed
                                  minSpeedIdx = idx
                              }
                          })

        // 匹配降采样后的坐标点（补全大括号，避免逻辑错误）
        let maxSpeedPoint = drawPoints[0], minSpeedPoint = drawPoints[0]
        const targetMaxPoint = originalPoints[maxSpeedIdx]
        const targetMinPoint = originalPoints[minSpeedIdx]
        drawPoints.forEach(p => {
                               // 补全大括号，确保条件判断完整
                               if (Math.abs(p.x - targetMaxPoint.x) <= 1) {
                                   maxSpeedPoint = p
                               }
                               if (Math.abs(p.x - targetMinPoint.x) <= 1) {
                                   minSpeedPoint = p
                               }
                           })

        // 绘制标注点（提前绘制，避免被文本覆盖）
        ctx.fillStyle = "#FF9800" // 橙色标注最大速度
        ctx.fillRect(maxSpeedPoint.x - 2, maxSpeedPoint.y - 2, 4, 4)
        ctx.fillStyle = "#F44336" // 红色标注最小速度
        ctx.fillRect(minSpeedPoint.x - 2, minSpeedPoint.y - 2, 4, 4)

        // ========== 文本标签容错+增强显示 ==========


        /*
        ctx.save()
        // 1. 确保pixelRatio有效（兜底值，避免undefined）
        const effectivePixelRatio = pixelRatio || 1
        // 2. 增大字体（12px更醒目，适配像素比）
        const fontSize = 12 * effectivePixelRatio
        // 3. 规范字体格式（加粗+无衬线，确保安卓兼容）
        ctx.font = `bold ${fontSize}px sans-serif`
        // 4. 增强文本对比度（加黑色描边，避免被渐变覆盖）
        ctx.strokeStyle = isDark ? "#000000" : "#000000" // 黑色描边
        ctx.lineWidth = 1.5 * effectivePixelRatio // 细描边，不突兀
        // 5. 文本填充色（高对比度）
        ctx.fillStyle = isDark ? "#FFFFFF" : "#FFFFFF" // 白色填充，无论深浅色都醒目

        // ========== 文本坐标容错（避免越界） ==========
        // 最大速度文本坐标（确保在画布内）
        const maxTextX = Math.max(5, Math.min(canvasWidth - 60,
                                              maxSpeedPoint.x + 5))
        const maxTextY = Math.max(fontSize + 2, Math.min(canvasHeight - 2,
                                                         maxSpeedPoint.y - 5))
        // 最小速度文本坐标（确保在画布内）
        const minTextX = Math.max(5, Math.min(canvasWidth - 60,
                                              minSpeedPoint.x + 5))
        const minTextY = Math.max(fontSize + 2, Math.min(canvasHeight - 2,
                                                         minSpeedPoint.y + 15))

        // 绘制文本（先描边后填充，增强可读性）
        // 最大速度
        ctx.strokeText(`${maxSpeed.toFixed(1)}km/h`, maxTextX, maxTextY)
        ctx.fillText(`${maxSpeed.toFixed(1)}km/h`, maxTextX, maxTextY)
        // 最小速度
        ctx.strokeText(`${minSpeed.toFixed(1)}km/h`, minTextX, minTextY)
        ctx.fillText(`${minSpeed.toFixed(1)}km/h`, minTextX, minTextY)

        ctx.restore()
        */
    }

    // HSV转RGB工具函数
    function hsvToRgb(h, s, v) {
        let r, g, b
        const i = Math.floor(h / 60)
        const f = h / 60 - i
        const p = v * (1 - s)
        const q = v * (1 - f * s)
        const t = v * (1 - (1 - f) * s)

        switch (i % 6) {
        case 0:
            r = v
            g = t
            b = p
            break
        case 1:
            r = q
            g = v
            b = p
            break
        case 2:
            r = p
            g = v
            b = t
            break
        case 3:
            r = p
            g = q
            b = v
            break
        case 4:
            r = t
            g = p
            b = v
            break
        case 5:
            r = v
            g = p
            b = q
            break
        }

        return {
            "r": Math.round(r * 255),
            "g": Math.round(g * 255),
            "b": Math.round(b * 255)
        }
    }

    function drawAltitudeCurve_old(ctx, altitudeData, canvasWidth, canvasHeight) {
        if (altitudeData.length < 2) {
            ctx.fillStyle = "rgba(150, 150, 150, 0.3)"
            ctx.fillRect(0, 0, canvasWidth, canvasHeight)
            return
        }

        ctx.clearRect(0, 0, canvasWidth, canvasHeight)

        // 1. 计算海拔极值，确定绝对映射范围
        const altMin = Math.min(...altitudeData)
        const altMax = Math.max(...altitudeData)
        // 取正负方向的最大绝对值，保证0海拔居中
        const maxAbsAlt = Math.max(Math.abs(altMin), Math.abs(altMax))
        // 总映射范围：-maxAbsAlt ~ +maxAbsAlt
        const totalAltRange = maxAbsAlt * 2
        // 0海拔对应的Y坐标（画布垂直中线）
        const zeroAltY = canvasHeight / 2

        const pointCount = altitudeData.length
        const points = []
        const segmentWidth = canvasWidth / (pointCount - 1)

        // 2. 计算每个海拔点的坐标（绝对海拔映射）
        altitudeData.forEach((altitude, index) => {
                                 const x = index * segmentWidth
                                 // 核心映射公式：
                                 // - 正海拔：y = zeroAltY - (altitude / maxAbsAlt) * zeroAltY
                                 // - 负海拔：y = zeroAltY + (Math.abs(altitude) / maxAbsAlt) * zeroAltY
                                 // - 0海拔：y = zeroAltY
                                 const ratio = altitude / maxAbsAlt
                                 const y = zeroAltY - (ratio * zeroAltY)
                                 // 边界保护（避免超出画布）
                                 const clampedY = Math.max(0, Math.min(
                                                               canvasHeight, y))
                                 points.push({
                                                 "x": x,
                                                 "y": clampedY,
                                                 "originalY": y
                                             })
                             })

        // 3. 绘制0海拔基准线（X轴）
        ctx.beginPath()
        ctx.moveTo(0, zeroAltY)
        ctx.lineTo(canvasWidth, zeroAltY)
        ctx.strokeStyle = isDark ? "rgba(255,255,255,0.6)" : "rgba(0,0,0,0.6)"
        ctx.lineWidth = 1
        ctx.stroke()

        // 4. 填充海拔区域（区分正负海拔）
        ctx.beginPath()
        // 从第一个海拔点开始
        ctx.moveTo(points[0].x, points[0].y)
        // 绘制平滑曲线
        for (var i = 1; i < points.length; i++) {
            const p = points[i]
            const prev = points[i - 1]
            const controlX = (prev.x + p.x) / 2
            ctx.quadraticCurveTo(controlX, (prev.y + p.y) / 2, p.x, p.y)
        }

        // 分两种情况闭合路径（保证正负海拔填充正确）
        const lastPoint = points[points.length - 1]
        if (lastPoint.y >= zeroAltY) {
            // 最后一个点在0海拔下方 → 向右到画布边缘，再向下到画布底，向左到起点下方，闭合
            ctx.lineTo(canvasWidth, lastPoint.y)
            ctx.lineTo(canvasWidth, canvasHeight)
            ctx.lineTo(0, canvasHeight)
            ctx.lineTo(points[0].x, points[0].y)
        } else {
            // 最后一个点在0海拔上方 → 向右到画布边缘，再向上到画布顶，向左到起点上方，闭合
            ctx.lineTo(canvasWidth, lastPoint.y)
            ctx.lineTo(canvasWidth, 0)
            ctx.lineTo(0, 0)
            ctx.lineTo(points[0].x, points[0].y)
        }
        ctx.closePath()

        // 填充色：正海拔偏蓝，负海拔偏红（直观区分）
        ctx.fillStyle = isDark ? "rgba(76, 175, 255, 0.4)" // 深色模式-蓝
                               : "rgba(33, 150, 243, 0.3)" // 浅色模式-蓝
        ctx.fill()

        // 5. 绘制负海拔区域补充填充（可选，增强区分度）
        ctx.beginPath()
        ctx.moveTo(points[0].x, points[0].y)
        for (var i = 1; i < points.length; i++) {
            const p = points[i]
            const prev = points[i - 1]
            const controlX = (prev.x + p.x) / 2
            ctx.quadraticCurveTo(controlX, (prev.y + p.y) / 2, p.x, p.y)
        }
        // 仅闭合到0海拔线
        ctx.lineTo(lastPoint.x, zeroAltY)
        ctx.lineTo(points[0].x, zeroAltY)
        ctx.closePath()
        ctx.fillStyle = isDark ? "rgba(255, 102, 102, 0.4)" // 深色模式-红（负海拔）
                               : "rgba(255, 87, 34, 0.3)" // 浅色模式-橙红（负海拔）
        ctx.fill()

        // 6. 绘制海拔轮廓线（主曲线）
        ctx.beginPath()
        ctx.moveTo(points[0].x, points[0].y)
        for (var i = 1; i < points.length; i++) {
            const p = points[i]
            const prev = points[i - 1]
            const controlX = (prev.x + p.x) / 2
            ctx.quadraticCurveTo(controlX, (prev.y + p.y) / 2, p.x, p.y)
        }
        ctx.strokeStyle = isDark ? "#2196F3" : "#1976D2"
        ctx.lineWidth = 1.5
        ctx.stroke()

        // 7. 可选：绘制最大/最小海拔标注（增强可读性）
        // 找到最大/最小海拔点（修复核心逻辑）
        let maxAlt = altitudeData[0]
        // 初始最大海拔值
        let maxAltIdx = 0
        // 初始最大海拔索引
        let minAlt = altitudeData[0]
        // 初始最小海拔值
        let minAltIdx = 0
        // 初始最小海拔索引

        // 直接遍历原始海拔数组，找最大/最小值的索引（简单直接，无错误）
        altitudeData.forEach((alt, idx) => {
                                 if (alt > maxAlt) {
                                     maxAlt = alt
                                     maxAltIdx = idx
                                 }
                                 if (alt < minAlt) {
                                     minAlt = alt
                                     minAltIdx = idx
                                 }
                             })

        // 根据索引获取对应的坐标点
        const maxAltPoint = points[maxAltIdx]
        const minAltPoint = points[minAltIdx]

        // 标注最大海拔（橙色，醒目）
        ctx.fillStyle = isDark ? "#FF9800" : "#F57C00"
        ctx.fillRect(maxAltPoint.x - 2, maxAltPoint.y - 2, 4, 4)

        // 给最大海拔点加文本标签（优化版）
        ctx.save() // 保存当前绘图状态，避免影响后续绘制
        // 1. 适配设备像素比（关键：移动端/高分屏必加）
        const fontSize = 10 * pixelRatio
        // 基于设备像素比放大字体
        // 2. 规范字体格式（加粗+字号+字体，确保生效）
        ctx.font = `bold ${fontSize}px sans-serif`
        // 3. 文本颜色（增强对比度）
        ctx.fillStyle = isDark ? "#FFFFFF" : "#000000"
        // 4. 容错：确保文本坐标在画布内（避免超出可视区域）
        const textX = Math.max(5, Math.min(canvasWidth - 50, maxAltPoint.x + 5))
        // 左右不越界
        const textY = Math.max(fontSize + 2, Math.min(canvasHeight - 2,
                                                      maxAltPoint.y - 5))
        // 上下不越界
        // 5. 绘制文本（保留1位小数，更整洁）
        ctx.fillText(`${maxAlt.toFixed(1)}m`, textX, textY)
        ctx.restore() // 恢复绘图状态

        // （可选）给最小海拔也加文本标签
        ctx.fillStyle = isDark ? "#F44336" : "#D32F2F"
        ctx.fillRect(minAltPoint.x - 2, minAltPoint.y - 2, 4, 4)

        ctx.save()
        ctx.font = `bold ${fontSize}px sans-serif`
        ctx.fillStyle = isDark ? "#FFFFFF" : "#000000"
        const minTextX = Math.max(5, Math.min(canvasWidth - 50,
                                              minAltPoint.x + 5))
        const minTextY = Math.max(fontSize + 2, Math.min(canvasHeight - 2,
                                                         minAltPoint.y + 10))
        // 向下偏移，避免和点重叠
        ctx.fillText(`${minAlt.toFixed(1)}m`, minTextX, minTextY)
        ctx.restore()

        // 可选：如果最大/最小点重合，偏移最大点避免覆盖
        if (maxAltIdx === minAltIdx) {
            ctx.fillStyle = isDark ? "#FF9800" : "#F57C00"
            ctx.fillRect(maxAltPoint.x + 3, maxAltPoint.y - 2, 4, 4) // 右移3像素
        }
    }

    // ===================== 原有绘制函数（融合降采样） =====================
    function drawAltitudeCurve(ctx, altitudeData, canvasWidth, canvasHeight) {
        if (altitudeData.length < 2) {
            ctx.fillStyle = "rgba(150, 150, 150, 0.3)"
            ctx.fillRect(0, 0, canvasWidth, canvasHeight)
            return
        }

        ctx.clearRect(0, 0, canvasWidth, canvasHeight)

        // 1. 计算海拔极值，确定绝对映射范围
        const altMin = Math.min(...altitudeData)
        const altMax = Math.max(...altitudeData)
        const maxAbsAlt = Math.max(Math.abs(altMin), Math.abs(altMax))
        const zeroAltY = canvasHeight / 2

        const pointCount = altitudeData.length
        const segmentWidth = canvasWidth / (pointCount - 1)

        // 2. 计算原始坐标点（保留海拔值，供降采样用）
        const originalPoints = []
        // 重命名为originalPoints，区分降采样后的点
        altitudeData.forEach((altitude, index) => {
                                 const x = index * segmentWidth
                                 const ratio = altitude / maxAbsAlt
                                 const y = zeroAltY - (ratio * zeroAltY)
                                 const clampedY = Math.max(0, Math.min(
                                                               canvasHeight, y))
                                 originalPoints.push({
                                                         "x": x,
                                                         "y": clampedY,
                                                         "originalY": y,
                                                         "alt": altitude // 保留原始海拔值，供降采样后找极值用
                                                     })
                             })

        // ===================== 核心新增：降采样逻辑 =====================
        const MAX_DRAW_POINTS = 200
        // 最多绘制200个点（可根据需求调整）
        let drawPoints = originalPoints
        // 最终用于绘制的点

        // 仅当点数超过阈值时降采样
        if (originalPoints.length > MAX_DRAW_POINTS) {
            // 道格拉斯-普克降采样（容差适配画布宽度，越大降采样越狠）
            const epsilon = canvasWidth / 200
            drawPoints = douglasPeucker(originalPoints, epsilon)

            // 兜底：如果降采样后仍超量，用等间隔采样
            if (drawPoints.length > MAX_DRAW_POINTS) {
                drawPoints = intervalSample(drawPoints, MAX_DRAW_POINTS)
            }
        }

        // 3. 绘制0海拔基准线（X轴）
        ctx.beginPath()
        ctx.moveTo(0, zeroAltY)
        ctx.lineTo(canvasWidth, zeroAltY)
        ctx.strokeStyle = isDark ? "rgba(255,255,255,0.6)" : "rgba(0,0,0,0.6)"
        ctx.lineWidth = 1
        ctx.stroke()

        // 4. 填充海拔区域（区分正负海拔）→ 改用drawPoints绘制
        ctx.beginPath()
        ctx.moveTo(drawPoints[0].x, drawPoints[0].y)
        // 绘制平滑曲线（仅遍历降采样后的点）
        for (var i = 1; i < drawPoints.length; i++) {
            const p = drawPoints[i]
            const prev = drawPoints[i - 1]
            const controlX = (prev.x + p.x) / 2
            ctx.quadraticCurveTo(controlX, (prev.y + p.y) / 2, p.x, p.y)
        }

        // 分两种情况闭合路径（保证正负海拔填充正确）
        const lastPoint = drawPoints[drawPoints.length - 1]
        if (lastPoint.y >= zeroAltY) {
            ctx.lineTo(canvasWidth, lastPoint.y)
            ctx.lineTo(canvasWidth, canvasHeight)
            ctx.lineTo(0, canvasHeight)
            ctx.lineTo(drawPoints[0].x, drawPoints[0].y)
        } else {
            ctx.lineTo(canvasWidth, lastPoint.y)
            ctx.lineTo(canvasWidth, 0)
            ctx.lineTo(0, 0)
            ctx.lineTo(drawPoints[0].x, drawPoints[0].y)
        }
        ctx.closePath()

        // 填充色：正海拔偏蓝，负海拔偏红
        ctx.fillStyle = isDark ? "rgba(76, 175, 255, 0.4)" : "rgba(33, 150, 243, 0.3)"
        ctx.fill()

        // 5. 绘制负海拔区域补充填充 → 改用drawPoints
        ctx.beginPath()
        ctx.moveTo(drawPoints[0].x, drawPoints[0].y)
        for (var i = 1; i < drawPoints.length; i++) {
            const p = drawPoints[i]
            const prev = drawPoints[i - 1]
            const controlX = (prev.x + p.x) / 2
            ctx.quadraticCurveTo(controlX, (prev.y + p.y) / 2, p.x, p.y)
        }
        // 仅闭合到0海拔线
        ctx.lineTo(lastPoint.x, zeroAltY)
        ctx.lineTo(drawPoints[0].x, zeroAltY)
        ctx.closePath()
        ctx.fillStyle = isDark ? "rgba(255, 102, 102, 0.4)" : "rgba(255, 87, 34, 0.3)"
        ctx.fill()

        // 6. 绘制海拔轮廓线（主曲线）→ 改用drawPoints
        ctx.beginPath()
        ctx.moveTo(drawPoints[0].x, drawPoints[0].y)
        for (var i = 1; i < drawPoints.length; i++) {
            const p = drawPoints[i]
            const prev = drawPoints[i - 1]
            const controlX = (prev.x + p.x) / 2
            ctx.quadraticCurveTo(controlX, (prev.y + p.y) / 2, p.x, p.y)
        }
        ctx.strokeStyle = isDark ? "#2196F3" : "#1976D2"
        ctx.lineWidth = 1.5
        ctx.stroke()

        // 7. 绘制最大/最小海拔标注（兼容降采样，保留原始极值）
        // 第一步：找原始数据的最大/最小海拔（避免降采样丢失极值）
        let maxAlt = altitudeData[0]
        let maxAltIdx = 0
        let minAlt = altitudeData[0]
        let minAltIdx = 0

        altitudeData.forEach((alt, idx) => {
                                 if (alt > maxAlt) {
                                     maxAlt = alt
                                     maxAltIdx = idx
                                 }
                                 if (alt < minAlt) {
                                     minAlt = alt
                                     minAltIdx = idx
                                 }
                             })

        // 第二步：从drawPoints中匹配对应极值点（优先找原始索引最近的点）
        let maxAltPoint = drawPoints[0]
        let minAltPoint = drawPoints[0]

        // 方案1：如果原始极值点在drawPoints中，直接用
        const targetMaxPoint = originalPoints[maxAltIdx]
        const targetMinPoint = originalPoints[minAltIdx]

        drawPoints.forEach(p => {
                               // 匹配坐标（误差≤1px）
                               if (Math.abs(p.x - targetMaxPoint.x) <= 1)
                               maxAltPoint = p
                               if (Math.abs(p.x - targetMinPoint.x) <= 1)
                               minAltPoint = p
                           })

        // 标注最大海拔（橙色，醒目）
        ctx.fillStyle = isDark ? "#FF9800" : "#F57C00"
        ctx.fillRect(maxAltPoint.x - 2, maxAltPoint.y - 2, 4, 4)

        // 给最大海拔点加文本标签（优化版）
        ctx.save()
        const fontSize = 10 * pixelRatio
        ctx.font = `bold ${fontSize}px sans-serif`
        ctx.fillStyle = isDark ? "#FFFFFF" : "#000000"
        const textX = Math.max(5, Math.min(canvasWidth - 50, maxAltPoint.x + 5))
        const textY = Math.max(fontSize + 2, Math.min(canvasHeight - 2,
                                                      maxAltPoint.y - 5))
        ctx.fillText(`${maxAlt.toFixed(1)}m`, textX, textY)
        ctx.restore()

        // 给最小海拔加文本标签
        ctx.fillStyle = isDark ? "#F44336" : "#D32F2F"
        ctx.fillRect(minAltPoint.x - 2, minAltPoint.y - 2, 4, 4)

        ctx.save()
        ctx.font = `bold ${fontSize}px sans-serif`
        ctx.fillStyle = isDark ? "#FFFFFF" : "#000000"
        const minTextX = Math.max(5, Math.min(canvasWidth - 50,
                                              minAltPoint.x + 5))
        const minTextY = Math.max(fontSize + 2, Math.min(canvasHeight - 2,
                                                         minAltPoint.y + 10))
        ctx.fillText(`${minAlt.toFixed(1)}m`, minTextX, minTextY)
        ctx.restore()

        // 可选：如果最大/最小点重合，偏移最大点避免覆盖
        if (maxAltIdx === minAltIdx) {
            ctx.fillStyle = isDark ? "#FF9800" : "#F57C00"
            ctx.fillRect(maxAltPoint.x + 3, maxAltPoint.y - 2, 4, 4)
        }
    }

    // 道格拉斯-普克算法：降采样数组，保留曲线关键特征
    // points: 原始[{x: 像素x, y: 像素y, alt: 海拔值}]
    // epsilon: 容差（越大降采样越狠，建议0.5~2，根据Canvas宽度调整）
    function douglasPeucker(points, epsilon) {
        if (points.length <= 2)
            return points

        // 计算点到线段的垂直距离
        function distanceToSegment(p, p1, p2) {
            const dx = p2.x - p1.x
            const dy = p2.y - p1.y
            if (dx === 0 && dy === 0)
                return Math.hypot(p.x - p1.x, p.y - p1.y)

            const t = ((p.x - p1.x) * dx + (p.y - p1.y) * dy) / (dx * dx + dy * dy)
            const closestX = t < 0 ? p1.x : (t > 1 ? p2.x : p1.x + t * dx)
            const closestY = t < 0 ? p1.y : (t > 1 ? p2.y : p1.y + t * dy)
            return Math.hypot(p.x - closestX, p.y - closestY)
        }

        let maxDist = 0
        let index = 0
        const start = 0
        const end = points.length - 1

        // 找到离首尾线段最远的点
        for (var i = start + 1; i < end; i++) {
            const dist = distanceToSegment(points[i], points[start],
                                           points[end])
            if (dist > maxDist) {
                maxDist = dist
                index = i
            }
        }

        // 递归保留关键节点
        if (maxDist > epsilon) {
            const left = douglasPeucker(points.slice(start, index + 1), epsilon)
            const right = douglasPeucker(points.slice(index, end + 1), epsilon)
            return left.slice(0, -1).concat(right)
        } else {
            return [points[start], points[end]]
        }
    }

    // 简化版：等间隔采样（备用，适合极致性能场景）
    function intervalSample(points, maxCount) {
        if (points.length <= maxCount)
            return points
        const step = Math.ceil(points.length / maxCount)
        return points.filter((_, idx) => idx % step === 0)
    }

    function addItem(t0, t1, t2, t3, height) {
        view.model.append({
                              "text0": t0,
                              "text1": t1,
                              "text2": t2,
                              "text3": t3,
                              "myh": height
                          })
    }

    function insertItem(curIndex, t0, t1, t2, t3, t4, t5, t6, t7, speedData, altitudeData) {
        // 处理速度数据（原有逻辑保持不变）
        var speedArray = []
        if (speedData && speedData.hasOwnProperty("count")
                && typeof speedData.get === "function") {
            for (var i = 0; i < speedData.count; i++) {
                var item = speedData.get(i)
                var value = typeof item
                        === "object" ? (item.value !== undefined ? item.value : item) : item
                value = Number(value)
                if (!isNaN(value)) {
                    speedArray.push(value)
                }
            }
        } else if (Array.isArray(speedData)) {
            for (var j = 0; j < speedData.length; j++) {
                var val = Number(speedData[j])
                if (!isNaN(val)) {
                    speedArray.push(val)
                }
            }
        }

        // 新增：处理海拔数据（完全复用速度数据的格式逻辑）
        var altitudeArray = []
        if (altitudeData && altitudeData.hasOwnProperty("count")
                && typeof altitudeData.get === "function") {
            for (var k = 0; k < altitudeData.count; k++) {
                var altItem = altitudeData.get(k)
                var altValue = typeof altItem
                        === "object" ? (altItem.value
                                        !== undefined ? altItem.value : altItem) : altItem
                altValue = Number(altValue)
                if (!isNaN(altValue)) {
                    altitudeArray.push(altValue)
                }
            }
        } else if (Array.isArray(altitudeData)) {
            for (var l = 0; l < altitudeData.length; l++) {
                var altVal = Number(altitudeData[l])
                if (!isNaN(altVal)) {
                    altitudeArray.push(altVal)
                }
            }
        }

        console.log("insert阶段转换后的速度数组:", speedArray)
        console.log("insert阶段转换后的海拔数组:", altitudeArray) // 新增日志

        var speedJson = JSON.stringify(speedArray)
        var altitudeJson = JSON.stringify(altitudeArray) // 新增：海拔数据序列化

        // 模型插入时新增altitudeData字段
        view.model.insert(curIndex, {
                              "text0": t0,
                              "text1": t1,
                              "text2": t2,
                              "text3": t3,
                              "text4": t4,
                              "text5": t5,
                              "text6": t6,
                              "text7": t7,
                              "speedData": speedJson,
                              "altitudeData": altitudeJson // 新增：海拔数据字段
                          })

        var insertedItem = view.model.get(curIndex)
        console.log("模型中存储的速度JSON:", insertedItem.speedData)
        console.log("模型中存储的海拔JSON:", insertedItem.altitudeData) // 新增日志
    }

    function updateItem(curIndex, t0, t1, t2, t3, t4, t5, t6, height) {
        if (curIndex >= 0 && curIndex < view.model.count) {
            var existingItem = view.model.get(curIndex)
            existingItem.text0 = t0
            existingItem.text1 = t1
            existingItem.text2 = t2
            existingItem.text3 = t3
            existingItem.text4 = t4
            existingItem.text5 = t5
            existingItem.text6 = t6
            existingItem.myh = height
            view.model.set(curIndex, existingItem)
        } else {
            console.log("updateItem: 索引" + curIndex + "无效，不更新")
        }
    }

    function delItem(currentIndex) {
        view.model.remove(currentIndex)
    }

    function modifyItem(currentIndex, strTime, strText) {
        view.model.setProperty(currentIndex, "time", strTime)
        view.model.setProperty(currentIndex, "dototext", strText)
    }

    function modifyItemTime(currentIndex, strTime) {
        view.model.setProperty(currentIndex, "time", strTime)
    }

    function modifyItemType(currentIndex, type) {
        view.model.setProperty(currentIndex, "type", type)
    }

    function modifyItemText(currentIndex, strText) {
        view.model.setProperty(currentIndex, "dototext", strText)
    }

    function getColor() {
        var strColor = isDark ? "#455364" : "#ffffff"
        return strColor
    }

    function getFontColor() {
        return isDark ? "white" : "black"
    }

    Component {
        id: dragDelegate

        Rectangle {
            id: listItem
            width: ListView.view.width // 宽度=ListView可视宽度（一屏宽）
            height: ListView.view.height // 高度=ListView可视高度（一屏高）
            color: isDark ? "#333" : "#DDD"
            border.color: "#ccc"
            border.width: 1
            radius: 3

            // 选中状态红色竖条
            Rectangle {
                width: listItem.ListView.isCurrentItem ? 4 : 0
                height: parent.height
                color: isDark ? "#BBBBBB" : "#666666"
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                z: 10
                visible: false

                Behavior on width {
                    NumberAnimation {
                        duration: 300
                        easing.type: Easing.OutQuart
                    }
                }

                opacity: listItem.ListView.isCurrentItem ? 1 : 0
                Behavior on opacity {
                    NumberAnimation {
                        duration: 250
                        easing.type: Easing.OutQuart
                    }
                }
            }

            // 点击切换选中
            MouseArea {
                property point clickPos: "0,0"
                anchors.fill: parent
                // 使用箭头函数显式接收mouse参数
                onPressed: mouse => {
                               clickPos = Qt.point(mouse.x, mouse.y)
                           }
                onReleased: mouse => {
                                var delta = Qt.point(mouse.x - clickPos.x,
                                                     mouse.y - clickPos.y)
                            }
                onClicked: view.currentIndex = index
                onDoubleClicked: {

                }
            }

            // 全屏布局容器
            ColumnLayout {
                id: colLayout
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5

                // 标题行（带运动类型标记）
                Rectangle {
                    id: m_caption
                    implicitWidth: Math.min(listItem.width - 20,
                                            parent.width) // 动态计算隐式宽度
                    Layout.preferredHeight: 40
                    color: isDark ? "#333" : "#DDD"
                    border.color: isDark ? "#444" : "#CCC"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        spacing: 8

                        Rectangle {
                            id: sportTypeBlock
                            width: 24
                            height: 24
                            color: {
                                if (item0.text.indexOf(qsTr("Cycling")) !== -1)
                                    isDark ? "#5ABD5E" : "#4CAF50"
                                else if (item0.text.indexOf(
                                             qsTr("Hiking")) !== -1)
                                    isDark ? "#FFAB2C" : "#FF9800"
                                else if (item0.text.indexOf(
                                             qsTr("Running")) !== -1)
                                    isDark ? "#B746C9" : "#9C27B0"
                                else
                                    "transparent"
                            }
                            radius: 3
                        }

                        Text {
                            id: item0
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                            wrapMode: Text.NoWrap
                            font.bold: true
                            font.pointSize: baseFontSize * 1.0 // 标题字体放大1.2倍
                            text: text0
                            color: isDark ? "#FFFFFF" : "#333333"
                        }
                    }
                }

                Rectangle {
                    width: view.width
                    height: 0 // 空白高度
                    color: "transparent"
                }

                // 天气+文本行
                RowLayout {
                    id: weatherTextContainer
                    Layout.fillWidth: true

                    //Layout.preferredHeight: (Qt.platform.os === "android") ? (30) : (10)
                    Image {
                        id: weatherIcon
                        source: item6.text
                        sourceSize: Qt.size(42, 42)
                        fillMode: Image.PreserveAspectFit
                        Layout.alignment: Qt.AlignVCenter
                        visible: item6.text.length > 0
                    }

                    Text {
                        id: item1
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                        font.pointSize: baseFontSize
                        color: isDark ? "#BBB" : "#555"
                        text: text1
                        visible: text1.length > 0
                    }
                }

                Rectangle {
                    width: view.width
                    height: 5 // 空白高度
                    color: "transparent"
                }

                // 内容文本区（自适应剩余空间）
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true // 填充剩余高度
                    spacing: 4

                    // 总距离
                    Text {
                        id: item2
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        font.pointSize: baseFontSize
                        color: isDark ? "#FF6666" : "red"
                        text: text2
                        visible: text2.length > 0
                    }

                    Rectangle {
                        width: view.width
                        height: 5 // 空白高度
                        color: "transparent"
                    }

                    // 运动时长
                    Text {
                        id: item3
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        font.pointSize: baseFontSize
                        color: isDark ? "#DDD" : "#333"
                        text: text3
                        visible: text3.length > 0
                    }

                    Rectangle {
                        width: view.width
                        height: 5 // 空白高度
                        color: "transparent"
                    }

                    // 平均速度
                    Text {
                        id: item4
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        font.pointSize: baseFontSize
                        color: isDark ? "#1E90FF" : "blue"
                        text: text4
                        visible: text4.length > 0
                    }

                    // 速度图谱标签
                    Text {
                        id: speedLabel
                        Layout.fillWidth: true
                        text: qsTr("Speed Curve")
                        font.bold: true
                        font.pointSize: baseFontSize * 0.8 // 基于基准字体缩放，适配移动端
                        color: isDark ? "#FFFFFF" : "#333333"
                        horizontalAlignment: Text.AlignLeft // 左对齐，与内容呼应
                        Layout.bottomMargin: 4 // 与下方Canvas保持间距
                    }

                    // 速度图谱（放大高度）
                    Canvas {
                        id: speedRibbon

                        // 与标题宽度一致
                        //Layout.fillWidth: true
                        implicitWidth: Math.min(listItem.width - 20,
                                                parent.width) // 动态计算隐式宽度
                        Layout.preferredHeight: (Qt.platform.os === "android") ? (40) : (20)
                        Layout.topMargin: 4

                        onPaint: {
                            const ctx = getContext("2d")
                            ctx.resetTransform()
                            ctx.clearRect(0, 0, width, height)

                            const speedJson = model.speedData || "[]"
                            let speedArray = []
                            try {
                                speedArray = JSON.parse(speedJson)
                                speedArray = speedArray.filter(
                                            s => typeof s === "number"
                                            && !isNaN(s))
                            } catch (e) {
                                console.error("解析speedData失败:", e)
                                speedArray = []
                            }

                            if (speedArray.length < 2) {
                                ctx.fillStyle = "#AAAAAA"
                                ctx.fillRect(0, 0, width, height)
                                return
                            }

                            drawSpeedSpectrum(ctx, speedArray, width, height)
                        }
                    }

                    Rectangle {
                        width: view.width
                        height: 5 // 空白高度
                        color: "transparent"
                    }

                    // 累计爬升和下降
                    Text {
                        id: item5
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        font.pointSize: baseFontSize
                        color: isDark ? "#DDD" : "#333"
                        text: text5
                        visible: text5.length > 0
                    }

                    // 海拔图谱标签（新增）
                    Text {
                        id: altiLabel
                        Layout.fillWidth: true
                        text: qsTr("Terrain Curve")
                        font.bold: true
                        font.pointSize: baseFontSize * 0.8 // 基于基准字体缩放，适配移动端
                        color: isDark ? "#FFFFFF" : "#333333"
                        horizontalAlignment: Text.AlignLeft // 左对齐，与内容呼应
                        Layout.bottomMargin: 4 // 与下方Canvas保持间距
                    }

                    // 海拔图谱Canvas（新增）
                    Canvas {
                        id: altitudeCanvas
                        implicitWidth: Math.min(listItem.width - 20,
                                                parent.width) // 动态计算隐式宽度
                        //Layout.fillWidth: true
                        Layout.preferredHeight: (Qt.platform.os === "android") ? (50) : (30)
                        Layout.topMargin: 4

                        onPaint: {
                            const ctx = getContext("2d")
                            ctx.resetTransform()
                            ctx.clearRect(0, 0, width, height)

                            const altitudeJson = model.altitudeData || "[]"
                            let altitudeArray = []
                            try {
                                altitudeArray = JSON.parse(altitudeJson)
                                altitudeArray = altitudeArray.filter(
                                            a => typeof a === "number"
                                            && !isNaN(a))
                            } catch (e) {
                                console.error("解析altitudeData失败:", e)
                                altitudeArray = []
                            }

                            drawAltitudeCurve(ctx, altitudeArray, width, height)
                        }
                    }

                    // 地形距离分布可视化
                    ColumnLayout {
                        id: terrainColumn
                        //Layout.fillWidth: true
                        implicitWidth: Math.min(listItem.width - 20,
                                                parent.width)
                        Layout.topMargin: 4
                        spacing: 4

                        // 标题
                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Terrain Distance Distribution")
                            font.bold: true
                            font.pointSize: baseFontSize * 0.8
                            color: isDark ? "#FFFFFF" : "#333333"
                            horizontalAlignment: Text.AlignLeft
                            visible: false
                        }

                        // 1. 数据源 + 就绪标记
                        Text {
                            id: item7
                            text: text7 // 模型动态赋值的text7
                            visible: false
                            // 标记：item7组件是否真正初始化完成
                            property bool isReady: false
                            Component.onCompleted: {
                                isReady = true
                                console.log("item7 组件初始化完成，初始文本:", text)
                            }
                            // 仅保留日志，移除无效的forceLayout
                            onTextChanged: {
                                console.log("item7.text 动态更新:", text)
                            }
                        }

                        // 2. 核心：仅在item7就绪后计算，无多余刷新
                        property var terrainValues: {
                            try {
                                if (!item7 || !item7.isReady) {
                                    return []
                                }

                                const rawText = (item7.text || "").trim()
                                if (rawText === "") {
                                    console.log("item7.text 为空，无地形数据")
                                    return []
                                }

                                const reg = /(\d+\.?\d*) ?km/gi
                                const matches = rawText.match(reg) || []
                                if (matches.length === 0) {
                                    console.log("未匹配到任何km数值，文本：", rawText)
                                    return []
                                }

                                const values = matches.map(m => {
                                                               const numStr = m.replace(
                                                                   /km/gi,
                                                                   '').trim()
                                                               const num = parseFloat(
                                                                   numStr)
                                                               return isNaN(
                                                                   num) ? 0 : num
                                                           })
                                console.log("地形数值解析成功:", values)
                                return values
                            } catch (e) {
                                console.error("地形数值解析异常:", e)
                                return []
                            }
                        }

                        // 3. 动态绑定的地形数值
                        property real uphillKm: terrainValues.length > 0 ? terrainValues[0] : 0
                        property real flatKm: terrainValues.length > 1 ? terrainValues[1] : 0
                        property real downhillKm: terrainValues.length > 2 ? terrainValues[2] : 0
                        property real totalTerrainKm: uphillKm + flatKm + downhillKm

                        // ========== 条形图容器（加明确ID，移除自定义forceUpdate） ==========
                        Rectangle {
                            id: terrainBar // 明确ID，避免children索引
                            //Layout.fillWidth: true
                            //implicitWidth: Math.min(listItem.width - 20,
                            //                        parent.width)
                            implicitWidth: listItem.width - 20

                            Layout.preferredHeight: 15
                            color: isDark ? "#2D3748" : "#F5F5F5"
                            radius: 0
                            border.color: isDark ? "#4A5568" : "#E0E0E0"
                            border.width: 1

                            Row {
                                anchors.fill: parent
                                anchors.margins: 2
                                spacing: 0

                                // 上坡段（宽度绑定自动响应数值变化）
                                Rectangle {
                                    width: terrainColumn.totalTerrainKm
                                           > 0 ? (terrainColumn.uphillKm
                                                  / terrainColumn.totalTerrainKm) * parent.width : 0
                                    height: parent.height
                                    color: isDark ? "#ED8936" : "#FF9800"
                                    radius: 0

                                    Text {
                                        anchors.centerIn: parent
                                        text: terrainColumn.uphillKm.toFixed(
                                                  2) + "km"
                                        font.pointSize: baseFontSize * 0.7
                                        color: "#FFFFFF"
                                        font.bold: true
                                        visible: false // terrainColumn.uphillKm>0 //width > 30
                                    }
                                    Text {
                                        anchors.centerIn: parent
                                        text: qsTr("Up")
                                        font.pointSize: baseFontSize * 0.6
                                        color: "#FFFFFF"
                                        visible: false
                                    }
                                }

                                // 平路段
                                Rectangle {
                                    width: terrainColumn.totalTerrainKm
                                           > 0 ? (terrainColumn.flatKm
                                                  / terrainColumn.totalTerrainKm) * parent.width : 0
                                    height: parent.height
                                    color: isDark ? "#718096" : "#9E9E9E"
                                    radius: 0

                                    Text {
                                        anchors.centerIn: parent
                                        text: terrainColumn.flatKm.toFixed(
                                                  2) + "km"
                                        font.pointSize: baseFontSize * 0.7
                                        color: "#FFFFFF"
                                        font.bold: true
                                        visible: false // terrainColumn.flatKm>0 //width > 30
                                    }
                                    Text {
                                        anchors.centerIn: parent
                                        text: qsTr("Flat")
                                        font.pointSize: baseFontSize * 0.6
                                        color: "#FFFFFF"
                                        visible: false
                                    }
                                }

                                // 下坡段
                                Rectangle {
                                    width: terrainColumn.totalTerrainKm
                                           > 0 ? (terrainColumn.downhillKm
                                                  / terrainColumn.totalTerrainKm) * parent.width : 0
                                    height: parent.height
                                    color: isDark ? "#4299E1" : "#2196F3"
                                    radius: 0

                                    Text {
                                        anchors.centerIn: parent
                                        text: terrainColumn.downhillKm.toFixed(
                                                  2) + "km"
                                        font.pointSize: baseFontSize * 0.7
                                        color: "#FFFFFF"
                                        font.bold: true
                                        visible: false //terrainColumn.downhillKm>0// width > 30
                                    }
                                    Text {
                                        anchors.centerIn: parent
                                        text: qsTr("Down")
                                        font.pointSize: baseFontSize * 0.6
                                        color: "#FFFFFF"
                                        visible: false
                                    }
                                }
                            }
                        }

                        // ========== 图例+总计行（不变） ==========
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            // 上坡图例
                            RowLayout {
                                spacing: 4
                                Layout.alignment: Qt.AlignLeft
                                Rectangle {
                                    width: 12
                                    height: 12
                                    color: isDark ? "#ED8936" : "#FF9800"
                                    radius: 2
                                }
                                Text {
                                    text: qsTr("Uphill: ") + "\n"+terrainColumn.uphillKm.toFixed(
                                              2) + "km"
                                    //text: qsTr("Uphill")
                                    font.pointSize: baseFontSize * 0.7
                                    color: isDark ? "#E2E8F0" : "#424242"
                                }
                            }

                            // 平路图例
                            RowLayout {
                                spacing: 4
                                Layout.alignment: Qt.AlignLeft
                                Rectangle {
                                    width: 12
                                    height: 12
                                    color: isDark ? "#718096" : "#9E9E9E"
                                    radius: 2
                                }
                                Text {
                                    //text: qsTr("Flat")
                                    text: qsTr("Flat: ") + "\n"+ terrainColumn.flatKm.toFixed(
                                              2) + "km"
                                    font.pointSize: baseFontSize * 0.7
                                    color: isDark ? "#E2E8F0" : "#424242"
                                }
                            }

                            // 下坡图例
                            RowLayout {
                                spacing: 4
                                Layout.alignment: Qt.AlignLeft
                                Rectangle {
                                    width: 12
                                    height: 12
                                    color: isDark ? "#4299E1" : "#2196F3"
                                    radius: 2
                                }
                                Text {
                                    //text: qsTr("Downhill")
                                    text: qsTr("Downhill: ") + "\n"+ terrainColumn.downhillKm.toFixed(
                                              2) + "km"
                                    font.pointSize: baseFontSize * 0.7
                                    color: isDark ? "#E2E8F0" : "#424242"
                                }
                            }

                            // 总计
                            Text {
                                Layout.alignment: Qt.AlignRight
                                text: qsTr("Total: ") + terrainColumn.totalTerrainKm.toFixed(
                                          2) + "km"
                                font.pointSize: baseFontSize * 0.7
                                font.bold: true
                                color: isDark ? "#FFFFFF" : "#212121"
                                visible: false
                            }
                        }
                    }

                    Rectangle {
                        width: view.width
                        height: 5 // 空白高度
                        color: "transparent"
                    }

                    // 天气图标路径文件
                    Text {
                        id: item6
                        text: text6
                        visible: false
                    }
                }

                // 按钮区（底部固定）
                RowLayout {
                    id: buttonLayout
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBottom
                    spacing: 8
                    Layout.topMargin: 10

                    Button {
                        id: btnViewGpsTrack
                        width: 40
                        height: 40

                        //visible: listItem.ListView.isCurrentItem
                        contentItem: Image {
                            source: isDark ? "/res/track_l.svg" : "/res/track.svg"
                            sourceSize: Qt.size(24, 24)
                            fillMode: Image.PreserveAspectFit
                        }

                        background: Rectangle {
                            color: btnViewGpsTrack.down ? "#4CAF50" : (isDark ? "#444" : "#CCC")
                            radius: 4
                            border.color: "#4CAF50"
                            border.width: 1
                        }

                        onClicked: {
                            strGpsTime = item0.text + "-=-" + item1.text + "-=-"
                                    + item2.text + "-=-" + item4.text + "-=-" + item7.text
                            m_Steps.getGpsTrack()
                        }
                    }

                    Button {
                        id: btnRoute
                        width: 40
                        height: 40
                        visible: isShowRoute && listItem.ListView.isCurrentItem

                        contentItem: Image {
                            source: isDark ? "/res/route_l.svg" : "/res/route.svg"
                            sourceSize: Qt.size(24, 24)
                            fillMode: Image.PreserveAspectFit
                        }

                        background: Rectangle {
                            color: btnRoute.down ? "#4CAF50" : (isDark ? "#444" : "#CCC")
                            radius: 4
                            border.color: "#4CAF50"
                            border.width: 1
                        }

                        onClicked: {
                            strGpsTime = item0.text + "-=-" + item1.text + "-=-"
                                    + item2.text + "-=-" + item4.text
                            m_Steps.getRouteList(strGpsTime)
                        }
                    }
                }
            }
        }
    }

    // 水平滚动ListView（核心修改）
    ListView {
        id: view
        anchors {
            fill: parent
            margins: 4
        }
        model: ListModel {
            id: listmain
        }
        delegate: dragDelegate
        spacing: 5
        cacheBuffer: 100 // 增大缓存，优化滑动体验
        orientation: ListView.Horizontal // 水平滚动
        snapMode: ListView.SnapOneItem // 滚动对齐到单个条目（一屏一个）
        highlightRangeMode: ListView.StrictlyEnforceRange // 严格限制滚动范围
        currentIndex: 0 // 默认选中第一个条目

        // 水平滚动条
        ScrollBar.horizontal: ScrollBar {
            policy: ScrollBar.AsNeeded
            height: 8
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
        }
    }

    // 弹出窗口：显示路由数据列表
    Dialog {
        id: routeDialog
        objectName: "routeDialog"
        title: ""
        width: root.width * 1.0
        height: root.height * 1.0
        modal: true
        visible: false
        x: (root.width - width) / 2
        y: (root.height - height) / 2

        background: Rectangle {
            color: isDark ? "#333333" : "#F5F5F5"
            radius: 0
            border.color: isDark ? "#555555" : "#CCCCCC"
            border.width: 1
        }

        ListModel {
            id: routeModel
        }

        ListView {
            anchors.fill: parent
            model: routeModel
            spacing: 5
            cacheBuffer: 50
            boundsBehavior: Flickable.StopAtBounds

            delegate: Rectangle {
                width: routeDialog.width - 20
                height: colLayout.implicitHeight + 20

                property string currentLastPart: {
                    if (!address)
                        return ''
                    const parts = address.split('\n')
                    return parts[parts.length - 1] || ''
                }

                property bool hasSamePrev: {
                    if (index <= 0)
                        return false
                    const prevItem = routeModel.get(index - 1)
                    const prevParts = prevItem.address.split('\n')
                    const prevLastPart = prevParts[prevParts.length - 1] || ''
                    return prevLastPart === currentLastPart
                }

                property bool hasSameNext: {
                    if (index >= routeModel.count - 1)
                        return false
                    const nextItem = routeModel.get(index + 1)
                    const nextParts = nextItem.address.split('\n')
                    const nextLastPart = nextParts[nextParts.length - 1] || ''
                    return nextLastPart === currentLastPart
                }

                property bool isGroupFirst: !hasSamePrev
                property int sameGroupCount: {
                    if (!currentLastPart)
                        return 1
                    let count = 1
                    let prevIdx = index - 1
                    while (prevIdx >= 0) {
                        const prevItem = routeModel.get(prevIdx)
                        const prevParts = prevItem.address.split('\n')
                        const prevLast = (prevParts[prevParts.length - 1]
                                          || '').trim()
                        if (prevLast === currentLastPart) {
                            count++
                            prevIdx--
                        } else
                            break
                    }
                    let nextIdx = index + 1
                    while (nextIdx < routeModel.count) {
                        const nextItem = routeModel.get(nextIdx)
                        const nextParts = nextItem.address.split('\n')
                        const nextLast = (nextParts[nextParts.length - 1]
                                          || '').trim()
                        if (nextLast === currentLastPart) {
                            count++
                            nextIdx++
                        } else
                            break
                    }
                    return count
                }

                color: (hasSamePrev
                        || hasSameNext) ? (isDark ? "#2E7D32" : "#E8F5E9") : (isDark ? "#333" : "#EEE")
                radius: 5
                border.color: isDark ? "#555" : "#CCC"
                border.width: 1

                //anchors.horizontalCenter: parent.horizontalCenter
                ColumnLayout {
                    id: colLayout
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 5

                    Rectangle {
                        visible: isGroupFirst && sameGroupCount > 1
                        Layout.alignment: Qt.AlignRight
                        Layout.rightMargin: 5
                        width: textItem.implicitWidth + 4
                        height: textItem.implicitHeight + 2
                        color: isDark ? "#1B5E20" : "#4CAF50"
                        radius: 3

                        Text {
                            id: textItem
                            text: qsTr("Total %1 items").arg(sameGroupCount)
                            color: "white"
                            font.pointSize: 12
                            horizontalAlignment: Text.AlignRight
                            leftPadding: 2
                            rightPadding: 2
                            topPadding: 1
                            bottomPadding: 1
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: time
                        font.bold: true
                        color: isDark ? "#FFF" : "#000"
                        horizontalAlignment: Text.AlignLeft
                    }

                    Text {
                        Layout.fillWidth: true
                        text: latLon
                        color: isDark ? "#DDD" : "#333"
                        horizontalAlignment: Text.AlignLeft
                    }

                    Text {
                        Layout.fillWidth: true
                        text: address
                        color: isDark ? "#BBB" : "#666"
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                width: 8
            }
        }

        footer: Item {
            width: parent.width
            implicitHeight: footerLayout.implicitHeight + 20

            RowLayout {
                id: footerLayout
                spacing: 10
                Layout.fillWidth: true
                anchors.centerIn: parent
                anchors.margins: 10

                Button {
                    text: qsTr("Clear")
                    visible: false
                    onClicked: routeModel.clear()
                    background: Rectangle {
                        color: isDark ? "#F44336" : "#FF5722"
                        radius: 5
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                Button {
                    id: btnClose
                    text: qsTr("Close")
                    onClicked: routeDialog.visible = false
                    background: Rectangle {
                        color: btnClose.down ? (isDark ? "#388E3C" : "#4CAF50") : (isDark ? "#4CAF50" : "#8BC34A")
                        radius: 5
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }

        function addRouteItem(timeStr, latLonStr, addressStr) {
            routeModel.append({
                                  "time": timeStr,
                                  "latLon": latLonStr,
                                  "address": addressStr
                              })
        }

        function clearRouteModel() {
            routeModel.clear()
        }

        function setVisible(value) {
            routeDialog.visible = value
        }

        function isVisible() {
            return routeDialog.visible
        }
    }
}
