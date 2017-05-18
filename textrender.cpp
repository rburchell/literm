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

#include <QtGui>
#include "textrender.h"
#include "terminal.h"
#include "utilities.h"

Terminal* TextRender::sTerm = 0;
Util* TextRender::sUtil = 0;

/*!
 * \internal
 *
 * TextRender is a QQuickItem that acts as a view for data from a Terminal,
 * and serves as an interaction point with a Terminal.
 *
 * TextRender is organized of a number of different parts. The user is expected
 * to set a number of "delegates", which are the pieces instantiated by
 * TextRender to correspond with the data from the Terminal. For instance, there
 * is a background cell delegate (for coloring), a cell contents delegate (for
 * the text), a cursor delegate, and so on.
 *
 * TextRender organises its child delegate instances in a slightly complex way,
 * due to the amount of items it manages, and the requirements involved:
 *
 * TextRender
 *      contentItem
 *          backgroundContainer
 *              cellDelegates
 *          textContainer
 *              cellContentsDelegates
 *          overlayContainer
 *              cursorDelegate
 *              selectionDelegates
 *
 * The contentItem is separate from TextRender itself so that contentItem can
 * have visual effects applied (like a <1.0 opacity) without affecting items
 * that are placed inside TextRender on the user's side. This is used in the
 * mobile UX for instance, where the keyboard is placed inside TextRender, and
 * opacity on the keyboard and TextRender's contentItem are swapped when the
 * keyboard transitions to and from active state.
 */

TextRender::TextRender(QQuickItem *parent)
    : QQuickItem(parent)
    , newSelection(true)
    , iAllowGestures(true)
    , m_contentItem(0)
    , m_backgroundContainer(0)
    , m_textContainer(0)
    , m_overlayContainer(0)
    , m_cellDelegate(0)
    , m_cellContentsDelegate(0)
    , m_cursorDelegate(0)
    , m_cursorDelegateInstance(0)
    , m_selectionDelegate(0)
    , m_topSelectionDelegateInstance(0)
    , m_middleSelectionDelegateInstance(0)
    , m_bottomSelectionDelegateInstance(0)
    , m_dragMode(DragScroll)
    , m_dispatch_timer(0)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setFiltersChildMouseEvents(true);

    connect(QGuiApplication::clipboard(), SIGNAL(dataChanged()), this, SIGNAL(clipboardChanged()));

    connect(this,SIGNAL(widthChanged()),this,SLOT(redraw()));
    connect(this,SIGNAL(heightChanged()),this,SLOT(redraw()));

    iShowBufferScrollIndicator = false;

    Q_ASSERT(sTerm);
    connect(sTerm, SIGNAL(windowTitleChanged(const QString&)), this, SLOT(handleTitleChanged(const QString&)));
    connect(sTerm, SIGNAL(visualBell()), this, SIGNAL(visualBell()));
    connect(sTerm, SIGNAL(displayBufferChanged()), this, SLOT(redraw()));
    connect(sTerm, SIGNAL(cursorPosChanged(QPoint)), this, SLOT(redraw()));
    connect(sTerm, SIGNAL(termSizeChanged(int,int)), this, SLOT(redraw()));
    connect(sTerm, SIGNAL(termSizeChanged(int,int)), this, SIGNAL(terminalSizeChanged()));
    connect(sTerm, SIGNAL(selectionChanged()), this, SLOT(redraw()));
    connect(sTerm, SIGNAL(scrollBackBufferAdjusted(bool)), this, SLOT(handleScrollBack(bool)));
    connect(sTerm, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
}

TextRender::~TextRender()
{
}

void TextRender::copy()
{
    QClipboard *cb = QGuiApplication::clipboard();
    cb->clear();
    cb->setText(selectedText());
}

void TextRender::paste()
{
    QClipboard *cb = QGuiApplication::clipboard();
    QString cbText = cb->text();
    sTerm->paste(cbText);
}

bool TextRender::canPaste() const
{
    QClipboard *cb = QGuiApplication::clipboard();

    return !cb->text().isEmpty();
}

void TextRender::deselect()
{
    sTerm->clearSelection();
}

QString TextRender::selectedText() const
{
    return sTerm->selectedText();
}

QSize TextRender::terminalSize() const
{
    return QSize(sTerm->columns(), sTerm->rows());
}

