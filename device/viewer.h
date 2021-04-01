#ifndef VIEWER_H
#define VIEWER_H
#include <QGraphicsView>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/qgraphicssvgitem.h>
#include <QWheelEvent>
#include <QtMath>
#include "map.h"

class coord; //Misha's
class graph; //Misha's

enum viewer_error_code
{
    VEC_SUCCESFULL,
    VEC_FAILED_TO_OPEN_FILE,
    VEC_ENTITY_CREATION_ERROR
};


class viewer: public QGraphicsView
{
    Q_OBJECT
    enum renderer_type
    {
        RENDERER_NATIVE,
        RENDERER_OPENGL,
        RENDERER_IMAGE
    };

 public:
    explicit viewer(QWidget *parent = nullptr);
    bool InitMap(const QString &fileName);       // Saving new map imame
    void ClearMap();                            // deleting img ptr

    void ViewMap();                             // Draws map
    void ViewObject();                          // Drows something (preparing for future) f.e. selected shop
    void ViewGraph();                           // Shows graph on map (for debug )
    void ViewPath(coord *from, coord *to);      // Shows path from firt point to second point (calls Graph)
    float zoomFactor();
    float GetMapPicScale();
    void Clear();
    ~viewer();

signals:
    void zoomChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void drawBackground(QPainter *p, const QRectF &) override;

private:
    void zoomBy(float factor);
    void SetRenderer(renderer_type type = RENDERER_NATIVE);
    void SetAntialiasing(bool antialiasing);


    renderer_type rendererType;

    QGraphicsSvgItem *svgItem;
    QGraphicsRectItem *backgroundItem;
    QGraphicsRectItem *outlineItem;
    QSvgRenderer *svgRenderer;
    QSvgRenderer *mapRenderer;
    QGraphicsScene *mapScene;

    QGraphicsSvgItem* mapPic;
    graph *path;
    QImage image;

};

#endif // VIEWER_H
