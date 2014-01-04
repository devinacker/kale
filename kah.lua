-- Kirby's Adventure hacking/debug/testing script
-- by Revenant

-- warning: bad code ahead
-- also don't blame me if you break the game with this :)

emu.speedmode("normal")

emu.message("script ver. 1.1\nclick/shift-click on page no. to change")

function writeword (addr, value)
	memory.writebyte(addr, value % 256)
	memory.writebyte(addr+1, math.floor(value/256))
end

function drawcursor(x, y, color)
	gui.box(x, y, x+16, y+16, "clear", color)
	gui.box(x+1, y+1, x+15, y+15, "clear", color)
end

cboxcolor   = "P25"
page        = 0
mouserepeat = 0
mouseheld   = false
repeatrate  = 16
exit_from   = 0
exits       = {}
numexits    = 0

addr_tileact    = 0x7f00
addr_leveldata  = 0x67ee
addr_hscreens   = addr_leveldata+0
addr_vscreens   = addr_leveldata+1
addr_screenlist = addr_leveldata+8
addr_tilemap    = addr_leveldata+0xda

addr_level    = 0x0562
addr_exitflag = 0x055F

addr_startscreen = 0x0564
addr_startpoint  = 0x0565

addr_playerxh = 0x0095
addr_playeryh = 0x00CB
addr_playerxl = 0x0083
addr_playeryl = 0x00B9

addr_scrollx = 0x00EB
addr_scrolly = 0x00EA

addr_mapptrl = 0x88A6
addr_mapptrh = 0x875F
addr_mapptrb = 0x84D1

addr_sprptrl = 0x8D0E
addr_sprptrh = 0x8BC7
addr_sprptrb = 0x8A80

addr_exitptrl = 0x8F82
addr_exitptrh = 0x90CB

behavior = {}
-- store tile behavior so it can be recalled with ctrl-click
memory.registerwrite(addr_tileact, 256, 
	function (addr, size)
		behavior[addr - addr_tileact] = memory.readbyte(addr) 
	end
)
savestate.registerload(
	function ()
		for i = 0, 255, 1 do
			behavior[i] = memory.readbyte(addr_tileact + i)
		end
	end
)

-- dump addresses /file offsets on level load
memory.registerexec(0xe648, 1, 
	function()
		print(string.format("loading level %03X", level))
		local bank = memory.readbyte(addr_mapptrb + level)
		local ptr  = memory.readbyte(addr_mapptrl + level)
		           + memory.readbyte(addr_mapptrh + level) * 256
		
		local offset = (bank * 8192) + ptr - 0xa000 + 16
		
		print(string.format("tilemap -> %02X:%04X (offset %06X)", bank, ptr, offset))

	end
)