QString TextRender::title() const
{
    return m_title;
}

void TextRender::handleTitleChanged(const QString &newTitle)
{
    if (m_title == newTitle)
        return;

    m_title = newTitle;
    emit titleChanged();
}

TextRender::DragMode TextRender::dragMode() const
{
    return m_dragMode;
}

void TextRender::setDragMode(DragMode dragMode)
{
    if (m_dragMode == dragMode)
        return;

    m_dragMode = dragMode;
    emit dragModeChanged();
}

void TextRender::setContentItem(QQuickItem *contentItem)
{
    Q_ASSERT(!m_contentItem); // changing this requires work
    m_contentItem = contentItem;
    m_contentItem->setParentItem(this);
    m_backgroundContainer = new QQuickItem(m_contentItem);
    m_backgroundContainer->setClip(true);
    m_textContainer = new QQuickItem(m_contentItem);
    m_textContainer->setClip(true);
    m_overlayContainer = new QQuickItem(m_contentItem);
    m_overlayContainer->setClip(true);
    polish();
}

void TextRender::setFont(const QFont &font)
{
    if (iFont == font)
        return;

    iFont = font;
    QFontMetricsF fontMetrics(iFont);
    iFontHeight = fontMetrics.height();
    iFontWidth = fontMetrics.maxWidth();
    iFontDescent = fontMetrics.descent();

    polish();
    emit fontChanged();
}

QFont TextRender::font() const
{
    return iFont;
}

/*!
 * \internal
 *
 * Ensures \a row and \a rowContents have enough delegates to display
 * \a columnCount columns
 */
void TextRender::ensureRowPopulated(QVector<QQuickItem*> &row, QVector<QQuickItem*> &rowContents, int columnCount)
{
    row.reserve(columnCount);
    rowContents.reserve(columnCount);

    for (int col = row.size(); col < columnCount; ++col) {
        QQuickItem *it = nullptr;

        it = qobject_cast<QQuickItem*>(m_cellDelegate->create(qmlContext(this)));
        it->setVisible(false);
        it->setParentItem(m_backgroundContainer);
        Q_ASSERT(it);
        row.append(it);

        it = qobject_cast<QQuickItem*>(m_cellContentsDelegate->create(qmlContext(this)));
        it->setVisible(false);
        it->setParentItem(m_textContainer);
        Q_ASSERT(it);
        rowContents.append(it);
    }
}

