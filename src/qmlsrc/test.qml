import QtQuick
import QtQuick.Controls

Item {
    id: root
    width: 600
    height: 800

    // Flickable 负责内容滚动
    Flickable {
        id: flickable
        anchors.fill: parent

        // 只允许垂直滚动
        flickableDirection: Flickable.VerticalFlick

        // 内容宽度 = 视口宽度（禁止水平滚动）
        contentWidth: flickable.width
        // 内容高度 = TextEdit 实际高度
        contentHeight: textEdit.implicitHeight

        TextEdit {
            id: textEdit
            width: parent.width // 宽度与 Flickable 一致
            text: "宇宙是包容一切的，在它之外不存在任何东西，甚至没有时空。霍金的无边界宇宙模型是有史以来的第一个自足的宇宙模型。在这个框架中量子力学的哥本哈根的波函数坍缩理论必须加以扬弃，因为不存在宇宙之外的智慧生物。这个理论的哲学和宗教的含义是非常深远的。
哈勃红移定律表明，我们的宇宙是从发生在大约一百五十亿年前的大爆炸膨胀而来的，而宇宙微波背景辐射正是大爆炸的残余。近年的宇宙背景探索者的探测结果显示，宇宙是极其各向同性的，其相关温度的相对起伏小于十万分之一，正是这些宝贵的起伏赋予宇宙以结构和生命。
宇宙学和黑洞是霍金的两个主要研究领域。霍金在经典物理的框架中证明了广义相对论的奇性定理和黑洞面积定理，在量子物理的框架中发现了黑洞蒸发现象并提出无边界的霍金宇宙模型。黑洞和宇宙有许多对偶之处。例如黑洞无毛定理对应于宇宙暴涨相的无毛定理，黑洞蒸发对应于宇宙的粒子生成，黑洞和暴涨相宇宙各具视界和辐射温度等热。现在霍金提出，黑洞蒸发在某种意义上可以看成粒子通过所谓的婴儿宇宙穿透到其他宇宙或同一宇宙的其他区域，这样就把他的两个研究领域统一起来。婴儿宇宙研究的主要成果是证明了宇宙常数必须为零，尽管当代物理学家的抱负远不止此。
宇宙学是新思想的摇篮，我们可望所有物理定律都会在此得到超越或升华。
"
            font.pixelSize: 25
            readOnly: true
            wrapMode: Text.WordWrap
            selectByMouse: true
        }

        // 滚动条
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            width: 8
        }
    }
}
