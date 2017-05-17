/*
    Copyright (C) 2017 Crimson AS <info@crimson.no>
    Copyright 2011-2012 Heikki Holstila <heikki.holstila@gmail.com>

    This file is part of FingerTerm.

    FingerTerm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    FingerTerm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FingerTerm.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TEXTRENDER_H
#define TEXTRENDER_H

#include <QQuickItem>

#include "terminal.h"

class Util;

class TextRender : public QQuickItem
{
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(DragMode dragMode READ dragMode WRITE setDragMode NOTIFY dragModeChanged)
    Q_PROPERTY(QQuickItem* contentItem READ contentItem WRITE setContentItem NOTIFY contentItemChanged)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(bool showBufferScrollIndicator READ showBufferScrollIndicator WRITE setShowBufferScrollIndicator NOTIFY showBufferScrollIndicatorChanged)
    Q_PROPERTY(bool allowGestures READ allowGestures WRITE setAllowGestures NOTIFY allowGesturesChanged)
    Q_PROPERTY(QQmlComponent* cellDelegate READ cellDelegate WRITE setCellDelegate NOTIFY cellDelegateChanged)
    Q_PROPERTY(QQmlComponent* cellContentsDelegate READ cellContentsDelegate WRITE setCellContentsDelegate NOTIFY cellContentsDelegateChanged)
    Q_PROPERTY(QQmlComponent* cursorDelegate READ cursorDelegate WRITE setCursorDelegate NOTIFY cursorDelegateChanged)
    Q_PROPERTY(QQmlComponent* selectionDelegate READ selectionDelegate WRITE setSelectionDelegate NOTIFY selectionDelegateChanged)
    Q_PROPERTY(int contentHeight READ contentHeight NOTIFY contentHeightChanged)
    Q_PROPERTY(int visibleHeight READ visibleHeight NOTIFY visibleHeightChanged)
    Q_PROPERTY(int contentY READ contentY NOTIFY contentYChanged)

    Q_OBJECT
public:
    explicit TextRender(QQuickItem *parent = 0);
    virtual ~TextRender();
    void updatePolish() override;

    int contentHeight() const;
    int visibleHeight() const;
    int contentY() const;

    QString title() const;

    enum DragMode {
        DragOff,
        DragGestures,
        DragScroll,
        DragSelect
    };
    Q_ENUMS(DragMode)

    DragMode dragMode() const;
    void setDragMode(DragMode dragMode);

    static void setUtil(Util *util);
    static void setTerminal(Terminal *terminal);

    QQuickItem *contentItem() const { return m_contentItem; }
    void setContentItem(QQuickItem *contentItem);

    QFont font() const;
    void setFont(const QFont &font);
    bool showBufferScrollIndicator() { return iShowBufferScrollIndicator; }
    void setShowBufferScrollIndicator(bool s) { if(iShowBufferScrollIndicator!=s) { iShowBufferScrollIndicator=s; emit showBufferScrollIndicatorChanged(); } }

    Q_INVOKABLE QPointF cursorPixelPos();
    Q_INVOKABLE QSizeF cursorPixelSize();

    bool allowGestures();
    void setAllowGestures(bool allow);

    QQmlComponent *cellDelegate() const;
    void setCellDelegate(QQmlComponent *delegate);
    QQmlComponent *cellContentsDelegate() const;
    void setCellContentsDelegate(QQmlComponent *delegate);
    QQmlComponent *cursorDelegate() const;
    void setCursorDelegate(QQmlComponent *delegate);
    QQmlComponent *selectionDelegate() const;
    void setSelectionDelegate(QQmlComponent *delegate);

signals:
    void contentItemChanged();
    void fontChanged();
    void showBufferScrollIndicatorChanged();
    void allowGesturesChanged();
    void cellDelegateChanged();
    void cellContentsDelegateChanged();
    void cursorDelegateChanged();
    void selectionDelegateChanged();
    void visualBell();
    void titleChanged();
    void dragModeChanged();
    void contentHeightChanged();
    void visibleHeightChanged();
    void contentYChanged();

public slots:
    void redraw();

protected:
    bool childMouseEventFilter(QQuickItem *item,  QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private slots:
    void handleScrollBack(bool reset);
    void handleTitleChanged(const QString &title);

private:
    Q_DISABLE_COPY(TextRender)

    enum PanGesture { PanNone, PanLeft, PanRight, PanUp, PanDown };

    void drawBgFragment(QQuickItem *cellContentsDelegate, qreal x, qreal y, int width, TermChar style);
    void drawTextFragment(QQuickItem *cellContentsDelegate, qreal x, qreal y, QString text, TermChar style);
    void paintFromBuffer(const TerminalBuffer &buffer, int from, int to, qreal &y, int &yDelegateIndex);
    QPointF charsToPixels(QPoint pos);
    void selectionHelper(QPointF scenePos, bool selectionOngoing);
    void ensureRowPopulated(QVector<QQuickItem*> &row, QVector<QQuickItem*> &rowContents, int columnCount);

    qreal fontWidth() { return iFontWidth; }
    qreal fontHeight() { return iFontHeight; }
    qreal fontDescent() { return iFontDescent; }
    int fontPointSize() { return iFont.pointSize(); }

    /**
     * Scroll the back buffer on drag.
     *
     * @param now The current position
     * @param last The last position (or start position)
     * @return The new value for last (modified by any consumed offset)
     **/
    QPointF scrollBackBuffer(QPointF now, QPointF last);
    void doGesture(PanGesture gesture);

    bool newSelection;
    QPointF dragOrigin;

    QFont iFont;
    qreal iFontWidth;
    qreal iFontHeight;
    qreal iFontDescent;
    bool iShowBufferScrollIndicator;
    bool iAllowGestures;

    static Terminal *sTerm;
    static Util *sUtil;

    QQuickItem *m_contentItem;
    QQuickItem *m_backgroundContainer;
    QQuickItem *m_textContainer;
    QQuickItem *m_overlayContainer;
    QQmlComponent *m_cellDelegate;
    QVector<QVector<QQuickItem*>> m_cells;
    QQmlComponent *m_cellContentsDelegate;
    QVector<QVector<QQuickItem*>> m_cellsContent;
    QQmlComponent *m_cursorDelegate;
    QQuickItem *m_cursorDelegateInstance;
    QQmlComponent *m_selectionDelegate;
    QQuickItem *m_topSelectionDelegateInstance;
    QQuickItem *m_middleSelectionDelegateInstance;
    QQuickItem *m_bottomSelectionDelegateInstance;
    DragMode m_dragMode;
    QString m_title;
    int m_dispatch_timer;
};

#endif // TEXTRENDER_H
