#include "device/viewer.h"
#include "device/fine_graph.h"
#include <QtSvg/QSvgRenderer>
#include <QtSvg/qgraphicssvgitem.h>
#include <QWheelEvent>
#include <QtMath>
#include <QGestureEvent>
#include <QGraphicsColorizeEffect>
#include <QLabel>
#include <QFont>

#include <QDebug>

#ifndef QT_NO_OPENGL
#include <QtOpenGL/QGLWidget>
#endif

viewer::viewer(QWidget *parent) :
    QGraphicsView(parent),
    svgRenderer(nullptr),
    rendererType(RENDERER_IMAGE),
    svgItem(nullptr),
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

        QPainter p(viewport());
        QGraphicsView::render(&p);
        p.end();
        QGraphicsView::paintEvent(event);
    }
    else
    {
        QGraphicsView::paintEvent(event);
    }
}

void viewer::AddUnstableVisible(QString id)
{
    if (unstableVisibleItems.find(id) == unstableVisibleItems.end() && id != "")
    {
        QGraphicsSvgItem *newItem = new QGraphicsSvgItem();
        newItem->setSharedRenderer(svgRenderer);
        newItem->setElementId(id);

        newItem->setVisible(true);
        newItem->setZValue(1);
        QRectF bound = svgRenderer->boundsOnElement(id);
        newItem->setPos(bound.x(), bound.y());  // this code is must to set correct posisiton

        unstableVisibleItems[id] = newItem;
        mapScene->addItem(unstableVisibleItems[id]);
    }
}

void viewer::AddSelectable(QString id)
{
    QGraphicsSvgItem *newItem = new QGraphicsSvgItem();
    newItem->setSharedRenderer(svgRenderer);
    newItem->setElementId(id);

    newItem->setVisible(true);
    newItem->setZValue(-0.5);
    QRectF bound = svgRenderer->boundsOnElement(id);
    newItem->setPos(bound.x(), bound.y());  // this code is must to set correct posisiton

    selectableItems[id] = newItem;
    mapScene->addItem(selectableItems[id]);
}

void viewer::HighlightShop(QString id) // Clears automaticaly
{
    ClearHighlited();
    auto search = selectableItems.find(id);
    if (search == selectableItems.end())
       return;

    recoloredItem = selectableItems[id];

    QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect;
    effect->setColor(highlightColor);
    effect->setStrength(1);
    recoloredItem->setGraphicsEffect(effect);
}

void viewer::ClearHighlited()
{
    if (recoloredItem == nullptr)
    {
        return;
    }
    QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect;
    effect->setColor(highlightColor);
    effect->setStrength(0);

    recoloredItem->setGraphicsEffect(effect);
    recoloredItem = nullptr;
}