void TextRender::updatePolish()
{
    if (!m_contentItem)
        return;

    // ### these should be handled more carefully
    emit contentYChanged();
    emit visibleHeightChanged();
    emit contentHeightChanged();

    // Make sure the terminal's size is right
    QSize size((width() - 4) / iFontWidth, (height() - 4) / iFontHeight);
    sTerm->setTermSize(size);

    m_contentItem->setWidth(width());
    m_contentItem->setHeight(height());
    m_backgroundContainer->setWidth(width());
    m_backgroundContainer->setHeight(height());
    m_textContainer->setWidth(width());
    m_textContainer->setHeight(height());
    m_overlayContainer->setWidth(width());
    m_overlayContainer->setHeight(height());

    // If the height grows, make sure we have enough rows
    if (m_cells.size() < sTerm->rows()) {
        const int oldSize = m_cells.size();

        m_cells.resize(sTerm->rows());
        m_cellsContent.resize(sTerm->rows());

        const int columnCount = sTerm->columns();

        for (int row = oldSize; row < m_cells.size(); ++row) {
            auto &cellRow = m_cells[row];
            auto &contentsRow = m_cellsContent[row];
            ensureRowPopulated(cellRow, contentsRow, columnCount);
        }
    }

    // Ensure that there's sufficient width too, if that changed.
    if (m_cells[0].size() < sTerm->columns()) {
        const int columnCount = sTerm->columns();
        for (int row = 0; row < sTerm->rows(); ++row) {
            auto &cellRow = m_cells[row];
            auto &contentsRow = m_cellsContent[row];
            ensureRowPopulated(cellRow, contentsRow, columnCount);
        }
    }

    qreal y = 0;
    int yDelegateIndex = 0;
    if (sTerm->backBufferScrollPos() != 0 && sTerm->backBuffer().size()>0) {
        int from = sTerm->backBuffer().size() - sTerm->backBufferScrollPos();
        if(from<0)
            from=0;
        int to = sTerm->backBuffer().size();
        if(to-from > sTerm->rows())
            to = from + sTerm->rows();
        paintFromBuffer(sTerm->backBuffer(), from, to, y, yDelegateIndex);
        if(to-from < sTerm->rows() && sTerm->buffer().size()>0) {
            int to2 = sTerm->rows() - (to-from);
            if(to2 > sTerm->buffer().size())
                to2 = sTerm->buffer().size();
            paintFromBuffer(sTerm->buffer(), 0, to2, y, yDelegateIndex);
        }
    } else {
        int count = qMin(sTerm->rows(), sTerm->buffer().size());
        paintFromBuffer(sTerm->buffer(), 0, count, y, yDelegateIndex);
    }

    // paint any remaining rows unused
    for (; yDelegateIndex < m_cells.size(); ++yDelegateIndex) {
        for (int j=0;j<sTerm->columns(); j++) {
            m_cells.at(yDelegateIndex).at(j)->setVisible(false);
            m_cellsContent.at(yDelegateIndex).at(j)->setVisible(false);
        }
    }

    // cursor
    if (sTerm->showCursor()) {
        if (!m_cursorDelegateInstance) {
            m_cursorDelegateInstance = qobject_cast<QQuickItem*>(m_cursorDelegate->create(qmlContext(this)));
            m_cursorDelegateInstance->setVisible(false);
            m_cursorDelegateInstance->setParentItem(m_overlayContainer);
        }

        m_cursorDelegateInstance->setVisible(true);
        QPointF cursor = cursorPixelPos();
        QSizeF csize = cursorPixelSize();
        m_cursorDelegateInstance->setX(cursor.x());
        m_cursorDelegateInstance->setY(cursor.y());
        m_cursorDelegateInstance->setWidth(csize.width());
        m_cursorDelegateInstance->setHeight(csize.height());
        m_cursorDelegateInstance->setProperty("color", Terminal::defaultFgColor);
    } else if (m_cursorDelegateInstance) {
        m_cursorDelegateInstance->setVisible(false);
    }

    QRect selection = sTerm->selection();
    if (!selection.isNull()) {
        if (!m_topSelectionDelegateInstance) {
            m_topSelectionDelegateInstance = qobject_cast<QQuickItem*>(m_selectionDelegate->create(qmlContext(this)));
            m_topSelectionDelegateInstance->setVisible(false);
            m_topSelectionDelegateInstance->setParentItem(m_overlayContainer);

            m_middleSelectionDelegateInstance = qobject_cast<QQuickItem*>(m_selectionDelegate->create(qmlContext(this)));
            m_middleSelectionDelegateInstance->setVisible(false);
            m_middleSelectionDelegateInstance->setParentItem(m_overlayContainer);

            m_bottomSelectionDelegateInstance = qobject_cast<QQuickItem*>(m_selectionDelegate->create(qmlContext(this)));
            m_bottomSelectionDelegateInstance->setVisible(false);
            m_bottomSelectionDelegateInstance->setParentItem(m_overlayContainer);
        }

        if (selection.top() == selection.bottom()) {
            QPointF start = charsToPixels(selection.topLeft());
            QPointF end = charsToPixels(selection.bottomRight());
            m_topSelectionDelegateInstance->setVisible(false);
            m_bottomSelectionDelegateInstance->setVisible(false);
            m_middleSelectionDelegateInstance->setVisible(true);
            m_middleSelectionDelegateInstance->setX(start.x());
            m_middleSelectionDelegateInstance->setY(start.y());
            m_middleSelectionDelegateInstance->setWidth(end.x() - start.x() + fontWidth());
            m_middleSelectionDelegateInstance->setHeight(end.y() - start.y() + fontHeight());
        } else {
            m_topSelectionDelegateInstance->setVisible(true);
            m_bottomSelectionDelegateInstance->setVisible(true);
            m_middleSelectionDelegateInstance->setVisible(true);

            QPointF start = charsToPixels(selection.topLeft());
            QPointF end = charsToPixels(QPoint(sTerm->columns(), selection.top()));
            m_topSelectionDelegateInstance->setX(start.x());
            m_topSelectionDelegateInstance->setY(start.y());
            m_topSelectionDelegateInstance->setWidth(end.x() - start.x() + fontWidth());
            m_topSelectionDelegateInstance->setHeight(end.y() - start.y() + fontHeight());

            start = charsToPixels(QPoint(1, selection.top() + 1));
            end = charsToPixels(QPoint(sTerm->columns(), selection.bottom() - 1));

            m_middleSelectionDelegateInstance->setX(start.x());
            m_middleSelectionDelegateInstance->setY(start.y());
            m_middleSelectionDelegateInstance->setWidth(end.x() - start.x() + fontWidth());
            m_middleSelectionDelegateInstance->setHeight(end.y() - start.y() + fontHeight());

            start = charsToPixels(QPoint(1, selection.bottom()));
            end = charsToPixels(selection.bottomRight());

            m_bottomSelectionDelegateInstance->setX(start.x());
            m_bottomSelectionDelegateInstance->setY(start.y());
            m_bottomSelectionDelegateInstance->setWidth(end.x() - start.x() + fontWidth());
            m_bottomSelectionDelegateInstance->setHeight(end.y() - start.y() + fontHeight());
        }
    } else if (m_topSelectionDelegateInstance) {
        m_topSelectionDelegateInstance->setVisible(false);
        m_bottomSelectionDelegateInstance->setVisible(false);
        m_middleSelectionDelegateInstance->setVisible(false);
    }
}

