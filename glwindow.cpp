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

#include <QtGui>

#ifdef Q_OS_UNIX
#include <QX11Info>
#endif

//#include <qmeegographicssystemhelper.h>
#include "glwindow.h"

GLWindow::GLWindow(QWidget *parent)
    :   QWidget(parent),
      m_previousTime(0),
      m_currentTime(0),
      m_frameTime(0.0f),
      m_fps(0.0f),
      m_paused(true),
      m_timerID(0)
{
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_StyledBackground, false);
    setAttribute(Qt::WA_PaintUnclipped);

    //QMeeGoGraphicsSystemHelper::setSwitchPolicy(QMeeGoGraphicsSystemHelper::NoSwitch);
}

GLWindow::~GLWindow()
{
    destroy();

    if (m_timerID)
        killTimer(m_timerID);
}

void GLWindow::create()
{
    createEGL();

    m_currentTime = getTickCount();
    m_previousTime = m_currentTime;
    m_fps = 0.0f;
    m_frameTime = 0.0f;
}

void GLWindow::destroy()
{
    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(m_eglDisplay, m_eglSurface);
}

unsigned int GLWindow::getTickCount() const
{
    QTime now = QTime::currentTime();

    return (now.hour() * 3600 + now.minute() * 60 + now.second() * 1000 + now.msec());
}

float GLWindow::getFrameTime() const
{
    return m_frameTime;
}

float GLWindow::getFramePerSeconds() const
{
    return m_fps;
}

void GLWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
}

EGLNativeWindowType GLWindow::getWindow()
{
    return (EGLNativeWindowType)winId();
}

void GLWindow::pause()
{
    if (m_paused)
        return;

    m_paused = true;
    killTimer(m_timerID);
    m_timerID = 0;
}

void GLWindow::resume()
{
    if (!m_paused)
        return;

    m_paused = false;
    m_timerID = startTimer(0);
}

bool GLWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::ActivationChange) {
        if (isActiveWindow())
            resume();
        else
            pause();

        return false;
    }

    return QObject::eventFilter(object, event);
}

void GLWindow::resizeEvent(QResizeEvent *event)
{
    if (event->size().width() == event->oldSize().width() &&
            event->size().height() == event->oldSize().height()) {
        return;
    }

    glViewport(0, 0, event->size().width(), event->size().height());

    QWidget::resizeEvent(event);
}

void GLWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    render();
}

void GLWindow::createEGL()
{
    GLint errVal;
    m_eglDisplay = 0;
    m_eglConfig = 0;
    m_eglSurface = 0;
    m_eglContext = 0;

    m_eglDisplay = eglGetDisplay((EGLNativeDisplayType)x11Info().display());

    EGLint majorVersion;
    EGLint minorVersion;

    if (!eglInitialize(m_eglDisplay, &majorVersion, &minorVersion)) {
        cleanupAndExit(m_eglDisplay);
    }

    EGLint pi32ConfigAttribs[13];
    pi32ConfigAttribs[0] = EGL_SURFACE_TYPE; pi32ConfigAttribs[1] = EGL_WINDOW_BIT;
    pi32ConfigAttribs[2] = EGL_RENDERABLE_TYPE; pi32ConfigAttribs[3] = EGL_OPENGL_ES2_BIT;
    pi32ConfigAttribs[4] = EGL_DEPTH_SIZE; pi32ConfigAttribs[5] = 8;

    pi32ConfigAttribs[6] = EGL_NONE;

    EGLint pi32ContextAttribs[3];
    pi32ContextAttribs[0] = EGL_CONTEXT_CLIENT_VERSION; pi32ContextAttribs[1] = 2;

    pi32ContextAttribs[2] = EGL_NONE;

    EGLint configs;

    if (!eglChooseConfig(m_eglDisplay, pi32ConfigAttribs, &m_eglConfig, 1, &configs)
            || (configs != 1)) {
        cleanupAndExit(m_eglDisplay);
    }

    m_eglSurface = eglCreateWindowSurface(m_eglDisplay, m_eglConfig, getWindow(), NULL);
    if ((errVal = eglGetError()) != EGL_SUCCESS) {
        cleanupAndExit(m_eglDisplay);
    }

    m_eglContext = eglCreateContext(m_eglDisplay, m_eglConfig, NULL, pi32ContextAttribs);
    if ((errVal = eglGetError()) != EGL_SUCCESS) {
        cleanupAndExit(m_eglDisplay);
    }

    eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
    if ((errVal = eglGetError()) != EGL_SUCCESS) {
        cleanupAndExit(m_eglDisplay);
    }
}

void GLWindow::render()
{
    m_previousTime = m_currentTime;
    m_currentTime = getTickCount();
    m_frameTime = (float)(m_currentTime - m_previousTime) * 0.001f;

    if (m_frameTime != 0.0f) {
        m_fps = 1.0f / m_frameTime;
    } else {
        m_fps = 100000.0f;
    }

    if (!eglSwapBuffers(m_eglDisplay, m_eglSurface)) {

        GLint errVal = eglGetError();

        if (errVal == EGL_BAD_ALLOC || errVal == EGL_BAD_SURFACE) {
            if (errVal == EGL_BAD_ALLOC)
                eglDestroySurface(m_eglDisplay, m_eglSurface);

            m_eglSurface = eglCreateWindowSurface(m_eglDisplay,
                                                m_eglConfig,
                                                getWindow(),
                                                NULL);
            eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
        } else {
            cleanupAndExit(m_eglDisplay);
        }
    }
}

void GLWindow::cleanupAndExit(EGLDisplay eglDisplay)
{
    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(m_eglDisplay, m_eglSurface);
    eglTerminate(eglDisplay);

    exit(0);
}

GLuint GLWindow::loadTexture(const QString &imageFile)
{
    QImage image(imageFile);

    if (image.width() < 1 || image.height() < 1) {
        // Failed to load the texture
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLuint *pTexData = new GLuint[image.width() * image.height()];
    GLuint *sdata = (GLuint*)image.bits();
    GLuint *tdata = pTexData;

    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            *tdata = ((*sdata&255) << 16) | (((*sdata>>8)&255) << 8)
                    | (((*sdata>>16)&255) << 0) | (((*sdata>>24)&255) << 24);

            sdata++;
            tdata++;
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pTexData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    delete [] pTexData;
    return texture;
}
