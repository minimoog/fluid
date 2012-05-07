/* Copyright (c) 2010, Antonie Jovanoski
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

#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QWidget>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

class GLWindow : public QWidget
{
    Q_OBJECT
public:
    explicit GLWindow(QWidget *parent = 0);
    virtual ~GLWindow();
    
    void create();
    void destroy();

    unsigned int getTickCount() const;
    float getFrameTime() const;
    float getFramePerSeconds() const;

    GLuint loadTexture(const QString& imageFile);

protected:
    bool eventFilter(QObject *object, QEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void timerEvent(QTimerEvent *event);

private:
    void createEGL();
    void reinitEGL();
    void render();
    void cleanupAndExit(EGLDisplay eglDisplay);
    EGLNativeWindowType getWindow();

protected:
    EGLDisplay m_eglDisplay;
    EGLConfig m_eglConfig;
    EGLSurface m_eglSurface;
    EGLContext m_eglContext;

    unsigned int m_previousTime;
    unsigned int m_currentTime;
    float m_frameTime;
    float m_fps;
    bool m_paused;
    int m_timerID;

signals:
    
public slots:
    void pause();
    void resume();
    
};

#endif // GLWINDOW_H
