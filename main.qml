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

    function toggle_fullscreen()
    {
        if(window.visibility == Window.FullScreen)
        {
            window.visibility = Window.Windowed
            cursor.cursorShape = Qt.ArrowCursor
        }
        else
        {
            window.visibility = Window.FullScreen
            cursor.cursorShape = Qt.BlankCursor
        }
    }

    Item {
        focus: true
        Keys.onPressed: {
            switch(event.key)
            {
                case Qt.Key_Escape: Qt.quit(); break;
                case Qt.Key_F11: toggle_fullscreen(); break;
            }
        }
    }

    Timer {
        interval: 250
        running: true
        repeat: true
        onTriggered: {
            var now = new Date();
            time.text = Qt.formatDateTime(now, "HH:mm:ss");
            date.text = Qt.formatDateTime(now, "dd.MM.yyyy");
        }
    }

    Text {
        id: time
        objectName: "time"
        color: "#05952d"
        text: "00:00:00"
        font.pixelSize: 60
        y: 15
        anchors.horizontalCenter: parent.horizontalCenter
        verticalAlignment: Text.AlignTop
        horizontalAlignment: Text.AlignHCenter
    }

    Text {
        id: date
        color: "#ffffff"
        text: "01.01.2000"
        font.pixelSize: 30
        y: 93
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalAlignment: Text.AlignHCenter
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
                color: "#000000"
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

    MouseArea {
        id: cursor
        anchors.fill: parent
        enabled: false
        cursorShape: Qt.BlankCursor
    }
}
