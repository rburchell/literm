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

#ifndef UTIL_H
#define UTIL_H

#include <QtCore>

#include "textrender.h"

class Terminal;
class TextRender;
class QQuickView;

class Util : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString windowTitle READ windowTitle WRITE setWindowTitle NOTIFY windowTitleChanged)
    Q_PROPERTY(int windowOrientation READ windowOrientation WRITE setWindowOrientation NOTIFY windowOrientationChanged)
    Q_PROPERTY(bool visualBellEnabled READ visualBellEnabled CONSTANT)
    Q_PROPERTY(QString fontFamily READ fontFamily CONSTANT)
    Q_PROPERTY(int uiFontSize READ uiFontSize CONSTANT)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(TextRender::DragMode dragMode READ dragMode WRITE setDragMode NOTIFY dragModeChanged)
    Q_PROPERTY(int keyboardMode READ keyboardMode WRITE setKeyboardMode NOTIFY keyboardModeChanged)
    Q_PROPERTY(int keyboardFadeOutDelay READ keyboardFadeOutDelay WRITE setKeyboardFadeOutDelay NOTIFY keyboardFadeOutDelayChanged)
    Q_PROPERTY(QString keyboardLayout READ keyboardLayout WRITE setKeyboardLayout NOTIFY keyboardLayoutChanged)
    Q_PROPERTY(int extraLinesFromCursor READ extraLinesFromCursor CONSTANT)
    Q_PROPERTY(QString charset READ charset CONSTANT)
    Q_PROPERTY(int keyboardMargins READ keyboardMargins CONSTANT)
    Q_PROPERTY(int orientationMode READ orientationMode WRITE setOrientationMode NOTIFY orientationModeChanged)
    Q_PROPERTY(bool showWelcomeScreen READ showWelcomeScreen WRITE setShowWelcomeScreen NOTIFY showWelcomeScreenChanged)
    Q_PROPERTY(QByteArray terminalEmulator READ terminalEmulator CONSTANT)
    Q_PROPERTY(QString terminalCommand READ terminalCommand CONSTANT)
    Q_ENUMS(KeyboardMode)
    Q_ENUMS(DragMode)
    Q_ENUMS(OrientationMode)

public:
    enum KeyboardMode {
        KeyboardOff,
        KeyboardFade,
        KeyboardMove
    };

    enum OrientationMode {
        OrientationAuto,
        OrientationLandscape,
        OrientationPortrait
    };

    explicit Util(const QString &settingsFile, QObject *parent = 0);
    virtual ~Util();

    QByteArray terminalEmulator() const;
    QString terminalCommand() const;

    void setWindow(QQuickView* win);
    void setWindowTitle(QString title);
    QString windowTitle();
    int windowOrientation();
    void setWindowOrientation(int orientation);

    Q_INVOKABLE void openNewWindow();
    Q_INVOKABLE QString getUserMenuXml();

    Q_INVOKABLE QString versionString();
    Q_INVOKABLE QString configPath();
    QVariant settingsValue(QString key, const QVariant &defaultValue = QVariant());
    void setSettingsValue(QString key, QVariant value);

    int uiFontSize();

    int fontSize();
    void setFontSize(int size);

    Q_INVOKABLE void keyPressFeedback();
    Q_INVOKABLE void keyReleaseFeedback();
    Q_INVOKABLE void notifyText(QString text);
    Q_INVOKABLE void fakeKeyPress(int key, int modifiers);

    Q_INVOKABLE void copyTextToClipboard(QString str);

    bool visualBellEnabled();

    QString fontFamily();

    TextRender::DragMode dragMode();
    void setDragMode(TextRender::DragMode mode);

    int keyboardMode();
    void setKeyboardMode(int mode);

    int keyboardFadeOutDelay();
    void setKeyboardFadeOutDelay(int delay);

    QString keyboardLayout();
    void setKeyboardLayout(const QString &layout);

    int extraLinesFromCursor();
    QString charset();
    int keyboardMargins();

    int orientationMode();
    void setOrientationMode(int mode);

    bool showWelcomeScreen();
    void setShowWelcomeScreen(bool value);

signals:
    void notify(QString msg);
    void windowTitleChanged();
    void windowOrientationChanged();
    void fontSizeChanged();
    void dragModeChanged();
    void keyboardModeChanged();
    void keyboardFadeOutDelayChanged();
    void keyboardLayoutChanged();
    void orientationModeChanged();
    void showWelcomeScreenChanged();

private:
    Q_DISABLE_COPY(Util)

    QSettings m_settings;
    QQuickView* iWindow;
};

#endif // UTIL_H
