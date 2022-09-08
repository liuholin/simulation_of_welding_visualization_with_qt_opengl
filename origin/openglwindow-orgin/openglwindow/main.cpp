#include "openglwindow.h"

#include <QGuiApplication>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QtMath>


//! [1]
class WaveWindow : public OpenGLWindow
{
public:
    using OpenGLWindow::OpenGLWindow;

    void initialize() override;
    void render() override;

private:
    GLint m_posAttr = 0;
    GLint m_colAttr = 0;
    GLint m_matrixUniform = 0;

    QOpenGLShaderProgram *m_program = nullptr;
    int m_frame = 0;
    bool m_right = true;
};
//! [1]

//! [2]
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(16);

    WaveWindow window1;
    window1.setFormat(format);
    window1.resize(640, 480);
    window1.show();
    window1.setAnimating(true);

    return app.exec();
}
//! [2]


//! [3]
static const char *vertexShaderSource =
    "attribute highp vec4 posAttr;\n"
    "attribute lowp vec4 colAttr;\n"
    "varying lowp vec4 col;\n"
    "uniform highp mat4 matrix;\n"
    "void main() {\n"
    "   col = colAttr;\n"
    "   gl_Position = matrix * posAttr;\n"
    "}\n";

static const char *fragmentShaderSource =
    "varying lowp vec4 col;\n"
    "void main() {\n"
    "   gl_FragColor = col;\n"
    "}\n";
//! [3]

//! [4]
void WaveWindow::initialize()
{
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();

    m_posAttr = m_program->attributeLocation("posAttr");
    Q_ASSERT(m_posAttr != -1);
    m_colAttr = m_program->attributeLocation("colAttr");
    Q_ASSERT(m_colAttr != -1);
    m_matrixUniform = m_program->uniformLocation("matrix");
    Q_ASSERT(m_matrixUniform != -1);
}
//! [4]

//! [5]
void WaveWindow::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale *0.5, height() * retinaScale *0.5);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    QMatrix4x4 matrix;
    matrix.perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    matrix.translate(0, 0, -2);
    //matrix.rotate(100.0f * m_frame / screen()->refreshRate(), 0, 1, 0);

    m_program->setUniformValue(m_matrixUniform, matrix);

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

    glVertexAttribPointer(m_posAttr, 2, GL_FLOAT, GL_FALSE, 0, pipe_vertices);
    glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, pipe_colors);

    glEnableVertexAttribArray(m_posAttr);
    glEnableVertexAttribArray(m_colAttr);

    glDrawArrays(GL_LINE_STRIP, 0, 9);

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

    m_program->setUniformValue(m_matrixUniform, matrix);

    glVertexAttribPointer(m_posAttr, 2, GL_FLOAT, GL_FALSE, 0, torch_vertices);
    glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, torch_colors);

    glDrawArrays(GL_LINE_STRIP, 0, 8);
    //glDisableVertexAttribArray(m_colAttr);
    //glDisableVertexAttribArray(m_posAttr);

    //glClear(GL_COLOR_BUFFER_BIT);


    glDisableVertexAttribArray(m_colAttr);
    glDisableVertexAttribArray(m_posAttr);

    m_program->release();

    ++m_frame;
}
//! [5]
