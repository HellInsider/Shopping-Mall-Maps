#if 0 // does not compile
#include "graph.h"
#include <QFile>
#include <QXmlStreamReader>
#include "graph_alternative.h"

const QString FLOOR_TOKEN("g");
const QString EDGE_TOKEN("path");
const QString CONNECTION_TOKEN("circle");

const std::pair<QString,QString> LAYER_ATTRIBUTE("inkscape:groupmode", "layer");
const QString FLOOR_NUMBER_ATTRIBUTE("inkscape:label");
const QString FLOOR_PATHS_ATTRIBUTE("inkscape:label");
const QString NAME_PARTS_SEPARATOR("_");


void ParsePath(QString path, edge& res)
{
    Q_ASSERT(path.at(0) == "M");
    QStringList parts = path.split(" ", Qt::SkipEmptyParts);

    QStringList startXY = parts.at(1).split(",", Qt::SkipEmptyParts);
    QStringList endXY;

    point start(startXY.at(0).toDouble(), startXY.at(1).toDouble());
    point end = start;


    if (parts.at(2)=="H")
    {
        end.x += parts.at(3).toDouble();
        goto end_inited_mark;
    }
    if (parts.at(2)=="V")
    {
        end.y += parts.at(3).toDouble();
        goto end_inited_mark;
    }

    // practically this is else-part of both if() above. used goto to keep code simple
    endXY = parts.at(2).split(",",  Qt::SkipEmptyParts);
    end = point(endXY.at(0).toDouble(), endXY.at(1).toDouble());


    end_inited_mark:

    res.start = start < end ? start : end;
    res.end = start < end ? end : start;
    res.len = (end-start).magnitude();
}

void ParseConnection(QString connection, floor_connection& res)
{
    Q_ASSERT(path.at(0) == "M");
    QStringList parts = path.split(" ", Qt::SkipEmptyParts);

    QStringList startXY = parts.at(1).split(",", Qt::SkipEmptyParts);
    QStringList endXY;

    point start(startXY.at(0).toDouble(), startXY.at(1).toDouble());
    point end = start;


    if (parts.at(2)=="H")
    {
        end.x += parts.at(3).toDouble();
        goto end_inited_mark;
    }
    if (parts.at(2)=="V")
    {
        end.y += parts.at(3).toDouble();
        goto end_inited_mark;
    }

    // practically this is else-part of both if() above. used goto to keep code simple
    endXY = parts.at(2).split(",",  Qt::SkipEmptyParts);
    end = point(endXY.at(0).toDouble(), endXY.at(1).toDouble());


    end_inited_mark:

    res.start = start < end ? start : end;
    res.end = start < end ? end : start;
    res.len = (end-start).magnitude();
}




void CreateGraphFromSvg (QXmlStreamReader& reader, graph_alternative* toSet)
{
    reader.readNextStartElement();
    while (reader.readNextStartElement())
    {
        if(reader.name()!="g")
        {
            reader.readNextStartElement();
            continue;
        }
        reader.attributes().contains()

    }
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
                           res->isUnique[i] = (reader.readElementText() == "true");
                           goto for_break;
                           break;
                       case DA_SHORT_INFO:
                           res->shortInfo.append(reader.readElementText());
                           goto for_break;
                           break;
                       case DA_OPENS_AT:
                           res->opensAt[i] = QTime::fromString(reader.readElementText(), "hh:mm:ss:zzz");
                           goto for_break;
                           break;
                       case DA_CLOSES_AT:
                           res->closesAt[i] = QTime::fromString(reader.readElementText(), "hh:mm:ss:zzz");
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
           Q_ASSERT(reader.isEndElement());
           reader.skipCurrentElement();
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
#endif
