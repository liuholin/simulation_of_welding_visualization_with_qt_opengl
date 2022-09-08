#include "squircle.h"

#include <QtQuick/qquickwindow.h>
#include <QOpenGLShaderProgram>
#include <QOpenGLContext>
#include <QtCore/QRunnable>
#include <QMatrix4x4>

//! [7]
Squircle::Squircle()
    : m_t(0)
    , m_renderer(nullptr)
{
    connect(this, &QQuickItem::windowChanged, this, &Squircle::handleWindowChanged);
}
//! [7]

//! [8]
void Squircle::setT(qreal t)
{
    if (t == m_t)
        return;
    m_t = t;
    emit tChanged();
    if (window())
        window()->update();
}
//! [8]

//! [1]
void Squircle::handleWindowChanged(QQuickWindow *win)
{
    if (win) {
        connect(win, &QQuickWindow::beforeSynchronizing, this, &Squircle::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, this, &Squircle::cleanup, Qt::DirectConnection);
//! [1]
//! [3]
        // Ensure we start with cleared to black. The squircle's blend mode relies on this.
        win->setColor(Qt::black);
    }
}
//! [3]

//! [6]
void Squircle::cleanup()
{
    delete m_renderer;
    m_renderer = nullptr;
}

class CleanupJob : public QRunnable
{
public:
    CleanupJob(SquircleRenderer *renderer) : m_renderer(renderer) { }
    void run() override { delete m_renderer; }
private:
    SquircleRenderer *m_renderer;
};

void Squircle::releaseResources()
{
    window()->scheduleRenderJob(new CleanupJob(m_renderer), QQuickWindow::BeforeSynchronizingStage);
    m_renderer = nullptr;
}

SquircleRenderer::~SquircleRenderer()
{
    delete m_program;
}
//! [6]

//! [9]
void Squircle::sync()
{
    if (!m_renderer) {
        m_renderer = new SquircleRenderer();
        connect(window(), &QQuickWindow::beforeRendering, m_renderer, &SquircleRenderer::init, Qt::DirectConnection);
        connect(window(), &QQuickWindow::beforeRenderPassRecording, m_renderer, &SquircleRenderer::paint, Qt::DirectConnection);
    }
    m_renderer->setViewportSize(window()->size() * window()->devicePixelRatio());
    m_renderer->setT(m_t);
    m_renderer->setWindow(window());
}
//! [9]

//! [4]
void SquircleRenderer::init()
{
    if (!m_program) {
        QSGRendererInterface *rif = m_window->rendererInterface();
        Q_ASSERT(rif->graphicsApi() == QSGRendererInterface::OpenGL || rif->graphicsApi() == QSGRendererInterface::OpenGLRhi);

        initializeOpenGLFunctions();

        m_program = new QOpenGLShaderProgram();
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex,
                                                    "attribute highp vec4 vertices;"
                                                    "attribute lowp vec3 mycolor;"
                                                    "varying highp vec3 color;"
                                                    "uniform highp mat4 matrix;\n"
                                                    "void main() {"
                                                    "    gl_Position = matrix * vertices;"
                                                    "    color= mycolor;"
                                                    "}");
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment,
                                                    "varying highp vec3 color;"
                                                    "void main() {"
                                                    "    gl_FragColor = vec4(color, 1.0);"
                                                    "}");

        m_program->bindAttributeLocation("vertices", 0);
        m_program->bindAttributeLocation("mycolor", 1);
        m_program->link();

    }
}

//! [4] //! [5]
void SquircleRenderer::paint()
{
    // Play nice with the RHI. Not strictly needed when the scenegraph uses
    // OpenGL directly.
    m_window->beginExternalCommands();

    m_program->bind();

    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);

    QMatrix4x4 matrix;
    matrix.perspective(60.0f, 3.0f/4.0f, 0.1f, 100.0f);
    matrix.translate(0, 0, -2);
    int matrixLocation = m_program->uniformLocation("matrix");
    m_program->setUniformValue(matrixLocation, matrix);

    static const GLfloat pipe_vertices[] = {
         0.0f,  0.0f,
        -1.0f,  0.0f,
        -1.0f,  1.0f,
        -0.5f,  1.0f,
         0.0f,  0.0f,
        0.5f,  1.0f,
        1.0f,  1.0f,
        1.0f,  0.0f,
        0.0f,  0.0f
    };

    static const GLfloat torch_vertices[] = {
         0.0f,  0.0f,
        -0.05f,  0.0f,
        -0.025,  0.05f,
         0.0f,  0.05f,
         0.025f,  0.05f,
        0.05f,  0.0f,
        0.0f,  0.0f,
        0.0f,  0.5f
    };

    static const GLfloat pipe_colors[] = {
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f
    };

    static const GLfloat torch_colors[] = {
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    // This example relies on (deprecated) client-side pointers for the vertex
    // input. Therefore, we have to make sure no vertex buffer is bound.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_program->setAttributeArray(0, GL_FLOAT, pipe_vertices, 2);
    m_program->setAttributeArray(1, GL_FLOAT, pipe_colors, 3);

    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height()*0.5);

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glDrawArrays(GL_LINE_STRIP, 0, 9);


    //draw torch

    matrix.setToIdentity();
    matrix.perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    if(m_right)
    {
        matrix.translate(-0.2+0.002 * (m_frame % 200), 0.5, -2);
        if(m_frame % 200==199) m_right=false;
    }
    else
    {
        matrix.translate(0.2-0.002 * (m_frame % 200), 0.5, -2);
        if(m_frame % 200==199) m_right=true;
     }
    //matrix.rotate(100.0f * m_frame / screen()->refreshRate(), 0, 1, 0);

    m_program->setUniformValue(matrixLocation, matrix);

    m_program->setAttributeArray(0, GL_FLOAT, torch_vertices, 2);
    m_program->setAttributeArray(1, GL_FLOAT, torch_colors, 3);

    glDrawArrays(GL_LINE_STRIP, 0, 8);

    m_program->disableAttributeArray(0);
    m_program->disableAttributeArray(1);
    m_program->release();
    ++m_frame;

    m_window->endExternalCommands();
}
//! [5]