-- dump sprite table address on level load
memory.registerexec(0xa646, 1, 
	function()
		if (memory.readbyte(0xa646) ~= 0xac) then
			return
		end
	
		local bank = memory.readbyte(addr_sprptrb + level)
		sprptr     = memory.readbyte(addr_sprptrl + level)
		           + memory.readbyte(addr_sprptrh + level) * 256
		
		local offset = (bank * 8192) + sprptr - 0xa000 + 16
		
		print(string.format("sprites -> %02X:%04X (offset %06X)", bank, sprptr, offset))

	end
)
memory.registerexec(0xe68a, 1, 
	function()
		local screens = memory.readbyte(sprptr)
		local numspr = memory.readbyte(sprptr+screens+1)
		
		-- TODO finish this, this is just from the unused page
		local sprites = {
		[0x00] = "Waddle Dee (slow)",
		[0x01] = "Waddle Dee (medium)",
		[0x02] = "Waddle Dee (fast)",
		[0x03] = "Waddle Dee (very fast)",
		
		[0x04] = "Waddle Doo (slow)",
		[0x05] = "Waddle Doo (medium)",
		[0x06] = "Waddle Doo (fast)",
		[0x07] = "Waddle Doo (very fast)",
		
		[0x08] = "Shotzo (slow)",
		[0x09] = "Shotzo (medium)",
		[0x0a] = "Shotzo (fast)",
		[0x0b] = "Shotzo (very fast)",
		[0x0c] = "Shotzo (3-shot up)",
		[0x0d] = "Shotzo (3-shot up-right)",
		[0x0e] = "Shotzo (3-shot up-left)",
		
		[0x0f] = "Sparky (slow)",
		[0x10] = "Sparky (fast)",

		[0x11] = "Poppy Bros. Jr. (slow)",
		[0x12] = "Poppy Bros. Jr. (medium)",
		[0x13] = "Poppy Bros. Jr. (fast)",
		
		[0x14] = "Poppy Bros. Jr. on apple (slow)",
		[0x15] = "Poppy Bros. Jr. on apple (medium)",
		[0x16] = "Poppy Bros. Jr. on apple (fast)",
		
		[0x17] = "Poppy Bros. Jr. on tomato (slow)",
		[0x18] = "Poppy Bros. Jr. on tomato (medium)",
		[0x19] = "Poppy Bros. Jr. on tomato (fast)",
		
		[0x1a] = "Laser Ball (slow)",
		[0x1b] = "Laser Ball (medium)",
		[0x1c] = "Laser Ball (fast)",
		
		[0x1d] = "Blipper (slow homing)",
		[0x1e] = "Blipper (fast homing)",
		[0x1f] = "Blipper (sinking?)",
		[0x20] = "Blipper (horizontal)",
		[0x21] = "Blipper (jump)",
		
		[0x22] = "Bounder 1?",
		[0x23] = "Bounder 2",
		
		[0x24] = "Hothead 1?",
		[0x25] = "Hothead 2",
		
		[0x26] = "Parasol Waddle Doo (hold)",
		[0x27] = "Parasol Waddle Dee (hold)",
		[0x28] = "Parasol Waddle Doo (fly away)",
		[0x29] = "Parasol Waddle Dee (fly away)",
		[0x2a] = "Parasol Shotzo (fly away)",
		[0x2b] = "Parasol Waddle Doo (chase)",
		[0x2c] = "Parasol Waddle Dee (chase)",
		[0x2d] = "Parasol Shotzo (chase)",
		
		[0x2e] = "Blade Knight",
		
		[0x2f] = "Bubbles (slow)",
		[0x30] = "Bubbles (fast)",
		
		[0x31] = "Noddy (slow)",
		[0x32] = "Noddy (fast)",
		
		[0x33] = "Coner (slow)",
		[0x34] = "Coner (fast)",
		
		[0x35] = "Maxim Tomato",
		[0x36] = "1up",
		[0x37] = "Invincibility Candy",
		
		[0x38] = "Waddle Dee (slow walk?)",
		[0x39] = "Waddle Dee (medium walk?)",
		[0x3a] = "Waddle Dee (fast walk?)",
		[0x3b] = "Waddle Dee (very fast walk?)",
		[0x3c] = "Waddle Dee (slow jump)",
		[0x3d] = "Waddle Dee (medium jump)",
		[0x3e] = "Waddle Dee (fast jump)",
		[0x3f] = "Waddle Dee (very fast jump)",
		
		[0x40] = "Bomber",
		
		[0x41] = "Flamer (peaceful slow)",
		[0x42] = "Flamer (peaceful medium)",
		[0x43] = "Flamer (peaceful fast)",
		
		[0x44] = "Twister 1",
		[0x45] = "Twister 2",
		[0x46] = "Twister 3",
		
		[0x47] = "Flamer (slow attack)", 
		[0x48] = "Flamer (medium attack)",
		
		[0x49] = "Meta Knight throwing candy",
		
		[0x4a] = "Squishy (walking slow)",
		[0x4b] = "Squishy (walking fast)",
		[0x4c] = "Squishy (appearing suddenly)",
		[0x4d] = "Squishy (floating slow)",
		[0x4e] = "Squishy (floating fast)",
		
		[0x4f] = "Nut bomb (fast left?)",
		
		[0x50] = "Bronto Burt (slow wave)",
		[0x51] = "Bronto Burt (fast wave)",
		[0x52] = "Bronto Burt (slow homing)",
		[0x53] = "Bronto Burt (fast homing)",
		[0x54] = "Bronto Burt (slow drop in)",
		[0x55] = "Bronto Burt (fast drop in)",
		[0x56] = "Bronto Burt (slow diagonal)",
		[0x57] = "Bronto Burt (fast diagonal)",
		[0x58] = "Bronto Burt (slow chasing)",
		[0x59] = "Bronto Burt (fast chasing)",
		[0x5a] = "Bronto Burt (jump up)",
		[0x5b] = "Bronto Burt (fast jump up?)",
		
		[0x5c] = "Glunk (not shooting)",
		[0x5d] = "Glunk (shooting)",
		
		[0x5e] = "Nut bomb (slow left?)",
		[0x5f] = "Nut bomb (slow right?)",
		
		[0x60] = "Slippy (slow)",
		[0x61] = "Slippy (fast)",
		
		[0x62] = "Starman (slow walk)",
		[0x63] = "Starman (fast walk)",
		[0x64] = "Starman (peaceful)",
		[0x65] = "Starman (slow fly)",
		[0x66] = "Starman (fast fly)",
		
		[0x67] = "Sir Kibble (stationary?)",
		[0x68] = "Sir Kibble (stationary, dodges?)",
		[0x69] = "Sir Kibble (slow walk?)",
		[0x6a] = "Sir Kibble (fast walk?)",
		
		[0x6b] = "Kabu (slow jump)",
		[0x6c] = "Kabu (fast jump)",
		[0x6d] = "Kabu (slow disappear)",
		[0x6e] = "Kabu (fast disappear)",
		[0x6f] = "Kabu (slide)",
		
		[0x70] = "Gordo (stationary)",
		[0x71] = "Gordo (vertical 1)",
		[0x72] = "Gordo (vertical 2)",
		[0x73] = "Gordo (horizontal 1)",
		[0x74] = "Gordo (horizontal 2)",
		[0x75] = "Gordo (vertical float)",
		
		[0x76] = "Scarfy (slow hover)",
		[0x77] = "Scarfy (fast hover)",
		[0x78] = "Scarfy (slow drop in left)",
		[0x79] = "Scarfy (fast drop in left)",
		[0x7a] = "Scarfy (slow drop in right)",
		[0x7b] = "Scarfy (fast drop in right)",
		[0x7c] = "Scarfy (slow rise in left)",
		[0x7d] = "Scarfy (fast rise in left)",
		[0x7e] = "Scarfy (slow rise in right)",
		[0x7f] = "Scarfy (fast rise in right)",
		
		[0x80] = "Meta Knight battle (2-3, room 0B4)",
		[0x81] = "Meta Knight battle (unused, room 13F)",
		[0x82] = "Meta Knight battle (6-6, room 06A)",
		[0x83] = "Meta Knight battle (3-4, room 0DB)",
		[0x84] = "Meta Knight battle (4-5, room 108)",
		[0x85] = "Meta Knight battle (5-4, room 124)",
		[0x86] = "Meta Knight battle (unused, room 13E?)",
		[0x87] = "Meta Knight battle (unused, room 141 or 144?)",
		
		[0x88] = "Wheelie (slow)",
		[0x89] = "Wheelie (fast)",
		[0x8a] = "Wheelie (faster?)",
		
		[0x8b] = "Meta Knight battle (Shotzo)",
		[0x8c] = "Meta Knight battle (dummy)",
		[0x8d] = "Meta Knight battle (dummy)",
		[0x8e] = "Meta Knight battle (dummy)",
		
		-- 8F: unused
		[0x90] = "Rocky",
		
		[0x91] = "Pep Drink",
		[0x92] = "UFO",
		[0x93] = "Cool Spook",
		[0x94] = "Pengy (slow)",
		[0x95] = "Pengy (fast)",
		[0x96] = "Broom Hatter",
		[0x97] = "Chilly (slow)",
		[0x98] = "Chilly (fast)",
		[0x99] = "Cappy",
		[0x9a] = "Spiny (slow)",
		[0x9b] = "Spiny (fast)",
		
		[0x9c] = "Twizzy (slow straight)",
		[0x9d] = "Twizzy (fast straight)",
		[0x9e] = "Twizzy (slow up/down homing)",
		[0x9f] = "Twizzy (fast up/down homing)",
		[0xa0] = "Twizzy (slow drop in)",
		[0xa1] = "Twizzy (fast drop in)",
		[0xa2] = "Twizzy (slow fly diagonal)",
		[0xa3] = "Twizzy (fast fly diagonal)",
		[0xa4] = "Twizzy (slow homing)",
		[0xa5] = "Twizzy (fast homing)",
		[0xa6] = "Twizzy (waiting 1?)",
		[0xa7] = "Twizzy (waiting 2?)",
		[0xa8] = "Twizzy (hopping)",
		[0xa9] = "Twizzy (slow jump up)",
		[0xaa] = "Twizzy (fast jump up)",
		[0xab] = "Twizzy (hovering)",
		
		[0xac] = "Coconut (fast right)",
		
		-- AD-AF unused
		[0xb0] = "Mr. Frosty (slow)",
		[0xb1] = "Mr. Frosty (fast)",
		[0xb2] = "Bonkers (slow)",
		[0xb3] = "Bonkers (fast)",
		[0xb4] = "Grand Wheelie (slow)",
		[0xb5] = "Grand Wheelie (fast)",
		[0xb6] = "Buggzy (slow)",
		[0xb7] = "Buggzy (fast)",
		[0xb8] = "Rolling Turtle (slow)",
		[0xb9] = "Rolling Turtle (fast)",
		[0xba] = "Mr. Tick Tock (slow)",
		[0xbb] = "Mr. Tick Tock (fast)",
		[0xbc] = "Poppy Bros. Sr. (slow)",
		[0xbd] = "Poppy Bros. Sr. (fast)",
		[0xbe] = "Fire Lion (slow)",
		[0xbf] = "Fire Lion (fast)",
		
		-- C0-CF unused
		[0xd0] = "Whispy Woods",
		-- D1 unused
		[0xd2] = "Paint Roller",
		[0xd3] = "Mr. Shine & Mr. Bright",
		[0xd4] = "Heavy Mole",
		[0xd5] = "Kracko",
		-- D6 unused
		[0xd7] = "Meta Knight",
		[0xd8] = "King Dedede",
		[0xd9] = "Nightmare Orb",
		[0xda] = "Nightmare",
		
		-- DB-DF unused
		[0xe0] = "Museum Sparky",
		[0xe1] = "Museum Laser Ball",
		[0xe2] = "Museum Hothead",
		[0xe3] = "Museum Flamer",
		[0xe4] = "Museum Blade Knight",
		[0xe5] = "Museum Bubbles",
		[0xe6] = "Museum Noddy",
		[0xe7] = "Museum Starman",
		[0xe8] = "Museum Sir Kibble",
		[0xe9] = "Museum Twister",
		[0xea] = "Museum Wheelie",
		[0xeb] = "Museum Hammer(?)",
		[0xec] = "Museum Rocky",
		[0xed] = "Museum Spiny",
		[0xee] = "Museum Pengi",
		[0xef] = "Museum Chilly",
		
		[0xf0] = "Warp Star (level 1)",
		[0xf1] = "Warp Star (level 2)",
		[0xf2] = "Warp Star (level 3)",
		[0xf3] = "Warp Star (level 4)",
		[0xf4] = "Warp Star (level 5)",
		[0xf5] = "Warp Star (level 6)",
		[0xf6] = "Warp Star (level 7)",
		[0xf7] = "Warp Star",
		[0xf8] = "Warp Star 2?",
		[0xf9] = "Cannon",
		[0xfa] = "Cannon 2?",
		[0xfb] = "Fuse",
		-- FC/FD unused
		[0xfe] = "Floor switch",
		[0xff] = "Ceiling switch"
		}
		
		local firstspr = 0
		for i = 0, screens - 1, 1 do
			local lastspr = memory.readbyte(sprptr+2+i)
			for j = firstspr, lastspr - 1, 1 do
				local start = memory.readbyte(sprptr+2+screens+j)
				local type  = memory.readbyte(sprptr+2+screens+numspr+j)
				
				print(string.format("sprite #%d: screen %1X (%1X, %1X), type %02X (%s)",
				                    j, i, math.floor(start/16), start % 16, type,
				                    sprites[type] or "unknown"))
			end
			firstspr = lastspr
		end
		
	end
)

