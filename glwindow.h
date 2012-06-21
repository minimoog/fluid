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
    GLuint loadVertexShader(const QString& filename);
    GLuint loadFragmentShader(const QString& filename);
    void checkShader(GLuint shader);
    void checkProgram(GLuint program);
    void glError(const char *file, int line);

public slots:
    void pause();
    void resume();

protected:
    bool eventFilter(QObject *object, QEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void timerEvent(QTimerEvent *event);
    void mousePressEvent(QMouseEvent *event);

    virtual void initializeGL();
    virtual void renderGL();
    virtual void resizeGL(int w, int h);

private:
    void createEGL();
    //void reinitEGL();
    void render();
    void eval();
    void cleanupAndExit(EGLDisplay eglDisplay);
    EGLNativeWindowType getWindow();

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

    GLuint m_renderTexture[2];
    GLuint m_fbo[2];
    int m_whichRenderTarget;    //0 or 1 ### TODO

    GLuint m_programEval;
    GLuint m_vsEval;
    GLuint m_fsEval;

    GLuint m_programRender;
    GLuint m_fsRender;

    GLuint m_programDroplet;
    GLuint m_fsDroplet;

    GLuint m_texture;

    GLuint m_vbo;
};

#endif // GLWINDOW_H
