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
Rectangle {
    objectName: "splitter"
    readonly property bool isSplitter: true
    color: "red"
    z: 100
    MouseArea {
        anchors.fill: parent

        property int childIndexBefore: -1
        property int childIndexAfter: -1
        property int pressX
        property int pressY

        onPressed: {
            var layout = parent.parent
            var splitter = parent

            for (var i = 0; i < layout.children.length; ++i) {
                if (layout.children[i] == splitter) {
                    childIndexAfter = i + 1
                    break
                }
                childIndexBefore = i
            }

            pressX = mouse.x
            pressY = mouse.y
        }

        onPositionChanged: {
            var deltaX = pressX - mouse.x
            var deltaY = pressY - mouse.y

            var layout = parent.parent
            if (layout.isHorizontal) {
                var ratioDelta = (deltaX / layout.width)
                layout.children[childIndexBefore].widthRatio += -ratioDelta
                layout.children[childIndexAfter].widthRatio += ratioDelta
            } else {
                var ratioDelta = (deltaY / layout.height)
                layout.children[childIndexBefore].heightRatio += -ratioDelta
                layout.children[childIndexAfter].heightRatio += ratioDelta
            }

            layout.layout()
        }

        onReleased: {
            pressX = 0
            pressY = 0
            childIndexBefore = -1
            childIndexAfter = -1
        }
    }
}


