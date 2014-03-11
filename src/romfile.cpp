/*
  romfile.cpp
  Contains functions for loading and saving data to/from a ROM file.

  Automatically detects valid game ROMs and allows reading/writing by CPU addresses, automatically
  adjusting for a 512-byte copier header if necessary.

  This code is released under the terms of the MIT license.
  See COPYING.txt for details.
*/

#include <QFile>
#include <QMessageBox>
#include <QSettings>

#include <cstring>
#include <cstdio>

#include "romfile.h"
#include "compress.h"

ROMFile::ROMFile() : QFile(),
    version(kirby_jp),
    numPRGBanks(0),
    numCHRBanks(0)
{}

ROMFile::version_e ROMFile::getVersion() const {
    return version;
}

/*
  Converts a LoROM address to a file offset.

  Returns the corresponding file offset if successful, otherwise
  returns -1.
*/
uint ROMFile::toOffset(romaddr_t address) const {
    uint offset = (address.addr % BANK_SIZE) + ((address.bank & 0x7F) * BANK_SIZE)
                  + HEADER_SIZE;

    return offset;
}

uint ROMFile::getNumPRGBanks() const {
    return numPRGBanks;
}
uint ROMFile::getNumCHRBanks() const {
    return numCHRBanks;
}

/*
  Opens the file and also verifies that it is one of the ROMS supported
  by the editor; displays a dialog and returns false on failure.

  TODO: actually implement version checking if need be
*/

bool ROMFile::openROM(OpenMode flags) {
    if (!this->open(flags))
        return false;

    numPRGBanks = 0;
    numCHRBanks = 0;

    // get size of PRG/CHR banks
    this->seek(4);
    this->read((char*)&numPRGBanks, 1);
    // iNES uses 16kb PRG banks, MMC3 uses 8kb
    numPRGBanks *= 2;
    this->seek(5);
    this->read((char*)&numCHRBanks, 1);
    // iNES uses 8kb CHR banks, we use 1kb
    numCHRBanks *= 8;

    return true;
}

/*
  Reads data from a file into a pre-existing char buffer.
  If "size" == 0, the data is decompressed, with a maximum decompressed
  size of 65,536 bytes (64 kb).

  Returns the size of the data read from the file, or 0 if the read was
  unsuccessful.
*/
size_t ROMFile::readData(romaddr_t addr, uint size, void *buffer) {
    if (!size) {
        char packed[DATA_SIZE];
        this->seek(toOffset(addr));
        this->read(packed, DATA_SIZE);
        return unpack((uint8_t*)packed, (uint8_t*)buffer);
    } else {
        this->seek(toOffset(addr));
        return read((char*)buffer, size);
    }
}

uint8_t ROMFile::readByte(romaddr_t addr) {
    uint8_t data;
    readData(addr, 1, &data);
    return data;
}

uint16_t ROMFile::readInt16(romaddr_t addr) {
    uint16_t data;
    readData(addr, 2, &data);
    return data;
}

uint32_t ROMFile::readInt32(romaddr_t addr) {
    uint32_t data;
    readData(addr, 2, &data);
    return data;
}

/*
  Reads a 24-bit ROM pointer from a file, then dereferences the pointer and
  reads from the address pointed to. If "size" == 0, the data is decompressed.

  Returns the size of data read, or 0 if unsuccessful.

  readFromShortPointer takes a constant bank number instead of bank pointer.
*/
romaddr_t ROMFile::readPointer(romaddr_t addrL, romaddr_t addrH, romaddr_t addrB,
                               uint offset) {
    return {this->readByte(addrB + offset) & 0x7Fu,
            this->readByte(addrH + offset)*256u + this->readByte(addrL + offset)};
}
romaddr_t ROMFile::readShortPointer(romaddr_t addrL, romaddr_t addrH, uint bank,
                               uint offset) {
    return {bank,
            this->readByte(addrH + offset)*256u + this->readByte(addrL + offset)};
}

size_t ROMFile::readFromPointer(romaddr_t addrL, romaddr_t addrH, romaddr_t addrB,
                                uint size, void *buffer, uint offset) {
    memset(buffer, 0, 0x10000);
    romaddr_t addr = readPointer(addrL, addrH, addrB, offset);
    if (addr.addr)
        return this->readData(addr, size, buffer);

    return 0;
}
size_t ROMFile::readFromShortPointer(romaddr_t addrL, romaddr_t addrH, uint bank,
                                     uint size, void *buffer, uint offset) {
    memset(buffer, 0, 0x10000);
    romaddr_t addr = readShortPointer(addrL, addrH, bank, offset);
    if (addr.addr)
        return this->readData(addr, size, buffer);

    return 0;
}

