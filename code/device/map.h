#ifndef MAP_H
#define MAP_H

#include <QObject>
#include <QMap>
#include <QtSvg/qgraphicssvgitem.h>

class shops_data;

class graph_alternative;




#define __LEAVE_MAP_AS_IT_IS (QString(""))

enum map_error_code
{
    _MEC_DONE,
    _MEC_FAILED_TO_OPEN_FILE,
    _MEC_ENTITY_CREATION_ERROR
};

class layer
{
    QString layerName;
    friend class graph_parser;
public:
    const QString& GetName() const
    {
        return layerName;
    }
};

class shops_on_floor : public layer
{
    QMap<QString, quint32> namesAndIndexes;
    friend class graph_parser;
    friend class map;
public:
    const QMap<QString, quint32>& GetShops() const
    {
        return namesAndIndexes;
    }
};

class background_on_floor : public layer
{
};

class paths_on_floor : public layer
{
    std::vector<QString> graphObjects;
    friend class graph_parser;
public:
    const std::vector<QString>& GetObjects() const
    {
        return graphObjects;
    }
};

class floor_layer : public layer
{
    background_on_floor bckgrndLr;
    shops_on_floor shopsLr;
    paths_on_floor pathsLr;
    friend class graph_parser;
    friend class map;
public:
//    const background_on_floor& getBckgrndLr() const;
//    const shops_on_floor& getShopsLr() const;
//    const paths_on_floor& getPathsLr() const;
    background_on_floor GetBckgrndLr() const;
    shops_on_floor GetShopsLr() const;
    paths_on_floor GetPathsLr() const;
};





class map : public QObject // holds information about current map
{
    Q_OBJECT

private:
    QString svgMapFileName; // *.svg file to be drawn as map
    std::vector<floor_layer>* floorsIdData; // svg ID info for each floor
    graph_alternative* paths; // paths graph, based on svg pic

    QLatin1String mapInfoFileName; // *xml file, containing extra information for map objects
    shops_data* info; // loaded and parsed extras data from file above
    QMap<QString, quint32> object_indexes; // their indexes


    // inner functions. im too lazy to incapsulate

    void ClearObjects();
    void ClearFloorData();
    void ClearGraph();
    void ClearAll();

    void OnChange(quint8 flags);

    bool OpenFile(const QString &fileName);//IDD

public:

    explicit map(QObject *parent = nullptr);
    ~map();

    quint32 GetIndex(QLatin1String id);  //returns an ordinal number which is used to access info
    bool Exists (QLatin1String id); // checks if the object exists
    shops_data* GetInfo (void); // allows to acces extra info. DO NOT MODIFY THIS INFO. IT WILL HURT.

signals:
    // v = new svg pic ptr
    //NOTE: might be nullptr, check it!
    void MapPictureDataChanged(std::vector<floor_layer>* svgIds);

    // g = new graph ptr
    //NOTE: might be nullptr, check it!
    void PathGraphChanged(graph_alternative* graph);

    // i = new info
    // NOTE: might be empty
    void MapInfoChanged(shops_data const * const & info, QMap<QString, quint32> indexes);

    public slots:
    // called on changing the mall to show
    // call with __LEAVE_MAP_AS_IT_IS instead of parameter you want to keep unchanged
    void SetAnotherMap(QString const & mapToSet, QString const & extrasToSet);

};

#endif // MAP_H
