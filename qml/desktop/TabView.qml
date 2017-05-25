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

Column {
    id: root
    property alias currentIndex: tabBar.currentIndex
    property int count: tabContainer.children.length
    property Item activeTabItem
    TabBar {
        id: tabBar
        z: 1
        tabArray: tabContainer.tabTitles
        tabCount: 0

        onRemoveIndex: {
            root.removeTab(index)
        }
    }
    Item {
        id: tabContainer
        height: parent.height - tabBar.height
        width: parent.width
        property var tabTitles: []
    }

    onCurrentIndexChanged: {
        fixupVisibility()
    }

    function fixupVisibility() {
        var set = false
        for (var i = 0; i < tabContainer.children.length; ++i) {
            var child = tabContainer.children[i]
            child.visible = i == currentIndex
            if (child.visible) {
                child.forceActiveFocus();
                activeTabItem = child
                set = true
            }
        }

        if (!set) {
            activeTabItem = null
        }
    }

    function _updateTabTitle() {
        // NOTE NOTE NOTE! We are called bound, meaning 'this' refers to the tab
        // item delegate, not the TabView!
        for (var i = 0; i < tabContainer.children.length; ++i) {
            var obj = tabContainer.children[i]
            if (obj == this) {
                tabContainer.tabTitles[i] = this.title

                // Force binding updates for TabView. Bit yuck, but that's what
                // we get for using a JS array sadly.
                tabBar.tabCount = 0
                tabBar.tabCount = tabContainer.tabTitles.length
                return
            }
        }
    }

    function addTab(titleText, comp) {
        if (comp.status != Component.Ready) {
            console.warn("Component not ready! " + comp.errorString)
            return
        }
        var tabInstance = comp.createObject(tabContainer)
        var title = tabInstance.title ? tabInstance.title : "Shell " + tabContainer.children.length
        tabInstance.titleChanged.connect(_updateTabTitle.bind(tabInstance))
        tabContainer.tabTitles.push(title);
        tabBar.tabCount = tabContainer.tabTitles.length

        // Force focus if need be.
        fixupVisibility();

        return tabInstance
    }

    function removeTab(tabIndex) {
        var tab = tabContainer.children[tabIndex]
        tab.parent = null
        tab.destroy()
        tabContainer.tabTitles.splice(tabIndex, 1)
        tabBar.tabCount = tabContainer.tabTitles.length
        var ci = currentIndex
        if (tabIndex >= ci)
            ci--
        if (ci < 0)
            ci = 0
        currentIndex = ci

        // We must call this. If currentIndex hasn't changed (i.e. closing
        // index 0), we need to fix up visibility to show what was formerly tab
        // 1.
        fixupVisibility(); 
    }

    function getTab(tabIndex) {
        return tabContainer.children[tabIndex]
    }
}
