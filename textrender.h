/*
    Copyright (C) 2017-2020 Crimson AS <info@crimson.no>

    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is furnished to
    do so, subject to the following conditions:

        The above copyright notice and this permission notice shall be included
        in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// NB: This file contains code originally inspired by Heikki Holstila (in 2011-2012).
// It has subsequently been rewritten. Many thanks for his work, though.

#pragma once
#include <QQuickItem>
#include "terminal.h"

class TextRender : public QQuickItem
{
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(DragMode dragMode READ dragMode WRITE setDragMode NOTIFY dragModeChanged)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(QSizeF cellSize READ cellSize NOTIFY cellSizeChanged)
    Q_PROPERTY(bool showBufferScrollIndicator READ showBufferScrollIndicator WRITE setShowBufferScrollIndicator NOTIFY showBufferScrollIndicatorChanged)
    Q_PROPERTY(bool allowGestures READ allowGestures WRITE setAllowGestures NOTIFY allowGesturesChanged)
    Q_PROPERTY(QQmlComponent* cellContentsDelegate READ cellContentsDelegate WRITE setCellContentsDelegate NOTIFY cellContentsDelegateChanged)
    Q_PROPERTY(QQmlComponent* cursorDelegate READ cursorDelegate WRITE setCursorDelegate NOTIFY cursorDelegateChanged)
    Q_PROPERTY(QQmlComponent* selectionDelegate READ selectionDelegate WRITE setSelectionDelegate NOTIFY selectionDelegateChanged)
    Q_PROPERTY(int contentHeight READ contentHeight NOTIFY contentHeightChanged)
    Q_PROPERTY(int visibleHeight READ visibleHeight NOTIFY visibleHeightChanged)
    Q_PROPERTY(int contentY READ contentY NOTIFY contentYChanged)
    Q_PROPERTY(QSize terminalSize READ terminalSize NOTIFY terminalSizeChanged)
    Q_PROPERTY(QString selectedText READ selectedText NOTIFY selectionChanged)
    Q_PROPERTY(bool canPaste READ canPaste NOTIFY clipboardChanged)
    Q_PROPERTY(QString charset READ charset WRITE setCharset NOTIFY charsetChanged)
    Q_PROPERTY(QString terminalCommand READ terminalCommand WRITE setTerminalCommand NOTIFY terminalCommandChanged)
    Q_PROPERTY(QByteArray terminalEnvironment READ terminalEnvironment WRITE setTerminalEnvironment NOTIFY terminalEnvironmentChanged)

    Q_OBJECT
public:
    explicit TextRender(QQuickItem *parent = 0);
    virtual ~TextRender();

    Q_INVOKABLE const QStringList printableLinesFromCursor(int lines);
    Q_INVOKABLE void putString(QString str);
    Q_INVOKABLE const QStringList grabURLsFromBuffer();

    QString charset() const;
    void setCharset(const QString &charset);
    QString terminalCommand() const;
    void setTerminalCommand(const QString &terminalCommand);
    QByteArray terminalEnvironment() const;
    void setTerminalEnvironment(const QByteArray &terminalEnvironment);

    bool canPaste() const;
    Q_INVOKABLE void copy();
    Q_INVOKABLE void paste();
    Q_INVOKABLE void deselect();

    QString selectedText() const;

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

    QSize terminalSize() const;
    QFont font() const;
    void setFont(const QFont &font);
    bool showBufferScrollIndicator() { return iShowBufferScrollIndicator; }
    void setShowBufferScrollIndicator(bool s) { if(iShowBufferScrollIndicator!=s) { iShowBufferScrollIndicator=s; emit showBufferScrollIndicatorChanged(); } }

    Q_INVOKABLE QPointF cursorPixelPos();
    QSizeF cellSize();

    bool allowGestures();
    void setAllowGestures(bool allow);

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
    void cellSizeChanged();
    void showBufferScrollIndicatorChanged();
    void allowGesturesChanged();
    void cellContentsDelegateChanged();
    void cursorDelegateChanged();
    void selectionDelegateChanged();
    void visualBell();
    void titleChanged();
    void dragModeChanged();
    void contentHeightChanged();
    void visibleHeightChanged();
    void contentYChanged();
    void terminalSizeChanged();
    void clipboardChanged();
    void selectionChanged();
    void charsetChanged();
    void terminalCommandChanged();
    void terminalEnvironmentChanged();
    void displayBufferChanged();
    void panLeft();
    void panRight();
    void panUp();
    void panDown();
    void hangupReceived();

public slots:
    void redraw();
    void mousePress(float eventX, float eventY);
    void mouseMove(float eventX, float eventY);
    void mouseRelease(float eventX, float eventY);

protected:
    void updatePolish() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void componentComplete() override;
    QSGNode* updatePaintNode(QSGNode* oldNode, QQuickItem::UpdatePaintNodeData *updatePaintNodeData) override;

private slots:
    void handleScrollBack(bool reset);
    void handleTitleChanged(const QString &title);

private:
    Q_DISABLE_COPY(TextRender)

    enum PanGesture { PanNone, PanLeft, PanRight, PanUp, PanDown };

    struct BgFragment {
        qreal x = 0;
        qreal y = 0;
        qreal width = 0;
        qreal opacity = 0;
        TermChar style;
    };
    void drawBgFragment(qreal x, qreal y, qreal width, qreal opacity, TermChar style);
    void drawTextFragment(QQuickItem *cellContentsDelegate, qreal x, qreal y, QString text, TermChar style);
    void paintFromBuffer(const TerminalBuffer &buffer, int from, int to, qreal &y, int &yDelegateIndex);
    QPointF charsToPixels(QPoint pos);
    void selectionHelper(QPointF scenePos, bool selectionOngoing);

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

    QQuickItem *fetchFreeCellContent();

    QPointF dragOrigin;
    bool m_activeClick;

    QFont iFont;
    qreal iFontWidth;
    qreal iFontHeight;
    qreal iFontDescent;
    bool iShowBufferScrollIndicator;
    bool iAllowGestures;

    QQuickItem *m_textContainer;
    QQuickItem *m_overlayContainer;
    QQmlComponent *m_cellContentsDelegate;
    QVector<QQuickItem*> m_cellsContent;
    QVector<QQuickItem*> m_freeCellsContent;
    QQmlComponent *m_cursorDelegate;
    QQuickItem *m_cursorDelegateInstance;
    QQmlComponent *m_selectionDelegate;
    QQuickItem *m_topSelectionDelegateInstance;
    QQuickItem *m_middleSelectionDelegateInstance;
    QQuickItem *m_bottomSelectionDelegateInstance;
    DragMode m_dragMode;
    QString m_title;
    int m_dispatch_timer;
    Terminal m_terminal;
    QVector<BgFragment> m_bgFragments;
};