void viewer::ChangeVisibility(QString id, bool isVisible)
{
    if (unstableVisibleItems.find(id)!=unstableVisibleItems.end())
    {
        unstableVisibleItems[id]->setVisible(isVisible);
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
    QRectF bound = svgRenderer->boundsOnElement("layer9");
    mapPic->setPos(bound.x(), bound.y());  // this code is must to set correct posisiton
    mapScene->clear();
    resetTransform();

    mapPic->setFlags(QGraphicsItem::ItemClipsToShape);
    mapPic->setCacheMode(QGraphicsItem::NoCache);
    mapPic->setZValue(-1);
    mapScene->addItem(mapPic);

    mapScene->setSceneRect(bound);

    setTransform(QTransform::fromScale(totalScaleFactor * 8,
                                       totalScaleFactor * 8));
    return true;
}

void viewer::Clear()
{
    ClearSelectables();
    ClearLabels();
    ClearHighlited();
    ClearPolyline();
    ClearUnstableVisible();
    if (svgRenderer)
        delete svgRenderer;
    if (mapPic)
        delete mapPic;
}

void viewer::AddLabel(QString text, int x, int y, QString idToLabeling, QWidget *parent)
{
    QLabel * l = new QLabel(parent);
    l->setText(text);
    l->setMargin(1);
    QFont font("Consolas");
    //font.setStyleHint(QFont::Fantasy, QFont::PreferBitmap);
    //font.setWeight(QFont::ExtraLight);
    font.setPointSizeF(1.25);
    l->setFont(font);
    QRectF bound = svgRenderer->boundsOnElement(idToLabeling);
    l->move(bound.center().x(), bound.center().y());  // this code is must to set correct posisiton
    itemsLabels.push_back(l);
    mapScene->addWidget(l);
}

void viewer::AddPolyline(path* currentPath, quint32 currentFloorIndex)
{
    if (currentPath == nullptr)
    {
        return;
    }
    ClearPolyline();
    QPen color;
    color.setWidthF(0.2);
    color.setColor(QColor(0x66,0x80, 0x00, 0xff));
    color.setStyle(Qt::DashLine);
    //mapPic->setZValue(-0.1);
    SetAntialiasing(1);

    if (currentFloorIndex == currentPath->startFloorIndex)
    {
        std::list<point> &toDraw = currentPath->prePath;
        auto p = toDraw.begin();
        auto next = p++;
        while (p!= toDraw.end() && next != toDraw.end())
        {
            pathLines.push_back(mapScene->addLine(p->x, p->y, next->x, next->y, color));
            p++;
            next++;
        }
    }
    if (currentFloorIndex == currentPath->endFloorIndex)
    {
        std::list<point> &toDraw = currentPath->postPath;
        auto p = toDraw.begin();
        auto next = p++;
        while (p!= toDraw.end() && next != toDraw.end())
        {
            pathLines.push_back(mapScene->addLine(p->x, p->y, next->x, next->y, color));
            p++;
            next++;
        }
    }

}

void viewer::ClearUnstableVisible()
{
    for (auto & item : unstableVisibleItems)
        delete item.second;
    unstableVisibleItems.clear();
}


void viewer::ClearSelectables()
{
    for (auto & item : selectableItems)
        delete item.second;
    selectableItems.clear();
}

void viewer::ClearLabels()
{
    for (size_t i =  0; i < itemsLabels.size(); i++)
        delete itemsLabels[i];
    itemsLabels.clear();
}


void viewer::ClearPolyline()
{
    for (size_t i =  0; i < pathLines.size(); i++)
        delete pathLines[i];
    pathLines.clear();
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
    //mapPic->setScale(factor);
    emit ZoomChanged();
}


bool viewer::viewportEvent(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::TouchBegin:
    {
        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
        QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();


        if (touchPoints.count() == 1)
        {
            //For Arina
            //On single touch event.
            //If it looks strange, you can move this block from "Touch Begin" to "TouchUpdate" or "TouchEnd"
            //For example highlighting:
/*                 static bool a = true;
                 if(a)
                     HighlightShop("2_shop_4");
                 else
                     ClearHighlited();
                 a = !a;
*/

         }
    break;
    }
    case QEvent::TouchUpdate:
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

            if(totalScaleFactor * currentScaleFactor <= 3 * maxZoom && totalScaleFactor * currentScaleFactor >= 3 / maxZoom)
            {
                if (touchEvent->touchPointStates() & Qt::TouchPointReleased)    // if one of the fingers is released, remember the current scale
                {                                                               // factor so that adding another finger later will continue zooming
                    totalScaleFactor *= currentScaleFactor;                     // by adding new scale factor to the existing remembered value.
                    currentScaleFactor = 1;
                }
                setTransform(QTransform::fromScale(totalScaleFactor * currentScaleFactor,
                                                   totalScaleFactor * currentScaleFactor));
            }

        }

        return true;
        break;
    }
    case QEvent::TouchEnd:
    {
        break;
    }
    default:
        break;
    }
    return QGraphicsView::viewportEvent(event);
}

void viewer::ChangeBgrLayer(QString id)
{
    mapPic->setElementId(id);
}

