import QtQuick 2.15
import QtQuick.Window 2.15
import VideoItem 1.0
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQuick.Layouts
import "footer"
Window {
    width: 1920
    height: 1080
    visible: true
    title: qsTr("视频播放器")
    property bool isPlay: flase;
    Timer{
        id:getPlayProgress;
        interval: 10;
        repeat: true;
        onTriggered: {
            var progress = videoitem.getPlayProgress();
            videoProgress.value = progress;
        }
    }

    VideoItem{
        id:videoitem
        anchors.fill: parent
        Keys.enabled: true  // 不设置按键使能，获取不了按键事件
        focus: true         // 不设置焦点，获取不了键盘事件
        Keys.onPressed: {

            // if(event.key === 81)
            // {
            //       console.log("text:"+event.text)
            //     console.log("按下q")
            //     videoitem.pause()
            // }

        }

        Keys.onSpacePressed:
        {
            videoitem.pause()
        }
        Keys.onLeftPressed:
        {
            videoitem.seek(-10);
        }
        Keys.onRightPressed:
        {
            videoitem.seek(10);
        }

    }

    Button {
        id: button
        x: 29
        y: 27
        text: qsTr("Play")
        focusPolicy: Qt.NoFocus  // 禁止按钮获取焦点
        onClicked: {
            videoitem.setUrl("C:\\Users\\zha\\Desktop\\testvideo\\一路向北.mp4")
            console.log("开始")
            videoitem.start()
            getPlayProgress.start();
            button.visible = false;
        }
    }
    Footer{
        id:videoProgress
        width: parent.width
        anchors.bottom: parent.bottom
        value: 20;
    }


}