-- get level exit information
memory.registerexec(0xa0fb, 1,
	function()
		if (exit_from == level) then
			return
		end
		
		exit_from = level
		
		-- get # of exits in this level
		local ptr  = memory.readbyte(addr_exitptrl + level)
		           + memory.readbyte(addr_exitptrh + level) * 256
		local next = memory.readbyte(addr_exitptrl + level + 1)
		           + memory.readbyte(addr_exitptrh + level + 1) * 256
		numexits = (next - ptr) / 5
		
		print(string.format("exits -> ??:%04X (offset %06X)", ptr, 0x24000 + ptr - 0x8000 + 16))
		
		for i = 0, numexits-1, 1 do
			exit = {}
			
			local b = memory.readbyte(ptr + (i*5))
			exit.type = math.floor(b / 16)
			exit.screen = b % 16
			
			b = memory.readbyte(ptr + (i*5) + 1)
			exit.x = math.floor(b / 16)
			exit.y = b % 16
			
			exit.level = memory.readbyte(ptr + (i*5) + 2)
			
			b = memory.readbyte(ptr + (i*5) + 3)
			if (b >= 0x80) then
				exit.level = exit.level + 256
			end
			exit.type = exit.type + (math.floor(b / 16) % 8)*16
			exit.to_screen = b % 16
			
			b = memory.readbyte(ptr + (i*5) + 4)
			exit.to_x = math.floor(b / 16)
			exit.to_y = b % 16
			
			exits[i] = exit
			--print(string.format("exit %d: type %1X scr %02X (%1X, %1X) -> %03X scr %02X (%1X, %1X)",
			--                    i, exit.type, exit.screen, exit.x, exit.y,
			--                    exit.level, exit.to_screen, exit.to_x, exit.to_y))
		end
	end
)

