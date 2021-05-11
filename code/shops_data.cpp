#include "shops_data.h"


#include <QFile>
#include <QXmlStreamReader>
#include <QMap>

const QString XML_DOC_NAMES[] = {"SVG_ID",
                                 "short_name",
                                 "is_a_shop",
                                 "short_info",
                                 "opens",
                                 "closes",
                                 "full_info",
                                 "coord"};



shops_data::shops_data(): shortNames(), shortInfo(), fullInfo()
{
    isUnique= nullptr;
    opensAt= nullptr;
    closesAt= nullptr;
}

shops_data::shops_data(quint32 size): shortNames(), shortInfo(), fullInfo()
{
    isUnique =  new bool[size];
    opensAt = new QTime[size];
    closesAt =  new QTime[size];
    exits = new point[size];

}

const QStringList &shops_data::getSvgIds() const
{
    return svgIds;
}

const QStringList& shops_data::getShortNames() const
{
    return shortNames;
}

bool *shops_data::getUniqueness() const
{
    return isUnique;
}

QStringList shops_data::getShortInfos() const
{
    return shortInfo;
}

QTime *shops_data::getOpeningTimes() const
{
    return opensAt;
}

QTime *shops_data::getClosingTimes() const
{
    return closesAt;
}

point *shops_data::getExits() const
{
    return exits;
}

const QStringList &shops_data::getFullInfos() const
{
    return fullInfo;
}

one_shop_data shops_data::getOneShopParams(quint32 index)
{
    one_shop_data res;
    res.svgId = svgIds[index];
    res.shortName = shortNames[index];
    res.isUnique = isUnique[index];
    res.shortInfo = shortInfo[index];
    res.opensAt = opensAt[index];
    res.closesAt = closesAt[index];
    res.fullInfo = fullInfo[index];
    res.exit = exits[index];

    return res;
}


void DestroyInfo (shops_data *& i)
{
    i->shortNames.clear();
    delete[] i->isUnique;
    i->shortInfo.clear();
    delete[] i->opensAt;
    delete[] i->closesAt;
    i->fullInfo.clear();
    delete[] i->exits;
    delete i;
    i = nullptr;
}

shops_data* ReadInfo(QXmlStreamReader& reader, QMap<QString, quint32>& indexes)
{
    reader.readNextStartElement();
    if (reader.name()!="size")
    {
        return nullptr; // add error hadling
    }
    quint32 size = reader.readElementText().toUInt();
    shops_data* res = new shops_data(size);
    quint32 curr_index = 0;


    while(curr_index < size)
    {
        reader.readNextStartElement();
        if(reader.name() == "InfoLine")
        {
           while(reader.readNextStartElement())
           {
               for (int i = DA_FIRST_CODE; i<=DA_LAST_CODE; i++)
               {
                   if (reader.name() == XML_DOC_NAMES[i])
                   {
                       switch (i)
                       {
                       case DA_SVG_ID:
                       {
                           QString svgId = reader.readElementText();
                           indexes.insert(svgId, curr_index);
                           res->svgIds.append(svgId);
                           goto for_break;
                       }
                           break;
                       case DA_SHORT_NAME:
                           res->shortNames.append(reader.readElementText());
                           goto for_break;
                           break;
                       case DA_IS_UNIQUE:
                           res->isUnique[curr_index] = (reader.readElementText() == "true");
                           goto for_break;
                           break;
                       case DA_SHORT_INFO:
                           res->shortInfo.append(reader.readElementText());
                           goto for_break;
                           break;
                       case DA_OPENS_AT:
                           res->opensAt[curr_index] = QTime::fromString(reader.readElementText(), "hh:mm:ss:zzz");
                           goto for_break;
                           break;
                       case DA_CLOSES_AT:
                           res->closesAt[curr_index] = QTime::fromString(reader.readElementText(), "hh:mm:ss:zzz");
                           goto for_break;
                           break;
                       case DA_FULL_INFO:
                           res->fullInfo.append(reader.readElementText());
                           goto for_break;
                           break;
                       case DA_EXIT_POSITION:
                           reader.readNextStartElement();
                           res->exits[curr_index].x = reader.readElementText().toDouble();
                           reader.readNextStartElement();
                           res->exits[curr_index].y = reader.readElementText().toDouble();
                           goto for_break;
                           break;
                       }
                   }
               }
           for_break:
               ;
           }
           Q_ASSERT(reader.isEndElement());
           if (res->fullInfo.size() == curr_index)
           {
               res->fullInfo.append("");
           }
           if (res->shortInfo.size() == curr_index)
           {
               res->shortInfo.append("");
           }
           if (res->shortNames.size() == curr_index)
           {
               res->shortNames.append("");
           }
           //reader.skipCurrentElement();
//           while (reader.isEndElement())
//           {
//               QStringRef dbg = reader.name();

//           }
           curr_index++;
        }
    }
    return res;
}

shops_data* InitInfoFromFile(QString const & file, QMap<QString, quint32>& indexes)
{
    QXmlStreamReader reader;
    QFile inputFile(file);
    if(!inputFile.open(QFile::ReadOnly | QFile::Text))
    {
        return nullptr; // add error hadling
    }

    reader.setDevice(&inputFile);

    if (reader.readNextStartElement())
    {
        if (reader.name() == "Info")
        {
            return ReadInfo(reader, indexes);
        }
        else
        {
         return nullptr; // add error hadling
        }
    }
    return nullptr;
}







