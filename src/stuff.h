#ifndef STUFF_H
#define STUFF_H

#include <QString>
#include <map>

typedef std::map<uint, QString> StringMap;

extern const StringMap tileTypes;
extern const StringMap exitTypes;
extern const StringMap spriteTypes;

QString tileType(uint);
QString exitType(uint);
QString spriteType(uint);

#endif // STUFF_H
