#ifndef VIEWER_H
#define VIEWER_H
#include <QGraphicsView>
#include <QGestureEvent>

#include "map.h"

class coord;
class path;
// class graph;
class QLabel;

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
    void ViewPath();      // Shows path from firt point to second point (calls Graph)
    float ZoomFactor();
    float GetMapPicScale();
    void AddUnstableVisible(QString id);
    void AddSelectable(QString id);
    void ChangeVisibility(QString id, bool isVisible);
    void ChangeBgrLayer(QString id);
    void Clear();
    void AddLabel(QString text, int x, int y, QString idToLabeling = "", QWidget *parent = nullptr);
    void AddPolyline(path* currentPath, quint32 currentFloorIndex);
    void ClearPolyline();
    void ClearLabels();
    void ClearUnstableVisible();
    void ClearSelectables();

    ~viewer();

signals:
    void ZoomChanged();


protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    bool viewportEvent(QEvent *event) override;

    void drawBackground(QPainter *p, const QRectF &) override;

private:
    void ZoomBy(float factor);
    void SetRenderer(renderer_type type = RENDERER_NATIVE);
    void SetAntialiasing(bool antialiasing);

    renderer_type rendererType;

    std::map<QString, QGraphicsSvgItem*> unstableVisibleItems;
    std::map<QString, QGraphicsSvgItem*> selectableItems;
    QGraphicsSvgItem *svgItem;
    QSvgRenderer *svgRenderer;
    QGraphicsScene *mapScene;
    std::vector<QGraphicsItem *>pathLines;

    QGraphicsSvgItem* mapPic;
    //graph *path;
    QImage image;

    std::vector<QLabel*> itemsLabels;

    bool isPathNeeded = true;
    qreal totalScaleFactor = 1;
    qreal maxZoom = 10.0;  //Maybe later I will add "speed" of zoom.
                          //When scale is high or low you will need make more gestures to zoom.
                          //In the limitation gesture will not affect on scale.
};

#endif // VIEWER_H
