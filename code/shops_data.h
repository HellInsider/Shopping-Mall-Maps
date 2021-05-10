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



class QXmlStreamReader;

class shops_data
{

    QStringList shortName;
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

    // on deafault sets toSet pointer to the needed value (NOT  SAFE, DO NOT TRY TO MODIFY DATA YOU RECIEVE THIS WAY).
    // With index provided copies data of what_needed sort of the one object to the *toSet var - in that case you may do with the data anything you want
    //template<typename T>
    void Access(QString* toSet, data_accessable what_needed, quint32 index = QUINT_MAX)
    {
        switch(what_needed)
        {
        case DA_SHORT_NAME:
            if (index == QUINT_MAX)
            {
                ;//toSet = shortName;
            }
            else
            {
                *toSet = shortName[index];
            }
            break;
        case DA_IS_UNIQUE:
            if (index == QUINT_MAX)
            {
               ;// toSet = isUnique;
            }
            else
            {
                *toSet = isUnique[index];
            }
            break;
        case DA_SHORT_INFO:
            if (index == QUINT_MAX)
            {
               ;// toSet = shortInfo;
            }
            else
            {
                *toSet = shortInfo[index];
            }
            break;
        case DA_OPENS_AT:
            if (index == QUINT_MAX)
            {
               ;// toSet = opensAt;
            }
            else
            {
              ;//  *toSet = opensAt[index];
            }
            break;
        case DA_CLOSES_AT:
            if (index == QUINT_MAX)
            {
               ;// toSet = closesAt;
            }
            else
            {
               // *toSet = closesAt[index];
            }
            break;
        case DA_FULL_INFO:
            if (index == QUINT_MAX)
            {
              //  toSet = fullInfo;
            }
            else
            {
               // *toSet = fullInfo[index];
            }
            break;
        }
    }

};


void DestroyInfo (shops_data *& i);
shops_data* InitInfoFromFile(QString const & file, QMap<QString, quint32>& indexes);





#endif // SHOP_DATA_H
