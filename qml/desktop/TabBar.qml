/*******************************************************************************
 * Copyright (C) 2017 Robin Burchell <robin+git@viroteck.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *******************************************************************************/

import QtQuick 2.6

Rectangle {
    id: tabBar
    color: "black"
    width: parent.width
    height: tabCount > 1 ? _tabHeight : 0
    visible: height > 0

    // "Public" API
    // Which tab is active?
    property int currentIndex

    // Our data store. This is owned by TabView (and should only be modified
    // there).
    property var tabArray
    property int tabCount

    // Ask the view to remove this index
    signal removeIndex(int index)

    // "Private" API below here
    // How long to run animations for?
    property int _animationDuration: 100

    // How high to make our tabs?
    property int _tabHeight: tm.height + 10
    TextMetrics {
        id: tm
        text: "Hello, world"
    }


    // Constraints for the size a tab can take up
    property int _maxTabWidth: (width / _minVisibleTabs)
    property real _minTabWidth: (width / _maxVisibleTabs) // real is deliberate, otherwise loss of precision stacks up

    // How many tabs can appear in the bar before we begin to abbreviate?
    readonly property int _maxVisibleTabs: 8
    readonly property int _minVisibleTabs: 2

    // Current size all tabs should be (evaluated here, so it's only calculated
    // once).
    property real _currentTabWidth: Math.max(tabBar._minTabWidth, Math.min(tabBar._maxTabWidth, tabBar.width / tabBar.tabCount))
    Behavior on _currentTabWidth {
        // ### ideally, we disable the animation when processing a window // resize..
        NumberAnimation {
            duration: tabBar._animationDuration
        }
    }

    ListView {
        id: tabListView
        orientation: Qt.Horizontal
        interactive: false
        anchors.fill: parent
        model: tabBar.tabCount
        delegate: Rectangle {
            id: delegateItem
            color: tabBar.currentIndex == index ? "#eeeeee" : "#999999"
            ListView.onAdd: SequentialAnimation {
                PropertyAction {
                    target: delegateItem
                    property: "width"
                    value: 0
                }
                NumberAnimation {
                    target: delegateItem
                    property: "width"
                    to: tabBar._currentTabWidth
                    duration: tabBar._animationDuration
                    easing.type: Easing.InOutQuad
                }
            }
            ListView.onRemove: SequentialAnimation {
                PropertyAction {
                    target: delegateItem;
                    property: "ListView.delayRemove";
                    value: true
                }
                NumberAnimation {
                    target: delegateItem
                    property: "width"
                    to: 0
                    duration: tabBar._animationDuration
                    easing.type: Easing.InOutQuad
                }
                PropertyAction {
                    target: delegateItem;
                    property: "ListView.delayRemove";
                    value: false
                }
            }

            width: tabBar._currentTabWidth
            height: tabBar._tabHeight

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    tabBar.currentIndex = index
                }
            }

            MouseArea {
                id: closeIcon
                height: parent.height
                width: 10
                anchors.left: parent.left
                anchors.leftMargin: 10
                onClicked: tabBar.removeIndex(index)

                // TODO: an icon or something would be nice...
                Text {
                    text: "x"
                    anchors.centerIn: parent
                }
            }

            Text {
                id: tabTitle
                text: tabBar.tabArray[index]
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: closeIcon.right
                anchors.right: parent.right
                anchors.margins: 10
                elide: Text.ElideRight
            }

            Rectangle {
                anchors.right: parent.right
                width: 1
                height: parent.height
                color: "black"
            }
        }
    }

    // TODO: an overflow menu of sorts.
    Rectangle {
        x: parent.width - width
        height: parent.height
        width: 20
        color: "red"
        visible: tabBar.tabCount > tabBar._maxVisibleTabs

        MouseArea {
            anchors.fill: parent
            onClicked: {
                pickerDialog.visible = true
            }
        }

        Rectangle {
            id: pickerDialog
            visible: false
            height: childrenRect.height
            width: childrenRect.width
            color: "white"
            x: parent.width - (width + 10)
            y: 10

            Column {
                Repeater {
                    model: tabBar.tabCount
                    delegate: Rectangle {
                        width: 250
                        height: 25
                        visible: index >= tabBar._maxVisibleTabs
                        color: ma.containsMouse ? "#999999" : "white"

                        MouseArea {
                            id: ma
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                tabBar.currentIndex = index
                                pickerDialog.visible = false
                            }
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width - 20
                            x: 10
                            elide: Text.ElideRight
                            text: tabBar.tabArray[index]
                        }
                        Rectangle {
                            color: "black"
                            width: parent.width
                            height: 1
                            anchors.top: parent.top
                        }
                        Rectangle {
                            color: "black"
                            width: parent.width
                            height: 1
                            anchors.bottom: parent.bottom
                        }
                        Rectangle {
                            color: "black"
                            height: parent.height
                            width: 1
                            anchors.left: parent.left
                        }
                        Rectangle {
                            color: "black"
                            height: parent.height
                            width: 1
                            anchors.right: parent.right
                        }
                    }
                }
            }
        }
    }
}

