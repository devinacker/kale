/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef FILE_H
#define FILE_H

#include <cstdio>
#include <QFile>
#include <cstdint>

#define BANK_SIZE 0x2000
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

    ROMFile::ROMFile::version_e getVersion() const;

    uint toOffset(romaddr_t addr);

    size_t       readData(romaddr_t addr, uint size, void *buffer);
    uint8_t      readByte(romaddr_t addr);
    uint16_t     readInt16(romaddr_t addr);
    uint32_t     readInt32(romaddr_t addr);
    size_t       readFromPointer(romaddr_t addrL, romaddr_t addrH, romaddr_t addrB,
                                 uint size, void *buffer, uint offset = 0);
    size_t       readFromShortPointer(romaddr_t addrL, romaddr_t addrH, uint bank,
                                      uint size, void *buffer, uint offset = 0);
    uint writeData(romaddr_t addr, uint size, void *buffer);
    uint writeByte(romaddr_t addr, uint8_t data);
    uint writeInt16(romaddr_t addr, uint16_t data);
    uint writeInt32(romaddr_t addr, uint32_t data);
    uint writeToPointer(romaddr_t ptr, romaddr_t addr, uint size, void *buffer);

private:
    ROMFile::version_e version;
};

#endif // FILE_H