void TextRender::paintFromBuffer(const TerminalBuffer &buffer, int from, int to, qreal &y, int &yDelegateIndex)
{
    const int leftmargin = 2;
    int cutAfter = property("cutAfter").toInt() + iFontDescent;

    TermChar tmp = sTerm->zeroChar;
    TermChar nextAttrib = sTerm->zeroChar;
    TermChar currAttrib = sTerm->zeroChar;
    qreal currentX = leftmargin;

    for(int i=from; i<to; i++, yDelegateIndex++) {
        y += iFontHeight;

        // ### if the background containers also had a container per row, we
        // could set the opacity there, rather than on each fragment.
        qreal opacity = 1.0;
        if(y >= cutAfter)
            opacity = 0.3;

        const auto &lineBuffer = buffer.at(i);;
        int xcount = qMin(lineBuffer.count(), sTerm->columns());

        // background for the current line
        currentX = leftmargin;
        qreal fragWidth = 0;
        int xDelegateIndex = 0;
        for(int j=0; j<xcount; j++) {
            fragWidth += iFontWidth;
            if (j==0) {
                tmp = lineBuffer.at(j);
                currAttrib = tmp;
                nextAttrib = tmp;
            } else if (j<xcount-1) {
                nextAttrib = lineBuffer.at(j+1);
            }

            if (currAttrib.attrib != nextAttrib.attrib ||
                currAttrib.bgColor != nextAttrib.bgColor ||
                currAttrib.fgColor != nextAttrib.fgColor ||
                j==xcount-1)
            {
                QQuickItem *backgroundRectangle = m_cells.at(yDelegateIndex).at(xDelegateIndex++);
                drawBgFragment(backgroundRectangle, currentX, y-iFontHeight+iFontDescent, std::ceil(fragWidth), currAttrib);
                backgroundRectangle->setOpacity(opacity);
                currentX += fragWidth;
                fragWidth = 0;
                currAttrib.attrib = nextAttrib.attrib;
                currAttrib.bgColor = nextAttrib.bgColor;
                currAttrib.fgColor = nextAttrib.fgColor;
            }
        }

        // Mark all remaining background cells unused.
        for (int j=xDelegateIndex;j<m_cells.at(yDelegateIndex).size(); j++) {
            m_cells.at(yDelegateIndex).at(j)->setVisible(false);
        }

        // text for the current line
        QString line;
        currentX = leftmargin;
        xDelegateIndex = 0;
        for (int j=0; j<xcount; j++) {
            tmp = lineBuffer.at(j);
            line += tmp.c;
            if (j==0) {
                currAttrib = tmp;
                nextAttrib = tmp;
            } else if(j<xcount-1) {
                nextAttrib = lineBuffer.at(j+1);
            }

            if (currAttrib.attrib != nextAttrib.attrib ||
                currAttrib.bgColor != nextAttrib.bgColor ||
                currAttrib.fgColor != nextAttrib.fgColor ||
                j==xcount-1)
            {
                QQuickItem *foregroundText = m_cellsContent.at(yDelegateIndex).at(xDelegateIndex++);
                drawTextFragment(foregroundText, currentX, y-iFontHeight+iFontDescent, line, currAttrib);
                foregroundText->setOpacity(opacity);
                currentX += iFontWidth*line.length();
                line.clear();
                currAttrib.attrib = nextAttrib.attrib;
                currAttrib.bgColor = nextAttrib.bgColor;
                currAttrib.fgColor = nextAttrib.fgColor;
            }
        }

        // Mark all remaining foreground cells unused.
        for (int j=xDelegateIndex;j<m_cellsContent.at(yDelegateIndex).size(); j++) {
            m_cellsContent.at(yDelegateIndex).at(j)->setVisible(false);
        }
    }
}

