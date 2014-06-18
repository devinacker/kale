#include <stdexcept>
#include "stuff.h"

QString hexFormat(int number, uint digits) {
    return QString::number(number, 16).rightJustified(digits, QLatin1Char('0')).toUpper();
}

QString fromStringMap(const StringMap& map, uint type) {
    if (map.count(type))
        return map.at(type);

    return QString("%1: unknown").arg(hexFormat(type, 2));
}

const StringMap tileTypes ({
    {0x00, "00: background"},
    {0x01, "01: push up"},
    {0x02, "02: push up more"},
    {0x03, "03: push up even more"},
    {0x04, "04: push to right"},
    {0x05, "05: push to right more"},
    {0x06, "06: push to right even more"},
    {0x07, "07: push down"},
    {0x08, "08: push down more"},
    {0x09, "09: push down even more"},
    {0x0a, "0A: push to left"},
    {0x0b, "0B: push to left more"},
    {0x0c, "0C: push to left even more"},
    {0x0d, "0D: solid"},
    {0x0e, "0E: platform, press down to drop"},
    {0x0f, "0F: platform"},
    {0x10, "10: floor slope up"},
    {0x11, "11: floor slope down"},
    {0x12, "12: floor halfslope up, bottom"},
    {0x13, "13: floor halfslope up, top"},
    {0x14, "14: floor halfslope down, bottom"},
    {0x15, "15: floor halfslope down, top"},
    {0x16, "16: ceiling slope down"},
    {0x17, "17: ceiling slope up"},
    {0x18, "18: ceiling halfslope down, bottom"},
    {0x19, "19: ceiling halfslope down, top"},
    {0x1a, "1A: ceiling halfslope up, bottom"},
    {0x1b, "1B: ceiling halfslope up, top"},
    {0x1c, "1C: star block"},

    {0x1d, "1D: breakable"},
    {0x1e, "1E: breakable (hard)"},
    {0x1f, "1F: breakable (hard)(?)"},
    {0x20, "20: bomb"},
    {0x21, "21: bomb chain"},

    {0x22, "22: ladder top"},
    {0x23, "23: ladder"},
    {0x24, "24: spikes"},
    {0x25, "25: hammer stake"},
    {0x28, "28: door"},

    // 30 - 5f are underwater
    {0x30, "30: water background"},
    {0x31, "31: water push up"},
    {0x32, "32: water push up more"},
    {0x33, "33: water push up even more"},
    {0x34, "34: water push to right"},
    {0x35, "35: water push to right more"},
    {0x36, "36: water push to right even more"},
    {0x37, "37: water push down"},
    {0x38, "38: water push down more"},
    {0x39, "39: water push down even more"},
    {0x3a, "3A: water push to left"},
    {0x3b, "3B: water push to left more"},
    {0x3c, "3C: water push to left even more"},
    {0x3d, "3D: water solid"},
    {0x3e, "3E: water platform, press down to drop"},
    {0x3f, "3F: water platform"},
    {0x40, "40: water floor slope up"},
    {0x41, "41: water floor slope down"},
    {0x42, "42: water floor halfslope up, bottom"},
    {0x43, "43: water floor halfslope up, top"},
    {0x44, "44: water floor halfslope down, bottom"},
    {0x45, "45: water floor halfslope down, top"},
    {0x46, "46: water ceiling slope down"},
    {0x47, "47: water ceiling slope up"},
    {0x48, "48: water ceiling halfslope down, bottom"},
    {0x49, "49: water ceiling halfslope down, top"},
    {0x4a, "4A: water ceiling halfslope up, bottom"},
    {0x4b, "4B: water ceiling halfslope up, top"},
    {0x4c, "4C: water star block"},

    {0x4d, "4D: water breakable"},
    {0x4e, "4E: water breakable (hard)"},
    {0x4f, "4F: water breakable (hard)(?)"},
    {0x50, "50: water bomb"},
    {0x51, "51: water bomb chain"},

    {0x52, "52: water ladder top"},
    {0x53, "53: water ladder"},
    {0x54, "54: water spikes"},
    {0x55, "55: water hammer stake"},
    {0x58, "58: water door"},

    // 60 - 7e = ice, etc
    {0x60, "60: ice"},
    {0x61, "61: ice platform, press down to drop"},
    {0x62, "62: ice platform(?)"},
    {0x63, "63: ice floor slope up"},
    {0x64, "64: ice floor slope down"},
    {0x65, "65: ice floor halfslope up, bottom"},
    {0x66, "66: ice floor halfslope up, top"},
    {0x67, "67: ice floor halfslope down, bottom"},
    {0x68, "68: ice floor halfslope down, top"},
    {0x69, "69: ice ceiling slope down"},
    {0x6a, "6A: ice ceiling slope up"},
    {0x6b, "6B: ice ceiling halfslope down, bottom"},
    {0x6c, "6C: ice ceiling halfslope down, top"},
    {0x6d, "6D: ice ceiling halfslope up, bottom"},
    {0x6e, "6E: ice ceiling halfslope up, top"},
    {0x6f, "6F: ice star block"},
    {0x70, "70: ice breakable"},
    {0x71, "71: ice breakable (hard)"},
    {0x72, "72: ice breakable (hard)"},
    {0x73, "73: ice bomb"},
    {0x74, "74: ice bomb chain"},
    {0x75, "75: ice ladder top"},

    {0x76, "76: breakable background"},
    {0x77, "77: background bomb chain"},

    {0x79, "79: fuse upper left"},
    {0x7a, "7A: fuse horizontal"},
    {0x7b, "7B: fuse upper right"},
    {0x7c, "7C: fuse vertical"},
    {0x7d, "7D: fuse lower left"},
    {0x7e, "7E: fuse lower right"},

    {0xFF, "FF: background"},
});

