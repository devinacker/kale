/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef FILE_H
#define FILE_H

#include <cstdio>
#include <QFile>
#include <QImage>
#include <cstdint>
#include "compress.h"

#define BANK_SIZE 0x2000
#define CHR_SIZE  0x400
#define HEADER_SIZE 16

struct romaddr_t {
    uint bank;
    uint addr;

    romaddr_t operator+(uint offset) const {
        return {bank, addr + offset};
    }
};

class ROMFile: public QFile {
public:

    //TODO: these
    enum version_e {
        kirby_jp  = 0
    };

    ROMFile();

    bool         openROM(OpenMode flags);

    version_e getVersion() const;
    uint getNumPRGBanks() const;
    uint getNumCHRBanks() const;

    uint toOffset(romaddr_t addr) const;

    size_t       readData(romaddr_t addr, uint size, void *buffer);
    uint8_t      readByte(romaddr_t addr);
    uint16_t     readInt16(romaddr_t addr);
    uint32_t     readInt32(romaddr_t addr);
    romaddr_t    readPointer(romaddr_t addrL, romaddr_t addrH, romaddr_t addrB, uint offset);
    romaddr_t    readShortPointer(romaddr_t addrL, romaddr_t addrH, uint bank, uint offset);
    size_t       readFromPointer(romaddr_t addrL, romaddr_t addrH, romaddr_t addrB,
                                 uint size, void *buffer, uint offset = 0);
    size_t       readFromShortPointer(romaddr_t addrL, romaddr_t addrH, uint bank,
                                      uint size, void *buffer, uint offset = 0);
    uint writeData(romaddr_t addr, uint size, const void *buffer);
    uint writeByte(romaddr_t addr, uint8_t data);
    uint writeInt16(romaddr_t addr, uint16_t data);
    uint writeInt32(romaddr_t addr, uint32_t data);
    uint writeToPointer(romaddr_t ptrL, romaddr_t ptrH, romaddr_t ptrB, romaddr_t addr,
                        uint size, const void *buffer, uint offset = 0);
    uint writeToShortPointer(romaddr_t ptrL, romaddr_t ptrH, romaddr_t addr,
                             uint size, const void *buffer, uint offset = 0);

    QImage readCHRBank(uint bank);
private:
    version_e version;
    uint numPRGBanks, numCHRBanks;
};

// small helper for generating and sorting compressed (or not) data
// to be organized within the ROM based on size
struct DataChunk {
    enum type_e {
        level, tileset, enemy, banks
    };

    uint8_t  data[DATA_SIZE];
    uint16_t size;
    type_e   type;
    uint     num;

    DataChunk(const void *src, uint16_t size, type_e type, uint num):
        size(size), type(type), num(num)
    {
        // level maps and tilesets get compressed, other types don't
        // (and the CHR bank tables don't even need a real pointer)
        if (type == level || type == tileset) {
            this->size = pack((uint8_t*)src, size, this->data, 0);
        } else if (type == enemy) {
            memcpy(data, src, size);
        }
    }

    bool operator< (const DataChunk &other) const {
        if (size == other.size)
            return num < other.num;

        return size < other.size;
    }
};

#endif // FILE_H