void TextRender::drawBgFragment(QQuickItem *cellDelegate, qreal x, qreal y, int width, TermChar style)
{
    if (style.attrib & TermChar::NegativeAttribute) {
        QRgb c = style.fgColor;
        style.fgColor = style.bgColor;
        style.bgColor = c;
    }

    QColor qtColor;

    if (sTerm->inverseVideoMode() && style.bgColor == Terminal::defaultBgColor) {
        qtColor = Terminal::defaultFgColor;
    } else {
        qtColor = style.bgColor;
    }

    cellDelegate->setX(x);
    cellDelegate->setY(y);
    cellDelegate->setWidth(width);
    cellDelegate->setHeight(iFontHeight);
    cellDelegate->setProperty("color", qtColor);
    cellDelegate->setVisible(true);
}

void TextRender::drawTextFragment(QQuickItem *cellContentsDelegate, qreal x, qreal y, QString text, TermChar style)
{
    if (style.attrib & TermChar::NegativeAttribute) {
        QRgb c = style.fgColor;
        style.fgColor = style.bgColor;
        style.bgColor = c;
    }
    if (style.attrib & TermChar::BoldAttribute) {
        iFont.setBold(true);
    } else if(iFont.bold()) {
        iFont.setBold(false);
    }
    if (style.attrib & TermChar::UnderlineAttribute) {
        iFont.setUnderline(true);
    } else if(iFont.underline()) {
        iFont.setUnderline(false);
    }
    if (style.attrib & TermChar::ItalicAttribute) {
        iFont.setItalic(true);
    } else if(iFont.italic()) {
        iFont.setItalic(false);
    }

    QColor qtColor;

    if (sTerm->inverseVideoMode() && style.fgColor == Terminal::defaultFgColor) {
        qtColor = Terminal::defaultBgColor;
    } else {
        qtColor = style.fgColor;
    }

    cellContentsDelegate->setX(x);
    cellContentsDelegate->setY(y);
    cellContentsDelegate->setHeight(iFontHeight);
    cellContentsDelegate->setProperty("color", qtColor);
    cellContentsDelegate->setProperty("text", text);
    cellContentsDelegate->setProperty("font", iFont);

    if (style.attrib & TermChar::BlinkAttribute) {
        cellContentsDelegate->setProperty("blinking", true);
    } else {
        cellContentsDelegate->setProperty("blinking", false);
    }

    cellContentsDelegate->setVisible(true);
}

void TextRender::redraw()
{
    if (m_dispatch_timer)
        return;

    m_dispatch_timer = startTimer(3);
}

void TextRender::timerEvent(QTimerEvent *)
{
    killTimer(m_dispatch_timer);
    m_dispatch_timer = 0;
    polish();
}

void TextRender::setUtil(Util *util)
{
    sUtil = util;
}

void TextRender::setTerminal(Terminal *terminal)
{
    sTerm = terminal;
}

bool TextRender::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    QMouseEvent *mev = static_cast<QMouseEvent*>(event);

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        QMouseEvent cp(mev->type(), mapFromItem(item, mev->localPos()), mev->windowPos(), mev->screenPos(), mev->button(), mev->buttons(), mev->modifiers(), mev->source());
        mousePressEvent(&cp);
        break;
    }
    case QEvent::MouseButtonRelease: {
        QMouseEvent cp(mev->type(), mapFromItem(item, mev->localPos()), mev->windowPos(), mev->screenPos(), mev->button(), mev->buttons(), mev->modifiers(), mev->source());
        mouseReleaseEvent(&cp);
        break;
    }
    case QEvent::MouseMove: {
        QMouseEvent cp(mev->type(), mapFromItem(item, mev->localPos()), mev->windowPos(), mev->screenPos(), mev->button(), mev->buttons(), mev->modifiers(), mev->source());
        mouseMoveEvent(&cp);
        break;
    }
    default:
        break;
    }

    return false;
}