const StringMap exitTypes ({
    {0x00, "00: normal"},
    {0x01, "01: stage 1"},
    {0x02, "02: stage 2"},
    {0x03, "03: stage 3"},
    {0x04, "04: stage 4"},
    {0x05, "05: stage 5"},
    {0x06, "06: stage 6"},
    {0x07, "07: stage 7"},
    {0x10, "10: end of stage"},
    {0x18, "18: museum"},
    {0x19, "19: arena"},
    {0x1a, "1A: quick draw (easy)"},
    {0x1b, "1B: egg catch (easy)"},
    {0x1c, "1C: crane fever (easy)"},
    {0x1d, "1D: warp star"},
    {0x1e, "1E: previous level"},
    {0x1f, "1F: next level"},
    {0x2a, "2A: quick draw (medium)"},
    {0x2b, "2B: egg catch (medium)"},
    {0x2c, "2C: crane fever (medium)"},
    {0x3a, "3A: quick draw (hard)"},
    {0x3b, "3B: egg catch (hard)"},
    {0x3c, "3C: crane fever (hard)"}
});

const StringMap spriteTypes ({
     {0x00, "00: Waddle Dee (slow)"},
     {0x01, "01: Waddle Dee (medium)"},
     {0x02, "02: Waddle Dee (fast)"},
     {0x03, "03: Waddle Dee (very fast)"},

     {0x04, "04: Waddle Doo (slow)"},
     {0x05, "05: Waddle Doo (medium)"},
     {0x06, "06: Waddle Doo (fast)"},
     {0x07, "07: Waddle Doo (very fast)"},

     {0x08, "08: Shotzo (slow)"},
     {0x09, "09: Shotzo (medium)"},
     {0x0a, "0A: Shotzo (fast)"},
     {0x0b, "0B: Shotzo (very fast)"},
     {0x0c, "0C: Shotzo (3-shot up)"},
     {0x0d, "0D: Shotzo (3-shot up-right)"},
     {0x0e, "0E: Shotzo (3-shot up-left)"},

     {0x0f, "0F: Sparky (slow)"},
     {0x10, "10: Sparky (fast)"},

     {0x11, "11: Poppy Bros. Jr. (slow)"},
     {0x12, "12: Poppy Bros. Jr. (medium)"},
     {0x13, "13: Poppy Bros. Jr. (fast)"},

     {0x14, "14: Poppy Bros. Jr. on apple (slow)"},
     {0x15, "15: Poppy Bros. Jr. on apple (medium)"},
     {0x16, "16: Poppy Bros. Jr. on apple (fast)"},

     {0x17, "17: Poppy Bros. Jr. on tomato (slow)"},
     {0x18, "18: Poppy Bros. Jr. on tomato (medium)"},
     {0x19, "19: Poppy Bros. Jr. on tomato (fast)"},

     {0x1a, "1A: Laser Ball (slow)"},
     {0x1b, "1B: Laser Ball (medium)"},
     {0x1c, "1C: Laser Ball (fast)"},

     {0x1d, "1D: Blipper (slow homing)"},
     {0x1e, "1E: Blipper (fast homing)"},
     {0x1f, "1F: Blipper (sinking?)"},
     {0x20, "20: Blipper (horizontal)"},
     {0x21, "21: Blipper (jump)"},

     {0x22, "22: Bounder 1?"},
     {0x23, "23: Bounder 2"},

     {0x24, "24: Hothead 1?"},
     {0x25, "25: Hothead 2"},

     {0x26, "26: Parasol Waddle Doo (hold)"},
     {0x27, "27: Parasol Waddle Dee (hold)"},
     {0x28, "28: Parasol Waddle Doo (fly away)"},
     {0x29, "29: Parasol Waddle Dee (fly away)"},
     {0x2a, "2A: Parasol Shotzo (fly away)"},
     {0x2b, "2B: Parasol Waddle Doo (chase)"},
     {0x2c, "2C: Parasol Waddle Dee (chase)"},
     {0x2d, "2D: Parasol Shotzo (chase)"},

     {0x2e, "2E: Blade Knight"},

     {0x2f, "2F: Bubbles (slow)"},
     {0x30, "30: Bubbles (fast)"},

     {0x31, "31: Noddy (slow)"},
     {0x32, "32: Noddy (fast)"},

     {0x33, "33: Coner (slow)"},
     {0x34, "34: Coner (fast)"},

     {0x35, "35: Maxim Tomato"},
     {0x36, "36: 1up"},
     {0x37, "37: Invincibility Candy"},

     {0x38, "38: Waddle Dee (slow walk?)"},
     {0x39, "39: Waddle Dee (medium walk?)"},
     {0x3a, "3A: Waddle Dee (fast walk?)"},
     {0x3b, "3B: Waddle Dee (very fast walk?)"},
     {0x3c, "3C: Waddle Dee (slow jump)"},
     {0x3d, "3D: Waddle Dee (medium jump)"},
     {0x3e, "3E: Waddle Dee (fast jump)"},
     {0x3f, "3F: Waddle Dee (very fast jump)"},

     {0x40, "40: Bomber"},

     {0x41, "41: Flamer (peaceful slow)"},
     {0x42, "42: Flamer (peaceful medium)"},
     {0x43, "43: Flamer (peaceful fast)"},

     {0x44, "44: Twister 1"},
     {0x45, "45: Twister 2"},
     {0x46, "46: Twister 3"},

     {0x47, "47: Flamer (slow attack)"},
     {0x48, "48: Flamer (medium attack)"},

     {0x49, "49: Meta Knight throwing candy"},

     {0x4a, "4A: Squishy (walking slow)"},
     {0x4b, "4B: Squishy (walking fast)"},
     {0x4c, "4C: Squishy (appearing suddenly)"},
     {0x4d, "4D: Squishy (floating slow)"},
     {0x4e, "4E: Squishy (floating fast)"},

     {0x4f, "4F: Nut bomb (fast left?)"},

     {0x50, "50: Bronto Burt (slow wave)"},
     {0x51, "51: Bronto Burt (fast wave)"},
     {0x52, "52: Bronto Burt (slow homing)"},
     {0x53, "53: Bronto Burt (fast homing)"},
     {0x54, "54: Bronto Burt (slow drop in)"},
     {0x55, "55: Bronto Burt (fast drop in)"},
     {0x56, "56: Bronto Burt (slow diagonal)"},
     {0x57, "57: Bronto Burt (fast diagonal)"},
     {0x58, "58: Bronto Burt (slow chasing)"},
     {0x59, "59: Bronto Burt (fast chasing)"},
     {0x5a, "5A: Bronto Burt (jump up)"},
     {0x5b, "5B: Bronto Burt (fast jump up?)"},

     {0x5c, "5C: Glunk (not shooting)"},
     {0x5d, "5D: Glunk (shooting)"},

     {0x5e, "5E: Nut bomb (slow left?)"},
     {0x5f, "5F: Nut bomb (slow right?)"},

     {0x60, "60: Slippy (slow)"},
     {0x61, "61: Slippy (fast)"},

     {0x62, "62: Starman (slow walk)"},
     {0x63, "63: Starman (fast walk)"},
     {0x64, "64: Starman (peaceful)"},
     {0x65, "65: Starman (slow fly)"},
     {0x66, "66: Starman (fast fly)"},

     {0x67, "67: Sir Kibble (stationary?)"},
     {0x68, "68: Sir Kibble (stationary, dodges?)"},
     {0x69, "69: Sir Kibble (slow walk?)"},
     {0x6a, "6A: Sir Kibble (fast walk?)"},

     {0x6b, "6B: Kabu (slow jump)"},
     {0x6c, "6C: Kabu (fast jump)"},
     {0x6d, "6D: Kabu (slow disappear)"},
     {0x6e, "6E: Kabu (fast disappear)"},
     {0x6f, "6F: Kabu (slide)"},

     {0x70, "70: Gordo (stationary)"},
     {0x71, "71: Gordo (vertical 1)"},
     {0x72, "72: Gordo (vertical 2)"},
     {0x73, "73: Gordo (horizontal 1)"},
     {0x74, "74: Gordo (horizontal 2)"},
     {0x75, "75: Gordo (vertical float)"},

     {0x76, "76: Scarfy (slow hover)"},
     {0x77, "77: Scarfy (fast hover)"},
     {0x78, "78: Scarfy (slow drop in left)"},
     {0x79, "79: Scarfy (fast drop in left)"},
     {0x7a, "7A: Scarfy (slow drop in right)"},
     {0x7b, "7B: Scarfy (fast drop in right)"},
     {0x7c, "7C: Scarfy (slow rise in left)"},
     {0x7d, "7D: Scarfy (fast rise in left)"},
     {0x7e, "7E: Scarfy (slow rise in right)"},
     {0x7f, "7F: Scarfy (fast rise in right)"},

     {0x80, "80: Meta Knight battle (2-3, room 0B4)"},
     {0x81, "81: Meta Knight battle (unused, room 13F)"},
     {0x82, "82: Meta Knight battle (6-6, room 06A)"},
     {0x83, "83: Meta Knight battle (3-4, room 0DB)"},
     {0x84, "84: Meta Knight battle (4-5, room 108)"},
     {0x85, "85: Meta Knight battle (5-4, room 124)"},
     {0x86, "86: Meta Knight battle (unused, room 13E?)"},
     {0x87, "87: Meta Knight battle (unused, room 141 or 144?)"},

     {0x88, "88: Wheelie (slow)"},
     {0x89, "89: Wheelie (fast)"},
     {0x8a, "8A: Wheelie (faster?)"},

     {0x8b, "8B: Meta Knight battle (Shotzo)"},
     {0x8c, "8C: Meta Knight battle (dummy)"},
     {0x8d, "8D: Meta Knight battle (dummy)"},
     {0x8e, "8E: Meta Knight battle (dummy)"},

     // 8F: unused
     {0x90, "90: Rocky"},

     {0x91, "91: Pep Drink"},
     {0x92, "92: UFO"},
     {0x93, "93: Cool Spook"},
     {0x94, "94: Pengy (slow)"},
     {0x95, "95: Pengy (fast)"},
     {0x96, "96: Broom Hatter"},
     {0x97, "97: Chilly (slow)"},
     {0x98, "98: Chilly (fast)"},
     {0x99, "99: Cappy"},
     {0x9a, "9A: Spiny (slow)"},
     {0x9b, "9B: Spiny (fast)"},

     {0x9c, "9C: Twizzy (slow straight)"},
     {0x9d, "9D: Twizzy (fast straight)"},
     {0x9e, "9E: Twizzy (slow up/down homing)"},
     {0x9f, "9F: Twizzy (fast up/down homing)"},
     {0xa0, "A0: Twizzy (slow drop in)"},
     {0xa1, "A1: Twizzy (fast drop in)"},
     {0xa2, "A2: Twizzy (slow fly diagonal)"},
     {0xa3, "A3: Twizzy (fast fly diagonal)"},
     {0xa4, "A4: Twizzy (slow homing)"},
     {0xa5, "A5: Twizzy (fast homing)"},
     {0xa6, "A6: Twizzy (waiting 1?)"},
     {0xa7, "A7: Twizzy (waiting 2?)"},
     {0xa8, "A8: Twizzy (hopping)"},
     {0xa9, "A9: Twizzy (slow jump up)"},
     {0xaa, "AA: Twizzy (fast jump up)"},
     {0xab, "AB: Twizzy (hovering)"},

     {0xac, "AC: Nut bomb (fast right)"},

     // AD-AF unused
     {0xb0, "B0: Mr. Frosty (slow)"},
     {0xb1, "B1: Mr. Frosty (fast)"},
     {0xb2, "B2: Bonkers (slow)"},
     {0xb3, "B3: Bonkers (fast)"},
     {0xb4, "B4: Grand Wheelie (slow)"},
     {0xb5, "B5: Grand Wheelie (fast)"},
     {0xb6, "B6: Buggzy (slow)"},
     {0xb7, "B7: Buggzy (fast)"},
     {0xb8, "B8: Rolling Turtle (slow)"},
     {0xb9, "B9: Rolling Turtle (fast)"},
     {0xba, "BA: Mr. Tick Tock (slow)"},
     {0xbb, "BB: Mr. Tick Tock (fast)"},
     {0xbc, "BC: Poppy Bros. Sr. (slow)"},
     {0xbd, "BD: Poppy Bros. Sr. (fast)"},
     {0xbe, "BE: Fire Lion (slow)"},
     {0xbf, "BF: Fire Lion (fast)"},

     // C0-CF unused
     {0xd0, "D0: Whispy Woods"},
     // D1 unused
     {0xd2, "D2: Paint Roller"},
     {0xd3, "D3: Mr. Shine & Mr. Bright"},
     {0xd4, "D4: Heavy Mole"},
     {0xd5, "D5: Kracko"},
     // D6 unused
     {0xd7, "D7: Meta Knight"},
     {0xd8, "D8: King Dedede"},
     {0xd9, "D9: Nightmare Orb"},
     {0xda, "DA: Nightmare"},

     // DB-DF unused
     {0xe0, "E0: Museum Sparky"},
     {0xe1, "E1: Museum Laser Ball"},
     {0xe2, "E2: Museum Hothead"},
     {0xe3, "E3: Museum Flamer"},
     {0xe4, "E4: Museum Blade Knight"},
     {0xe5, "E5: Museum Bubbles"},
     {0xe6, "E6: Museum Noddy"},
     {0xe7, "E7: Museum Starman"},
     {0xe8, "E8: Museum Sir Kibble"},
     {0xe9, "E9: Museum Twister"},
     {0xea, "EA: Museum Wheelie"},
     {0xeb, "EB: Museum Hammer(?)"},
     {0xec, "EC: Museum Rocky"},
     {0xed, "ED: Museum Spiny"},
     {0xee, "EE: Museum Pengi"},
     {0xef, "EF: Museum Chilly"},

     {0xf0, "F0: Warp Star (level 1)"},
     {0xf1, "F1: Warp Star (level 2)"},
     {0xf2, "F2: Warp Star (level 3)"},
     {0xf3, "F3: Warp Star (level 4)"},
     {0xf4, "F4: Warp Star (level 5)"},
     {0xf5, "F5: Warp Star (level 6)"},
     {0xf6, "F6: Warp Star (level 7)"},
     {0xf7, "F7: Warp Star"},
     {0xf8, "F8: Warp Star 2"},
     {0xf9, "F9: Cannon"},
     {0xfa, "FA: Cannon 2"},
     {0xfb, "FB: Fuse"},
     // FC/FD unused
     {0xfe, "FE: Floor switch"},
     {0xff, "FF: Ceiling switch"}
});

