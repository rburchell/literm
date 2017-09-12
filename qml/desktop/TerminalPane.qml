/*
 * Copyright (C) 2017 Robin Burchell <robin+git@viroteck.net>
 *
 * This file is part of FingerTerm.
 *
 * FingerTerm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * FingerTerm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FingerTerm.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.6
import literm 1.0

TextRender {
    id: textrender
    focus: true

    opacity: activeItem == this ? 1.0 : 0.5

    property real splitRatio: 1.0

    onWidthChanged: console.log("w", width)
    onHeightChanged: console.log("h", height)

    onPanLeft: {
        util.notifyText(util.panLeftTitle)
        textrender.putString(util.panLeftCommand)
    }
    onPanRight: {
        util.notifyText(util.panRightTitle)
        textrender.putString(util.panRightCommand)
    }
    onPanUp: {
        util.notifyText(util.panUpTitle)
        textrender.putString(util.panUpCommand)
    }
    onPanDown: {
        util.notifyText(util.panDownTitle)
        textrender.putString(util.panDownCommand)
    }

    onDisplayBufferChanged: {
        textrender.cutAfter = textrender.height
    }
    charset: util.charset
    terminalCommand: util.terminalCommand
    terminalEnvironment: util.terminalEmulator
    onTitleChanged: {
        util.windowTitle = title
    }
    dragMode: util.dragMode
    onVisualBell: {
        if (util.visualBellEnabled)
            bellTimer.start()
    }
    contentItem: Item {
        anchors.fill: parent
        Behavior on opacity {
            NumberAnimation { duration: textrender.duration; easing.type: Easing.InOutQuad }
        }
        Behavior on y {
            NumberAnimation { duration: textrender.duration; easing.type: Easing.InOutQuad }
        }
    }

    ScrollDecorator {
        color: "#DFDFDF"
        anchors.right: parent.right
        anchors.rightMargin: window.paddingMedium
        y: ((parent.contentY + (parent.visibleHeight/2)) / parent.contentHeight) * (parent.height - height)
        height: (parent.visibleHeight / parent.contentHeight) * parent.height
        visible: parent.contentHeight > parent.visibleHeight
    }

    cellDelegate: Rectangle {
    }
    cellContentsDelegate: Text {
        id: text
        property bool blinking: false

        textFormat: Text.PlainText
        opacity: blinking ? 0.5 : 1.0
        SequentialAnimation {
            running: blinking
            loops: Animation.Infinite
            NumberAnimation {
                target: text
                property: "opacity"
                to: 0.8
                duration: 200
            }
            PauseAnimation {
                duration: 400
            }
            NumberAnimation {
                target: text
                property: "opacity"
                to: 0.5
                duration: 200
            }
        }
    }
    cursorDelegate: Rectangle {
        id: cursor
        opacity: 0.5
        SequentialAnimation {
            running: Qt.application.state == Qt.ApplicationActive
            loops: Animation.Infinite
            NumberAnimation {
                target: cursor
                property: "opacity"
                to: 0.8
                duration: 200
            }
            PauseAnimation {
                duration: 400
            }
            NumberAnimation {
                target: cursor
                property: "opacity"
                to: 0.5
                duration: 200
            }
        }
    }
    selectionDelegate: Rectangle {
        color: "blue"
        opacity: 0.5
    }

    Rectangle {
        id: bellTimerRect
        visible: opacity > 0
        opacity: bellTimer.running ? 0.5 : 0.0
        anchors.fill: parent
        color: "#ffffff"
        Behavior on opacity {
            NumberAnimation {
                duration: 200
            }
        }
    }

    property int duration
    property int cutAfter: height

    font.family: util.fontFamily
    font.pointSize: util.fontSize
    allowGestures: !menu.showing && !urlWindow.show && !aboutDialog.show && !layoutWindow.show

    onCutAfterChanged: {
        // this property is used in the paint function, so make sure that the element gets
        // painted with the updated value (might not otherwise happen because of caching)
        textrender.redraw();
    }
}