void TextRender::mousePressEvent(QMouseEvent *event)
{
    if (!allowGestures())
        return;

    qreal eventX = event->localPos().x();
    qreal eventY = event->localPos().y();
    dragOrigin = QPointF(eventX, eventY);
    newSelection = true;
}

void TextRender::mouseMoveEvent(QMouseEvent *event)
{
    qreal eventX = event->localPos().x();
    qreal eventY = event->localPos().y();
    QPointF eventPos(eventX, eventY);

    if (!allowGestures())
        return;

    if(m_dragMode == DragScroll) {
        dragOrigin = scrollBackBuffer(eventPos, dragOrigin);
    }
    else if(m_dragMode == DragSelect) {
        selectionHelper(eventPos, true);
    }
}

void TextRender::mouseReleaseEvent(QMouseEvent *event)
{
    qreal eventX = event->localPos().x();
    qreal eventY = event->localPos().y();
    QPointF eventPos(eventX, eventY);
    const int reqDragLength = 140;

    if (!allowGestures())
        return;

    if(m_dragMode == DragGestures) {
        int xdist = qAbs(eventPos.x() - dragOrigin.x());
        int ydist = qAbs(eventPos.y() - dragOrigin.y());
        if(eventPos.x() < dragOrigin.x()-reqDragLength && xdist > ydist*2)
            doGesture(PanLeft);
        else if(eventPos.x() > dragOrigin.x()+reqDragLength && xdist > ydist*2)
            doGesture(PanRight);
        else if(eventPos.y() > dragOrigin.y()+reqDragLength && ydist > xdist*2)
            doGesture(PanDown);
        else if(eventPos.y() < dragOrigin.y()-reqDragLength && ydist > xdist*2)
            doGesture(PanUp);
    }
    else if(m_dragMode == DragScroll) {
        scrollBackBuffer(eventPos, dragOrigin);
    }
    else if(m_dragMode == DragSelect) {
        selectionHelper(eventPos, false);
    }
}

void TextRender::wheelEvent(QWheelEvent *event)
{
    if (!event->pixelDelta().isNull()) {
        dragOrigin = scrollBackBuffer(dragOrigin + event->pixelDelta(), dragOrigin);
        event->accept();
    } else {
        // TODO: angleDelta.
    }
}

void TextRender::selectionHelper(QPointF scenePos, bool selectionOngoing)
{
    int yCorr = fontDescent();

    QPoint start(qRound((dragOrigin.x()+2) / fontWidth()),
                 qRound((dragOrigin.y()+yCorr) / fontHeight()));
    QPoint end(qRound((scenePos.x()+2) / fontWidth()),
               qRound((scenePos.y()+yCorr) / fontHeight()));

    if (start != end) {
        sTerm->setSelection(start, end, selectionOngoing);
        newSelection = false;
    }
}

void TextRender::handleScrollBack(bool reset)
{
    if (reset) {
        setShowBufferScrollIndicator(false);
    } else {
        setShowBufferScrollIndicator(sTerm->backBufferScrollPos() != 0);
    }
    redraw();
}

QPointF TextRender::cursorPixelPos()
{
    return charsToPixels(sTerm->cursorPos());
}

QPointF TextRender::charsToPixels(QPoint pos)
{
    qreal x = 2; // left margin
    x += iFontWidth * (pos.x() - 1); // 0 indexed, so -1

    qreal y = iFontHeight * (pos.y() - 1) + iFontDescent + 1;

    return QPointF(x, y);
}

QSizeF TextRender::cursorPixelSize()
{
    return QSizeF(iFontWidth, iFontHeight);
}

bool TextRender::allowGestures()
{
    return iAllowGestures;
}

void TextRender::setAllowGestures(bool allow)
{
    if (iAllowGestures != allow) {
        iAllowGestures = allow;
        emit allowGesturesChanged();
    }
}

QQmlComponent *TextRender::cellDelegate() const
{
    return m_cellDelegate;
}

