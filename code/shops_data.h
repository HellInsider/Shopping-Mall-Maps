#ifndef SHOP_DATA_H
#define SHOP_DATA_H
#include <QtGlobal>
#include <QTime>



#define QUINT_MAX 4294967295

enum data_accessable
{
    DA_SVG_ID=0,
    DA_SHORT_NAME,
    DA_IS_UNIQUE,
    DA_SHORT_INFO,
    DA_OPENS_AT,
    DA_CLOSES_AT,
    DA_FULL_INFO
};

#define DA_FIRST_CODE DA_SVG_ID
#define DA_LAST_CODE DA_FULL_INFO

struct one_shop_data
{
    QString shortName;
    bool isUnique;
    QString shortInfo;
    QTime opensAt;
    QTime closesAt;
    QString fullInfo;
};

class QXmlStreamReader;

class shops_data
{

    QStringList shortNames;
    bool* isUnique;
    QStringList shortInfo;
    QTime* opensAt;
    QTime* closesAt;
    QStringList fullInfo;

    shops_data();
    shops_data(quint32 sz);
    friend shops_data* ReadInfo(QXmlStreamReader& reader, QMap<QString, quint32>& indexes);
    friend void DestroyInfo (shops_data *& i);
public:


    const QStringList& getShortNames() const;
    bool *getUniqueness() const;
    QStringList getShortInfos() const;
    QTime *getOpeningTimes() const;
    QTime *getClosingTimes() const;
    QStringList getFullInfos() const;
    one_shop_data getOneShopParams(quint32 index);
};


void DestroyInfo (shops_data *& i);
shops_data* InitInfoFromFile(QString const & file, QMap<QString, quint32>& indexes);





#endif // SHOP_DATA_H
