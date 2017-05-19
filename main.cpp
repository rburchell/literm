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

#include "qplatformdefs.h"

#include <QtGui>
#include <QtQml>
#include <QQuickView>
#include <QDir>
#include <QString>

#include "terminal.h"
#include "textrender.h"
#include "utilities.h"
#include "version.h"
#include "keyloader.h"

static void copyFileFromResources(QString from, QString to);

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("Fingerterm");

    QGuiApplication app(argc, argv);

    QScreen* sc = app.primaryScreen();
    if(sc){
        sc->setOrientationUpdateMask(Qt::PrimaryOrientation
                                     | Qt::LandscapeOrientation
                                     | Qt::PortraitOrientation
                                     | Qt::InvertedLandscapeOrientation
                                     | Qt::InvertedPortraitOrientation);
    }

    qmlRegisterType<TextRender>("FingerTerm", 1, 0, "TextRender");
    qmlRegisterUncreatableType<Util>("FingerTerm", 1, 0, "Util", "Util is created by app");
    QQuickView view;

#if defined(DESKTOP_BUILD)
    bool fullscreen = app.arguments().contains("-fullscreen");
#else
    bool fullscreen = !app.arguments().contains("-nofs");
#endif

    QSize screenSize = QGuiApplication::primaryScreen()->size();

    if (fullscreen) {
        view.setWidth(screenSize.width());
        view.setHeight(screenSize.height());
    } else {
        view.setWidth(screenSize.width() / 2);
        view.setHeight(screenSize.height() / 2);
    }

    QString settings_path(QDir::homePath() + "/.config/FingerTerm");
    QDir dir;

    if (!dir.exists(settings_path)) {
        if (!dir.mkdir(settings_path))
            qWarning() << "Could not create fingerterm settings path" << settings_path;
    }

    QString settingsFile = settings_path + "/settings.ini";


    Terminal term;
    Util util(settingsFile);
    term.setUtil(&util);
    TextRender::setUtil(&util);
    TextRender::setTerminal(&term);

    QString startupErrorMsg;

    // copy the default config files to the config dir if they don't already exist
    copyFileFromResources(":/data/menu.xml", util.configPath()+"/menu.xml");
    copyFileFromResources(":/data/english.layout", util.configPath()+"/english.layout");
    copyFileFromResources(":/data/finnish.layout", util.configPath()+"/finnish.layout");
    copyFileFromResources(":/data/french.layout", util.configPath()+"/french.layout");
    copyFileFromResources(":/data/german.layout", util.configPath()+"/german.layout");
    copyFileFromResources(":/data/qwertz.layout", util.configPath()+"/qwertz.layout");

    KeyLoader keyLoader;
    keyLoader.setUtil(&util);
    bool ret = keyLoader.loadLayout(util.keyboardLayout());
    if(!ret) {
        // on failure, try to load the default one (english) directly from resources
        startupErrorMsg = "There was an error loading the keyboard layout.<br>\nUsing the default one instead.";
        util.setKeyboardLayout("english");
        ret = keyLoader.loadLayout(":/data/english.layout");
        if(!ret)
            qFatal("failure loading keyboard layout");
    }

    QQmlContext *context = view.rootContext();
    context->setContextProperty( "term", &term );
    context->setContextProperty( "util", &util );
    context->setContextProperty( "keyLoader", &keyLoader );
    context->setContextProperty( "startupErrorMessage", startupErrorMsg);

    term.setWindow(&view);
    util.setWindow(&view);

    QObject::connect(view.engine(),SIGNAL(quit()),&app,SLOT(quit()));

    QQmlFileSelector *selector = new QQmlFileSelector(view.engine());

    QStringList selectors;

    // Allow overriding the UX choice
    bool mobile = app.arguments().contains("-mobile");
    bool desktop = app.arguments().contains("-desktop");
    if (mobile)
        selectors << "mobile";
    if (desktop)
        selectors << "desktop";

#if defined(MOBILE_BUILD)
    selectors << "mobile";
#else
    selectors << "desktop";
#endif

    selector->setExtraSelectors(selectors);

    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:/qml/Main.qml"));

    QObject *root = view.rootObject();
    if(!root)
        qFatal("no root object - qml error");

    if (fullscreen) {
        view.showFullScreen();
    } else {
        view.show();
    }

    return app.exec();
}

static void copyFileFromResources(QString from, QString to)
{
    // copy a file from resources to the config dir if it does not exist there
    QFileInfo toFile(to);
    if(!toFile.exists()) {
        QFile newToFile(toFile.absoluteFilePath());
        QResource res(from);
        if (newToFile.open(QIODevice::WriteOnly)) {
            newToFile.write( reinterpret_cast<const char*>(res.data()) );
            newToFile.close();
        } else {
            qWarning() << "Failed to copy default config from resources to" << toFile.filePath();
        }
    }
}
