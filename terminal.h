/*
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

#ifndef TERMINAL_H
#define TERMINAL_H

#include <QObject>
#include <QRgb>
#include <QRect>
#include <QVector>

class PtyIFace;
class Util;
class QQuickView;

struct TermChar {
    enum TextAttributes {
        NoAttributes = 0x00,
        BoldAttribute = 0x01,
        ItalicAttribute = 0x02,
        UnderlineAttribute = 0x04,
        NegativeAttribute = 0x08,
        BlinkAttribute = 0x10
    };

    QChar c;
    QRgb fgColor;
    QRgb bgColor;
    TextAttributes attrib;
};
inline TermChar::TextAttributes operator~ (TermChar::TextAttributes a) { return (TermChar::TextAttributes)~(int)a; }
inline TermChar::TextAttributes operator| (TermChar::TextAttributes a, TermChar::TextAttributes b) { return (TermChar::TextAttributes)((int)a | (int)b); }
inline TermChar::TextAttributes operator& (TermChar::TextAttributes a, TermChar::TextAttributes b) { return (TermChar::TextAttributes)((int)a & (int)b); }
inline TermChar::TextAttributes operator^ (TermChar::TextAttributes a, TermChar::TextAttributes b) { return (TermChar::TextAttributes)((int)a ^ (int)b); }
inline TermChar::TextAttributes& operator|= (TermChar::TextAttributes& a, TermChar::TextAttributes b) { return (TermChar::TextAttributes&)((int&)a |= (int)b); }
inline TermChar::TextAttributes& operator&= (TermChar::TextAttributes& a, TermChar::TextAttributes b) { return (TermChar::TextAttributes&)((int&)a &= (int)b); }
inline TermChar::TextAttributes& operator^= (TermChar::TextAttributes& a, TermChar::TextAttributes b) { return (TermChar::TextAttributes&)((int&)a ^= (int)b); }

const QByteArray multiCharEscapes("().*+-/%#");

struct TermAttribs {
    QPoint cursorPos;

    bool wrapAroundMode;
    bool originMode;

    QRgb currentFgColor;
    QRgb currentBgColor;
    TermChar::TextAttributes currentAttrib;
};

typedef QVector<TermChar> TerminalLine;
typedef QVector<TerminalLine> TerminalBuffer;

class Terminal : public QObject
{
    Q_OBJECT

public:
    static QRgb defaultFgColor;
    static QRgb defaultBgColor;

    explicit Terminal(QObject *parent = 0);
    virtual ~Terminal() {}

    void setPtyIFace(PtyIFace* pty);
    void setWindow(QQuickView* win) { iWindow=win; }
    void setUtil(Util* util) { iUtil = util; }

    void insertInBuffer(const QString& chars);

    QPoint cursorPos();
    void setCursorPos(QPoint pos);
    bool showCursor();

    QSize termSize() { return iTermSize; }
    void setTermSize(QSize size);

    TerminalBuffer &buffer();
    TerminalBuffer &backBuffer() { return iBackBuffer; }

    TerminalLine &currentLine();

    bool inverseVideoMode() const { return m_inverseVideoMode; }

    void keyPress(int key, int modifiers, const QString& text="");
    Q_INVOKABLE const QStringList printableLinesFromCursor(int lines);
    Q_INVOKABLE void putString(QString str);

    void paste(const QString &text);
    Q_INVOKABLE const QStringList grabURLsFromBuffer();

    void scrollBackBufferFwd(int lines);
    void scrollBackBufferBack(int lines);
    int backBufferScrollPos() { return iBackBufferScrollPos; }
    void resetBackBufferScrollPos();

    QString selectedText();
    void setSelection(QPoint start, QPoint end, bool selectionOngoing);
    QRect selection();
    Q_INVOKABLE void clearSelection();
    bool hasSelection();

    int rows();
    int columns();

    bool useAltScreenBuffer() const { return iUseAltScreenBuffer; }

    TermChar zeroChar;

signals:
    void cursorPosChanged(QPoint newPos);
    void termSizeChanged(int rows, int columns);
    void displayBufferChanged();
    void selectionChanged();
    void scrollBackBufferAdjusted(bool reset);
    void selectionFinished();
    void visualBell();
    void windowTitleChanged(const QString &windowTitle);

private:
    Q_DISABLE_COPY(Terminal)
    static const int maxScrollBackLines = 300;

    void insertAtCursor(QChar c, bool overwriteMode=true, bool advanceCursor=true);
    void deleteAt(QPoint pos);
    void clearAt(QPoint pos);
    void eraseLineAtCursor(int from=-1, int to=-1);
    void clearAll(bool wholeBuffer=false);
    void ansiSequence(const QString& seq);
    void handleMode(int mode, bool set, const QString &extra);
    void handleSGR(const QList<int> &params, const QString &extra);
    void oscSequence(const QString& seq);
    void escControlChar(const QString& seq);
    void trimBackBuffer();
    void scrollBack(int lines, int insertAt=-1);
    void scrollFwd(int lines, int removeAt=-1);
    void resetTerminal();
    void resetTabs();
    void adjustSelectionPosition(int lines);
    void forwardTab();
    void backwardTab();

    PtyIFace* iPtyIFace;
    QQuickView* iWindow;
    Util* iUtil;

    TerminalBuffer iBuffer;
    TerminalBuffer iAltBuffer;
    TerminalBuffer iBackBuffer;
    QVector<QVector<int> > iTabStops;

    QSize iTermSize;
    bool iEmitCursorChangeSignal;

    bool iShowCursor;
    bool iUseAltScreenBuffer;
    bool iAppCursorKeys;
    bool iReplaceMode;
    bool iNewLineMode;
    bool m_inverseVideoMode;
    bool m_bracketedPasteMode;

    int iMarginTop;
    int iMarginBottom;

    int iBackBufferScrollPos;

    TermAttribs iTermAttribs;
    TermAttribs iTermAttribs_saved;
    TermAttribs iTermAttribs_saved_alt;

    QString escSeq;
    QString oscSeq;
    int escape;
    QRect iSelection;
    QVector<QRgb> iColorTable;
};

#endif // TERMINAL_H
