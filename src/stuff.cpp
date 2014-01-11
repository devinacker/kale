#include <stdexcept>
#include "stuff.h"

QString fromStringMap(const StringMap& map, uint type) {
    try {
        return map.at(type);
    } catch (std::out_of_range) {
        return QString("%1: unknown")
                .arg(QString::number(type, 16).rightJustified(2, QLatin1Char('0')).toUpper());
    }
}

const StringMap tileTypes ({
    {0x00, "00: none"},

    {0xFF, "FF: background"}
});

const StringMap exitTypes ({
    {0x00, "00: normal"},
    {0x01, "01: level 1"},
    {0x02, "01: level 2"},
    {0x03, "01: level 3"},
    {0x04, "01: level 4"},
    {0x05, "01: level 5"},
    {0x06, "01: level 6"},
    {0x07, "01: level 7"},
    {0x10, "10: end of level"},
    {0x18, "18: museum"},
    {0x19, "19: arena"},
    {0x1a, "1A: quick draw (easy)"},
    {0x1b, "1B: egg catch (easy)"},
    {0x1c, "1C: crane fever (easy)"},
    {0x1d, "1D: warp star"},
    {0x1e, "1E: previous world"},
    {0x1f, "1F: next world"},
    {0x2a, "1A: quick draw (medium)"},
    {0x2b, "1B: egg catch (medium)"},
    {0x2c, "1C: crane fever (medium)"},
    {0x3a, "1A: quick draw (hard)"},
    {0x3b, "1B: egg catch (hard)"},
    {0x3c, "1C: crane fever (hard)"}
});

const StringMap spriteTypes ({

});

QString tileType(uint type) {
    return fromStringMap(tileTypes, type);
}

QString exitType(uint type){
    return fromStringMap(exitTypes, type);
}

QString spriteType(uint type){
    return fromStringMap(spriteTypes, type);
}