void TextRender::setCellDelegate(QQmlComponent *component)
{
    if (m_cellDelegate == component)
        return;

    // ###
    //for (QVector<QQuickItem*> cells : qAsConst(m_cells)) {
    foreach (const QVector<QQuickItem*> &cells , m_cells) {
        qDeleteAll(cells);
    }
    m_cells.clear();
    m_cellDelegate = component;
    emit cellDelegateChanged();
    polish();
}

QQmlComponent *TextRender::cellContentsDelegate() const
{
    return m_cellContentsDelegate;
}

void TextRender::setCellContentsDelegate(QQmlComponent *component)
{
    if (m_cellContentsDelegate == component)
        return;

    // ###
    //for (QVector<QQuickItem*> cells : qAsConst(m_cellsContent)) {
    foreach (const QVector<QQuickItem*> &cells , m_cells) {
        qDeleteAll(cells);
    }
    m_cellsContent.clear();
    m_cellContentsDelegate = component;
    emit cellContentsDelegateChanged();
    polish();
}

QQmlComponent *TextRender::cursorDelegate() const
{
    return m_cursorDelegate;
}

void TextRender::setCursorDelegate(QQmlComponent *component)
{
    if (m_cursorDelegate == component)
        return;

    delete m_cursorDelegateInstance;
    m_cursorDelegateInstance = 0;
    m_cursorDelegate = component;

    emit cursorDelegateChanged();
}

QQmlComponent *TextRender::selectionDelegate() const
{
    return m_selectionDelegate;
}

void TextRender::setSelectionDelegate(QQmlComponent *component)
{
    if (m_selectionDelegate == component)
        return;

    delete m_topSelectionDelegateInstance;
    delete m_middleSelectionDelegateInstance;
    delete m_bottomSelectionDelegateInstance;
    m_topSelectionDelegateInstance = 0;
    m_middleSelectionDelegateInstance = 0;
    m_bottomSelectionDelegateInstance = 0;

    m_selectionDelegate = component;

    emit selectionDelegateChanged();
}

int TextRender::contentHeight() const
{
    if (sTerm->useAltScreenBuffer())
        return sTerm->buffer().size();
    else
        return sTerm->buffer().size() + sTerm->backBuffer().size();
}

int TextRender::visibleHeight() const
{
    return sTerm->buffer().size();
}

int TextRender::contentY() const
{
    if (sTerm->useAltScreenBuffer())
        return 0;

    int scrollPos = sTerm->backBuffer().size() - sTerm->backBufferScrollPos();
    return scrollPos;
}

QPointF TextRender::scrollBackBuffer(QPointF now, QPointF last)
{
    int xdist = qAbs(now.x() - last.x());
    int ydist = qAbs(now.y() - last.y());
    int fontSize = fontPointSize();

    int lines = ydist / fontSize;

    if(lines > 0 && now.y() < last.y() && xdist < ydist*2) {
        sTerm->scrollBackBufferFwd(lines);
        last = QPointF(now.x(), last.y() - lines * fontSize);
    } else if(lines > 0 && now.y() > last.y() && xdist < ydist*2) {
        sTerm->scrollBackBufferBack(lines);
        last = QPointF(now.x(), last.y() + lines * fontSize);
    }

    return last;
}

void TextRender::doGesture(PanGesture gesture)
{
    if( gesture==PanLeft ) {
        sUtil->notifyText(sUtil->settingsValue("gestures/panLeftTitle", "Alt-Right").toString());
        sTerm->putString(sUtil->settingsValue("gestures/panLeftCommand", "\\e\\e[C").toString(), true);
    }
    else if( gesture==PanRight ) {
        sUtil->notifyText(sUtil->settingsValue("gestures/panRightTitle", "Alt-Left").toString());
        sTerm->putString(sUtil->settingsValue("gestures/panRightCommand", "\\e\\e[D").toString(), true);
    }
    else if( gesture==PanDown ) {
        sUtil->notifyText(sUtil->settingsValue("gestures/panDownTitle", "Page Up").toString());
        sTerm->putString(sUtil->settingsValue("gestures/panDownCommand", "\\e[5~").toString(), true);
    }
    else if( gesture==PanUp ) {
        sUtil->notifyText(sUtil->settingsValue("gestures/panUpTitle", "Page Down").toString());
        sTerm->putString(sUtil->settingsValue("gestures/panUpCommand", "\\e[6~").toString(), true);
    }
}
