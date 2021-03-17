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
                                 "full_info"};


shops_data::shops_data(): shortName(), shortInfo(), fullInfo()
{
    isUnique= nullptr;
    opensAt= nullptr;
    closesAt= nullptr;
}

shops_data::shops_data(quint32 size): shortName(), shortInfo(), fullInfo()
{
    isUnique =  new bool[size];
    opensAt = new QTime[size];
    closesAt =  new QTime[size];
}


void DestroyInfo (shops_data *& i)
{
    i->shortName.clear();
    delete[] i->isUnique;
    i->shortInfo.clear();
    delete[] i->opensAt;
    delete[] i->closesAt;
    i->fullInfo.clear();
    delete i;
    i = nullptr;
}

shops_data* ReadInfo(QXmlStreamReader& reader, QMap<QLatin1String, quint32>& indexes)
{
    reader.readNextStartElement();
    if (reader.name()!="size")
    {
        return nullptr; // add error hadling
    }
    quint32 size = reader.readElementText().toUInt();
    shops_data* res = new shops_data(size);
    quint32 curr_index = 0;


    while(reader.readNextStartElement() && curr_index < size)
    {
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
                           indexes.insert(QLatin1String(reader.readElementText().toLatin1()), curr_index);
                           goto for_break;
                           break;
                       case DA_SHORT_NAME:
                           res->shortName.append(reader.readElementText());
                           goto for_break;
                           break;
                       case DA_IS_UNIQUE:
                           res->isUnique[i] = (reader.readElementText() == "true" ? true : false);
                           goto for_break;
                           break;
                       case DA_SHORT_INFO:
                           res->shortInfo.append(reader.readElementText());
                           goto for_break;
                           break;
                       case DA_OPENS_AT:
                           res->opensAt[i] = QTime::fromString(reader.readElementText(), "hh:mm:ss.zzz");
                           Q_ASSERT(res->opensAt[i].isValid());
                           goto for_break;
                           break;
                       case DA_CLOSES_AT:
                           res->closesAt[i] = QTime::fromString(reader.readElementText(), "hh:mm:ss.zzz");
                           Q_ASSERT(res->closesAt[i].isValid());
                           goto for_break;
                           break;
                       case DA_FULL_INFO:
                           res->fullInfo.append(reader.readElementText());
                           goto for_break;
                           break;
                       }
                   }
               }
           for_break:
               ;
           }

           curr_index++;
        }
    }
    return res;
}

shops_data* InitInfoFromFile(QString const & file, QMap<QLatin1String, quint32>& indexes)
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







