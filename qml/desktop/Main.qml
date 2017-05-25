/*
 * Copyright (C) 2017 Robin Burchell <robin+git@viroteck.net>
 * Copyright 2011-2012 Heikki Holstila <heikki.holstila@gmail.com>
 *
 * This work is free software. you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this work.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.6
import QtQuick.Window 2.2

Rectangle {
    id: window

    width: 540
    height: 960
    color: "#000000"

    property int fontSize: 14*pixelRatio

    property int fadeOutTime: 80
    property int fadeInTime: 350
    property real pixelRatio: 1.0

    Binding {
        target: window
        property: "pixelRatio"
        value: Window.window ? Window.window.devicePixelRatio : 1.0
    }

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

    TabView {
        id: tabView
        anchors.fill: parent

        Component.onCompleted: {
            createTab();
        }

        Component {
            id: terminalPaneComponent
            SplitView {
                anchors.fill: parent
            }
        }

        function createTab() {
            var tab = tabView.addTab("", terminalPaneComponent);
            // ### when last pane closed, close tab
            tabView.currentIndex = tabView.count - 1;
        }

        function closeTab(screenItem) {
            if (tabView.count == 1) {
                Qt.quit();
                return;
            }
            for (var i = 0; i < tabView.count; i++) {
                if (tabView.getTab(i) === screenItem) {
                    tabView.removeTab(i);
                    break;
                }
            }
        }

        Shortcut {
            sequence: Qt.platform.os == "osx" ? "Ctrl+C" : "Ctrl+Shift+C"
            onActivated: {
                tabView.activeTabItem.copy();
            }
        }
        Shortcut {
            sequence: Qt.platform.os == "osx" ? "Ctrl+C" : "Ctrl+Shift+V"
            onActivated: {
                if (tabView.activeTabItem.canPaste)
                    tabView.activeTabItem.paste();
            }
        }

        // hurgh, this is a bit ugly
        Shortcut {
            sequence: "Ctrl+1"
            onActivated: {
                if (tabView.count >= 2)
                    tabView.currentIndex = 0 // yes, this is right. 0 indexed.
            }
        }
        Shortcut {
            sequence: "Ctrl+2"
            onActivated: {
                if (tabView.count >= 2)
                    tabView.currentIndex = 1 // yes, this is right. 0 indexed.
            }
        }
        Shortcut {
            sequence: "Ctrl+3"
            onActivated: {
                if (tabView.count >= 3)
                    tabView.currentIndex = 2 // yes, this is right. 0 indexed.
            }
        }
        Shortcut {
            sequence: "Ctrl+4"
            onActivated: {
                if (tabView.count >= 4)
                    tabView.currentIndex = 3 // yes, this is right. 0 indexed.
            }
        }
        Shortcut {
            sequence: "Ctrl+5"
            onActivated: {
                if (tabView.count >= 5)
                    tabView.currentIndex = 4 // yes, this is right. 0 indexed.
            }
        }
        Shortcut {
            sequence: "Ctrl+6"
            onActivated: {
                if (tabView.count >= 6)
                    tabView.currentIndex = 5 // yes, this is right. 0 indexed.
            }
        }
        Shortcut {
            sequence: "Ctrl+7"
            onActivated: {
                if (tabView.count >= 7)
                    tabView.currentIndex = 6 // yes, this is right. 0 indexed.
            }
        }
        Shortcut {
            sequence: "Ctrl+8"
            onActivated: {
                if (tabView.count >= 8)
                    tabView.currentIndex = 7 // yes, this is right. 0 indexed.
            }
        }
        Shortcut {
            sequence: "Ctrl+9"
            onActivated: {
                if (tabView.count >= 9)
                    tabView.currentIndex = 8 // yes, this is right. 0 indexed.
            }
        }
        Shortcut {
            sequence: "Ctrl+0"
            onActivated: {
                if (tabView.count >= 10)
                    tabView.currentIndex = 9 // yes, this is right. 0 indexed.
            }
        }
        Shortcut {
            sequence: Qt.platform.os == "osx" ? "Ctrl+T" : "Ctrl+Shift+T"
            onActivated: {
                tabView.createTab();
            }
        }
        /*
        TODO
        Shortcut {
            sequence: Qt.platform.os == "osx" ? "Ctrl+W" : "Ctrl+Shift+W"
            onActivated: {
                tabView.closeTab(tabView.activeTabItem)
            }
        }*/
        Shortcut {
            sequence: "Ctrl+Shift+]"
            onActivated: {
                tabView.currentIndex = (tabView.currentIndex + 1) % tabView.count;
            }
        }
        Shortcut {
            sequence: "Ctrl+Shift+["
            onActivated: {
                if (tabView.currentIndex > 0) {
                    tabView.currentIndex--;
                } else {
                    tabView.currentIndex = tabView.count -1;
                }
            }
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

    MenuLiterm {
        id: menu
        anchors.fill: parent
        activeTerminal: tabView.activeTabItem
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
            var str = "<font size=\"+3\">literm " + util.versionString() + "</font><br>\n" +
                    "<font size=\"+1\">" +
                    "Source code:<br>\n<a href=\"https://github.com/rburchell/literm/\">https://github.com/rburchell/literm/</a>\n\n"
                    "Config files for adjusting settings are at:<br>\n" +
                    util.configPath() + "/<br><br>\n"
            str += "</font>";
            return str;
        }
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

    Component.onCompleted: {
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
