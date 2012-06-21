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

#include <QtDebug>
#include <QtGui>

#ifdef Q_OS_UNIX
#include <QX11Info>
#endif

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

//#include <qmeegographicssystemhelper.h>
#include "glwindow.h"

static const float s_vertices[]= {
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f
};

GLWindow::GLWindow(QWidget *parent)
    :   QWidget(parent),
      m_previousTime(0),
      m_currentTime(0),
      m_frameTime(0.0f),
      m_fps(0.0f),
      m_paused(true),
      m_timerID(0),
      m_whichRenderTarget(0)
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

    resizeGL(event->size().width(), event->size().height());

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

    initializeGL();
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

    renderGL();

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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

    delete [] pTexData;
    return texture;
}

GLuint GLWindow::loadVertexShader(const QString &filename)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    QFile file(filename);

    if (file.exists()) {
        file.open(QIODevice::ReadOnly | QIODevice::Text);

        QByteArray vertexShaderSource = file.readAll();
        vertexShaderSource.append('\0');

        const char * code = vertexShaderSource.constData();

        glShaderSource(vertexShader, 1, &code, NULL);
        glCompileShader(vertexShader);
        checkShader(vertexShader);

        return vertexShader;
    }

    return 0;
}

GLuint GLWindow::loadFragmentShader(const QString &filename)
{
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    QFile file(filename);

    if (file.exists()) {
        file.open(QIODevice::ReadOnly | QIODevice::Text);

        QByteArray fragmentShaderSource = file.readAll();
        fragmentShaderSource.append('\0');

        const char * code = fragmentShaderSource.constData();

        glShaderSource(fragmentShader, 1, &code, NULL);
        glCompileShader(fragmentShader);
        checkShader(fragmentShader);

        return fragmentShader;
    }

    return 0;
}

void GLWindow::checkShader(GLuint shader)
{
    GLint status;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE)
        qDebug() << "Compile error";
    else
        qDebug() << "Compile ok";

    int loglen;
    char logBuffer[1024];
    glGetShaderInfoLog(shader, sizeof(logBuffer), &loglen, logBuffer);

    if (loglen > 0)
        qDebug() << logBuffer;
}

void GLWindow::checkProgram(GLuint program)
{
    GLint status;

    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
        qDebug() << "Linker error";
    else
        qDebug() << "Linker ok";

    int loglen;
    char logbuffer[1024];

    glGetProgramInfoLog(program, sizeof(logbuffer), &loglen, logbuffer);
    if (loglen > 0)
        qDebug() << logbuffer;

    glValidateProgram(program);
    glGetProgramInfoLog(program, sizeof(logbuffer), &loglen, logbuffer);

    if (loglen > 0)
        qDebug() << logbuffer;
}

void GLWindow::glError(const char *file, int line)
{
    GLenum err (glGetError());

    while (err!=GL_NO_ERROR) {
        QString error;

        switch(err) {
        case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
        case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
        case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
        case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
        }

        qDebug() << "GL_" << error << " - " << file << ":" << line;

        err=glGetError();
    }
}

void GLWindow::mousePressEvent(QMouseEvent *event)
{
    qDebug() << event->x() << event->y();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_renderTexture[m_whichRenderTarget]);
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(m_programDroplet);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void GLWindow::initializeGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glGenTextures(2, m_renderTexture);

    glBindTexture(GL_TEXTURE_2D, m_renderTexture[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 854, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, m_renderTexture[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 854, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(2, m_fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTexture[0], 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo[1]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTexture[1], 0);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //vertex array stuff
    glEnableVertexAttribArray(0);

    //shader stuff

    //evaluation shader
    m_programEval = glCreateProgram();
    //m_vsEval = loadVertexShader("simple.vsh");
    m_vsEval = loadVertexShader("/opt/fluid/bin/simple.vsh");
    //m_fsEval = loadFragmentShader("eval.fsh");
    m_fsEval = loadFragmentShader("/opt/fluid/bin/eval.fsh");
    glAttachShader(m_programEval, m_vsEval);
    glAttachShader(m_programEval, m_fsEval);

    glBindAttribLocation(m_programEval, 0, "vertexPos");

    glLinkProgram(m_programEval);
    checkProgram(m_programEval);

    //render program
    m_programRender = glCreateProgram();
    m_fsRender = loadFragmentShader("/opt/fluid/bin/render.fsh");
    glAttachShader(m_programRender, m_vsEval);
    glAttachShader(m_programRender, m_fsRender);

    glBindAttribLocation(m_programRender, 0, "vertexPos");

    glLinkProgram(m_programRender);
    checkProgram(m_programRender);

    //droplet program
    m_programDroplet = glCreateProgram();
    m_fsDroplet = loadFragmentShader("/opt/fluid/bin/droplet.fsh");
    glAttachShader(m_programDroplet, m_vsEval);
    glAttachShader(m_programDroplet, m_fsDroplet);

    glBindAttribLocation(m_programDroplet, 0, "vertexPos");

    glLinkProgram(m_programDroplet);
    checkProgram(m_programDroplet);

    //texture
    m_texture = loadTexture("/opt/fluid/bin/Trees.jpg");

    //vbo
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertices), s_vertices, GL_STATIC_DRAW);

    //viewport
    glViewport(0, 0, 854, 480);

    //uniforms, eval
    glUseProgram(m_programEval);

    int texUnit0Index = glGetUniformLocation(m_programEval, "texUnit0");
    glUniform1i(texUnit0Index, 0);

    int widthIndex = glGetUniformLocation(m_programEval, "width");
    glUniform1f(widthIndex, (float)854);

    int heightIndex = glGetUniformLocation(m_programEval, "height");
    glUniform1f(heightIndex, (float)480);

    //uniform, render
    glUseProgram(m_programRender);
    texUnit0Index = glGetUniformLocation(m_programRender, "texUnit0");
    glUniform1i(texUnit0Index, 0);

    int texUnit1Index = glGetUniformLocation(m_programRender, "texUnit1");
    glUniform1i(texUnit1Index, 1);

    widthIndex = glGetUniformLocation(m_programRender, "width");
    glUniform1f(widthIndex, (float)854);

    heightIndex = glGetUniformLocation(m_programRender, "height");
    glUniform1f(heightIndex, (float)480);

    //uniform, droplet
    glUseProgram(m_programDroplet);
    int dropletPosition = glGetUniformLocation(m_programDroplet, "droplet_position");
    float dps[] = { 300.0f, 200.0f };
    glUniform2fv(dropletPosition, 1, dps);

    int resolutionIndex = glGetUniformLocation(m_programDroplet, "resolution");
    glUniform2f(resolutionIndex, 854.0f, 480.0f);
}

void GLWindow::eval()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderTexture[1 - m_whichRenderTarget]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_renderTexture[m_whichRenderTarget]);

    glUseProgram(m_programEval);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_whichRenderTarget = 1 - m_whichRenderTarget;
}

void GLWindow::renderGL()
{
    eval();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_renderTexture[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glUseProgram(m_programRender);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void GLWindow::resizeGL(int w, int h)
{

}
