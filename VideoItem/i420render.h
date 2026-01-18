#ifndef I42_RENDER_H
#define I42_RENDER_H
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLShader>
#include <QOpenGLPixelTransferOptions>

#include <QObject>
const static int bufferSize = 1280*720;
struct YUVData{
    YUVData(){
        Y.reserve(bufferSize);
        U.reserve(bufferSize);
        V.reserve(bufferSize);
    }
    QByteArray Y;
    QByteArray U;
    QByteArray V;
    int yLineSize;
    int uLineSize;
    int vLineSize;
    int height;
};
class I420Render : public QOpenGLFunctions
{
public:
    I420Render();
    ~I420Render();

    void init();
    void updateTextureInfo(int w, int h);
    void updateTextureData(const YUVData &data);
    void paint();
    void resize(int w, int h);

private:
    //shader程序
    QOpenGLShaderProgram m_program;
    QOpenGLTexture *mTexY = nullptr,*mTexU=nullptr,*mTexV=nullptr;

    bool mTextureAlloced = false;

    QVector<QVector2D> vertices;
    QVector<QVector2D> textures;
};

#endif // I42_RENDER_H
