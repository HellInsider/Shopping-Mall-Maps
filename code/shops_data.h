#ifndef SHOP_DATA_H
#define SHOP_DATA_H
#include <QtGlobal>
#include <QTime>
#include "device/Vector2.hpp"



#define QUINT_MAX 4294967295

using point = Vector2<double>;

enum data_accessable
{
    DA_SVG_ID=0,
    DA_SHORT_NAME,
    DA_IS_UNIQUE,
    DA_SHORT_INFO,
    DA_OPENS_AT,
    DA_CLOSES_AT,
    DA_FULL_INFO,
    DA_EXIT_POSITION
};

#define DA_FIRST_CODE DA_SVG_ID
#define DA_LAST_CODE DA_EXIT_POSITION

struct one_shop_data
{
    QString svgId;
    QString shortName;
    bool isUnique;
    QString shortInfo;
    QTime opensAt;
    QTime closesAt;
    QString fullInfo;
    point exit;
};

class QXmlStreamReader;

class shops_data
{
    QStringList svgIds;
    QStringList shortNames;
    bool* isUnique;
    QStringList shortInfo;
    QTime* opensAt;
    QTime* closesAt;
    QStringList fullInfo;
    point* exits;


    shops_data();
    shops_data(quint32 sz);
    friend shops_data* ReadInfo(QXmlStreamReader& reader, QMap<QString, quint32>& indexes);
    friend void DestroyInfo (shops_data *& i);
public:

    const QStringList& getSvgIds() const;
    const QStringList& getShortNames() const;
    bool *getUniqueness() const;
    QStringList getShortInfos() const;
    QTime *getOpeningTimes() const;
    QTime *getClosingTimes() const;
    point *getExits() const;
    const QStringList& getFullInfos() const;
    one_shop_data getOneShopParams(quint32 index);
};


void DestroyInfo (shops_data *& i);
shops_data* InitInfoFromFile(QString const & file, QMap<QString, quint32>& indexes);





#endif // SHOP_DATA_H
