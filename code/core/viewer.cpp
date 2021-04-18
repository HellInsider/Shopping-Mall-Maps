#include "device/viewer.h"
#include "device/graph.h"
#include <QtSvg/QSvgRenderer>
#include <QtSvg/qgraphicssvgitem.h>
#include <QWheelEvent>
#include <QtMath>
#include <QGestureEvent>

#include <QDebug>

#ifndef QT_NO_OPENGL
#include <QtOpenGL/QGLWidget>
#endif

viewer::viewer(QWidget *parent) :
    QGraphicsView(parent),
    svgRenderer(nullptr),
    rendererType(RENDERER_IMAGE),
    svgItem(nullptr),
    backgroundItem(nullptr),
    outlineItem(nullptr),
    mapPic(nullptr)
{
    mapScene = new QGraphicsScene();
    setScene(mapScene);
    setTransformationAnchor(AnchorUnderMouse);
    setDragMode(ScrollHandDrag);
    setViewportUpdateMode(FullViewportUpdate);
    setAttribute(Qt::WA_AcceptTouchEvents);
}

void viewer::paintEvent(QPaintEvent *event)
{

   if (rendererType == RENDERER_IMAGE)
    {
        if (image.size() != viewport()->size())
        {
            image = QImage(viewport()->size(), QImage::Format_ARGB32_Premultiplied);
        }

        // My way
        QPainter p(this);
        QGraphicsView::render(&p);
        p.end();
        QGraphicsView::paintEvent(event);

        //QGraphicsSvgItem *black = new QGraphicsSvgItem();
        //black->setSharedRenderer(svgRenderer);
        //black->setElementId(QLatin1String("layer2"));
        //svgRenderer->render(&p, QLatin1String("layer2"));

        //QPainter imagePainter(&image);
        //QGraphicsView::render(&imagePainter);
        //imagePainter.end();

        //QPainter p(viewport());

        // Other way
        //QPainter p(viewport());
        //p.drawImage(0, 0, image);
        //if (isPathNeeded)
        //{
        //    ViewPath();
        //}
    }
    else
    {
        QGraphicsView::paintEvent(event);
    }
}

void viewer::drawBackground(QPainter *p, const QRectF &)
{
    p->save();
    p->resetTransform();
    p->drawTiledPixmap(viewport()->rect(), backgroundBrush().texture());
    p->restore();
}

void viewer::SetAntialiasing(bool antialiasing)
{
    setRenderHint(QPainter::Antialiasing, antialiasing);
}

void viewer::SetRenderer(renderer_type type)
{
    rendererType = type;

#ifndef QT_NO_OPENGL
    if (rendererType == RENDERER_OPENGL)
    {
        setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    }
    else
#endif
    {
        setViewport(new QWidget);
    }
}

bool viewer::InitMap(const QString &fileName)
{
    qDebug() << "init";
    Clear();

    svgRenderer = new QSvgRenderer(fileName);
    mapPic = new QGraphicsSvgItem();
    mapPic->setSharedRenderer(svgRenderer);

    mapScene->clear();
    resetTransform();

    mapPic->setFlags(QGraphicsItem::ItemClipsToShape);
    mapPic->setCacheMode(QGraphicsItem::NoCache);
    mapPic->setZValue(1);

    backgroundItem = new QGraphicsRectItem(mapPic->boundingRect());
    backgroundItem->setBrush(Qt::blue);
    backgroundItem->setPen(Qt::NoPen);
    backgroundItem->setVisible(backgroundItem ? backgroundItem->isVisible() : false);
    backgroundItem->setZValue(-1);

    outlineItem = new QGraphicsRectItem(mapPic->boundingRect());
    QPen outline(Qt::black, 2, Qt::DashLine);
    outline.setCosmetic(true);
    outlineItem->setPen(outline);
    outlineItem->setBrush(Qt::NoBrush);
    outlineItem->setVisible(outlineItem ? outlineItem->isVisible() : true);
    outlineItem->setZValue(1);

    mapScene->addItem(backgroundItem);
    mapScene->addItem(mapPic);
    mapScene->addItem(outlineItem);

    mapScene->setSceneRect(outlineItem->boundingRect().adjusted(-10, -10, 10, 10));

    return true;
}

void viewer::Clear()
{
    if (svgRenderer)
        delete svgRenderer;
    if (backgroundItem)
        delete backgroundItem;
    if (outlineItem)
        delete outlineItem;
    if (mapPic)
        delete mapPic;
}

viewer::~viewer()
{
    Clear();
    delete mapScene;
}

void viewer::ClearMap()
{
    if (mapPic)
        delete mapPic;

    mapPic = nullptr;
}

void viewer::ViewMap()
{
    if(mapPic != nullptr)
    {
        mapPic->show();
    }
}

void viewer::ViewObject()
{

}

void viewer::ViewGraph()
{

}

void viewer::ViewPath()
{
    graph w;
    w.adjacencyList = {
    {{555,555},0,{{1,5},{2,6},{3,4}}},
    {{30,485},0,{{0,5},{2,6},{5,9}}},
    {{60,100},0,{{0,6},{1,6},{5,7}}},
    {{200,30},0,{{0,4},{4,14}}},
    {{120,30},0,{{3,14},{6,5}}},
    {{120,308},0,{{1,9},{2,7},{6,5}}},
    {{160,0},0,{{5,5},{4,5}}}
    };
    std::vector<vertex_graph> path = w.SearchWay(0,6);
    for (int i = 0; i < path.size() - 1; ++i)
    {
        QPainter painter(viewport());
        painter.setPen(QPen(Qt::red, 3, Qt::DotLine, Qt::RoundCap));
        painter.drawLine(path[i].vertexCoordinates.x, path[i].vertexCoordinates.y,
                         path[i + 1].vertexCoordinates.x, path[i + 1].vertexCoordinates.y);
    }
}

float viewer::GetMapPicScale()
{
    return mapPic->scale();
}

float viewer::ZoomFactor()
{
    return transform().m11();
}

void viewer::wheelEvent(QWheelEvent *event)
{
    ZoomBy(qPow(1.2, event->angleDelta().y() / 240.0));      //Special for Arina and Yakov:
}                                                            //If we don't need scale with mouse wheel, delete
                                                             //functions wheelEvent, ZoomFactor, GetMapPicScale and ZoomBy.

void viewer::ZoomBy(float factor)
{
    //const qreal currentZoom = ZoomFactor();
    const float currentZoom = mapPic->scale();
    if ((factor < 1 && currentZoom < 0.1) || (factor > 1 && currentZoom > 10))
        return;
    scale(factor, factor);
    mapPic->setScale(factor);
    emit ZoomChanged();
}


bool viewer::viewportEvent(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
        QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();

        if (touchPoints.count() == 2)
        {                                                       // determine scale factor
            const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
            const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
            qreal currentScaleFactor =
                    QLineF(touchPoint0.pos(), touchPoint1.pos()).length()
                    / QLineF(touchPoint0.startPos(), touchPoint1.startPos()).length();

            if (touchEvent->touchPointStates() & Qt::TouchPointReleased)    // if one of the fingers is released, remember the current scale
            {                                                               // factor so that adding another finger later will continue zooming
                totalScaleFactor *= currentScaleFactor;                     // by adding new scale factor to the existing remembered value.
                currentScaleFactor = 1;
            }
            setTransform(QTransform::fromScale(totalScaleFactor * currentScaleFactor,
                                               totalScaleFactor * currentScaleFactor));
        }
        return true;
    }
    default:
        break;
    }
    return QGraphicsView::viewportEvent(event);
}

