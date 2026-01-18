import QtQuick 2.15
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Basic
Item {
    ColumnLayout{
        width:parent.width;
        anchors.bottom: parent.bottom;
        spacing: 0;
        ProgressBar{
            id:videoProgress
            width: parent.width
            height: 8  // 进度条高度（可根据需求调整）
            to:100;
            background: Rectangle {
                width: videoProgress.width
                height: videoProgress.height
                color: "#E0E0E0"  // 背景灰色（可自定义）
                radius: height / 2  // 圆角半径设为高度的一半，实现完全圆角
                // 若想固定圆角大小，可直接写数值：radius: 4
            }

            // 自定义进度填充样式（进度条的已完成部分）
            contentItem: Rectangle {
                width: videoProgress.visualPosition * videoProgress.width
                height: videoProgress.height
                color: "#2196F3"  // 进度条蓝色（可自定义）
                radius: height / 2  // 与背景圆角保持一致
                // 可选：添加进度条边缘的细微阴影
                border.color: "#1976D2"
                border.width: 0.5
            }

            // 可选：添加鼠标悬浮/点击的交互效果（提升体验）
            hoverEnabled: true
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                // 点击进度条跳转位置
                onClicked: function(mouse) {  // 显式声明mouse形参
                    // 根据点击位置计算进度值
                    videoProgress.value = (mouse.x / videoProgress.width) * videoProgress.to;
                }
            }
        }

        Rectangle{
            width:parent.width;
            height: 80;
            color: "black";       // 基础颜色：黑色
            opacity: 0.8;        // 整体透明度2%（0=完全透明，1=完全不透明）
            RowLayout
            {
                anchors.centerIn: parent;
                Image {
                    id: playBtn
                    scale: 0.3
                    source: isPlay ? "qrc:/img/play.svg":"qrc:/img/puase.svg";
                    MouseArea
                    {
                        anchors.fill: parent;
                        onClicked: {
                            isPlay = !isPlay;
                        }
                    }
                }
            }
        }
    }
}
