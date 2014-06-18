#ifndef STUFF_H
#define STUFF_H

#include <QString>
#include <map>

QString hexFormat(int num, uint digits = 0);

typedef std::map<uint, QString> StringMap;

extern const StringMap tileTypes;
extern const StringMap exitTypes;
extern const StringMap spriteTypes;
extern const StringMap musicNames;

QString tileType(uint);
QString exitType(uint);
QString spriteType(uint);

#endif // STUFF_H
