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
		--local screens = memory.readbyte(sprptr)
		--              * memory.readbyte(sprptr+1)
		local screens = hscreens * vscreens
		
		local numspr = memory.readbyte(sprptr+screens+1)
		
		-- TODO finish this, this is just from the unused page
		local sprites = {
		[0x17] = "Poppy Bros. Jr. on tomato (slow)",
		[0x18] = "Poppy Bros. Jr. on tomato (medium)",
		[0x19] = "Poppy Bros. Jr. on tomato (fast)",
		
		[0x41] = "Flamer (slow)",
		[0x42] = "Flamer (medium)",
		[0x43] = "Flamer (fast)",
		
		[0x4d] = "Squishy (slow)",
		[0x4e] = "Squishy (fast)",
		
		[0x52] = "Bronto Burt (slow, adjusts Y)",
		[0x53] = "Bronto Burt (fast, adjusts Y)",
		
		[0x62] = "Starman (slow walk)",
		[0x63] = "Starman (fast walk)",
		
		[0x64] = "Starman (slow fly)",
		[0x65] = "Starman (fast fly)",
		
		[0x81] = "Meta Knight battle (room 13F)",
		[0x86] = "Meta Knight battle (room ???)",
		[0x87] = "Meta Knight battle (room 141)",
		[0x8b] = "Meta Knight battle (shotzo)",
		[0x8c] = "Meta Knight battle (dummy)",
		[0x8d] = "Meta Knight battle (dummy)",
		[0x8e] = "Meta Knight battle (dummy)",
		
		[0xa0] = "Twizzy (drop, slow fly)",
		[0xa1] = "Twizzy (drop, fast fly)",
		[0xa2] = "Twizzy (slow fly diagonal)",
		[0xa3] = "Twizzy (fast fly diagonal)",
		[0xa9] = "Twizzy (slow fly from ground)",
		[0xaa] = "Twizzy (fast fly from ground)",
		
		[0xe0] = "Museum Sparky",
		[0xe3] = "Museum Flamer",
		[0xe8] = "Museum Sir Kibble",
		[0xeb] = "Museum Hammer(?)",
		[0xed] = "Museum Spiny",
		[0xee] = "Museum Pengi",
		[0xef] = "Museum Chilly",
		
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
			if (math.floor(b / 16) % 8 > 0) then
				exit.type = exit.type + 0x10
			end
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