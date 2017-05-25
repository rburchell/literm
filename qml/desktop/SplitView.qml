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

FocusScope {
    id: root

    property var title: activeItem.title

    Component {
        id: splitView

        Item {
            id: splitView
            objectName: "view"
            readonly property bool isSplitView: true
            property bool isHorizontal: true
            property int childCount: children.length

            property real widthRatio: 1.0
            property real heightRatio: 1.0

            function layout() {
                var cpos = 0
                for (var i = 0; i < childCount; ++i) {
                    var kid = children[i]

                    if (kid.hasOwnProperty("isSplitter")) {
                        if (isHorizontal) {
                            kid.width = 10
                            kid.height = height
                        } else {
                            kid.height = 10
                            kid.width = width
                        }
                        if (isHorizontal) {
                            kid.x = cpos - kid.width / 2
                            kid.y = 0
                        } else {
                            kid.x = 0
                            kid.y = cpos - kid.height / 2
                        }
                    } else {
                        kid.width = width * kid.widthRatio
                        kid.height = height * kid.heightRatio
                        if (isHorizontal) {
                            kid.x = cpos 
                            cpos += kid.width
                            kid.y = 0
                        } else {
                            kid.x = 0
                            kid.y = cpos
                            cpos += kid.height
                        }
                    }
                }
            }

            // XXX Delay to next frame?
            onWidthChanged: layout()
            onHeightChanged: layout()
        }
    }

    Component {
        id: delegate
        TerminalPane {
        }
    }
    Component {
        id: splitter
        TerminalPaneSplitter {
        }
    }

    property Item rootItem
    Component.onCompleted: {
        // rootItem is a splitView, for consistency and easier sizing/splitting behavior.
        rootItem = splitView.createObject(root, { 'anchors.fill': root })
        // give it a child, though
        activeItem = delegate.createObject(rootItem)

        // ### this is not generic
        activeItem.hangupReceived.connect(function() {
            removeAndDestroyItem(activeItem)
            //closeTab(tab)
        });

        rootItem.layout()
    }

    // poor man's QQuickItem::stackAfter. find the object, then record
    // and remove everything after it...
    function reparentIntoEther(wrapper, searchFor) {
        var tmp = []
        var found = false

        for (var i = 0; i < wrapper.children.length; ++i) {
            var o = wrapper.children[i]
            if (o == searchFor) {
                found = true
            } else if (found) {
                tmp.push(wrapper.children[i])
            }
        }

        // reparent them all to nowhere (but they're safe, referenced in tmp)
        for (var i = 0; i < tmp.length; ++i) {
            tmp[i].parent = null
        }

        return tmp
    }

    function parentBack(wrapper, tmp) {
        // reparent them back
        for (var i = 0; i < tmp.length; ++i) {
            tmp[i].parent = wrapper
        }
    }

    function split(item, horizontal) {
        // parentSplit is kept the same as item.parent, just for convenience
        var parentSplit = item.parent

        if (!parentSplit.hasOwnProperty('isSplitView') || parentSplit.isHorizontal !== horizontal) {
            var tmp = reparentIntoEther(parentSplit, item)
            // Parent isn't a same-orientation split view, so we'll need to wrap activeItem in a new one
            var newSplit = splitView.createObject(parentSplit, { 'isHorizontal': horizontal })
            parentBack(parentSplit, tmp)

            newSplit.widthRatio = item.widthRatio
            newSplit.heightRatio = item.heightRatio

            item.widthRatio = 1.0
            item.heightRatio = 1.0

            item.parent = newSplit
            parentSplit.layout()
            parentSplit = newSplit
        }

        var wrapper = item.parent
        var tmp = reparentIntoEther(wrapper, item)

        // Halve the current size
        if (horizontal) {
            item.widthRatio /= 2
        } else {
            item.heightRatio /= 2
        }

        splitter.createObject(parentSplit)
        var sibling = delegate.createObject(parentSplit)
        sibling.widthRatio = item.widthRatio
        sibling.heightRatio = item.heightRatio

        parentBack(wrapper, tmp)

        parentSplit.layout()
        return sibling
    }

    property Item activeItem
    Shortcut {
        sequence: "Ctrl+D"
        onActivated: {
            activeItem = split(activeItem, true)
            activeItem.forceActiveFocus()
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+D"
        onActivated: {
            activeItem = split(activeItem, false)
        }
    }


    function removeAndDestroyItem(item) {
        var layout = item.parent

        for (var i = 0; i < layout.children.length; ++i) {
            if (layout.children[i] == item) {
                var oldW = item.widthRatio
                var oldH = item.heightRatio

                // First let's destroy the right splitter.
                if (i == 0) {
                    // If we remove the first item, we must remove the first splitter.
                    layout.children[i].parent = null
                    layout.children[i].parent = null
                } else {
                    // Otherwise, we can just remove the splitter "behind" the
                    // object.
                    layout.children[i-1].parent = null
                    // And the object itself.
                    layout.children[i-1].parent = null

                    // If we remove the last item, we need to decrement so we
                    // can find the previous item successfully.
                    if (i - 1 == layout.children.length) {
                        i -= 2
                    }
                }

                if (layout.children.length > 1) {
                    // If > 1, then there must be at least two items (and a
                    // splitter) left in this layout.
                    //
                    // Find the next item, and increase its size.
                    var nextChild = layout.children[i]
                    if (layout.isHorizontal) {
                        nextChild.widthRatio += oldW
                    } else {
                        nextChild.heightRatio += oldH
                    }
                    layout.layout()
                    return nextChild
                } else {
                    // If it wasn't on the other hand, then the layout has a
                    // single child left in it. Remove the layout, demoting the
                    // item to being a sibling in the parent's layout.
                    //
                    // Sole exception here is the root layout, which must not be
                    // messed with. In that case, resize the child, but leave it
                    // intact.
                    if (layout == rootItem) {
                        var item = layout.children[0]
                        item.widthRatio = layout.widthRatio
                        item.heightRatio = layout.heightRatio
                        layout.layout()
                        return item
                    } else {
                        var item = layout.children[0]
                        var oldLayout = item.parent
                        var newLayout = oldLayout.parent

                        // Stack into the right place in newLayout
                        var tmp = reparentIntoEther(newLayout, oldLayout)

                        item.parent = layout.parent
                        item.widthRatio = layout.widthRatio
                        item.heightRatio = layout.heightRatio

                        parentBack(item.parent, tmp)

                        // Destroy old layout, and relayout.
                        layout.parent = null
                        item.parent.layout()
                        return item
                    }
               }
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+W"
        onActivated: {
            var tmp = removeAndDestroyItem(activeItem)
            if (tmp.hasOwnProperty('isSplitView')) {
                // If a view is returned, set active to the last child.
                activeItem = tmp.children[tmp.children.length - 1]
            } else {
                activeItem = tmp
            }
        }
    }
}

