#ifndef LINE3DNODE_H
#define LINE3DNODE_H

#include <QObject>
#include <QColor>
#include <QVector3D>
#include <QGLPainter>

#include "more/structure.h"
#include "more/geofunctions.h"

class Line3DNode : public QObject
{
    Q_OBJECT
public:
    explicit Line3DNode(QObject *parent = 0);
    void createLine(GeoCoords pos1, GeoCoords pos2);
    void createParallelLine(GeoCoords pos1, GeoCoords pos2);
    void draw(QGLPainter *painter);
    void setObjectName(QString value){_objectName = value;}
    QString objectName(){return _objectName;}
    void setLineWidth(float value){_lineWidth = value;}
    float lineWidth(){return _lineWidth;}
    void setLineColor(QColor value){_color = value;}
    QColor lineColor(){return _color;}
    
private:
    float _lineWidth;
    QColor _color;
    GeoCoords _pos1;
    GeoCoords _pos2;
    QVector3DArray verts;
    QString _objectName;

signals:
    
public slots:
    
};

#endif // LINE3DNODE_H
