#include "tilecachenumbersclass.h"

SimpleGranuleCacheNumber::SimpleGranuleCacheNumber(const qint32 granuleId, const qint32 productId, QObject *parent) :
    QObject(parent)
{
    _granuleId = granuleId;
    _productId = productId;
}

TiledGranuleCacheNumber::TiledGranuleCacheNumber(const qint32 granuleId, const qint32 productId, QObject *parent) :
    QObject(parent)
{
    _granuleId = granuleId;
    _productId = productId;
    _height = 0;
    _transparency = 0;
}

void TiledGranuleCacheNumber::setTransparency(qint32 granuleId, qint32 transparency)
{
    if (granuleId == _granuleId)
        _transparency = transparency;
}