/*
  Writes data to an ROM address in a file.
  Since this only deals with MMC3 ROMs, offsets will be moved up
  to 8kb boundaries when needed.

  Returns the next available address to write data to.
*/
uint ROMFile::writeData(romaddr_t addr, uint size, const void *buffer) {
    uint offset = toOffset(addr);
    uint spaceLeft = BANK_SIZE - (addr.addr % BANK_SIZE);

    // move offset forward if there's not enough space left in the bank
    if (size > spaceLeft)
        offset += spaceLeft;

    // now write data to file
    this->seek(offset);
    this->write((const char*)buffer, size);

    // return new ROM address
    // (TODO: redo this)
    //return toAddress(pos());
    return 0;
}

uint ROMFile::writeByte(romaddr_t addr, uint8_t data) {
    return writeData(addr, 1, &data);
}

uint ROMFile::writeInt16(romaddr_t addr, uint16_t data) {
    return writeData(addr, 2, &data);
}

uint ROMFile::writeInt32(romaddr_t addr, uint32_t data) {
    return writeData(addr, 4, &data);
}

/*
  Writes data to an offset in a file, and then writes the 24-bit pointer
  to that data into a second offset.

  TODO: rework pointers
*/
uint ROMFile::writeToPointer(romaddr_t ptrL, romaddr_t ptrH, romaddr_t ptrB, romaddr_t addr,
                            uint size, const void *buffer, uint offset) {
    writeByte(ptrL + offset, addr.addr & 0xFF);
    writeByte(ptrH + offset, addr.addr >> 8);
    writeByte(ptrB + offset, addr.bank);

    return writeData(addr, size, buffer);
}

uint ROMFile::writeToShortPointer(romaddr_t ptrL, romaddr_t ptrH, romaddr_t addr,
                            uint size, const void *buffer, uint offset) {
    writeByte(ptrL + offset, addr.addr & 0xFF);
    writeByte(ptrH + offset, addr.addr >> 8);

    return writeData(addr, size, buffer);
}

/*
 * Read a 1KB CHR ROM bank and return it as an 8-bit QImage with default grey palettes.
 * The QImage will be 512x32, or 64x4 NES tiles.
 * Each row of tiles (8 pixels) represents the same tiles with one of the 4 possible palettes.
 * The palettes use color indices 1-3, 4-6, 7-9, and 10-12, and index 0 is BG color.
 */
QImage ROMFile::readCHRBank(uint bank) {
    uchar  chr[CHR_SIZE];
    QImage tiles(512, 32, QImage::Format_Indexed8);

    this->seek(16 + (numPRGBanks * BANK_SIZE) + (bank * CHR_SIZE));
    this->read((char*)chr, CHR_SIZE);

    for (uint line = 0; line < 8; line++) {
        uchar* lines[] = {
            tiles.scanLine(line),
            tiles.scanLine(line + 8),
            tiles.scanLine(line + 16),
            tiles.scanLine(line + 24)
        };
        for (uint tile = 0; tile < 64; tile++) {
            uchar plane0 = chr[tile*16 + line];
            uchar plane1 = chr[tile*16 + line + 8];
            for (uint col = 0; col < 8; col++) {
                uint idx = tile * 8 + col;
                //uchar color = ((plane0 >> col) & 1) + ((plane1 >> col) & 1) * 2;
                uchar color = ((plane0 >> (7 - col)) & 1) + ((plane1 >> (7 - col)) & 1) * 2;
                if (color) {
                    lines[0][idx] = color;
                    lines[1][idx] = color + 3;
                    lines[2][idx] = color + 6;
                    lines[3][idx] = color + 9;
                } else {
                    lines[0][idx] = 0;
                    lines[1][idx] = 0;
                    lines[2][idx] = 0;
                    lines[3][idx] = 0;
                }
            }
        }
    }

    // set default palette
    tiles.setColor(0,  0xFFFFFFFF);
    tiles.setColor(1,  0xFFBFBFBF);
    tiles.setColor(4,  0xFFBFBFBF);
    tiles.setColor(7,  0xFFBFBFBF);
    tiles.setColor(10, 0xFFBFBFBF);
    tiles.setColor(2,  0xFF7F7F7F);
    tiles.setColor(5,  0xFF7F7F7F);
    tiles.setColor(8,  0xFF7F7F7F);
    tiles.setColor(11, 0xFF7F7F7F);
    tiles.setColor(3,  0xFF3F3F3F);
    tiles.setColor(6,  0xFF3F3F3F);
    tiles.setColor(9,  0xFF3F3F3F);
    tiles.setColor(12, 0xFF3F3F3F);

    return tiles;
}
