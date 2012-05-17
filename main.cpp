/* Copyright (c) 2012, Antonie Jovanoski
 *
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact e-mail: Antonie Jovanoski <minimoog77_at_gmail.com>
 */

#include <QtGui/QApplication>
#include "glwindow.h"

#if defined(Q_WS_X11)
#include <X11/Xlib.h>
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#if defined(Q_WS_X11)
    XInitThreads();
#endif

    GLWindow *glwindow = new GLWindow();
    glwindow->create();
    glwindow->setWindowState(Qt::WindowNoState);
    glwindow->showFullScreen();
    glwindow->resume();

    int result = a.exec();

    glwindow->destroy();
    delete glwindow;

    //QCoreApplication::setAttribute(Qt::AA_NativeWindows, true);
    //QCoreApplication::setAttribute(Qt::AA_ImmediateWidgetCreation, true);

    //return a.exec();

    return result;
}