pages = {
-- page 1: metatile info
[2] = function()
	topline = "tile info"
	
	local ktile = memory.readbyte(addr_tilemap + ((screen * 12) + ptiley) * 16 + ptilex)
	local ctile = memory.readbyte(addr_tilemap + ((ss * 12) + ctiley) * 16 + ctilex)

	local kact  = memory.readbyte(addr_tileact + ktile)
	local cact  = memory.readbyte(addr_tileact + ctile)
	
	local tiletypes = {
	[0x00] = "background",
	[0x01] = "push up",
	[0x02] = "push up more",
	[0x03] = "push up even more",
	[0x04] = "push to right",
	[0x05] = "push to right more",
	[0x06] = "push to right even more",
	[0x07] = "push down",
	[0x08] = "push down more",
	[0x09] = "push down even more",
	[0x0a] = "push to left",
	[0x0b] = "push to left more",
	[0x0c] = "push to left even more",
	[0x0d] = "solid",
	[0x0e] = "platform, press down to drop",
	[0x0f] = "platform",
	[0x10] = "floor slope up",
	[0x11] = "floor slope down",
	[0x12] = "floor halfslope up, bottom",
	[0x13] = "floor halfslope up, top",
	[0x14] = "floor halfslope down, bottom",
	[0x15] = "floor halfslope down, top",
	[0x16] = "ceiling slope down",
	[0x17] = "ceiling slope up",
	[0x18] = "ceiling halfslope down, bottom",
	[0x19] = "ceiling halfslope down, top",
	[0x1a] = "ceiling halfslope up, bottom",
	[0x1b] = "ceiling halfslope up, top",
	[0x1c] = "star block",
	-- becomes tile & 0xf7 when destroyed?
	[0x1d] = "breakable",
	[0x1e] = "breakable with ability(?)",
	[0x1f] = "breakable with ability(?)",
	[0x20] = "bomb",
	[0x21] = "bomb chain",
	
	[0x22] = "ladder top",
	[0x23] = "ladder",
	[0x24] = "spikes",
	[0x25] = "hammer stake",
	[0x28] = "door",
	
	-- 30 - 5f are underwater
	
	-- 60 - 7e = ??
	[0x60] = "ice",
	[0x61] = "ice, press down to drop",
	[0x62] = "ice (?)",
	[0x63] = "ice floor slope up",
	[0x64] = "ice floor slope down",
	[0x65] = "ice floor halfslope up, bottom",
	[0x66] = "ice floor halfslope up, top",
	[0x67] = "ice floor halfslope down, bottom",
	[0x68] = "ice floor halfslope down, top",
	[0x69] = "ice ceiling slope down",
	[0x6a] = "ice ceiling slope up",
	[0x6b] = "ice ceiling halfslope down, bottom",
	[0x6c] = "ice ceiling halfslope down, top",
	[0x6d] = "ice ceiling halfslope up, bottom",
	[0x6e] = "ice ceiling halfslope up, top",
	[0x6f] = "ice breakable(?)",
	[0x70] = "ice breakable with ability(?)",
	[0x71] = "ice breakable with ability(?)",
	[0x72] = "ice spikes",
	
	-- verify these ones (73-78) are used
	
	[0x79] = "fuse upper left",
	[0x7a] = "fuse horizontal",
	[0x7b] = "fuse upper right",
	[0x7c] = "fuse vertical",
	[0x7d] = "fuse lower left",
	[0x7e] = "fuse lower right",
	
	[0xFF] = "background",
	}
	
	ktype = tiletypes[kact] or "unknown"
	if (kact >= 0x30 and kact < 0x60) then
		ktype = "water, " .. (tiletypes[kact - 0x30] or "unknown")
	end
	ctype = tiletypes[cact] or "unknown"
	if (cact >= 0x30 and cact < 0x60) then
		ctype = "water, " .. (tiletypes[cact - 0x30] or "unknown")
	end
	
	textlines[0] = string.format("K: tile %02X act %02X (%s)", ktile, kact, ktype)
	-- textlines[3] = string.format("Cursor is on X:%02X Y:%02X of screen %02X", ctilex, ctiley, ss)
	if (ymouse < 176) then
		textlines[1] = string.format("C: tile %02X act %02X (%s)", ctile, cact, ctype)
		textlines[2] = "(shift-)click to change, ctrl-click to restore"
		
		drawcursor(cursorx, cursory, cboxcolor)
		-- check if mouse is clicked
		if (inpt.leftclick and mouserepeat == 0) then
			if (inpt.control) then
				if (behavior[ctile] == nil) then
					emu.message("No saved behavior!")
				else
					cact = behavior[ctile]
				end
			elseif (inpt.shift) then
				if (cact == 0) then
					cact = 0x7e
				else
					cact = cact - 1
				end
			else
				if (cact < 0x7e) then
					cact = cact + 1
				else
					cact = 0
				end
			end
			memory.writebyte(addr_tileact + ctile, cact)
		end
	end
	
end,
--
-- page 2: jump around
--
[1] = function()
	topline = "fast movement"
	
	if (ymouse < 176) then
		textlines[0] = string.format("Cursor: %02X %02X %02X", ss, ctilex, ctiley)
		textlines[1] = "(click to teleport)"
		
		drawcursor(cursorx, cursory, cboxcolor)
		if (inpt.leftclick and not mouseheld) then
			local newy = (ssy * 192) + (ctiley * 16)
			
			memory.writebyte(addr_playerxh, ssx)
			memory.writebyte(addr_playeryh, math.floor(newy / 256))
			memory.writebyte(addr_playerxl, ctilex * 16 + 8)
			memory.writebyte(addr_playeryl, newy % 256)
		end
	end
	
	gui.text(136, 176, "screen layout")
	for i = vscreens-1, 0, -1 do
		for j = 0, hscreens-1, 1 do
			local snum = memory.readbyte(addr_screenlist + (hscreens * i) + j)
			gui.text(136 + 16*j, 176+8 + 8*i, string.format("%02X", snum))
		end
	end
	
end,
--
-- page 3: level select
--
[0] = function()
	topline = "level select"
	
	textlines[0] = "Next room"
	textlines[1] = "Previous room"
	textlines[2] = "Debug room"
	
	if (inpt.leftclick and not mouseheld) then
		local option = math.floor((inpt.ymouse-176-32) / 8)
		
		if (option < 0) then
			return
		elseif (option == 0) then
			level = level + 1
		elseif (option == 1) then
			level = level - 1
		elseif (option == 2) then
			level = 0x00A8
		end
		
		writeword(addr_level, level % 0x200)
		memory.writebyte(addr_exitflag, 0xFF)
		-- burn a couple of frames so that pressing up to exit works all the time
		joy = joypad.get(1)
		joy.up = true
		joypad.set(1, joy)
		emu.frameadvance()
		joypad.set(1, joy)
		emu.frameadvance()
		joypad.set(1, joy)
		
		memory.writebyte(addr_startscreen, 0)
		-- (4, 6) - somewhere "safe" near middle of screen
		memory.writebyte(addr_startpoint, 0x46)
	end
	
end,
-- 
-- page 4: door info
--
[3] = function()
	topline = "door info"
	
	textlines[0] = "mouse over an exit for info"
	if (ymouse >= 176) then
		return
	end
	
	local exittypes = {
	[0x0] = "normal",
	[0x1] = "level 1",
	[0x2] = "level 2",
	[0x3] = "level 3",
	[0x4] = "level 4",
	[0x5] = "level 5",
	[0x6] = "level 6",
	[0x7] = "level 7",
	[0x10] = "end of level",
	[0x18] = "museum",
	[0x19] = "arena",
	[0x1a] = "quick draw 1",
	[0x1b] = "egg catch 1",
	[0x1c] = "crane fever 1",
	[0x1d] = "warp star",
	[0x1e] = "previous world",
	[0x1f] = "next world",
	[0x2a] = "quick draw 2",
	[0x2b] = "egg catch 2",
	[0x2c] = "crane fever 2",
	[0x3a] = "quick draw 3",
	[0x3b] = "egg catch 3",
	[0x3c] = "crane fever 3",
	}
	
	drawcursor(cursorx, cursory, cboxcolor)
	for i = 0, numexits - 1, 1 do
		exit = exits[i]
		if (exit.screen == ss and exit.x == ctilex and exit.y == ctiley) then
			local type = exittypes[exit.type] or "unknown"
		
			textlines[0] = string.format("Exit at screen %1X (%1X, %1X)", 
			                             exit.screen, exit.x, exit.y)
			textlines[1] = string.format("to: room %03X screen %1X (%1X, %1X)", 
			                             exit.level, exit.to_screen, exit.to_x, exit.to_y)
			textlines[2] = string.format("type: %02X (%s)", 
			                             exit.type, type)
			
			i = numexits
		end
	end
end,
}
numpages = 4

