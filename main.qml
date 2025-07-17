import QtQuick 2.15
import QtQuick.Window 2.15
import VideoItem 1.0
import QtQuick.Controls
Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("视频播放器")

    VideoItem{
        id:videoitem
        anchors.fill: parent
        // width: 200
        // height: 200
        //color:"#000000";
        // onVideoInfoReady:{
        //     width = width1;
        //     height = height1;

        // }
    }

    Button {
        id: button
        x: 29
        y: 27
        text: qsTr("Play")

        onClicked: {
            videoitem.setUrl("C:\\Users\\zha\\Desktop\\testvideo\\一路向北.mp4")
            console.log("开始")
            videoitem.start()
        }
    }
}