const StringMap musicNames ({
    {0x00, "00: Vegetable Valley"},
    {0x01, "01: Vegetable Valley (no intro)"},
    {0x02, "02: Invincible"},
    {0x03, "03: Green Greens"},
    {0x04, "04: Green Greens (no intro)"},
    {0x05, "05: End of Level"},
    {0x06, "06: Rainbow Resort (overworld)"},
    {0x07, "07: Kirby Dance"},
    {0x08, "08: Kirby Dance (short)"},
    {0x09, "09: Crane Game"},
    {0x0A, "0A: Crane Game (Fast)"},
    {0x0B, "0B: Crane Game (End)"},
    {0x0C, "0C: Boss Theme"},
    {0x0D, "0D: Final Boss Intro?"},
    {0x0E, "0E: Final Boss?"},
    {0x0F, "0F: Sky Stages"},
    {0x10, "10: Kirby's Dream Land"},
    {0x11, "11: Forest Stages"},
    {0x12, "12: Forest Stages (no intro)"},
    {0x13, "13: Arena / Egg Catch"},
    {0x14, "14: Arena / Egg Catch (no intro)"},
    {0x15, "15: Orange Ocean (overworld)"},
    {0x17, "17: Silence"},
    {0x1B, "1B: Rainbow Resort"},
    {0x1C, "1C: Rainbow Resort (no intro)"},
    {0x1E, "1E: Yogurt Yard"},
    {0x1F, "1F: Yogurt Yard (no intro)"},
    {0x20, "20: Grape Garden"},
    {0x21, "21: Grape Garden (no intro)"},
    {0x22, "22: Vegetable Valley (overworld)"},
    {0x23, "23: Grape Garden (overworld)"},
    {0x24, "24: Yogurt Yard (overworld)"},
    {0x28, "28: Ice Cream Island"},
    {0x29, "29: Ice Cream Island (no intro)"},
    {0x2A, "2A: Orange Ocean"},
    {0x2B, "2B: Orange Ocean (no intro)"},
    {0x2C, "2C: Ice Cream Island (overworld)"},
    {0x2D, "2D: Butter Building (overworld)"},
    {0x32, "32: Butter Building"},
    {0x33, "33: Butter Building (no intro)"},
    {0x35, "35: Title Theme"},
    {0x36, "36: Museum / Warp Star"},
    {0x37, "37: Ending"},
    {0x38, "38: Fountain of Dreams"},
    {0x39, "39: Nightmare"}
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