while true do
	topline = ""
	textlines = {
	[0] = "",
	[1] = "",
	[2] = "",
	[3] = "",
	}

	-- capture mouse input
	inpt = input.get();
	xmouse = math.floor(inpt.xmouse / 16) * 16
	ymouse = math.floor(inpt.ymouse / 16) * 16
	
	-- game state stuff
	level = memory.readword(0x0562)
	hscreens = memory.readbyte(addr_hscreens)
	vscreens = memory.readbyte(addr_vscreens)
	
	playerx = memory.readbyte(addr_playerxl)
	        + (memory.readbyte(addr_playerxh) * 256)
	playery = memory.readbyte(addr_playeryl)
	        + (memory.readbyte(addr_playeryh) * 256)
	
	screenx = math.floor(playerx / 256)
	screeny = math.floor(playery / 192)
	screen  = memory.readbyte(addr_screenlist + (hscreens * screeny) + screenx)
	
	scrollx = memory.readbyte(addr_scrollx)
	scrolly = memory.readbyte(addr_scrolly)
	
	ptilex = math.floor(playerx/16) % 16
	ptiley = math.floor(playery/16) % 12
	
	-- position to draw cursor, accounting for scrolling
	cursorx = math.floor((inpt.xmouse + (scrollx % 16))/ 16) * 16 - (scrollx % 16)
	cursory = math.floor((inpt.ymouse + (scrolly % 16))/ 16) * 16 - (scrolly % 16)
	-- tile coordinate that cursor is on (within a screen)
	ctilex = math.floor((scrollx + cursorx + (scrollx % 16)) / 16) % 16
	ctiley = math.floor((scrolly + cursory + (scrolly % 16)) / 16) % 12
	-- screen that cursor is on
	ssx = screenx
	ssy = screeny
	if (scrollx >= playerx % 256 and inpt.xmouse < 256 - scrollx) then
		ssx = screenx - 1
	elseif (scrollx < playerx % 256 and inpt.xmouse >= 256 - scrollx) then
		ssx = screenx + 1
	end
	
	-- this seems to handle >2 screen tall levels fine?
	if (scrolly % 192 >= playery % 192 and inpt.ymouse < 192 - (scrolly % 192)) then
		ssy = screeny - 1
	elseif (scrolly % 192 < playery % 192 and inpt.ymouse >= 192 - (scrolly % 192)) then
		ssy = screeny + 1
	end
	ss = memory.readbyte(addr_screenlist + (hscreens * ssy) + ssx)

	pages[page]()
	
	gui.text(8, 176, string.format("Page %d/%d: %s", page+1, numpages, topline))
	gui.text(8, 176+8, string.format("Level %03X %02X %02X", level, ptilex + screenx * 16, ptiley + screeny * 16))
		
	-- cycle to next page?
	if (inpt.leftclick and mouserepeat == 0 and
		inpt.ymouse >= 176 and inpt.ymouse < 176+8) then
		if (inpt.shift) then
			page = (page + numpages - 1) % numpages
		else
			page = (page + 1) % numpages
		end
	end

	for i = 0, 3, 1 do
		gui.text(8, 176+32+(8*i), textlines[i])
	end
	
	if (inpt.leftclick) then
		mouserepeat = (mouserepeat + 1) % repeatrate
		mouseheld = true
	else
		mouserepeat = 0
		mouseheld = false
	end
	
	-- debug - line between screens
	-- this code suuuuuuuccckkkkkssss
	gui.line(256 - scrollx, 0, 256 - scrollx, 176)
	if (192 - scrolly < 176) then
		gui.line(0, 192 - scrolly, 256, 192 - scrolly)
	end
	
	if (scrolly > (playery % 192)) then
		dispscreen = screen - hscreens
	else
		dispscreen = screen
	end
	
	if (scrollx == 0) then
		gui.text(0, 0, string.format("Screen %02X", dispscreen))
		if (192 - scrolly < 176) then
			gui.text(0, 192 - scrolly, string.format("Screen %02X", dispscreen + hscreens))
		end
	elseif (scrollx < (playerx % 256)) then
		gui.text(  0 - scrollx, 0, string.format("Screen %02X", dispscreen))
		gui.text(256 - scrollx, 0, string.format("Screen %02X", dispscreen+1))
		
		if (192 - scrolly < 176) then
			gui.text(  0 - scrollx, 192 - scrolly, string.format("Screen %02X", dispscreen + hscreens))
			gui.text(256 - scrollx, 192 - scrolly, string.format("Screen %02X", dispscreen + hscreens +1))
		end
	else
		gui.text(256 - scrollx, 0, string.format("Screen %02X", dispscreen))
		if (192 - scrolly < 176) then
			gui.text(256 - scrollx, 192 - scrolly, string.format("Screen %02X", dispscreen + hscreens))
		end
	end
	
	emu.frameadvance()
end