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
    version(kirby_jp)
{}

ROMFile::version_e ROMFile::getVersion() const {
    return version;
}

/*
  Converts a LoROM address to a file offset.

  Returns the corresponding file offset if successful, otherwise
  returns -1.
*/
uint ROMFile::toOffset(romaddr_t address) {
    uint offset = (address.addr % BANK_SIZE) + (address.bank * BANK_SIZE)
                  + HEADER_SIZE;

    return offset;
}

/*
  Opens the file and also verifies that it is one of the ROMS supported
  by the editor; displays a dialog and returns false on failure.

  TODO: actually implement version checking if need be
*/

bool ROMFile::openROM(OpenMode flags) {
    if (!this->open(flags))
        return false;

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
    //romaddr_t addr = {this->readByte(addrB),
    romaddr_t addr = readPointer(addrL, addrH, addrB, offset);
    return this->readData(addr, size, buffer);
}
size_t ROMFile::readFromShortPointer(romaddr_t addrL, romaddr_t addrH, uint bank,
                                     uint size, void *buffer, uint offset) {
    romaddr_t addr = readShortPointer(addrL, addrH, bank, offset);
    return this->readData(addr, size, buffer);
}

/*
  Writes data to an ROM address in a file.
  Since this only deals with MMC3 ROMs, offsets will be moved up
  to 8kb boundaries when needed.

  Returns the next available address to write data to.
*/
uint ROMFile::writeData(romaddr_t addr, uint size, void *buffer) {
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
  Writes data to an offset in a file, and then writes the 24-bit SNES pointer
  to that data into a second offset.

  TODO: rework pointers
*/
uint ROMFile::writeToPointer(romaddr_t pointer, romaddr_t addr,
                            uint size, void *buffer) {
    return 0;
}
