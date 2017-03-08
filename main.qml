import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3

Window {
    id: window
    visible: true
    width: 320
    height: 240
    color: "#000000"
    title: "ntp monitor"
    visibility: Window.FullScreen

    signal restart_ntp()

    Item {
        focus: true
        Keys.onPressed: {
            switch(event.key)
            {
                case Qt.Key_Escape: Qt.quit(); break;
                case Qt.Key_F11: window.visibility = window.visibility == Window.FullScreen ? Window.Windowed : Window.FullScreen; break;
            }
        }
    }

    Text {
        id: time
        objectName: "time"
        y: 15
        color: "#05952d"
        text: "00:00:00"
        anchors.horizontalCenter: parent.horizontalCenter
        verticalAlignment: Text.AlignTop
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 60
    }

    Text {
        id: date
        objectName: "date"
        y: 93
        color: "#ffffff"
        text: "27 Jan 2000"
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 30
    }


    RowLayout {
        id: row
        y: 178
        height: 62
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        spacing: 5

        Rectangle {
            id: pps
            objectName: "pps"
            Layout.fillWidth: true
            height: 62
            color: "#b70808"

            Text {
                id: pps_text
                text: "PPS"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 35
            }
        }

        Rectangle {
            id: gps
            objectName: "gps"
            Layout.fillWidth: true
            height: 62
            color: "#b70808"

            Text {
                id: gps_text
                text: "GPS"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: 35
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Rectangle {
            id: restart
            Layout.fillWidth: true
            height: 62
            color: "#b70808"

            Text {
                id: restart_text
                text: "RST"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: 35
                anchors.verticalCenter: parent.verticalCenter
            }

            MouseArea {
                anchors.fill: parent
                onClicked: restart_ntp()
            }
        }
    }
}
