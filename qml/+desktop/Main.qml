/*
 * Copyright (C) 2017 Robin Burchell <robin+git@viroteck.net>
 * Copyright 2011-2012 Heikki Holstila <heikki.holstila@gmail.com>
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

import QtQuick 2.0
import FingerTerm 1.0
import QtQuick.Window 2.0

Rectangle {
    id: window

    width: 540
    height: 960
    focus: true
    color: "#000000"
    Keys.onPressed: {
        term.keyPress(event.key,event.modifiers,event.text);
    }

    property int fontSize: 14*pixelRatio

    property int fadeOutTime: 80
    property int fadeInTime: 350
    property real pixelRatio: window.width / 540

    // layout constants
    property int buttonWidthSmall: 60*pixelRatio
    property int buttonWidthLarge: 180*pixelRatio
    property int buttonWidthHalf: 90*pixelRatio

    property int buttonHeightSmall: 48*pixelRatio
    property int buttonHeightLarge: 68*pixelRatio

    property int headerHeight: 20*pixelRatio

    property int radiusSmall: 5*pixelRatio
    property int radiusMedium: 10*pixelRatio
    property int radiusLarge: 15*pixelRatio

    property int paddingSmall: 5*pixelRatio
    property int paddingMedium: 10*pixelRatio

    property int fontSizeSmall: 14*pixelRatio
    property int fontSizeLarge: 24*pixelRatio

    property int uiFontSize: util.uiFontSize * pixelRatio

    property int scrollBarWidth: 6*window.pixelRatio

    TextRender {
        id: textrender

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
        cellDelegate: Rectangle {
        }
        cellContentsDelegate: Text {
        }
        cursorDelegate: Rectangle {
            id: cursor
            opacity: 0.5
            SequentialAnimation {
                running: true
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

        height: parent.height
        width: parent.width
        font.family: util.fontFamily
        font.pointSize: util.fontSize
        allowGestures: !menu.showing && !urlWindow.show && !aboutDialog.show && !layoutWindow.show

        onCutAfterChanged: {
            // this property is used in the paint function, so make sure that the element gets
            // painted with the updated value (might not otherwise happen because of caching)
            textrender.redraw();
        }
    }

    MouseArea {
        //top right corner menu button
        x: window.width - width
        width: menuImg.width + 60*window.pixelRatio
        height: menuImg.height + 30*window.pixelRatio
        opacity: 0.5
        onClicked: menu.showing = true

        Image {
            id: menuImg

            anchors.centerIn: parent
            source: "qrc:/icons/menu.png"
            scale: window.pixelRatio
        }
    }

    Image {
        // terminal buffer scroll indicator
        source: "qrc:/icons/scroll-indicator.png"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        visible: textrender.showBufferScrollIndicator
        scale: window.pixelRatio
    }

    Timer {
        id: bellTimer

        interval: 80
    }

    Connections {
        target: util
        onNotify: {
            textNotify.text = msg;
            textNotifyAnim.enabled = false;
            textNotify.opacity = 1.0;
            textNotifyAnim.enabled = true;
            textNotify.opacity = 0;
        }
    }

    MenuFingerterm {
        id: menu
        anchors.fill: parent
    }

    Text {
        // shows large text notification in the middle of the screen (for gestures)
        id: textNotify

        anchors.centerIn: parent
        color: "#ffffff"
        opacity: 0
        font.pointSize: 40*window.pixelRatio

        Behavior on opacity {
            id: textNotifyAnim
            NumberAnimation { duration: 500; }
        }
    }

    Rectangle {
        // visual key press feedback...
        // easier to work with the coordinates if it's here and not under keyboard element
        id: visualKeyFeedbackRect

        property string label

        visible: false
        radius: window.radiusSmall
        color: "#ffffff"

        Text {
            color: "#000000"
            font.pointSize: 34*window.pixelRatio
            anchors.centerIn: parent
            text: visualKeyFeedbackRect.label
        }
    }

    NotifyWin {
        id: aboutDialog

        text: {
            var str = "<font size=\"+3\">FingerTerm " + util.versionString() + "</font><br>\n" +
                    "<font size=\"+1\">" +
                    "by Heikki Holstila &lt;<a href=\"mailto:heikki.holstila@gmail.com?subject=FingerTerm\">heikki.holstila@gmail.com</a>&gt;<br><br>\n\n" +
                    "Config files for adjusting settings are at:<br>\n" +
                    util.configPath() + "/<br><br>\n" +
                    "Source code:<br>\n<a href=\"https://git.merproject.org/mer-core/fingerterm/\">https://git.merproject.org/mer-core/fingerterm/</a>"
            if (term.rows != 0 && term.columns != 0) {
                str += "<br><br>Current window title: <font color=\"gray\">" + util.windowTitle.substring(0,40) + "</font>"; //cut long window title
                if(util.windowTitle.length>40)
                    str += "...";
                str += "<br>Current terminal size: <font color=\"gray\">" + term.columns + "×" + term.rows + "</font>";
                str += "<br>Charset: <font color=\"gray\">" + util.charset + "</font>";
            }
            str += "</font>";
            return str;
        }
        onDismissed: util.showWelcomeScreen = false
    }

    NotifyWin {
        id: errorDialog
    }

    UrlWindow {
        id: urlWindow
    }

    LayoutWindow {
        id: layoutWindow
    }

    Connections {
        target: term
        onDisplayBufferChanged: {
            textrender.cutAfter = textrender.height;
            textrender.y = 0;
        }
    }

    Component.onCompleted: {
        if (util.showWelcomeScreen)
            aboutDialog.show = true
        if (startupErrorMessage != "") {
            showErrorMessage(startupErrorMessage)
        }
    }

    function showErrorMessage(string)
    {
        errorDialog.text = "<font size=\"+2\">" + string + "</font>";
        errorDialog.show = true
    }
}
