/***************************************************************************

    Namco PuckMan

    driver by Nicola Salmoria and many others

    Games supported:
        * PuckMan
        * Pac-Man Plus
        * Ms. Pac-Man
        * Crush Roller
        * Ponpoko
        * Eyes
        * Mr. TNT
        * Gorkans
        * Lizard Wizard
        * The Glob
        * Dream Shopper
        * Van Van Car
        * Ali Baba and 40 Thieves
        * Jump Shot
        * Shoot the Bull
        * Big Bucks
        * Driving Force
        * Eight Ball Action
        * Porky
        * MTV Rock-N-Roll Trivia (Part 2)
        * Woodpecker


    Known issues:
        * mystery items in Ali Baba don't work correctly because of protection

    Known to exist but dumps needed
        * Eeeek!
        * Ms Pac Plus
        * Ms Pac Man Twin
        * MTV Rock-N-Roll Trivia (Part 2), 1 bad rom. It's not a bad dump, the rom is bad.

****************************************************************************

    Pac-Man memory map (preliminary)

    0000-3fff ROM
    4000-43ff Video RAM
    4400-47ff Color RAM
    4800-4bff RAM Dream Shopper, Van Van Car only.  Pacman uses this block due to a bug in the routine to
         translate the internal pacman location to a screen address when in the right tunnel.
    4c00-4fff RAM
    8000-bfff ROM Ms Pac-Man, Ponpoko, Lizard Wizard, Dream Shopper, Van Van Car, Woodpecker, Ali Babba all use
         portions of the upper memory area.  Pacman and most bootlegs don't have an A15 line to the cpu so most
         boards that use upper memmory have an auxillary board that plugs into the cpu socket with a ribbon cable.
         There is also a common Ms Pacman hack for pacman bootlegs to wire A15 from the cpu to the address decoder
         in place of the refresh line. The extra eproms are stacked on lower eproms or placed in unused sockets for
         2k roms.


    memory mapped ports:

    read:
    5000      IN0
    5040      IN1
    5080      DSW 1
    50c0      DSW 2 (Ponpoko, mschamp, Van Van Car, Rock and Roll Trivia 2 only)
    see the input_ports definition below for details on the input bits

    write:
    4ff0-4fff 8 pairs of two bytes:
              the first byte contains the sprite image number (bits 2-7), Y flip (bit 0),
              X flip (bit 1); the second byte the color.  Note: Only Ponpoko has 8 sprites
              an original Midway Pacman board containls only the center 6 sprites.
    5000      interrupt enable
    5001      sound enable
    5002      latch at location 8K has no connection for this address
    5003      flip screen
    5004      1 player start lamp
    5005      2 players start lamp
    5006      coin lockout                      Note: no boards are known to contain the output
            transistors to drive a lamp or coin lockout.   The schematics and boards show no connection
            to the output of the latch at location 8k
    5007      coin counter
    5040-5044 sound voice 1 accumulator (nibbles) (used by the sound hardware only)
    5045      sound voice 1 waveform (nibble)
    5046-5049 sound voice 2 accumulator (nibbles) (used by the sound hardware only)
    504a      sound voice 2 waveform (nibble)
    504b-504e sound voice 3 accumulator (nibbles) (used by the sound hardware only)
    504f      sound voice 3 waveform (nibble)
    5050-5054 sound voice 1 frequency (nibbles)
    5055      sound voice 1 volume (nibble)
    5056-5059 sound voice 2 frequency (nibbles)
    505a      sound voice 2 volume (nibble)
    505b-505e sound voice 3 frequency (nibbles)
    505f      sound voice 3 volume (nibble)
    5060-506f Sprite coordinates, x/y pairs for 8 sprites
    50c0      Watchdog reset

    I/O ports:
    OUT on port $0 sets the interrupt vector


****************************************************************************

    Make Trax protection description:

    Make Trax has a "Special" chip that it uses for copy protection.
    The following chart shows when reads and writes may occur:

    AAAAAAAA AAAAAAAA
    11111100 00000000  <- address bits
    54321098 76543210
    xxx1xxxx 01xxxxxx - read data bits 4 and 7
    xxx1xxxx 10xxxxxx - read data bits 6 and 7
    xxx1xxxx 11xxxxxx - read data bits 0 through 5

    xxx1xxxx 00xxx100 - write to Special
    xxx1xxxx 00xxx101 - write to Special
    xxx1xxxx 00xxx110 - write to Special
    xxx1xxxx 00xxx111 - write to Special

    In practical terms, it reads from Special when it reads from
    location $5040-$50FF.  Note that these locations overlap our
    inputs and Dip Switches.  Yuk.

    I don't bother trapping the writes right now, because I don't
    know how to interpret them.  However, comparing against Crush
    Roller gives most of the values necessary on the reads.

    Instead of always reading from $5040, $5080, and $50C0, the Make
    Trax programmers chose to read from a wide variety of locations,
    probably to make debugging easier.  To us, it means that for the most
    part we can just assign a specific value to return for each address and
    we'll be OK.  This falls apart for the following addresses:  $50C0, $508E,
    $5090, and $5080.  These addresses should return multiple values.  The other
    ugly thing happening is in the ROMs at $3AE5.  It keeps checking for
    different values of $50C0 and $5080, and weird things happen if it gets
    the wrong values.  The only way I've found around these is to patch the
    ROMs using the same patches Crush Roller uses.  The only thing to watch
    with this is that changing the ROMs will break the beginning checksum.
    That's why we use the rom opcode decode function to do our patches.

    Incidentally, there are extremely few differences between Crush Roller
    and Make Trax.  About 98% of the differences appear to be either unused
    bytes, the name of the game, or code related to the protection.  I've
    only spotted two or three actual differences in the games, and they all
    seem minor.

    If anybody cares, here's a list of disassembled addresses for every
    read and write to the Special chip (not all of the reads are
    specifically for checking the Special bits, some are for checking
    player inputs and Dip Switches):

    Writes: $0084, $012F, $0178, $023C, $0C4C, $1426, $1802, $1817,
        $280C, $2C2E, $2E22, $3205, $3AB7, $3ACC, $3F3D, $3F40,
        $3F4E, $3F5E
    Reads:  $01C8, $01D2, $0260, $030E, $040E, $0416, $046E, $0474,
        $0560, $0568, $05B0, $05B8, $096D, $0972, $0981, $0C27,
        $0C2C, $0F0A, $10B8, $10BE, $111F, $1127, $1156, $115E,
        $11E3, $11E8, $18B7, $18BC, $18CA, $1973, $197A, $1BE7,
        $1C06, $1C9F, $1CAA, $1D79, $213D, $2142, $2389, $238F,
        $2AAE, $2BF4, $2E0A, $39D5, $39DA, $3AE2, $3AEA, $3EE0,
        $3EE9, $3F07, $3F0D


*****************************************************************************************
notes:
------
- Pacman: The Cabinet Type "dip switch" actually comes from the edge connector.  "Dip switches" #7(0x40) and #8(0x80)
  are really solder pads. The actual dip #7 is Rack advance from IN0 and dip #8 is video freeze which is handled in
  hardware.

- Pacman: Pacman contains a bug in the up direction of pacman animation. The circle animation is 1 pixel to low.
  Pucman set 1 has a patch to fix it at 1700-1704

- Puckman set 1 is likely a bootleg since the protection is patched out, set 2 would likely
  be correct if the roms were split differently.  Nicola had said that he had a readme that mentioned 2k roms,
  it is likely they were all 2k or all 4k, not mixed. I have seen a puckman cocktail cab with masked 4k roms.
  Any code changes to the first two roms will affect the random number generator so it should be assumed to play
  differently.

- puckman set 3 (previously labeled(harder?):
  npacmod.6j and NAMCOPAC.6J
  00000031: AF 25    ;3031 is sub for fail rom check.
  00000032: C3 7C    ;301c is sub for pass rom check
  00000033: 1C E6    ;so it now clears the sum (reg A) and
  00000034: 30 F0    ;jumps to pass if it fails rom check.
  000007F8: 31 30  c 1981 / c 1980
  0000080B-12:   ghost / nickname
  00000FFF: 00 F1  checksum

- mspacmab: this is the equivalent of pacman with the ms pacman overlays permanently installed.  There are a few
  extra bytes that don't seemed to be used at all.  The Rom check is patched to never fail.  Many repair shops will
  install this code in all ms pacman boards.  Since the overlays move in and out during play on the original it is
  unknown if this version will play the same. It is possible to identify if a board is running this or the original
  code by observing the behavior on boot.

- mspacmnf,pacmanf: These sets are the results of replacing 6f with what is known as the speedup chip. These are more
  popular than the original with operators and players.  The pacman speedup breaks the attract mode and intermissions.
  There's a modern speedup chip that fixes this.

- Pacmod Harder compared to Pacman
  pacmanh.5E
  erase tiles $25:"."   $28-$2e:"NAMCO"   $5c:copyright C

  program code
  pacman/pacmod
  0843: 14 0F  Table of when ghosts come out of house.
  0844: 1E 14
  0845: 46 37
  0846: 00 04 pink,   level 1
  0847: 1E 18 blue
  0848: 3C 34 orange
  0849: 00 02         level 2
  084A: 00 06
  084B: 32 28
  084c: 00 00         level 3
  084D: 00 04
  084E: 00 08
  0FFF: 36 54  checksum
  2795: 29 00  pink ghost ai, points target to 2 tiles in front of pac instead of 4.
  281F: 40 24  orange ghost ai, change close/far breakpoint from $40 to $24.
  2FFF: 4C 91    checksum
  37F8,3d28,3d43:  change 1980 to 1981
  Notice the harder upgrade was 2 roms only. 6e,6h.  The others are from a newer set.

- joyman: Pacman maze hack, but they left the dots as is, the music has been mangled as well.

- ctrpllrp: Caterpillar is like Piranha in that it was sold as a unique game.  They were hoping no one would notice
  it uses pacman code for it's base.  Unlike Piranha as far as I know they got away with it.
  Differences include new music in the intermissions.

- mspacatk: This is the alternate maze chip. The mazes are new but the fruit paths are not changed, causing fruit
  to move through walls.  There are at least two different hacks of this chip to fix it.

- mspacpls: This romset is hacked.  mspacatk.2 is more commonly known as the "cheat chip" and is used on bootleg
  ms pacs  in place of boot2. It was created circa 1997 by Doc Cutlip and allows turbo speed and invulnerability.
  mspacatk.5 and .6 contain the decoded maze data for ms pac plus.  The actual dump is not available.
  I believe the actual version contains more than just new maze data.

- pacgal:  This is a common hack found on make trax and other boards.  Mostly they use 4k eproms but Make Trax allows
  4x2k for graphics. Boot5 and 6 are stacked on boot3 and 4, the refresh hack is used for addressing.   Usually
  they swap out the 4a color prom as well.  The bootlegger here was apparently too cheap to burn the prom and kept the
  make trax colors.  Other than that this is an insignificant set.

  Comparing files boot3 and PACMAN.7FH
  0000069F: 01 03   ;runs on boot only, probably garbage bit
  Comparing files boot4 and PACMAN.7HJ
  00000807-812:  ;Changes "MS PAC-MAN" to "PAC-GAL"
  00000D3E-D4C:  ;Erases  c MIDWAY MFG CO
  00000D97-DA0:  ;Changes MS PAC-MAN  to  PAC-GAL
  Comparing files 5f and 5F PacGal
  000007D5: 00 01 garbage bit in otherwise blank sprite $1f

- maketrax: All Make Trax boards are Crush Roller boards.  They have Crush Roller screen printed on them covered by
  a Williams sticker.  The roms also have another sticker under the Williams sticker.

- maketrxb:  This board looked slightly different than the standard make trax.  The usual Red and Yellow jumper wires
  for sync inversion were smaller and white and the stickers were slightly different.

- Pac and Paint: Even though this seems to be a unique game the marquee shows a paintbush.  The board has standard
  crush roller roms.  It is slightly different from a Make Trax board in that the sync inverter chip is missing and
  there are no jumper wires.

- jumpshtp: This board was aquired by Pokemame from a former midway employee.  The hardware is identical to normal boards
  except the daughterboard is missing the epoxy potting.  Board was labeled engineering sample. Code differences
  include dips, starting position and cheerleader text.

- sprglobp: This might be original or someone may have combined the program roms from the glob with roms from super.
  I believe there is a set that uses 1 more program rom than this set.

- Eyes, Mr. Tnt and Lizard Wizard share some code.

- Games that share code with Puckman: Ali Baba, Piranha, Caterpiller, Naughty Mouse, Pacman Plus, ms pacman.  From
  galaxian hardware steaking and the pacman bootlegs also.

- acitya contains a bug with the insurance in blackjack.  It's impossible to collect, so it's likely that acitya is
  earlier than bwcasino.

Easter eggs:
-----------
- Pacman, Ms Pacman, Piranha and similar display "MADE BY NAMCO" sideways in power pellets.  Hold start 1 and 2 and
  toggle the test switch to get to the convergence grid.  4xUp, 4xL, 4xR, 4xD.

Boards:
-------
- puckman is the same as pacman except they are slotted to break a part and have ribbon cables to connect the halves.

- All ms pacman boards are pacman boards with an auxillary cpu board installed on a ribbon cable and 5e,5f changed.

- Pacman Plus, Shoot the Bull, Jumpshot are epoxy potted auxillary cpu boards.  The graphics and
  color proms are changed as well.  They are labeled Authorized Enhancement Kit Bally/Midway.   Jumpshot=B3172

- The Eyes boards are similar to pacman boards except for the data swapping encryption.  The custom chips are integrated
  into the board and there is no voltage regulator section.  There is an extra rom at row 6 so row 7 is equivilant to
  pacman row 6.  Program roms are 4k but graphics roms have unpopulated areas for optional 2k roms.

- Piranha and Naughty Mouse use a board known as the GDP-01 bootleg.  It is similar to an eyes board with an extra row
  of eproms in row 6 to enable 2k program roms. Thee GDP-01 does not requiree a SBC, but 5 SBC chips can be left unpopulated
  and a SBC card(GDP-02) can be installed.

- Make Trax/Crush Roller boards are similar to pacman boards, the chip positions are even mostly the same.  There is no
  voltage regulator section.  Make Trax has sync inverter at 1S, jumper wires run from the edge connector to the
  inverter and back.  Make Trax/CR boards are easily identified because the edge connector is on the long side,
  offset to one side.  Make trax and Eyes pinouts are similar enough to test each other but are not playable.

- Atlantic City Action, Boardwalk Casino, The Glob, Super Glob, Beastie Feastie, EEEK! all use identical Epos boards
  with different pals.  Street heat, Drakton on Donkey Kong also use this board.  It is a cpu auxillary board.  It is
  not potted but all the chip labels are removed.

- Porky and 8ball action use the same plug in board for pacman.  It is much larger than most and requires some soldering
  to install.  It includes a new cpu and sound hardware.  Driving force uses a similar board and requires a lot of
  soldering.

- Lizard Wizard is a potted auxillary cpu board.

- Big Bucks is an auxillary cpu board.

- MTV Trivia is an auxillary cpu board.

- Truco Clemente runs on a pac bootleg with a handwired cpu/graphics auxillary board.  This supports banking of the
  roms and completely bypasses color proms and video output section of pacman.

- Vanvanb was found on a low quality board assumed to be a bootleg of an original.  The 2 sound chips were on a
  daughterboard that plugged into a 40 pin socket. Physically it is very different from pacman, although the fact that
  it uses the same falcon pinout as every other pac boot is a little suspicious.

- Ponpoko uses it's own board.

****************************************************************************/

#include "driver.h"
#include "pacman.h"
#include "cpu/s2650/s2650.h"
#include "sound/namco.h"
#include "sound/ay8910.h"
#include "sound/sn76496.h"

static UINT8 *decrypted_opcodes;

/*************************************
 *
 *  Machine init
 *
 *************************************/

MACHINE_RESET( mschamp )
{
	UINT8 *rom = memory_region(REGION_CPU1) + 0x10000;
	int whichbank = readinputportbytag("GAME") & 1;

	memory_configure_bank(1, 0, 2, &rom[0x0000], 0x8000);
	memory_configure_bank(2, 0, 2, &rom[0x4000], 0x8000);

	memory_set_bank(1, whichbank);
	memory_set_bank(2, whichbank);
}

static DRIVER_INIT( mspactwin )
{
	static UINT8 data_holder[0xc000];

	UINT8 *rom = memory_region(REGION_CPU1);
	int A;

	decrypted_opcodes = data_holder;
	for (A = 0x0000; A < 0x4000; A+=2) {

		/* decode opcode */
		decrypted_opcodes     [A  ] = BITSWAP8(rom[       A  ]       , 4, 5, 6, 7, 0, 1, 2, 3);
		decrypted_opcodes     [A+1] = BITSWAP8(rom[       A+1] ^ 0x9A, 6, 4, 5, 7, 2, 0, 3, 1);
		decrypted_opcodes[0x8000+A  ] = BITSWAP8(rom[0x8000+A  ]       , 4, 5, 6, 7, 0, 1, 2, 3);
		decrypted_opcodes[0x8000+A+1] = BITSWAP8(rom[0x8000+A+1] ^ 0x9A, 6, 4, 5, 7, 2, 0, 3, 1);

		/* decode operand */
		rom[       A  ] = BITSWAP8(rom[       A  ]       , 0, 1, 2, 3, 4, 5, 6, 7);
		rom[       A+1] = BITSWAP8(rom[       A+1] ^ 0xA3, 2, 4, 6, 3, 7, 0, 5, 1);
		rom[0x8000+A  ] = BITSWAP8(rom[0x8000+A  ]       , 0, 1, 2, 3, 4, 5, 6, 7);
		rom[0x8000+A+1] = BITSWAP8(rom[0x8000+A+1] ^ 0xA3, 2, 4, 6, 3, 7, 0, 5, 1);
	}
	/*this makes no sense to me at this point its no even mapped but ill leave it in just in case it goes pear shaped and needs added 

	for (A = 0x0000; A < 0x2000; A++) {
		decrypted_opcodes[0x6000+A] = decrypted_opcodes[A+0x2000];
		rom[0x6000+A  ] = BITSWAP8(rom[0x6000+A  ]       , 0, 1, 2, 3, 4, 5, 6, 7);
		rom[0x6000+A+1] = BITSWAP8(rom[0x6000+A+1] ^ 0xA3, 2, 4, 6, 3, 7, 0, 5, 1);
	}*/

		memory_configure_bank(1, 1, 1, rom         , 0);
		memory_configure_bank(2, 1, 1, &rom[0x2000], 0);
		memory_configure_bank(3, 1, 1, &rom[0x8000], 0);

		memory_configure_bank_decrypted(1, 1, 1, decrypted_opcodes         ,  0);
		memory_configure_bank_decrypted(2, 1, 1, &decrypted_opcodes[0x2000],  0);
		memory_configure_bank_decrypted(3, 1, 1, &decrypted_opcodes[0x8000],  0);

		memory_set_bank(1, 1);
		memory_set_bank(2, 1);
		memory_set_bank(3, 1);
}

/*************************************
 *
 *  Interrupts
 *
 *************************************/

static WRITE8_HANDLER( pacman_interrupt_vector_w )
{
	cpunum_set_input_line_vector(0, 0, data);
	cpunum_set_input_line(0, 0, CLEAR_LINE);
}


static INTERRUPT_GEN( pacman_interrupt )
{
	/* always signal a normal VBLANK */
	if (cpu_getiloops() == 0)
		irq0_line_hold();

	/* on other "VBLANK" opportunities, check to make sure the cheat is enabled */
	/* and that the speedup button is pressed */
	else
	{
		int portnum = port_tag_to_index("FAKE");
		if (portnum != -1)
		{
			UINT8 value = readinputport(portnum);
			if ((value & 7) == 5 || (value & 6) == 2)
				irq0_line_hold();
		}
	}
}

static INTERRUPT_GEN( mspactwin_interrupt )
{
	irq0_line_hold();
}

/*
   The piranha board has a sync bus controler card similar to Midway's pacman. It
   stores the LSB of the interupt vector using port 00 but it alters the byte to prevent
   it from running on normal pacman hardware and vice versa. I wrote a program to print
   out the even numbers and the vectors they convert to.  Thanks to Dave France for
   burning the roms.  The numbers that didn't print here convert to odd numbers.  It's
   slightly possible some numbers listed converted to odd numbers and coincidentally
   printed a valid even number.  Since it only uses 2 vectors($fa,$fc) I didn't complete
   the table or attempt to find the algorythm.

   David Widel
   Pacman@mailblocks.com
  out vec  out vec  out vec  out vec
  c0 44    80 04    40 44    00 04
  c2 40    82 00    42 C4    02 84
  c4 C4    84 84    44 C4    04 00
  c6 40    86 00
  c8 4C    88 0C    48 4C    08 0C
  ca 48    8A 08    4A CC    0A 8C
  cc CC    8C 8C    4C 48    0C 08
  ce 48    8E 08
  d0 54    90 14    50 54    10 14
  d2 50    92 10    52 D4    12 94
  d4 D4    94 94    54 50    14 10
  d6 50    96 10
  d8 5C    98 1C    58 5C    18 1C
  da 58    9A 18    5A DC    1A 9C
  dc DC    9C 9C    5C 58    1C 18
  de 58    9E 18
  e0 64    a0 24    60 64    20 24
  e2 60    a2 20    62 E4    22 A4
  e4 E4    a4 A4    64 60    24 20
  e6 60    a6 20
  e8 6C    a8 2C    68 6C    28 2C
  ea 68    aA 28    6A EC    2A AC
  ec EC    aC AC    6C 68    2C 28
  ee 68    aE 28
  f0 74    b0 34    70 74    30 34
  f2 70    b2 30    72 F4    32 84
  f4 F4    b4 B4    74 70    34 30
  f6 70    b6 30
  f8 7C    b8 3C    78 7C    38 3C
  fa 78    bA 38    7A FC    3A BC
  fc FC    bC BC    7C 78    3C 38
  fe 78    bE 38


Naughty Mouse uses the same board as Piranha with a different pal to encrypt the vectors.
Incidentally we don't know the actual name of this game.  Other than the word naughty at
the top of the playfield there's no name.

I haven't examined the code thoroughly but what I
did look at(sprite buffer), was copied from Pacman.  The addresses for the variables seem
to be the same as well.
*/

static WRITE8_HANDLER( piranha_interrupt_vector_w)
{
	if (data==0xfa) data=0x78;
	if (data==0xfc) data=0xfc;
	cpunum_set_input_line_vector( 0, 0, data );
}


static WRITE8_HANDLER( nmouse_interrupt_vector_w)
{
	if (data==0xbf) data=0x3c;
	if (data==0xc6) data=0x40;
	if (data==0xfc) data=0xfc;
	cpunum_set_input_line_vector( 0, 0, data );
}


/*************************************
 *
 *  LEDs/coin counters
 *
 *************************************/

static WRITE8_HANDLER( pacman_leds_w )
{
	set_led_status(offset,data & 1);
}


static WRITE8_HANDLER( pacman_coin_counter_w )
{
	coin_counter_w(offset,data & 1);
}


static WRITE8_HANDLER( pacman_coin_lockout_global_w )
{
	coin_lockout_global_w(~data & 0x01);
}



/*************************************
 *
 *  Ali Baba sound
 *
 *************************************/

static WRITE8_HANDLER( alibaba_sound_w )
{
	/* since the sound region in Ali Baba is not contiguous, translate the
       offset into the 0-0x1f range */
 	if (offset < 0x10)
		pacman_sound_w(offset, data);
	else if (offset < 0x20)
		spriteram_2[offset - 0x10] = data;
	else
		pacman_sound_w(offset - 0x10, data);
}


static READ8_HANDLER( alibaba_mystery_1_r )
{
	/* The return value determines what the mystery item is.  Each bit corresponds
       to a question mark */
	return rand() & 0x0f;
}


static READ8_HANDLER( alibaba_mystery_2_r )
{
	/* The single bit return value determines when the mystery is lit up.
       This is certainly wrong */
	static int mystery = 0;
	mystery++;
	return (mystery >> 10) & 1;
}



/*************************************
 *
 *  Make Trax input handlers
 *
 *************************************/

static READ8_HANDLER( maketrax_special_port2_r )
{
	int data = input_port_2_r(offset);
	int pc = activecpu_get_previouspc();

	if ((pc == 0x1973) || (pc == 0x2389)) return data | 0x40;

	switch (offset)
	{
		case 0x01:
		case 0x04:
			data |= 0x40; break;
		case 0x05:
			data |= 0xc0; break;
		default:
			data &= 0x3f; break;
	}

	return data;
}


static READ8_HANDLER( maketrax_special_port3_r )
{
	int pc = activecpu_get_previouspc();

	if (pc == 0x040e) return 0x20;

	if ((pc == 0x115e) || (pc == 0x3ae2)) return 0x00;

	switch (offset)
	{
		case 0x00:
			return 0x1f;
		case 0x09:
			return 0x30;
		case 0x0c:
			return 0x00;
		default:
			return 0x20;
	}
}

static READ8_HANDLER( korosuke_special_port2_r )
{
	int data = input_port_2_r(offset);
	int pc = activecpu_get_previouspc();

	if ((pc == 0x196e) || (pc == 0x2387)) return data | 0x40;

	switch (offset)
	{
		case 0x01:
		case 0x04:
			data |= 0x40; break;
		case 0x05:
			data |= 0xc0; break;
		default:
			data &= 0x3f; break;
	}

	return data;
}

static READ8_HANDLER( korosuke_special_port3_r )
{
	int pc = activecpu_get_previouspc();

	if (pc == 0x0445) return 0x20;

	if ((pc == 0x115b) || (pc == 0x3ae6)) return 0x00;

	switch (offset)
	{
		case 0x00:
			return 0x1f;
		case 0x09:
			return 0x30;
		case 0x0c:
			return 0x00;
		default:
			return 0x20;
	}
}

/*************************************
 *
 *  Zola kludge
 *
 *************************************/

static READ8_HANDLER( mschamp_kludge_r )
{
	static UINT8 counter;
	return counter++;
}

/************************************
 *
 *  Big Bucks questions roms handlers
 *
 ************************************/

static int bigbucks_bank = 0;

static WRITE8_HANDLER( bigbucks_bank_w )
{
	bigbucks_bank = data;
}

static READ8_HANDLER( bigbucks_question_r )
{

	UINT8 *question = memory_region(REGION_USER1);
	UINT8 ret;

	ret = question[(bigbucks_bank << 16) | (offset ^ 0xffff)];

	return ret;
}

/************************************
 *
 *  S2650 cpu based games
 *
 ************************************/

static INTERRUPT_GEN( s2650_interrupt )
{
	cpunum_set_input_line_and_vector(0, 0, HOLD_LINE, 0x03);
}

static READ8_HANDLER( drivfrcp_port1_r )
{
	switch (activecpu_get_pc())
	{
		case 0x0030:
		case 0x0291:
			return 0x01;
	}

	return 0;
}

static READ8_HANDLER( _8bpm_port1_r )
{
	switch (activecpu_get_pc())
	{
		case 0x0030:
		case 0x0466:
			return 0x01;
	}

	return 0;
}

static READ8_HANDLER( porky_port1_r )
{
	switch (activecpu_get_pc())
	{
		case 0x0034:
			return 0x01;
	}

	return 0;
}


/************************************
 *
 *  Rock-N-Roll Trivia (Part 2)
 *  questions roms and protection
 *  handlers
 *
 ************************************/

static UINT8 *rocktrv2_prot_data, rocktrv2_question_bank = 0;

static READ8_HANDLER( rocktrv2_prot1_data_r )
{
	return rocktrv2_prot_data[0] >> 4;
}

static READ8_HANDLER( rocktrv2_prot2_data_r )
{
	return rocktrv2_prot_data[1] >> 4;
}

static READ8_HANDLER( rocktrv2_prot3_data_r )
{
	return rocktrv2_prot_data[2] >> 4;
}

static READ8_HANDLER( rocktrv2_prot4_data_r )
{
	return rocktrv2_prot_data[3] >> 4;
}

static WRITE8_HANDLER( rocktrv2_prot_data_w )
{
	rocktrv2_prot_data[offset] = data;
}

static WRITE8_HANDLER( rocktrv2_question_bank_w )
{
	rocktrv2_question_bank = data;
}

static READ8_HANDLER( rocktrv2_question_r )
{
	UINT8 *question = memory_region(REGION_USER1);

	return question[offset | (rocktrv2_question_bank * 0x8000)];
}


static READ8_HANDLER( pacman_read_nop )
{
	// Return value of reading the bus with no devices enabled.
	// This seems to be common but more tests are needed. Ms Pacman reads bytes in sequence
	// until it hits a 0 for a delimiter, including empty areas.  It writes to "random"
	// addresses each time. This causes the maze to invert sometimes.  See code at $95c3 where
	// level($4e13)=134. DW
	return 0xff;
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( pacman_map, ADDRESS_SPACE_PROGRAM, 8 )
	//A lot of games don't have an a15 at the cpu.  Generally only games with a cpu daughter board can access the full 32k of romspace.
	AM_RANGE(0x0000, 0x3fff) AM_MIRROR(0x8000) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0xa000) AM_READWRITE(pacman_read_nop,MWA8_NOP)
	AM_RANGE(0x4c00, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_WRITE(pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP) // AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP) // AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_WRITE(pacman_sound_w) AM_BASE(&pacman_soundregs)
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram_2)
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ(input_port_2_r)	/* DSW1 */
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ(input_port_3_r)	/* DSW2 */
ADDRESS_MAP_END
// The Pacman code uses $5004 and $5005 for LED's and $5007 for coin lockout.  This hardware does not
// exist on any Pacman or Puckman board I have seen.  DW


static ADDRESS_MAP_START( mspacman_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READWRITE(MRA8_BANK1,MWA8_ROM)
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0xa000) AM_READWRITE(pacman_read_nop,MWA8_NOP)
	AM_RANGE(0x4c00, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_WRITE(pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP) // AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITE(mspacman_activate_rom) /* Not actually, just handy */ // AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_WRITE(pacman_sound_w) AM_BASE(&pacman_soundregs)
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram_2)
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ(input_port_2_r)	/* DSW1 */
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ(input_port_3_r)	/* DSW2 */
	AM_RANGE(0x8000, 0xbfff) AM_READWRITE(MRA8_BANK1,MWA8_ROM)
ADDRESS_MAP_END

READ8_HANDLER(mspactwin_spriteram_r)
{
	return spriteram[offset];
}

static ADDRESS_MAP_START( mspactwin_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROMBANK(1)
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK(2)
	AM_RANGE(0x4000, 0x47ff) AM_READ(MRA8_RAM)	/* video and color RAM */
    AM_RANGE(0x4c00, 0x4fef) AM_READ(MRA8_RAM)
	AM_RANGE(0x4ff0, 0x4fff) AM_READ(mspactwin_spriteram_r)	/*sprite codes at 4ff0-4fff */
	AM_RANGE(0x5000, 0x5000) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0x5040, 0x5040) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0x5080, 0x5080) AM_READ(input_port_4_r)	/* DSW1 */
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(3)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mspactwin_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROMBANK(1)
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK(2)
	AM_RANGE(0x4000, 0x43ff) AM_WRITE(mspactwin_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x4400, 0x47ff) AM_WRITE(pacman_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x4c00, 0x4fef) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x4ff0, 0x4fff) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x5000, 0x5000) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x5001, 0x5001) AM_WRITE(pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5003, 0x5003) AM_WRITE(pacman_flipscreen_w)
 	AM_RANGE(0x5004, 0x5005) AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_WRITE(mspacman_activate_rom)	/* Not actually, just handy */
 	AM_RANGE(0x5007, 0x5007) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_WRITE(pacman_sound_w) AM_BASE(&pacman_soundregs)
	AM_RANGE(0x5060, 0x506f) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram_2)
	AM_RANGE(0x50c0, 0x50c0) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(3)
	AM_RANGE(0xc000, 0xc3ff) AM_WRITE(mspactwin_videoram_w) /* mirror address for video ram, */
	AM_RANGE(0xc400, 0xc7ef) AM_WRITE(pacman_colorram_w) /* used to display HIGH SCORE and CREDITS */
ADDRESS_MAP_END


static ADDRESS_MAP_START( woodpek_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0xa000) AM_READWRITE(pacman_read_nop,MWA8_NOP)
	AM_RANGE(0x4c00, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_WRITE(pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP) // AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP) // AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_WRITE(pacman_sound_w) AM_BASE(&pacman_soundregs)
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram_2)
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ(input_port_2_r)	/* DSW1 */
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ(input_port_3_r)	/* DSW2 */
	AM_RANGE(0x8000, 0xbfff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( alibaba_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0xa000) AM_READWRITE(pacman_read_nop,MWA8_NOP)
	AM_RANGE(0x4c00, 0x4eef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ef0, 0x4eff) AM_MIRROR(0xa000) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x4f00, 0x4fff) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x506f) AM_MIRROR(0xaf00) AM_WRITE(alibaba_sound_w) AM_BASE(&pacman_soundregs)  /* the sound region is not contiguous */
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram_2) /* actually at 5050-505f, here to point to free RAM */
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf00) AM_WRITE(pacman_sound_enable_w)
	AM_RANGE(0x50c1, 0x50c1) AM_MIRROR(0xaf00) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x50c2, 0x50c2) AM_MIRROR(0xaf00) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x50c3, 0x50ff) AM_MIRROR(0xaf00) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ(input_port_2_r)	/* DSW1 */
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf00) AM_READ(alibaba_mystery_1_r)
	AM_RANGE(0x50c1, 0x50c1) AM_MIRROR(0xaf00) AM_READ(alibaba_mystery_2_r)
	AM_RANGE(0x50c2, 0x50ff) AM_MIRROR(0xaf00) AM_READ(pacman_read_nop)
	AM_RANGE(0x8000, 0x8fff) AM_ROM
	AM_RANGE(0x9000, 0x93ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0xa000, 0xa7ff) AM_MIRROR(0x1800) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( dremshpr_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x4800, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(interrupt_enable_w)
//  AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_WRITE(pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP) /* unknown */
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP) // AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP) // AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
//  AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_WRITE(pacman_sound_w) AM_BASE(&pacman_soundregs)
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram_2)
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ(input_port_2_r)	/* DSW1 */
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ(input_port_3_r)	/* DSW2 */
	AM_RANGE(0x8000, 0xbfff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( epos_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_MIRROR(0x8000) AM_READWRITE(MRA8_BANK1,MWA8_ROM)
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0xa000) AM_READWRITE(pacman_read_nop,MWA8_NOP)
	AM_RANGE(0x4c00, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_WRITE(pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP) // AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP) // AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_WRITE(pacman_sound_w) AM_BASE(&pacman_soundregs)
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram_2)
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ(input_port_2_r)	/* DSW1 */
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ(input_port_3_r)	/* DSW2 */
ADDRESS_MAP_END


static ADDRESS_MAP_START( vanvan_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x4800, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_WRITE(vanvan_bgcolor_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5004) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5005, 0x5006) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP) /* always written together with 5001 */
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram_2)
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITE(MWA8_NOP) /* ??? toggled before reading 5000 */
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ(input_port_2_r)	/* DSW1 */
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ(input_port_3_r)	/* DSW2 */
	AM_RANGE(0x8000, 0x8fff) AM_MIRROR(0x3000) AM_ROM
	/* probably a leftover from development: the Sanritsu version */
	/* writes the color lookup table here, while the Karateko version */
	/* writes garbage. */
	AM_RANGE(0xb800, 0xb87f) AM_WRITE(MWA8_NOP)
ADDRESS_MAP_END


static ADDRESS_MAP_START( s2650games_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_MIRROR(0x8000) AM_ROM
	AM_RANGE(0x1000, 0x13ff) AM_MIRROR(0xe000) AM_WRITE(s2650games_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x1400, 0x141f) AM_MIRROR(0xe000) AM_WRITE(s2650games_scroll_w)
	AM_RANGE(0x1420, 0x148f) AM_MIRROR(0xe000) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x1490, 0x149f) AM_MIRROR(0xe000) AM_WRITE(MWA8_RAM) AM_BASE(&s2650games_spriteram)
	AM_RANGE(0x14a0, 0x14bf) AM_MIRROR(0xe000) AM_WRITE(s2650games_tilesbank_w) AM_BASE(&s2650games_tileram)
	AM_RANGE(0x14c0, 0x14ff) AM_MIRROR(0xe000) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x1500, 0x1502) AM_MIRROR(0xe000) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x1503, 0x1503) AM_MIRROR(0xe000) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x1504, 0x1506) AM_MIRROR(0xe000) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x1507, 0x1507) AM_MIRROR(0xe000) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x1508, 0x155f) AM_MIRROR(0xe000) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x1560, 0x156f) AM_MIRROR(0xe000) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram_2)
	AM_RANGE(0x1570, 0x157f) AM_MIRROR(0xe000) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x1586, 0x1587) AM_MIRROR(0xe000) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x15c0, 0x15c0) AM_MIRROR(0xe000) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x15c7, 0x15c7) AM_MIRROR(0xe000) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x1500, 0x1500) AM_MIRROR(0xe000) AM_READ(input_port_0_r)
	AM_RANGE(0x1540, 0x1540) AM_MIRROR(0xe000) AM_READ(input_port_1_r)
	AM_RANGE(0x1580, 0x1580) AM_MIRROR(0xe000) AM_READ(input_port_2_r)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0xe000) AM_WRITE(s2650games_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x1c00, 0x1fef) AM_MIRROR(0xe000) AM_RAM
	AM_RANGE(0x1ff0, 0x1fff) AM_MIRROR(0xe000) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x2000, 0x2fff) AM_MIRROR(0x8000) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_MIRROR(0x8000) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_MIRROR(0x8000) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( rocktrv2_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_READWRITE(MRA8_RAM,pacman_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x4400, 0x47ff) AM_READWRITE(MRA8_RAM,pacman_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x4c00, 0x4fff) AM_RAM
	AM_RANGE(0x5000, 0x5000) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x5001, 0x5001) AM_WRITE(pacman_sound_enable_w)
	AM_RANGE(0x5003, 0x5003) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5007, 0x5007) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_WRITE(pacman_sound_w) AM_BASE(&pacman_soundregs)
	AM_RANGE(0x50c0, 0x50c0) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5fe0, 0x5fe3) AM_WRITE(rocktrv2_prot_data_w) AM_BASE(&rocktrv2_prot_data)
	AM_RANGE(0x5ff0, 0x5ff0) AM_WRITE(rocktrv2_question_bank_w)
	AM_RANGE(0x5000, 0x5000) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0x5040, 0x507f) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0x5080, 0x5080) AM_READ(input_port_2_r)	/* DSW1 */
	AM_RANGE(0x50c0, 0x50c0) AM_READ(input_port_3_r)	/* DSW2 */
	AM_RANGE(0x5fe0, 0x5fe0) AM_READ(rocktrv2_prot1_data_r)
	AM_RANGE(0x5fe4, 0x5fe4) AM_READ(rocktrv2_prot2_data_r)
	AM_RANGE(0x5fe8, 0x5fe8) AM_READ(rocktrv2_prot3_data_r)
	AM_RANGE(0x5fec, 0x5fec) AM_READ(rocktrv2_prot4_data_r)
	AM_RANGE(0x5fff, 0x5fff) AM_READ(input_port_3_r) /* DSW2 mirrored */
	AM_RANGE(0x6000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xffff) AM_READ(rocktrv2_question_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( bigbucks_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_READWRITE(MRA8_RAM,pacman_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x4400, 0x47ff) AM_READWRITE(MRA8_RAM,pacman_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x4c00, 0x4fff) AM_RAM
	AM_RANGE(0x5000, 0x5000) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x5001, 0x5001) AM_WRITE(pacman_sound_enable_w)
	AM_RANGE(0x5003, 0x5003) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5007, 0x5007) AM_WRITE(MWA8_NOP) /*?*/
	AM_RANGE(0x5040, 0x505f) AM_WRITE(pacman_sound_w) AM_BASE(&pacman_soundregs)
	AM_RANGE(0x50c0, 0x50c0) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x503f) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0x5040, 0x507f) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0x5080, 0x50bf) AM_READ(input_port_2_r)	/* DSW1 */
	AM_RANGE(0x50c0, 0x50ff) AM_READ(input_port_3_r)	/* DSW2 */
	AM_RANGE(0x5100, 0x5100) AM_WRITE(MWA8_NOP) /*?*/
	AM_RANGE(0x6000, 0x6000) AM_WRITE(bigbucks_bank_w)
	AM_RANGE(0x8000, 0x9fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( mschamp_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READWRITE(MRA8_BANK1,MWA8_ROM)
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_READWRITE(MRA8_RAM,pacman_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0xa000) AM_READWRITE(pacman_read_nop,MWA8_NOP)
	AM_RANGE(0x4c00, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_WRITE(pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP) // AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITE(MWA8_NOP) // AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_WRITE(pacman_sound_w) AM_BASE(&pacman_soundregs)
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram_2)
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ(input_port_2_r)	/* DSW1 */
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ(input_port_3_r)	/* DSW2 */
	AM_RANGE(0x8000, 0xbfff) AM_READWRITE(MRA8_BANK2,MWA8_ROM)
ADDRESS_MAP_END


/*************************************
 *
 *  Main CPU port handlers
 *
 *************************************/

static ADDRESS_MAP_START( writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(pacman_interrupt_vector_w)	/* Pac-Man only */
ADDRESS_MAP_END

static ADDRESS_MAP_START( vanvan_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x01, 0x01) AM_WRITE(SN76496_0_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(SN76496_1_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dremshpr_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x06, 0x06) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(AY8910_control_port_0_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( piranha_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(piranha_interrupt_vector_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( nmouse_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(nmouse_interrupt_vector_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( theglobp_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0xff) AM_READ(theglobp_decrypt_rom)	/* Switch protection logic */
ADDRESS_MAP_END

static ADDRESS_MAP_START( acitya_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0xff) AM_READ(acitya_decrypt_rom) /* Switch protection logic */
ADDRESS_MAP_END

static ADDRESS_MAP_START( mschamp_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_READ(mschamp_kludge_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bigbucks_readport, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0xffff) AM_READ(bigbucks_question_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( drivfrcp_readport, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0x00) AM_READ(MRA8_NOP)
	AM_RANGE(0x01, 0x01) AM_READ(drivfrcp_port1_r)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(input_port_3_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( _8bpm_readport, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0x00) AM_READ(MRA8_NOP)
	AM_RANGE(0x01, 0x01) AM_READ(_8bpm_port1_r)
	AM_RANGE(0xe0, 0xe0) AM_READ(MRA8_NOP)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(input_port_3_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( porky_readport, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x01, 0x01) AM_READ(porky_port1_r)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(input_port_3_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( s2650games_writeport, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_WRITE(SN76496_0_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

INPUT_PORTS_START( pacman )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )	PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )	PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )	PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )	PORT_PLAYER(1) PORT_4WAY
	PORT_DIPNAME(0x10, 0x10, "Rack Test (Cheat)" )	PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )	PORT_PLAYER(2) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )	PORT_PLAYER(2) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )	PORT_PLAYER(2) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )	PORT_PLAYER(2) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME(0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(   0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Cocktail ) )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Ghost Names" )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Alternate ) )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("FAKE")
	/* This fake input port is used to get the status of the fire button */
	/* and activate the speedup cheat if it is. */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME( "2x Speed (Cheat)" )
	PORT_DIPNAME( 0x06, 0x00, "2x Speed Cheat" )
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPSETTING(    0x02, "Enabled Always" )
	PORT_DIPSETTING(    0x04, "Enabled with Button" )
INPUT_PORTS_END


/* Ms. Pac-Man input ports are identical to Pac-Man, the only difference is */
/* the missing Ghost Names dip switch. */
INPUT_PORTS_START( mspacman )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT(    0x10, 0x10, IPT_DIPSWITCH_NAME ) PORT_NAME("Rack Test (Cheat)") PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("FAKE")
	/* This fake input port is used to get the status of the fire button */
	/* and activate the speedup cheat if it is. */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME( "2x Speed (Cheat)" )
	PORT_DIPNAME( 0x06, 0x00, "2x Speed Cheat" )
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPSETTING(    0x02, "Enabled Always" )
	PORT_DIPSETTING(    0x04, "Enabled with Button" )
INPUT_PORTS_END

INPUT_PORTS_START( mspactwin )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_DIPNAME( 0x10, 0x10, "Speed" ) //Jama
	PORT_DIPSETTING(    0x10, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME(0x80,  0x80, "Rack Test (Cheat)" )	PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* multiplexed player inputs */
	PORT_START	/* P1 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, "Free Play (Invalid)" ) /* causes watchdog reset at title screen, see comments above */
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, "None"  )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* Same as 'mspacman', but no fake input port */
INPUT_PORTS_START( mspacpls )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT(    0x10, 0x10, IPT_DIPSWITCH_NAME ) PORT_NAME("Rack Test (Cheat)") PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )	/* Also invincibility when playing */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )	/* Also speed-up when playing */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( mschamp )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT(    0x10, 0x10, IPT_DIPSWITCH_NAME ) PORT_NAME("Rack Test (Cheat)") PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("GAME")
	PORT_DIPNAME( 0x01, 0x01, "Game" )
	PORT_DIPSETTING(    0x01, "Champion Edition" )
	PORT_DIPSETTING(    0x00, "Super Zola Pac Gal" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( maketrax )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection */

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPNAME( 0x10, 0x10, "First Pattern" )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Teleport Holes" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
 	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection */

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( crush4 )
	PORT_INCLUDE( maketrax )

	PORT_START_TAG("GAME")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) // always select 2nd part of code
INPUT_PORTS_END

INPUT_PORTS_START( korosuke )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection */

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPNAME( 0x10, 0x10, "First Pattern" )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Teleport Holes" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
 	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection */

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( mbrush )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection in Make Trax */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection in Make Trax */

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection in Make Trax */

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( paintrlr )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection in Make Trax */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection in Make Trax */

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection in Make Trax */

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( ponpoko )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	/* The 2nd player controls are used even in upright mode */
	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "10000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x03, "50000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW 2")
	PORT_DIPNAME( 0x0f, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, "A 3/1 B 3/1" )
	PORT_DIPSETTING(    0x0e, "A 3/1 B 1/2" )
	PORT_DIPSETTING(    0x0f, "A 3/1 B 1/4" )
	PORT_DIPSETTING(    0x02, "A 2/1 B 2/1" )
	PORT_DIPSETTING(    0x0d, "A 2/1 B 1/1" )
	PORT_DIPSETTING(    0x07, "A 2/1 B 1/3" )
	PORT_DIPSETTING(    0x0b, "A 2/1 B 1/5" )
	PORT_DIPSETTING(    0x0c, "A 2/1 B 1/6" )
	PORT_DIPSETTING(    0x01, "A 1/1 B 1/1" )
	PORT_DIPSETTING(    0x06, "A 1/1 B 4/5" )
	PORT_DIPSETTING(    0x05, "A 1/1 B 2/3" )
	PORT_DIPSETTING(    0x0a, "A 1/1 B 1/3" )
	PORT_DIPSETTING(    0x08, "A 1/1 B 1/5" )
	PORT_DIPSETTING(    0x09, "A 1/1 B 1/6" )
	PORT_DIPSETTING(    0x03, "A 1/2 B 1/2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  /* Most likely unused */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  /* Most likely unused */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  /* Most likely unused */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


INPUT_PORTS_START( eyes )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "50000" )
	PORT_DIPSETTING(    0x20, "75000" )
	PORT_DIPSETTING(    0x10, "100000" )
	PORT_DIPSETTING(    0x00, "125000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  /* Not accessed */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( mrtnt )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "75000" )
	PORT_DIPSETTING(    0x20, "100000" )
	PORT_DIPSETTING(    0x10, "125000" )
	PORT_DIPSETTING(    0x00, "150000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( lizwiz )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "75000" )
	PORT_DIPSETTING(    0x20, "100000" )
	PORT_DIPSETTING(    0x10, "125000" )
	PORT_DIPSETTING(    0x00, "150000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( theglobp )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x14, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x0c, "Difficult" )
	PORT_DIPSETTING(    0x08, "Very Difficult" )
	PORT_DIPSETTING(    0x04, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( vanvan )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "20k and 100k" )
	PORT_DIPSETTING(    0x04, "40k and 140k" )
	PORT_DIPSETTING(    0x00, "70k and 200k" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )

	/* When all DSW2 are ON, there is no sprite collision detection */
	PORT_START_TAG("DSW 2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT(    0x02, 0x00, IPT_DIPSWITCH_NAME ) PORT_NAME("Invulnerability (Cheat)") PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )		/* Missile effect */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )		/* Killer car is destroyed */
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )			/* Killer car is not destroyed */
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


INPUT_PORTS_START( vanvank )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "20k and 100k" )
	PORT_DIPSETTING(    0x04, "40k and 140k" )
	PORT_DIPSETTING(    0x00, "70k and 200k" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_3C ) )

	/* When all DSW2 are ON, there is no sprite collision detection */
	PORT_START_TAG("DSW 2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT(    0x02, 0x00, IPT_DIPSWITCH_NAME ) PORT_NAME("Invulnerability (Cheat)") PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )		/* Missile effect */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )		/* Killer car is destroyed */
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )			/* Killer car is not destroyed */
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


INPUT_PORTS_START( dremshpr )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x00, "70000" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )

	PORT_START_TAG("DSW 2")
	/* turning this on crashes puts the */
	/* emulated machine in an infinite loop once in a while */
#if 0
	PORT_DIPNAME(    0x01, 0x00,"Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0xff, 0x00, DEF_STR( Unused ) )
#endif
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( alibaba )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT(0x10, 0x10, IPT_DIPSWITCH_NAME ) PORT_NAME("Rack Test (Cheat)") PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


INPUT_PORTS_START( jumpshot )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_PLAYER(2)
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x01, "Time"  )
//  PORT_DIPSETTING(    0x00,  "2 Minutes"  )
	PORT_DIPSETTING(    0x02,  "2 Minutes" )
	PORT_DIPSETTING(    0x03,  "3 Minutes" )
	PORT_DIPSETTING(    0x01,  "4 Minutes"  )
	PORT_DIPNAME( 0x04, 0x04, "Player Skin Tone" )
	PORT_DIPSETTING(    0x04, "Lighter" )
	PORT_DIPSETTING(    0x00, "Darker" )
	PORT_DIPNAME( 0x08, 0x08, "Player Skin Tone" )
	PORT_DIPSETTING(    0x08, "Lighter" )
	PORT_DIPSETTING(    0x00, "Darker" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "2 Players Game" )
	PORT_DIPSETTING(    0x20, "1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Credits" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( jumpshtp )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_PLAYER(2)
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x01, "Time Limit"  )
//  PORT_DIPSETTING(    0x00,  "Short"  )
	PORT_DIPSETTING(    0x02,  "Short" )
	PORT_DIPSETTING(    0x03,  "Average" )
	PORT_DIPSETTING(    0x01,  "Above Average"   )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Players Skin Tone" )
	PORT_DIPSETTING(    0x08, "Lighter" )
	PORT_DIPSETTING(    0x00, "Darker" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "2 Players Game" )
	PORT_DIPSETTING(    0x20, "1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Credits" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( shootbul )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x0f, 0x0f, IPT_TRACKBALL_X  ) PORT_SENSITIVITY(50) PORT_KEYDELTA(25)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x0f, 0x0f, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_REVERSE
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x07, 0x07, "Time"  )
	PORT_DIPSETTING(    0x01, "Short")
	PORT_DIPSETTING(    0x07, "Average" )
	PORT_DIPSETTING(    0x03, "Long" )
	PORT_DIPSETTING(    0x05, "Longer" )
	PORT_DIPSETTING(    0x06, "Longest" )
	PORT_DIPNAME( 0x08, 0x08, "Title Page Sounds"  )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/* New Atlantic City Action / Board Walk Casino Inputs
   Annoyingly enough, you can't get into service mode on bwcasino if the
   cocktail mode is set. To test player 2's inputs, select Upright Mode on
   the dipswitches, and enter test mode. Now select cocktail mode and you
   can test everything. Wierd. */

INPUT_PORTS_START( bwcasino )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_COCKTAIL PORT_PLAYER(2)

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1e, 0x1e, "Hands per Game" )
	PORT_DIPSETTING(    0x1e, "3" )
	PORT_DIPSETTING(    0x1c, "4" )
	PORT_DIPSETTING(    0x1a, "5" )
	PORT_DIPSETTING(    0x18, "6" )
	PORT_DIPSETTING(    0x16, "7" )
	PORT_DIPSETTING(    0x14, "8" )
	PORT_DIPSETTING(    0x12, "9" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x0e, "11" )
	PORT_DIPSETTING(    0x0c, "12" )
	PORT_DIPSETTING(    0x0a, "13" )
	PORT_DIPSETTING(    0x08, "14" )
	PORT_DIPSETTING(    0x06, "15" )
	PORT_DIPSETTING(    0x04, "16" )
	PORT_DIPSETTING(    0x02, "17" )
	PORT_DIPSETTING(    0x00, "18" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/* ATLANTIC CITY ACTION (acitya)
   Unlike "Boardwalk Casino", "Atlantic City Action" does not appear to
   have a cocktail mode, and uses service button connected differently to
   "Boardwalk" */

INPUT_PORTS_START( acitya )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1e, 0x1e, "Hands per Game" )
	PORT_DIPSETTING(    0x1e, "3" )
	PORT_DIPSETTING(    0x1c, "4" )
	PORT_DIPSETTING(    0x1a, "5" )
	PORT_DIPSETTING(    0x18, "6" )
	PORT_DIPSETTING(    0x16, "7" )
	PORT_DIPSETTING(    0x14, "8" )
	PORT_DIPSETTING(    0x12, "9" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x0e, "11" )
	PORT_DIPSETTING(    0x0c, "12" )
	PORT_DIPSETTING(    0x0a, "13" )
	PORT_DIPSETTING(    0x08, "14" )
	PORT_DIPSETTING(    0x06, "15" )
	PORT_DIPSETTING(    0x04, "16" )
	PORT_DIPSETTING(    0x02, "17" )
	PORT_DIPSETTING(    0x00, "18" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( nmouse )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT(    0x10, 0x10, IPT_DIPSWITCH_NAME ) PORT_NAME("Rack Test (Cheat)") PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME(0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(   0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Cocktail ) )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "10000" )
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

INPUT_PORTS_END

INPUT_PORTS_START( woodpek )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT(    0x10, 0x10, IPT_DIPSWITCH_NAME ) PORT_NAME("Rack Test (Cheat)") PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "10000" )
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( bigbucks )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	 ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x10, "Enable Category Adult Affairs" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x01, 0x00, "Time to bet / answer" )
	PORT_DIPSETTING(    0x00, "15 sec. / 10 sec." )
	PORT_DIPSETTING(    0x01, "20 sec. / 15 sec." )
	PORT_DIPNAME( 0x02, 0x00, "Continue if player busts" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Show correct answer" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START_TAG("DSW 2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( drivfrcp )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START_TAG("Sense")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END


INPUT_PORTS_START( 8bpm )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Start 1 / P1 Button 1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Start 2 / P1 Button 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START_TAG("Sense")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END


INPUT_PORTS_START( porky )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START_TAG("Sense")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END


INPUT_PORTS_START( rocktrv2 )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	 ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW 1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x1c, 0x10, "Questions Per Game" )
	PORT_DIPSETTING(    0x1c, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x14, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x04, "8" )
	PORT_DIPSETTING(    0x00, "9" )
	PORT_DIPNAME( 0x60, 0x60, "Clock Speed" )
	PORT_DIPSETTING(    0x60, "Beginner" )
	PORT_DIPSETTING(    0x40, "Intermed" )
	PORT_DIPSETTING(    0x20, "Pro" )
	PORT_DIPSETTING(    0x00, "Super - Pro" )
	PORT_DIPNAME( 0x80, 0x80,"Freeze Image" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW 2")
	PORT_DIPNAME( 0x01, 0x01, "Mode" )
	PORT_DIPSETTING(    0x01, "Amusement" )
	PORT_DIPSETTING(    0x00, "Credit" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "K.O. Switch" )
	PORT_DIPSETTING(    0x04, "Auto" )
	PORT_DIPSETTING(    0x00, "Manual" )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x70, "10000" )
	PORT_DIPSETTING(    0x60, "17500" )
	PORT_DIPSETTING(    0x50, "25000" )
	PORT_DIPSETTING(    0x40, "32500" )
	PORT_DIPSETTING(    0x30, "40000" )
	PORT_DIPSETTING(    0x20, "47500" )
	PORT_DIPSETTING(    0x10, "55000" )
	PORT_DIPSETTING(    0x00, "62500" )
	PORT_DIPNAME( 0x80, 0x80, "Music" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout tilelayout =
{
	8,8,	/* 8*8 characters */
    RGN_FRAC(1,2),    /* 256 characters */
    2,  /* 2 bits per pixel */
    { 0, 4 },   /* the two bitplanes for 4 pixels are packed into one byte */
    { 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 }, /* bits are packed in groups of four */
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
    16*8    /* every char takes 16 bytes */
};


static const gfx_layout spritelayout =
{
	16,16,	/* 16*16 sprites */
	RGN_FRAC(1,2),	/* 64 sprites */
	2,	/* 2 bits per pixel */
	{ 0, 4 },	/* the two bitplanes for 4 pixels are packed into one byte */
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8	/* every sprite takes 64 bytes */
};


static const gfx_layout crush4_tilelayout =
{
	8,8, /* 8*8 characters */
    RGN_FRAC(1,4),
    2,  /* 2 bits per pixel */
    { RGN_FRAC(1,2), RGN_FRAC(0,2)+4 },
    { 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 }, /* bits are packed in groups of four */
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
    16*8    /* every char takes 16 bytes */
};

static const gfx_layout crush4_spritelayout =
{
	16,16, /* 16*16 sprites */
	RGN_FRAC(1,4),
	2, 	/* 2 bits per pixel */
	{ RGN_FRAC(1,2), RGN_FRAC(0,2)+4 },
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8 	/* every sprite takes 64 bytes */
};


static const gfx_decode gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0x0000, &tilelayout,   0, 128 },
	{ REGION_GFX1, 0x1000, &spritelayout, 0, 128 },
	{ -1 } /* end of array */
};


static const gfx_decode s2650games_gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0x0000, &tilelayout,   0, 128 },
	{ REGION_GFX1, 0x4000, &spritelayout, 0, 128 },
	{ -1 } /* end of array */
};

static const gfx_decode crush4_gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0x0000, &crush4_tilelayout,   0, 128 },
	{ REGION_GFX1, 0x1000, &crush4_spritelayout, 0, 128 },
	{ -1 } /* end of array */
};

/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static struct namco_interface namco_interface =
{
	3,			/* number of voices */
	REGION_SOUND1	/* memory region */
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( pacman )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", Z80, 18432000/6)
	MDRV_CPU_PROGRAM_MAP(pacman_map,0)
	MDRV_CPU_IO_MAP(0,writeport)
	MDRV_CPU_VBLANK_INT(pacman_interrupt,2)
	MDRV_WATCHDOG_VBLANK_INIT(16)

	MDRV_FRAMES_PER_SECOND(60.606060)
	MDRV_VBLANK_DURATION(DEFAULT_REAL_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(36*8, 28*8)
	MDRV_VISIBLE_AREA(0*8, 36*8-1, 0*8, 28*8-1)
	MDRV_GFXDECODE(gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(32)
	MDRV_COLORTABLE_LENGTH(128*4)

	MDRV_PALETTE_INIT(pacman)
	MDRV_VIDEO_START(pacman)
	MDRV_VIDEO_UPDATE(pacman)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD_TAG("namco", NAMCO, 3072000/32)
	MDRV_SOUND_CONFIG(namco_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( piranha )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(pacman)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_IO_MAP(0,piranha_writeport)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( nmouse )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(pacman)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_IO_MAP(0,nmouse_writeport)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( mspacman )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(pacman)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mspacman_map,0)

	MDRV_MACHINE_RESET(mspacman)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( mspactwin )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(pacman)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mspactwin_readmem,mspactwin_writemem)
	MDRV_CPU_VBLANK_INT(mspactwin_interrupt,1)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( woodpek )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(pacman)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(woodpek_map,0)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( alibaba )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(pacman)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(alibaba_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( dremshpr )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(pacman)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(dremshpr_map,0)
	MDRV_CPU_IO_MAP(0,dremshpr_writeport)
	MDRV_CPU_VBLANK_INT(nmi_line_pulse,1)

	/* sound hardware */
	MDRV_SOUND_REPLACE("namco", AY8910, 14318000/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( theglobp )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(pacman)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(epos_map,0)
	MDRV_CPU_IO_MAP(theglobp_readport,writeport)

	MDRV_MACHINE_RESET(theglobp)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( acitya )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(pacman)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(epos_map,0)
	MDRV_CPU_IO_MAP(acitya_readport,writeport)

	MDRV_MACHINE_RESET(acitya)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( vanvan )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(pacman)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(vanvan_map,0)
	MDRV_CPU_IO_MAP(0,vanvan_writeport)
	MDRV_CPU_VBLANK_INT(nmi_line_pulse,1)

	/* video hardware */
	MDRV_VISIBLE_AREA(2*8, 34*8-1, 0*8, 28*8-1)

	/* sound hardware */
	MDRV_SOUND_REPLACE("namco", SN76496, 1789750)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
	MDRV_SOUND_ADD(SN76496, 1789750)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( bigbucks )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(pacman)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(bigbucks_map,0)
	MDRV_CPU_IO_MAP(bigbucks_readport,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,20)

	MDRV_VISIBLE_AREA(0*8, 36*8-1, 0*8, 28*8-1)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( s2650games )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(pacman)

	MDRV_CPU_REMOVE("main")
	MDRV_CPU_ADD_TAG("main", S2650, 18432000/6/2)	/* ??? */
	MDRV_CPU_PROGRAM_MAP(s2650games_map,0)
	MDRV_CPU_VBLANK_INT(s2650_interrupt,1)

	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(s2650games_gfxdecodeinfo)

	MDRV_VBLANK_DURATION(DEFAULT_REAL_60HZ_VBLANK_DURATION)

	MDRV_VIDEO_START(s2650games)
	MDRV_VIDEO_UPDATE(s2650games)

	/* sound hardware */
	MDRV_SOUND_REPLACE("namco", SN76496, 18432000/6/2/3)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
	MDRV_SOUND_ADD(SN76496, 18432000/6/2/3)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( drivfrcp )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(s2650games)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_IO_MAP(drivfrcp_readport,s2650games_writeport)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( 8bpm )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(s2650games)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_IO_MAP(_8bpm_readport,s2650games_writeport)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( porky )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(s2650games)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_IO_MAP(porky_readport,s2650games_writeport)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( rocktrv2 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(pacman)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(rocktrv2_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_VISIBLE_AREA(0*8, 36*8-1, 0*8, 28*8-1)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( mschamp )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(pacman)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mschamp_map,0)
	MDRV_CPU_IO_MAP(mschamp_readport,writeport)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_MACHINE_RESET(mschamp)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( crush4 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(mschamp)

	MDRV_GFXDECODE(crush4_gfxdecodeinfo)
MACHINE_DRIVER_END

/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( puckman )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "namcopac.6e",  0x0000, 0x1000, CRC(fee263b3) SHA1(87117ba5082cd7a615b4ec7c02dd819003fbd669) )
	ROM_LOAD( "namcopac.6f",  0x1000, 0x1000, CRC(39d1fc83) SHA1(326dbbf94c6fa2e96613dedb53702f8832b47d59) )
	ROM_LOAD( "namcopac.6h",  0x2000, 0x1000, CRC(02083b03) SHA1(7e1945f6eb51f2e51806d0439f975f7a2889b9b8) )
	ROM_LOAD( "namcopac.6j",  0x3000, 0x1000, CRC(7a36fe55) SHA1(01b4c38108d9dc4e48da4f8d685248e1e6821377) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pacman.5e",    0x0000, 0x1000, CRC(0c944964) SHA1(06ef227747a440831c9a3a613b76693d52a2f0a9) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( puckmanf )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "namcopac.6e",  0x0000, 0x1000, CRC(fee263b3) SHA1(87117ba5082cd7a615b4ec7c02dd819003fbd669) )
	ROM_LOAD( "nampfast.6f",  0x1000, 0x1000, CRC(51b38db9) SHA1(0a796f93462aec4758c2aa1c1f34cd05bb10a178) )
	ROM_LOAD( "namcopac.6h",  0x2000, 0x1000, CRC(02083b03) SHA1(7e1945f6eb51f2e51806d0439f975f7a2889b9b8) )
	ROM_LOAD( "namcopac.6j",  0x3000, 0x1000, CRC(7a36fe55) SHA1(01b4c38108d9dc4e48da4f8d685248e1e6821377) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pacman.5e",    0x0000, 0x1000, CRC(0c944964) SHA1(06ef227747a440831c9a3a613b76693d52a2f0a9) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( puckmod )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "namcopac.6e",  0x0000, 0x1000, CRC(fee263b3) SHA1(87117ba5082cd7a615b4ec7c02dd819003fbd669) )
	ROM_LOAD( "namcopac.6f",  0x1000, 0x1000, CRC(39d1fc83) SHA1(326dbbf94c6fa2e96613dedb53702f8832b47d59) )
	ROM_LOAD( "namcopac.6h",  0x2000, 0x1000, CRC(02083b03) SHA1(7e1945f6eb51f2e51806d0439f975f7a2889b9b8) )
	ROM_LOAD( "npacmod.6j",   0x3000, 0x1000, CRC(7d98d5f5) SHA1(39939bcd6fb785d0d06fd29f0287158ab1267dfc) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pacman.5e",    0x0000, 0x1000, CRC(0c944964) SHA1(06ef227747a440831c9a3a613b76693d52a2f0a9) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( puckmana )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "prg7",         0x3000, 0x0800, CRC(b6289b26) SHA1(d249fa9cdde774d5fee7258147cd25fa3f4dc2b3) )
	ROM_LOAD( "prg8",         0x3800, 0x0800, CRC(17a88c13) SHA1(eb462de79f49b7aa8adb0cc6d31535b10550c0ce) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "chg1",         0x0000, 0x0800, CRC(2066a0b7) SHA1(6d4ccc27d6be185589e08aa9f18702b679e49a4a) )
	ROM_LOAD( "chg2",         0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( pacman )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pacman.5e",    0x0000, 0x1000, CRC(0c944964) SHA1(06ef227747a440831c9a3a613b76693d52a2f0a9) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( pacmanf )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacfast.6f",   0x1000, 0x1000, CRC(720dc3ee) SHA1(7224d7acfa0144b681c71d7734a7337189835361) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pacman.5e",    0x0000, 0x1000, CRC(0c944964) SHA1(06ef227747a440831c9a3a613b76693d52a2f0a9) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( pacmod )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "pacmanh.6e",   0x0000, 0x1000, CRC(3b2ec270) SHA1(48fc607ad8d86249948aa377c677ae44bb8ad3da) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "pacmanh.6h",   0x2000, 0x1000, CRC(18811780) SHA1(ab34acaa3dbcafe8b20c2197f36641e471984487) )
	ROM_LOAD( "pacmanh.6j",   0x3000, 0x1000, CRC(5c96a733) SHA1(22ae15a6f088e7296f77c7487a350c4bd102f00e) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pacmanh.5e",   0x0000, 0x1000, CRC(299fb17a) SHA1(ad97adc2122482a9018bacd137df9d8f409ddf85) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( hangly )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "hangly.6e",    0x0000, 0x1000, CRC(5fe8610a) SHA1(d63eaebd85e10aa6c27bb7f47642dd403eeb6934) )
	ROM_LOAD( "hangly.6f",    0x1000, 0x1000, CRC(73726586) SHA1(cedddc5194589039dd8b64f07ab6320d7d4f55f9) )
	ROM_LOAD( "hangly.6h",    0x2000, 0x1000, CRC(4e7ef99f) SHA1(bd42e68b29b4d654dc817782ba00db69b7d2dfe2) )
	ROM_LOAD( "hangly.6j",    0x3000, 0x1000, CRC(7f4147e6) SHA1(0a7ac0e59d4d26fe52a2f4196c9f19e5ab677c87) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pacman.5e",    0x0000, 0x1000, CRC(0c944964) SHA1(06ef227747a440831c9a3a613b76693d52a2f0a9) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( hangly2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "hangly.6e",    0x0000, 0x1000, CRC(5fe8610a) SHA1(d63eaebd85e10aa6c27bb7f47642dd403eeb6934) )
	ROM_LOAD( "hangly2.6f",   0x1000, 0x0800, CRC(5ba228bb) SHA1(b0e902cdf98bee72d6ec8069eec96adce3245074) )
	ROM_LOAD( "hangly2.6m",   0x1800, 0x0800, CRC(baf5461e) SHA1(754586a6449fd54a342f260e572c1cd60ab70815) )
	ROM_LOAD( "hangly.6h",    0x2000, 0x1000, CRC(4e7ef99f) SHA1(bd42e68b29b4d654dc817782ba00db69b7d2dfe2) )
	ROM_LOAD( "hangly2.6j",   0x3000, 0x0800, CRC(51305374) SHA1(6197b606a0eedb11135d9f4f7a89aecc23fb2d33) )
	ROM_LOAD( "hangly2.6p",   0x3800, 0x0800, CRC(427c9d4d) SHA1(917bc3d571cbdd24d88327ecabfb5b3f6d39af0a) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pacmanh.5e",   0x0000, 0x1000, CRC(299fb17a) SHA1(ad97adc2122482a9018bacd137df9d8f409ddf85) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END

ROM_START( hangly3 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "hm1.6e",   0x0000, 0x0800, CRC(9d027c4a) SHA1(88e094880057451a75cdc2ce9477403021813982) )
	ROM_LOAD( "hm5.6k",	  0x0800, 0x0800, CRC(194c7189) SHA1(fd423bac2810015313841c7b935054565390fbd0) )
	ROM_LOAD( "hangly2.6f",   0x1000, 0x0800, CRC(5ba228bb) SHA1(b0e902cdf98bee72d6ec8069eec96adce3245074) ) /* hm2.6f */
	ROM_LOAD( "hangly2.6m",   0x1800, 0x0800, CRC(baf5461e) SHA1(754586a6449fd54a342f260e572c1cd60ab70815) ) /* hm6.6m */
	ROM_LOAD( "hm3.6h",   0x2000, 0x0800, CRC(08419c4a) SHA1(7e5001adad401080c788737c1d2349f218750442) )
	ROM_LOAD( "hm7.6n",   0x2800, 0x0800, CRC(ab74b51f) SHA1(1bce8933ed7807eb7aca9670df8994f8d1a8b5b7) )
	ROM_LOAD( "hm4.6j",   0x3000, 0x0800, CRC(5039b082) SHA1(086a6ac4742734167d283b1121fce29d8ac4a6cd) )
	ROM_LOAD( "hm8.6p",   0x3800, 0x0800, CRC(931770d7) SHA1(78fcf88e07ec5126c12c3297b62ca388809e947c) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "hm9.5e", 	0x0000, 0x0800, CRC(5f4be3cc) SHA1(eeb0e1e44549b99eab481d9ac016b4359e19fe30) )
	ROM_LOAD( "hm11.5h",    0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "hm10.5f", 	0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "hm12.5j", 	0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END

ROM_START( puckmanh )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "pm01.6e",      0x0000, 0x1000, CRC(5fe8610a) SHA1(d63eaebd85e10aa6c27bb7f47642dd403eeb6934) )
	ROM_LOAD( "pm02.6f",      0x1000, 0x1000, CRC(61d38c6c) SHA1(1406aacdc9c8a3776e5853d214380ad3124408f4) )
	ROM_LOAD( "pm03.6h",      0x2000, 0x1000, CRC(4e7ef99f) SHA1(bd42e68b29b4d654dc817782ba00db69b7d2dfe2) )
	ROM_LOAD( "pm04.6j",      0x3000, 0x1000, CRC(8939ddd2) SHA1(cf769bb34f711cfd0ee75328cd5dc07442f88607) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pm9.5e",       0x0000, 0x0800, CRC(2229ab07) SHA1(56000ed5009ae60c7f0498b5cac1b06da6ae270e) )
	ROM_LOAD( "pm11.5h",      0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "pm10.5f",      0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "pm12.5j",      0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( newpuckx )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "puckman.6e",   0x0000, 0x1000, CRC(a8ae23c5) SHA1(1481a4f083b563350744f9d25b1bcd28073875d6) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "puckman.6h",   0x2000, 0x1000, CRC(197443f8) SHA1(119aab12a9e1052c7b9a1f81e563740b41429a8c) )
	ROM_LOAD( "puckman.6j",   0x3000, 0x1000, CRC(2e64a3ba) SHA1(f86a921173f32211b18d023c2701664d13ae23be) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pacman.5e",    0x0000, 0x1000, CRC(0c944964) SHA1(06ef227747a440831c9a3a613b76693d52a2f0a9) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( pacheart )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )     /* 64k for code */
	ROM_LOAD( "1.6e",         0x0000, 0x0800, CRC(d844b679) SHA1(c4486198b3126bb8e05a308c53787e51065f77ae) )
	ROM_LOAD( "pacheart.pg2", 0x0800, 0x0800, CRC(b9152a38) SHA1(b6be2cb6bc7dd123503eb6bf1165dd1c99456813) )
	ROM_LOAD( "2.6f",         0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "pacheart.pg4", 0x1800, 0x0800, CRC(842d6574) SHA1(40e32d09cc8d701eb318716493a68cf3f95d3d6d) )
	ROM_LOAD( "3.6h",         0x2000, 0x0800, CRC(9045a44c) SHA1(a97d7016effbd2ace9a7d92ceb04a6ce18fb42f9) )
	ROM_LOAD( "7.6n",         0x2800, 0x0800, CRC(888f3c3e) SHA1(c2b5917bf13071131dd53ea76f0da86706db2d80) )
	ROM_LOAD( "pacheart.pg7", 0x3000, 0x0800, CRC(f5265c10) SHA1(9a320790d7a03fd6192a92d30b3e9c754bbc6a9d) )
	ROM_LOAD( "pacheart.pg8", 0x3800, 0x0800, CRC(1a21a381) SHA1(d5367a327d19fb57ba5e484bd4fda1b10953c040) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pacheart.ch1", 0x0000, 0x0800, CRC(c62bbabf) SHA1(f6f28ae33c2ab274105283b22b49ad243780a95e) )
	ROM_LOAD( "chg2",         0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "pacheart.ch3", 0x1000, 0x0800, CRC(ca8c184c) SHA1(833aa845824ed80777b62f03df36a920ad7c3656) )
	ROM_LOAD( "pacheart.ch4", 0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END


ROM_START( joyman )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )     /* 64k for code */
	ROM_LOAD( "1.6e",         0x0000, 0x0800, CRC(d844b679) SHA1(c4486198b3126bb8e05a308c53787e51065f77ae) )
	ROM_LOAD( "5.6k",         0x0800, 0x0800, CRC(ab9c8f29) SHA1(3753b8609c30d85d89acf745cf9303b77be440fd) )
	ROM_LOAD( "2.6f",         0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "6.6m",         0x1800, 0x0800, CRC(b3c8d32e) SHA1(8b336fca1300820308cd5c4efc60bf2ba4199302) )
	ROM_LOAD( "3.6h",         0x2000, 0x0800, CRC(9045a44c) SHA1(a97d7016effbd2ace9a7d92ceb04a6ce18fb42f9) )
	ROM_LOAD( "7.6n",         0x2800, 0x0800, CRC(888f3c3e) SHA1(c2b5917bf13071131dd53ea76f0da86706db2d80) )
	ROM_LOAD( "4.6j",         0x3000, 0x0800, CRC(00b553f8) SHA1(57f2e4a6da9f00935fead447b2123a8b95e5d672) )
	ROM_LOAD( "8.6p",         0x3800, 0x0800, CRC(5d5ce992) SHA1(ced7ed39cfc7ec7b2c0459e275577976109ee82f) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "9.5e",  0x0000, 0x0800, CRC(39b557bc) SHA1(0f602ec84cb25fced89699e430b95b5ae93c83bd) )
	ROM_LOAD( "11.5h", 0x0800, 0x0800, CRC(33e0289e) SHA1(c1b910bdc61e560a8c34298deb11401f718e7330) )
	ROM_LOAD( "10.5f", 0x1000, 0x0800, CRC(338771a6) SHA1(7cd68cc428986255d0de29aae894900519e7fda5) )
	ROM_LOAD( "12.5j", 0x1800, 0x0800, CRC(f4f0add5) SHA1(d71c54ef55a755ec1316623d183b4f615ef7c055) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END


ROM_START( piranha )
	ROM_REGION( 0x10000, REGION_CPU1,0 )     /* 64k for code */
	ROM_LOAD( "pir1.bin", 0x0000, 0x0800, CRC(69a3e6ea) SHA1(c54e5d039a03d3cbee7a5e21bf1e23f4fd913ea6) )
	ROM_LOAD( "pir5.bin", 0x0800, 0x0800, CRC(245e753f) SHA1(4c1183b8449e4e7995f81079953fe0e251251c60) )
	ROM_LOAD( "pir2.bin", 0x1000, 0x0800, CRC(62cb6954) SHA1(0e01c8463b130ab5518ce23368ad028c86cd0a32) )
	ROM_LOAD( "pir6.bin", 0x1800, 0x0800, CRC(cb0700bc) SHA1(1f5e91791ea25eb58d26b9627e98e0b6c1d9becf) )
	ROM_LOAD( "pir3.bin", 0x2000, 0x0800, CRC(843fbfe5) SHA1(6671a3c55ef70447f2a127438e0c39857f8bf6b1) )
	ROM_LOAD( "pir7.bin", 0x2800, 0x0800, CRC(73084d5e) SHA1(cb04a4c9dbf1672ddf478d2fe92b0ffd0159bb9e) )
	ROM_LOAD( "pir4.bin", 0x3000, 0x0800, CRC(4cdf6704) SHA1(97af8bbd08896dffd73e359ec46843dd673c4c9c) )
	ROM_LOAD( "pir8.bin", 0x3800, 0x0800, CRC(b86fedb3) SHA1(f5eaf7ccc1ecaa2417bcc077561efca8e7cb691a) )

	ROM_REGION( 0x2000, REGION_GFX1 , ROMREGION_DISPOSE)
	ROM_LOAD( "pir9.bin",  0x0000, 0x0800, CRC(0f19eb28) SHA1(0335189a06be01b97ca376d3682ed54df9b121e8) )
	ROM_LOAD( "pir11.bin", 0x0800, 0x0800, CRC(5f8bdabe) SHA1(eb6a0515a381a885b087d165aaefb0277a223715) )
	ROM_LOAD( "pir10.bin", 0x1000, 0x0800, CRC(d19399fb) SHA1(c0a75a08f77adb9d0010511c4b6ea99324c33c50) )
	ROM_LOAD( "pir12.bin", 0x1800, 0x0800, CRC(cfb4403d) SHA1(1642a4917be0621ebf5f705c7f68a2b75d1c78d3) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "piranha.4a", 0x0020, 0x0100, CRC(08c9447b) SHA1(5e4fbfcc7179fc4b1436af9bb709ffc381479315) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END

ROM_START( piranhao )
	ROM_REGION( 0x10000, REGION_CPU1,0 )     /* 64k for code */
	ROM_LOAD( "p1.bin", 0x0000, 0x0800, CRC(c6ce1bfc) SHA1(da145d67331cee292654a185fb09e773dd9d40cd) )
	ROM_LOAD( "p5.bin", 0x0800, 0x0800, CRC(a2655a33) SHA1(2253dcf5c8cbe278118aa1569cf456b13d8cf029) )
	ROM_LOAD( "pir2.bin", 0x1000, 0x0800, CRC(62cb6954) SHA1(0e01c8463b130ab5518ce23368ad028c86cd0a32) )
	ROM_LOAD( "pir6.bin", 0x1800, 0x0800, CRC(cb0700bc) SHA1(1f5e91791ea25eb58d26b9627e98e0b6c1d9becf) )
	ROM_LOAD( "pir3.bin", 0x2000, 0x0800, CRC(843fbfe5) SHA1(6671a3c55ef70447f2a127438e0c39857f8bf6b1) )
	ROM_LOAD( "pir7.bin", 0x2800, 0x0800, CRC(73084d5e) SHA1(cb04a4c9dbf1672ddf478d2fe92b0ffd0159bb9e) )
	ROM_LOAD( "p4.bin", 0x3000, 0x0800, CRC(9363a4d1) SHA1(4cb4a86d92a1f9bf233cac01aa266485a8bb7a34) )
	ROM_LOAD( "p8.bin", 0x3800, 0x0800, CRC(2769979c) SHA1(581592da26199b325de51791ddab66b474ab0413) )

	ROM_REGION( 0x2000, REGION_GFX1 , ROMREGION_DISPOSE )
	ROM_LOAD( "p9.bin",  0x0000, 0x0800, CRC(94eb7563) SHA1(c99741ce1aebdfb89628fbfaecf5ae6b2719a0ca) )
	ROM_LOAD( "p11.bin", 0x0800, 0x0800, CRC(a3606973) SHA1(72297e1a33102c6a48b4c65f2a0b9bfc75a2df36) )
	ROM_LOAD( "p10.bin", 0x1000, 0x0800, CRC(84165a2c) SHA1(95b24620fbf9bd0ec4dd2aeeb6d9305bd475dce2) )
	ROM_LOAD( "p12.bin", 0x1800, 0x0800, CRC(2699ba9e) SHA1(b91ff586defe65b200bea5ade7374c2c7579cd80) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "piranha.4a", 0x0020, 0x0100, CRC(08c9447b) SHA1(5e4fbfcc7179fc4b1436af9bb709ffc381479315) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END

ROM_START( piranhah )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "pr1.cpu",      0x0000, 0x1000, CRC(bc5ad024) SHA1(a3ed781b514a1068b24a7146a28f0a2adfaa2719) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "pr3.cpu",      0x2000, 0x1000, CRC(473c379d) SHA1(6e7985367c3e544b4cb98ba8291908df88eafe7f) )
	ROM_LOAD( "pr4.cpu",      0x3000, 0x1000, CRC(63fbf895) SHA1(d328bf3b8f307fb774614834edec211117148e64) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pr5.cpu",      0x0000, 0x0800, CRC(3fc4030c) SHA1(5e45f0c19cf96daa17afd2fa1c628d7ac7f4a79c) )
	ROM_LOAD( "pr7.cpu",      0x0800, 0x0800, CRC(30b9a010) SHA1(b0ba8b6cd430feb32d11d092e1959b9f5d240f1b) )
	ROM_LOAD( "pr6.cpu",      0x1000, 0x0800, CRC(f3e9c9d5) SHA1(709a75b2457f21f0f1a3d9e7f4c8579468ee5cad) )
	ROM_LOAD( "pr8.cpu",      0x1800, 0x0800, CRC(133d720d) SHA1(8af75ed9e115a996379acedd44d0c09332ec5a03) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END

ROM_START( abscam )
	ROM_REGION( 0x10000, REGION_CPU1,0 )     /* 64k for code */
	ROM_LOAD( "as0.bin", 0x0000, 0x0800, CRC(0b102302) SHA1(97f6399182db4f84efe482bf3a710aa45ca104ec) )
	ROM_LOAD( "as4.bin", 0x0800, 0x0800, CRC(3116a8ec) SHA1(259169bcc8fbe9fc73ca5100c3835a233351f530) )
	ROM_LOAD( "as1.bin", 0x1000, 0x0800, CRC(bc0281e0) SHA1(bcc6d63ede728d9b29f903489bfa80d94ec5cc00) )
	ROM_LOAD( "as5.bin", 0x1800, 0x0800, CRC(428ee2e8) SHA1(1477d1a86b32483ac0fdeea93512f517c9f66ce2) )
	ROM_LOAD( "as2.bin", 0x2000, 0x0800, CRC(e05d46ad) SHA1(87da57dbbe6ab5e1dd005fd68a982f1df917459c) )
	ROM_LOAD( "as6.bin", 0x2800, 0x0800, CRC(3ae9a8cb) SHA1(72896ad32cbdde90793788182958a943e35672f9) )
	ROM_LOAD( "as3.bin", 0x3000, 0x0800, CRC(b39eb940) SHA1(e144a1553c76ddee1c22ad1ed0cca241c2d03998) )
	ROM_LOAD( "as7.bin", 0x3800, 0x0800, CRC(16cf1c67) SHA1(0015fe64d476de87f1a030e7f2e735380dfcfd41) )

	ROM_REGION( 0x2000, REGION_GFX1 , ROMREGION_DISPOSE)
	ROM_LOAD( "as8.bin",  0x0000, 0x0800, CRC(61daabe5) SHA1(00503916d1d1011afe68898e3416718c0e63a298) )
	ROM_LOAD( "as10.bin", 0x0800, 0x0800, CRC(81d50c98) SHA1(6b61c666f68b5948e4facb8bac1378f986f993a7) )
	ROM_LOAD( "as9.bin", 0x1000, 0x0800, CRC(a3bd1613) SHA1(c59bb0a4d1fa5cbe596f41ee7b1a4a661ab5614b) )
	ROM_LOAD( "as11.bin", 0x1800, 0x0800, CRC(9d802b68) SHA1(4e8f37c2faedcfce91221a34c14f6490d578c80a) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "as4a.bin", 0x0020, 0x0100, CRC(1605b324) SHA1(336fce22caedbe69bcba9cea2b43e00f6f8e8067) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END


ROM_START( ctrpllrp )
	ROM_REGION( 0x10000, REGION_CPU1,0 )     /* 64k for code */
	ROM_LOAD( "c1.bin", 0x0000, 0x0800, CRC(9d027c4a) SHA1(88e094880057451a75cdc2ce9477403021813982) )
	ROM_LOAD( "c5.bin", 0x0800, 0x0800, CRC(f39846d3) SHA1(bc1137a45898210523cf8da1e6a2425e7c322884) )
	ROM_LOAD( "c2.bin", 0x1000, 0x0800, CRC(afa149a8) SHA1(207b842854ac9e015e12a2dae41105438cda1df9) )
	ROM_LOAD( "c6.bin", 0x1800, 0x0800, CRC(baf5461e) SHA1(754586a6449fd54a342f260e572c1cd60ab70815) )
	ROM_LOAD( "c3.bin", 0x2000, 0x0800, CRC(6bb282a1) SHA1(a96f25dc0f49ebe7e528e3297a112d778c6c3030) )
	ROM_LOAD( "c7.bin", 0x2800, 0x0800, CRC(fa2140f5) SHA1(123d31e653e8af78c6153702eca2e136c427ed64) )
	ROM_LOAD( "c4.bin", 0x3000, 0x0800, CRC(86c91e0e) SHA1(52af6a3af5b1363859f790470ca5860ef2a08566) )
	ROM_LOAD( "c8.bin", 0x3800, 0x0800, CRC(3d28134e) SHA1(45a257a0aca74e2ab36dd70097220d8be29cc87b) )

	ROM_REGION( 0x2000, REGION_GFX1 , ROMREGION_DISPOSE)
	ROM_LOAD( "c9.bin", 0x0000, 0x0800, CRC(1c4617be) SHA1(2b2b10f1256b4612e3e01ed1c8e2d7ccb6989f5d) )
	ROM_LOAD( "c11.bin", 0x0800, 0x0800, CRC(46f72fef) SHA1(daf334c78fdb73d43d524b733b763b290c602ae2) )
	ROM_LOAD( "c10.bin", 0x1000, 0x0800, CRC(ba9ec199) SHA1(626ab2eedf4c8d307dfad3b8863a67f8c34dda97) )
	ROM_LOAD( "c12.bin", 0x1800, 0x0800, CRC(41c09655) SHA1(cc639e660443b9dcb33f9aefe9af5d332591c466) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a", 0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END


ROM_START( pacplus )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "pacplus.6e",   0x0000, 0x1000, CRC(d611ef68) SHA1(8531c54ca6b0de0ea4ccc34e0e801ba9847e75bc) )
	ROM_LOAD( "pacplus.6f",   0x1000, 0x1000, CRC(c7207556) SHA1(8ba97215bdb75f0e70eb8d3223847efe4dc4fb48) )
	ROM_LOAD( "pacplus.6h",   0x2000, 0x1000, CRC(ae379430) SHA1(4e8613d51a80cf106f883db79685e1e22541da45) )
	ROM_LOAD( "pacplus.6j",   0x3000, 0x1000, CRC(5a6dff7b) SHA1(b956ae5d66683aab74b90469ad36b5bb361d677e) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pacplus.5e",   0x0000, 0x1000, CRC(022c35da) SHA1(57d7d723c7b029e3415801f4ce83469ec97bb8a1) )
	ROM_LOAD( "pacplus.5f",   0x1000, 0x1000, CRC(4de65cdd) SHA1(9c0699204484be819b77f0b212c792fe9e9fae5d) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "pacplus.7f",   0x0000, 0x0020, CRC(063dd53a) SHA1(2e43b46ec3b101d1babab87cdaddfa944116ec06) )
	ROM_LOAD( "pacplus.4a",   0x0020, 0x0100, CRC(e271a166) SHA1(cf006536215a7a1d488eebc1d8a2e2a8134ce1a6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( mspacman )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )	/* 64k for code+64k for decrypted code */
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) )
	ROM_LOAD( "u5",           0x8000, 0x0800, CRC(f45fbbcd) SHA1(b26cc1c8ee18e9b1daa97956d2159b954703a0ec) )
	ROM_LOAD( "u6",           0x9000, 0x1000, CRC(a90e7000) SHA1(e4df96f1db753533f7d770aa62ae1973349ea4cf) )
	ROM_LOAD( "u7",           0xb000, 0x1000, CRC(c82cd714) SHA1(1d8ac7ad03db2dc4c8c18ade466e12032673f874) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( mspacmnf )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )	/* 64k for code+64k for decrypted code */
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacfast.6f",   0x1000, 0x1000, CRC(720dc3ee) SHA1(7224d7acfa0144b681c71d7734a7337189835361) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) )
	ROM_LOAD( "u5",           0x8000, 0x0800, CRC(f45fbbcd) SHA1(b26cc1c8ee18e9b1daa97956d2159b954703a0ec) )
	ROM_LOAD( "u6",           0x9000, 0x1000, CRC(a90e7000) SHA1(e4df96f1db753533f7d770aa62ae1973349ea4cf) )
	ROM_LOAD( "u7",           0xb000, 0x1000, CRC(c82cd714) SHA1(1d8ac7ad03db2dc4c8c18ade466e12032673f874) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( mspacmab )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "boot1",        0x0000, 0x1000, CRC(d16b31b7) SHA1(bc2247ec946b639dd1f00bfc603fa157d0baaa97) )
	ROM_LOAD( "boot2",        0x1000, 0x1000, CRC(0d32de5e) SHA1(13ea0c343de072508908be885e6a2a217bbb3047) )
	ROM_LOAD( "boot3",        0x2000, 0x1000, CRC(1821ee0b) SHA1(5ea4d907dbb2690698db72c4e0b5be4d3e9a7786) )
	ROM_LOAD( "boot4",        0x3000, 0x1000, CRC(165a9dd8) SHA1(3022a408118fa7420060e32a760aeef15b8a96cf) )
	ROM_LOAD( "boot5",        0x8000, 0x1000, CRC(8c3e6de6) SHA1(fed6e9a2b210b07e7189a18574f6b8c4ec5bb49b) )
	ROM_LOAD( "boot6",        0x9000, 0x1000, CRC(368cb165) SHA1(387010a0c76319a1eab61b54c9bcb5c66c4b67a1) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( mspacmbe )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "boot1",        0x0000, 0x1000, CRC(d16b31b7) SHA1(bc2247ec946b639dd1f00bfc603fa157d0baaa97) )
	ROM_LOAD( "2.bin",        0x1000, 0x1000, CRC(04e6c486) SHA1(63aa3e6c49d345cccfe87dd3fdcddc75ab4a570d) )
	ROM_LOAD( "boot3",        0x2000, 0x1000, CRC(1821ee0b) SHA1(5ea4d907dbb2690698db72c4e0b5be4d3e9a7786) )
	ROM_LOAD( "boot4",        0x3000, 0x1000, CRC(165a9dd8) SHA1(3022a408118fa7420060e32a760aeef15b8a96cf) )
	ROM_LOAD( "boot5",        0x8000, 0x1000, CRC(8c3e6de6) SHA1(fed6e9a2b210b07e7189a18574f6b8c4ec5bb49b) )
	ROM_LOAD( "6.bin",        0x9000, 0x1000, CRC(206a9623) SHA1(20006f945c1b7b0e3c0415eecc0b148e5a6a1dfa) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( mspacmat )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )	/* 64k for code+64k for decrypted code */
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) )
	ROM_LOAD( "u5",           0x8000, 0x0800, CRC(f45fbbcd) SHA1(b26cc1c8ee18e9b1daa97956d2159b954703a0ec) )
	ROM_LOAD( "u6pacatk",     0x9000, 0x1000, CRC(f6d83f4d) SHA1(6135b187d6b968554d08f2ac00d3a3313efb8638) )
	ROM_LOAD( "u7",           0xb000, 0x1000, CRC(c82cd714) SHA1(1d8ac7ad03db2dc4c8c18ade466e12032673f874) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( mspacpls )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "boot1",        0x0000, 0x1000, CRC(d16b31b7) SHA1(bc2247ec946b639dd1f00bfc603fa157d0baaa97) )
	ROM_LOAD( "mspacatk.2",   0x1000, 0x1000, CRC(0af09d31) SHA1(6ff73e4da4910bcd2ca3aa299d8ffad23f8abf79) )
	ROM_LOAD( "boot3",        0x2000, 0x1000, CRC(1821ee0b) SHA1(5ea4d907dbb2690698db72c4e0b5be4d3e9a7786) )
	ROM_LOAD( "boot4",        0x3000, 0x1000, CRC(165a9dd8) SHA1(3022a408118fa7420060e32a760aeef15b8a96cf) )
	ROM_LOAD( "mspacatk.5",   0x8000, 0x1000, CRC(e6e06954) SHA1(ee5b266b1cc178df31fc1da5f66ef4911c653dda) )
	ROM_LOAD( "mspacatk.6",   0x9000, 0x1000, CRC(3b5db308) SHA1(c1ba630cb8fb665c4881a6cce9d3b0d4300bd0eb) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( pacgal )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "boot1",        0x0000, 0x1000, CRC(d16b31b7) SHA1(bc2247ec946b639dd1f00bfc603fa157d0baaa97) )
	ROM_LOAD( "boot2",        0x1000, 0x1000, CRC(0d32de5e) SHA1(13ea0c343de072508908be885e6a2a217bbb3047) )
	ROM_LOAD( "pacman.7fh",   0x2000, 0x1000, CRC(513f4d5c) SHA1(ae011b89422bd8cbb80389814500bc1427f6ecb2) )
	ROM_LOAD( "pacman.7hj",   0x3000, 0x1000, CRC(70694c8e) SHA1(d0d02f0997b44e1ba5ea27fc3f7af1b956e2a687) )
	ROM_LOAD( "boot5",        0x8000, 0x1000, CRC(8c3e6de6) SHA1(fed6e9a2b210b07e7189a18574f6b8c4ec5bb49b) )
	ROM_LOAD( "boot6",        0x9000, 0x1000, CRC(368cb165) SHA1(387010a0c76319a1eab61b54c9bcb5c66c4b67a1) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "pacman.5ef",   0x1000, 0x0800, CRC(65a3ee71) SHA1(cbbf700eefba2a5bf158983f2ca9688b7c6f5d2b) )
	ROM_LOAD( "pacman.5hj",   0x1800, 0x0800, CRC(50c7477d) SHA1(c04ec282a8cb528df5e38ad750d12ee71612695d) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.4a",    0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( mschamp )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )
	ROM_LOAD( "pm4.bin", 0x10000, 0x10000, CRC(7d6b6303) SHA1(65ad72a9188422653c02a48c07ed2661e1e36961) )	/* banked */

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pm5.bin", 0x0000, 0x0800, CRC(7fe6b9e2) SHA1(bfd0d84c7ef909ae078d8f60340682b3ff230aa6) )
	ROM_CONTINUE(        0x1000, 0x0800 )
	ROM_CONTINUE(        0x0800, 0x0800 )
	ROM_CONTINUE(        0x1800, 0x0800 )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
ROM_END


ROM_START( crush )
	ROM_REGION( 2*0x10000, REGION_CPU1, 0 )	/* 64k for code + 64k for opcode copy to hack protection */
	ROM_LOAD( "crushkrl.6e",  0x0000, 0x1000, CRC(a8dd8f54) SHA1(4e3a973ea74a9e145c6997513b98fc80aa478442) )
	ROM_LOAD( "crushkrl.6f",  0x1000, 0x1000, CRC(91387299) SHA1(3ad8c28e02c45667e32860953b157832445a82c8) )
	ROM_LOAD( "crushkrl.6h",  0x2000, 0x1000, CRC(d4455f27) SHA1(53f8ffc28be664fa8a2d756b4c70045a3f041bea) )
	ROM_LOAD( "crushkrl.6j",  0x3000, 0x1000, CRC(d59fc251) SHA1(024605e4485b0ac826217256e5356ed9a6c8ef34) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "maketrax.5e",  0x0000, 0x1000, CRC(91bad2da) SHA1(096197d0cb6d55bf72b5be045224f4bd6a9cfa1b) )
	ROM_LOAD( "maketrax.5f",  0x1000, 0x1000, CRC(aea79f55) SHA1(279021e6771dfa5bd0b7c557aae44434286d91b7) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( crush2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "tp1",          0x0000, 0x0800, CRC(f276592e) SHA1(68ebb7d9f70af868d99ec42c26bc55a54ba1f22c) )
	ROM_LOAD( "tp5a",         0x0800, 0x0800, CRC(3d302abe) SHA1(8ca5cd82d099b55e20f785489158231a1d99a430) )
	ROM_LOAD( "tp2",          0x1000, 0x0800, CRC(25f42e70) SHA1(66de8203c364fd90e8a2b5749c2e40665b2f5830) )
	ROM_LOAD( "tp6",          0x1800, 0x0800, CRC(98279cbe) SHA1(84b5e64bdbc25afab9b6f53e1719640e21a6feba) )
	ROM_LOAD( "tp3",          0x2000, 0x0800, CRC(8377b4cb) SHA1(f828a177f22db9093a00c31e39e16214ce0dc6de) )
	ROM_LOAD( "tp7",          0x2800, 0x0800, CRC(d8e76c8c) SHA1(7c3d7eb07b9256130141f71eba722f7823fd4c32) )
	ROM_LOAD( "tp4",          0x3000, 0x0800, CRC(90b28fa3) SHA1(ff58d2dfb016397daabe2996bc3a7b63d28a4cca) )
	ROM_LOAD( "tp8",          0x3800, 0x0800, CRC(10854e1b) SHA1(b3b9066d9a43796185c00ae12f7bb2bbf42e3a07) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "tpa",          0x0000, 0x0800, CRC(c7617198) SHA1(95b204af0345163f93811cc770ee0ca2851a39c1) )
	ROM_LOAD( "tpc",          0x0800, 0x0800, CRC(e129d76a) SHA1(c9256795c6d0929ade1f24b372dadc2a2b88d897) )
	ROM_LOAD( "tpb",          0x1000, 0x0800, CRC(d1899f05) SHA1(dce755511b6262b984a2bca329f454892e486a09) )
	ROM_LOAD( "tpd",          0x1800, 0x0800, CRC(d35d1caf) SHA1(65dd7861e05651485626465dc97215fed58db551) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( crush3 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "unkmol.4e",    0x0000, 0x0800, CRC(49150ddf) SHA1(5a20464a40d1d48606664779c85a7679073d7954) )
	ROM_LOAD( "unkmol.6e",    0x0800, 0x0800, CRC(21f47e17) SHA1(1194b5e8b0cce1f480acda3cb6c1fc65988bdc80) )
	ROM_LOAD( "unkmol.4f",    0x1000, 0x0800, CRC(9b6dd592) SHA1(6bb1b7ed95a7a8682a6ab58fa9f02c34beea8cd4) )
	ROM_LOAD( "unkmol.6f",    0x1800, 0x0800, CRC(755c1452) SHA1(a2da17ed0e526dad4d53d332467a3dfd3b2a8cab) )
	ROM_LOAD( "unkmol.4h",    0x2000, 0x0800, CRC(ed30a312) SHA1(15855904422eb603e5c5465bd038a3e8c666c10d) )
	ROM_LOAD( "unkmol.6h",    0x2800, 0x0800, CRC(fe4bb0eb) SHA1(70e480a75421ee0832456f1d30bf45a702192625) )
	ROM_LOAD( "unkmol.4j",    0x3000, 0x0800, CRC(072b91c9) SHA1(808df98c0cfd2367a39e06f30f920fd14887d922) )
	ROM_LOAD( "unkmol.6j",    0x3800, 0x0800, CRC(66fba07d) SHA1(4944d69a38fd823dad38b70433848017ae7027d7) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "unkmol.5e",    0x0000, 0x0800, CRC(338880a0) SHA1(beba1c71291394442b04fa5f4e1b833d7cf0fa8a) )
	ROM_LOAD( "unkmol.5h",    0x0800, 0x0800, CRC(4ce9c81f) SHA1(90a695ce4a45bde62bdbf09724a3ec6b45674660) )
	ROM_LOAD( "unkmol.5f",    0x1000, 0x0800, CRC(752e3780) SHA1(5730ebd8091eba5ac32ddd9db2f42d718b088753) )
	ROM_LOAD( "unkmol.5j",    0x1800, 0x0800, CRC(6e00d2ac) SHA1(aa3f1f3a3b6899bea717d97e4817b13159e552e5) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( crush4 )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )	/* 64k for code+64k for decrypted code */
	ROM_LOAD( "crtwt.2", 0x10000, 0x10000, CRC(adbd21f7) SHA1(984b005cd7a73f697715ecb7a4d806024cb7596d) )	/* banked */

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "crtwt.1", 0x0000, 0x0800, CRC(4250a9ea) SHA1(496a368afcf09c09205f7d0882320d2022e6fc98) )
	ROM_CONTINUE(        0x1000, 0x0800 )
	ROM_CONTINUE(        0x0800, 0x0800 )
	ROM_CONTINUE(        0x1800, 0x0800 )
	ROM_CONTINUE(        0x2000, 0x0800 )
	ROM_CONTINUE(        0x3000, 0x0800 )
	ROM_CONTINUE(        0x2800, 0x0800 )
	ROM_CONTINUE(        0x3800, 0x0800 )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.bin",   0x0020, 0x0100, CRC(2bc5d339) SHA1(446e234df94d9ef34c3191877bb33dd775acfdf5) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( maketrax )
	ROM_REGION( 2*0x10000, REGION_CPU1, 0 )	/* 64k for code + 64k for opcode copy to hack protection */
	ROM_LOAD( "maketrax.6e",  0x0000, 0x1000, CRC(0150fb4a) SHA1(ba41582d5432670654479b4bf6d938d2168858af) )
	ROM_LOAD( "maketrax.6f",  0x1000, 0x1000, CRC(77531691) SHA1(68a450bcc8d832368d0f1cb2815cb5c03451796e) )
	ROM_LOAD( "maketrax.6h",  0x2000, 0x1000, CRC(a2cdc51e) SHA1(80d80235cda3ce19c1dbafacf3d47b1325ad4728) )
	ROM_LOAD( "maketrax.6j",  0x3000, 0x1000, CRC(0b4b5e0a) SHA1(621aece612df612065f776696956ef3671421fac) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "maketrax.5e",  0x0000, 0x1000, CRC(91bad2da) SHA1(096197d0cb6d55bf72b5be045224f4bd6a9cfa1b) )
	ROM_LOAD( "maketrax.5f",  0x1000, 0x1000, CRC(aea79f55) SHA1(279021e6771dfa5bd0b7c557aae44434286d91b7) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( maketrxb )
	ROM_REGION( 2*0x10000, REGION_CPU1, 0 )	/* 64k for code + 64k for opcode copy to hack protection */
	ROM_LOAD( "maketrax.6e",  0x0000, 0x1000, CRC(0150fb4a) SHA1(ba41582d5432670654479b4bf6d938d2168858af) )
	ROM_LOAD( "maketrax.6f",  0x1000, 0x1000, CRC(77531691) SHA1(68a450bcc8d832368d0f1cb2815cb5c03451796e) )
	ROM_LOAD( "maketrxb.6h",  0x2000, 0x1000, CRC(6ad342c9) SHA1(5469f3952adc682725a71602b4a00a7751e48a99) )
	ROM_LOAD( "maketrxb.6j",  0x3000, 0x1000, CRC(be27f729) SHA1(0f7b873d33f751fa2fc54f9eede0598cb7d7f3c8) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "maketrax.5e",  0x0000, 0x1000, CRC(91bad2da) SHA1(096197d0cb6d55bf72b5be045224f4bd6a9cfa1b) )
	ROM_LOAD( "maketrax.5f",  0x1000, 0x1000, CRC(aea79f55) SHA1(279021e6771dfa5bd0b7c557aae44434286d91b7) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END

ROM_START( korosuke )
	ROM_REGION( 2*0x10000, REGION_CPU1, 0 )	/* 64k for code + 64k for opcode copy to hack protection */
	ROM_LOAD( "kr.6e",        0x0000, 0x1000, CRC(69f6e2da) SHA1(5f06523122d81a079bed080a16b44adb90aa95ad) )
	ROM_LOAD( "kr.6f",        0x1000, 0x1000, CRC(abf34d23) SHA1(6ae16fb8208037fd8b752076dd97e3da09e5cb8f) )
	ROM_LOAD( "kr.6h",        0x2000, 0x1000, CRC(76a2e2e2) SHA1(570aaed91279caab9274024e5a6176bdfe85bedd) )
	ROM_LOAD( "kr.6j",        0x3000, 0x1000, CRC(33e0e3bb) SHA1(43f5da486b9c44b0e4e8c909000786ee8ffee87f) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "kr.5e",        0x0000, 0x1000, CRC(e0380be8) SHA1(96eb7c5ef91342be67bd2a6c4958412d2572ba2a) )
	ROM_LOAD( "kr.5f",        0x1000, 0x1000, CRC(63fec9ee) SHA1(7d136362e08cceba9395c2c469d8fec451c5e396) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( mbrush )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "mbrush.6e",    0x0000, 0x1000, CRC(750fbff7) SHA1(986d20010d4fdd4bac916ac6b3a01bcd09d695ea) )
	ROM_LOAD( "mbrush.6f",    0x1000, 0x1000, CRC(27eb4299) SHA1(af2d7fdedcea766045fc2f20ae383024d1c35731) )
	ROM_LOAD( "mbrush.6h",    0x2000, 0x1000, CRC(d297108e) SHA1(a5bd11f26ba82b66a93d07e8cbc838ad9bd01413) )
	ROM_LOAD( "mbrush.6j",    0x3000, 0x1000, CRC(6fd719d0) SHA1(3de00981264cef24dc2c6277192e071144da2a88) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "tpa",          0x0000, 0x0800, CRC(c7617198) SHA1(95b204af0345163f93811cc770ee0ca2851a39c1) )
	ROM_LOAD( "mbrush.5h",    0x0800, 0x0800, CRC(c15b6967) SHA1(d8f16e2d6af5bf0f610d1e23614c531f67490da9) )
	ROM_LOAD( "mbrush.5f",    0x1000, 0x0800, CRC(d5bc5cb8) SHA1(269b82ae2b838c72ae06bff77412f22bb779ad2e) )  /* copyright sign was removed */
	ROM_LOAD( "tpd",          0x1800, 0x0800, CRC(d35d1caf) SHA1(65dd7861e05651485626465dc97215fed58db551) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( paintrlr )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "paintrlr.1",   0x0000, 0x0800, CRC(556d20b5) SHA1(c0a74def85bca108fc56726d22bbea1fc051e1ff) )
	ROM_LOAD( "paintrlr.5",   0x0800, 0x0800, CRC(4598a965) SHA1(866dbe7c0dbca10c5d5ec3efa3c79fb1ff1c5b56) )
	ROM_LOAD( "paintrlr.2",   0x1000, 0x0800, CRC(2da29c81) SHA1(e77f84e2f3136a116b75b40869e7f59404b0dbab) )
	ROM_LOAD( "paintrlr.6",   0x1800, 0x0800, CRC(1f561c54) SHA1(ef1159f2203ff6b5c17e3a79f32e8cafb12a49f7) )
	ROM_LOAD( "paintrlr.3",   0x2000, 0x0800, CRC(e695b785) SHA1(bc627a1a03d2e701fa4051acee469a4516cfb5bf) )
	ROM_LOAD( "paintrlr.7",   0x2800, 0x0800, CRC(00e6eec0) SHA1(e98850cf6e1762d08225a95f26a26766f8fa7303) )
	ROM_LOAD( "paintrlr.4",   0x3000, 0x0800, CRC(0fd5884b) SHA1(fa9614b625b3d71a6e9d5f883da625ad88e3eb5e) )
	ROM_LOAD( "paintrlr.8",   0x3800, 0x0800, CRC(4900114a) SHA1(47aee5bad136c19b203958b7ddac583d45018249) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "tpa",          0x0000, 0x0800, CRC(c7617198) SHA1(95b204af0345163f93811cc770ee0ca2851a39c1) )
	ROM_LOAD( "mbrush.5h",    0x0800, 0x0800, CRC(c15b6967) SHA1(d8f16e2d6af5bf0f610d1e23614c531f67490da9) )
	ROM_LOAD( "mbrush.5f",    0x1000, 0x0800, CRC(d5bc5cb8) SHA1(269b82ae2b838c72ae06bff77412f22bb779ad2e) )  /* copyright sign was removed */
	ROM_LOAD( "tpd",          0x1800, 0x0800, CRC(d35d1caf) SHA1(65dd7861e05651485626465dc97215fed58db551) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( ponpoko )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "ppokoj1.bin",  0x0000, 0x1000, CRC(ffa3c004) SHA1(d9e3186dcd4eb94d02bd24ad56030b248721537f) )
	ROM_LOAD( "ppokoj2.bin",  0x1000, 0x1000, CRC(4a496866) SHA1(4b8bd13e58040c30ca032b54fb47d889677e8c6f) )
	ROM_LOAD( "ppokoj3.bin",  0x2000, 0x1000, CRC(17da6ca3) SHA1(1a57767557c13fa3d08e4451fb9fb1f7219b26ef) )
	ROM_LOAD( "ppokoj4.bin",  0x3000, 0x1000, CRC(9d39a565) SHA1(d4835ee97c9b3c63504d8b576a11f0a3a97057ec) )
	ROM_LOAD( "ppoko5.bin",   0x8000, 0x1000, CRC(54ca3d7d) SHA1(b54299b00573fbd6d3278586df0c12c09235615d) )
	ROM_LOAD( "ppoko6.bin",   0x9000, 0x1000, CRC(3055c7e0) SHA1(ab3fb9c8846effdcea0569d08a84c5fa19057a8f) )
	ROM_LOAD( "ppoko7.bin",   0xa000, 0x1000, CRC(3cbe47ca) SHA1(577c79c016be26a9fc7895cef0f30bf3f0b15097) )
	ROM_LOAD( "ppokoj8.bin",  0xb000, 0x1000, CRC(04b63fc6) SHA1(9b86ae34aaefa2813d29a4f7b24cee40eadcc6a1) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ppoko9.bin",   0x0000, 0x1000, CRC(b73e1a06) SHA1(f1229e804eb15827b71f0e769a8c9e496c6d1de7) )
	ROM_LOAD( "ppoko10.bin",  0x1000, 0x1000, CRC(62069b5d) SHA1(1b58ad1c2cc2d12f4e492fdd665b726d50c80364) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( ponpokov )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "ppoko1.bin",   0x0000, 0x1000, CRC(49077667) SHA1(3e760cd4dbe5913e58d786caf510237ff635c775) )
	ROM_LOAD( "ppoko2.bin",   0x1000, 0x1000, CRC(5101781a) SHA1(a82fbd2418ac7866f9463092e9dd37fd7ba9b694) )
	ROM_LOAD( "ppoko3.bin",   0x2000, 0x1000, CRC(d790ed22) SHA1(2d32f91f6993232db40b44b35bd2503d85e5c874) )
	ROM_LOAD( "ppoko4.bin",   0x3000, 0x1000, CRC(4e449069) SHA1(d5e6e346f80e66eb0db530de9721d9b6f22e86ae) )
	ROM_LOAD( "ppoko5.bin",   0x8000, 0x1000, CRC(54ca3d7d) SHA1(b54299b00573fbd6d3278586df0c12c09235615d) )
	ROM_LOAD( "ppoko6.bin",   0x9000, 0x1000, CRC(3055c7e0) SHA1(ab3fb9c8846effdcea0569d08a84c5fa19057a8f) )
	ROM_LOAD( "ppoko7.bin",   0xa000, 0x1000, CRC(3cbe47ca) SHA1(577c79c016be26a9fc7895cef0f30bf3f0b15097) )
	ROM_LOAD( "ppoko8.bin",   0xb000, 0x1000, CRC(b39be27d) SHA1(c299d22d26da68bec8fc53c898523135ec4016fa) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ppoko9.bin",   0x0000, 0x1000, CRC(b73e1a06) SHA1(f1229e804eb15827b71f0e769a8c9e496c6d1de7) )
	ROM_LOAD( "ppoko10.bin",  0x1000, 0x1000, CRC(62069b5d) SHA1(1b58ad1c2cc2d12f4e492fdd665b726d50c80364) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( eyes )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "d7",           0x0000, 0x1000, CRC(3b09ac89) SHA1(a8f1c918da74495bb73172f39364dada38ae4713) )
	ROM_LOAD( "e7",           0x1000, 0x1000, CRC(97096855) SHA1(10d3b164bbbe5eee86e881a1434f0c114ee8adff) )
	ROM_LOAD( "f7",           0x2000, 0x1000, CRC(731e294e) SHA1(96c0308c146dbd85e244c4530af9ae8df78c86de) )
	ROM_LOAD( "h7",           0x3000, 0x1000, CRC(22f7a719) SHA1(eb000b606ecedd52bebbb232e661fb1ef205f8b0) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "d5",           0x0000, 0x1000, CRC(d6af0030) SHA1(652b779533e3f00e81cc102b78d367d503b06f33) )
	ROM_LOAD( "e5",           0x1000, 0x1000, CRC(a42b5201) SHA1(2e5cede3b6039c7bd5230de27d02aaa3f35a7b64) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.4a",    0x0020, 0x0100, CRC(d8d78829) SHA1(19820d1651423210083a087fb70ebea73ad34951) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( eyes2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "g38201.7d",    0x0000, 0x1000, CRC(2cda7185) SHA1(7ec3ee9bb90e6a1d83ad3aa12fd62184e07b1399) )
	ROM_LOAD( "g38202.7e",    0x1000, 0x1000, CRC(b9fe4f59) SHA1(2d97dc1a0458b406ca0c50d6b8bd0dbe58d21464) )
	ROM_LOAD( "g38203.7f",    0x2000, 0x1000, CRC(d618ba66) SHA1(76d93d8bc09bafac464ebfd002869e21535a365b) )
	ROM_LOAD( "g38204.7h",    0x3000, 0x1000, CRC(cf038276) SHA1(bcf4e129a151e2245e630cf865ce6cb009b405a5) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "g38205.5d",    0x0000, 0x1000, CRC(03b1b4c7) SHA1(a90b2fbaee2888ee4f0bcdf80a069c8594ef5ea1) )  /* this one has a (c) sign */
	ROM_LOAD( "e5",           0x1000, 0x1000, CRC(a42b5201) SHA1(2e5cede3b6039c7bd5230de27d02aaa3f35a7b64) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.4a",    0x0020, 0x0100, CRC(d8d78829) SHA1(19820d1651423210083a087fb70ebea73ad34951) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END

/* It's just a decrypted version of Eyes with the copyright changes...
 roms marked with a comment were in the set but we're not using them

 This is only supported because it's by Zaccaria, a known manufacturer of original games
 */
ROM_START( eyeszac )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
/**/ROM_LOAD( "11.bin",          0x0000, 0x0800, CRC(69c1602a) SHA1(47b0935406b7ee2f414de58da1d4e81c6277a0c2) ) // "no diagnostics, bad custom??" (unused)
	ROM_LOAD( "1.bin",           0x0000, 0x0800, CRC(a4a9d7a0) SHA1(f0b807d2fa347e50df52971aa7539a88f342bad6) )
	ROM_LOAD( "5.bin",           0x0800, 0x0800, CRC(c32b3f73) SHA1(80d2e987f0318b984e5c7c4d0b5faa262eebeca4) )
	ROM_LOAD( "2.bin",           0x1000, 0x0800, CRC(195b9473) SHA1(62eb16af38cc9004787dc55433ed3db11af44a4b) )
	ROM_LOAD( "6.bin",           0x1800, 0x0800, CRC(292886cb) SHA1(e77c3724c7cd8cd95014194ba4bb2f7e04afb0dd) )
//  ROM_LOAD( "33.bin",          0x2000, 0x0800, CRC(df983e1d) SHA1(7c06fc69b7d0424f7b9348649d5587ff4d6dfc2d) ) // alt rom with copyright removed (unused)
	ROM_LOAD( "3.bin",           0x2000, 0x0800, CRC(ff94b015) SHA1(6d8f43db3c98cadb35f70e3bff788e653dc132cd) )
	ROM_LOAD( "7.bin",           0x2800, 0x0800, CRC(9271c58c) SHA1(e6b8f1807c5852ae4e822d80719a4e8f8b036c31) )
	ROM_LOAD( "4.bin",           0x3000, 0x0800, CRC(965cf32b) SHA1(68cc573a24c74f2ab417d0330fc9523e77fda961) )
	ROM_LOAD( "8.bin",           0x3800, 0x0800, CRC(c254e92e) SHA1(023b45403ebc69c29516d77950dc69f05a1a130c) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "x.bin",           0x0000, 0x0800, CRC(59dce22e) SHA1(81eaef3e4d8299b5133b62d04460abfa519696f5) )
	ROM_LOAD( "c.bin",           0x0800, 0x0800, CRC(aaa7a537) SHA1(571d981ed2aad62d7c7f2798e9084228d45523d4) )
	ROM_LOAD( "b.bin",           0x1000, 0x0800, CRC(1969792b) SHA1(7c3e2ace75402ad227e6437785b7cfec4db88db8) )
	ROM_LOAD( "p.bin",           0x1800, 0x0800, CRC(99af4b30) SHA1(6a0939ff2fa7ae39a960dd4d9f9b7c01f57647c5) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.4a",    0x0020, 0x0100, CRC(d8d78829) SHA1(19820d1651423210083a087fb70ebea73ad34951) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END

ROM_START( mrtnt )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "tnt.1",        0x0000, 0x1000, CRC(0e836586) SHA1(5037b7c618f05bc3d6a33694729ae575b9aa7dbb) )
	ROM_LOAD( "tnt.2",        0x1000, 0x1000, CRC(779c4c5b) SHA1(5ecac4f5b64b306c73d8f57d5260b586789b3055) )
	ROM_LOAD( "tnt.3",        0x2000, 0x1000, CRC(ad6fc688) SHA1(e5729e4e42a5b9b3a26de8a44b3a78b49c8b1d8e) )
	ROM_LOAD( "tnt.4",        0x3000, 0x1000, CRC(d77557b3) SHA1(689746653b1e19fbcddd0d71db2b86d1019235aa) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "tnt.5",        0x0000, 0x1000, CRC(3038cc0e) SHA1(f8f5927ea4cbfda8fa7546abd766ba2e8b020004) )
	ROM_LOAD( "tnt.6",        0x1000, 0x1000, CRC(97634d8b) SHA1(4c0fa4bc44bbb4b4614b5cc05e811c469c0e78e8) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END

ROM_START( gorkans )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "gorkans8.rom",        0x0000, 0x0800, CRC(55100b18) SHA1(8f657c1b2865987b60d95960c5297a82bb1cc6e0) )
	ROM_LOAD( "gorkans4.rom",        0x0800, 0x0800, CRC(b5c604bf) SHA1(0f3608d630fba9d4734a3ef30199a5d1a067cdff) )
 	ROM_LOAD( "gorkans7.rom",        0x1000, 0x0800, CRC(b8c6def4) SHA1(58ac78fc5b3559ef771ca708a79089b7a00cf6b8) )
	ROM_LOAD( "gorkans3.rom",        0x1800, 0x0800, CRC(4602c840) SHA1(c77de0e991c44c2ee8a4537e264ac8fbb1b4b7db) )
	ROM_LOAD( "gorkans6.rom",        0x2000, 0x0800, CRC(21412a62) SHA1(ece44c3204cf182db23b594ebdc051b51340ba2b) )
	ROM_LOAD( "gorkans2.rom",        0x2800, 0x0800, CRC(a013310b) SHA1(847ba7ca033eaf49245bef49d6513619edec3472) )
	ROM_LOAD( "gorkans5.rom",        0x3000, 0x0800, CRC(122969b2) SHA1(0803e1ec5e5ed742ea83ff156ae75a2d48530f71) )
	ROM_LOAD( "gorkans1.rom",        0x3800, 0x0800, CRC(f2524b11) SHA1(1216b963e73c1de63cc323e361875f6810d83a05) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "gorkgfx4.rom",        0x0000, 0x0800, CRC(39cd0dbc) SHA1(8d6882dad94b26da8f0737e7f7f99946fe273f1b) )
	ROM_LOAD( "gorkgfx2.rom",        0x0800, 0x0800, CRC(33d52535) SHA1(e78ac5afa1ce996c41005c619ba2d2aa718497fc) )
	ROM_LOAD( "gorkgfx3.rom",        0x1000, 0x0800, CRC(4b6b7970) SHA1(1d8b65cad0b834fb920135fc907432042bc83db2) )
	ROM_LOAD( "gorkgfx1.rom",        0x1800, 0x0800, CRC(ed70bb3c) SHA1(7e51ddcf496f3b80fe186acc8bc6a0e574340346) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "gorkprom.4",   0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "gorkprom.1",   0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "gorkprom.3",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "gorkprom.2"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END

ROM_START( eggor )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "1.bin",        0x0000, 0x0800, CRC(818ed154) SHA1(8c0f555a3ab1d20a2c284d721b31278a0ddf9e51) )
	ROM_LOAD( "5.bin",        0x0800, 0x0800, CRC(a4b21d93) SHA1(923b7a06f9146c7bcda4cdb16b15d2bbbec95eab) )
	ROM_LOAD( "2.bin",        0x1000, 0x0800, CRC(5d7a23ed) SHA1(242fd973b0bde91c38e1f5e7f6c53d737019ec9c) )
	ROM_LOAD( "6.bin",        0x1800, 0x0800, CRC(e9dbca8d) SHA1(b66783d68df778910cc190159aba07b476ff01af) )
	ROM_LOAD( "3.bin",        0x2000, 0x0800, CRC(4318ab85) SHA1(eda9bb1bb8102e1c2cf838d0682732a45609f430) )
	ROM_LOAD( "7.bin",        0x2800, 0x0800, CRC(03214d7f) SHA1(0e1b602fbdedfe81452109912fed006653bdc455) )
	ROM_LOAD( "4.bin",        0x3000, 0x0800, CRC(dc805be4) SHA1(18604b221cd8af23ff8a05c954a42c3aa9e1948a) )
	ROM_LOAD( "8.bin",        0x3800, 0x0800, CRC(f9ae204b) SHA1(53022d2d7b83f44c46fdcca454815cf1f65c34d1) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "9.bin",        0x0000, 0x0800, CRC(96ad8626) SHA1(f003a6e1b00a51bfe326eac18658fafd58c88f88) )
	ROM_LOAD( "11.bin",       0x0800, 0x0800, CRC(cc324017) SHA1(ea96572e3e24714033688fe7ca99af2fc707c1d3) )
	ROM_LOAD( "10.bin",       0x1000, 0x0800, CRC(7c97f513) SHA1(6f78c7cde321ea6ac51d08d0e3620653d0af87db) )
	ROM_LOAD( "12.bin",       0x1800, 0x0800, CRC(2e930602) SHA1(4012ec0cc542061b27b9b508bedde3f2ffc11838) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	/* the board was stripped of its proms, these are the standard ones from Pacman, they look reasonable
       but without another board its impossible to say if they are actually good */
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, BAD_DUMP CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, BAD_DUMP CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END

ROM_START( lizwiz )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "6e.cpu",       0x0000, 0x1000, CRC(32bc1990) SHA1(467c9d70e07f403b6b9118aebe4e6d0abb40a5c1) )
	ROM_LOAD( "6f.cpu",       0x1000, 0x1000, CRC(ef24b414) SHA1(12fce48008c4f9387df0c84f3b0d7c5a1b35d898) )
	ROM_LOAD( "6h.cpu",       0x2000, 0x1000, CRC(30bed83d) SHA1(8c2458f98320c6887580c71632b544da0a582ba2) )
	ROM_LOAD( "6j.cpu",       0x3000, 0x1000, CRC(dd09baeb) SHA1(f91447ec1f06bf95106e6872d80dcb82e1d42ffb) )
	ROM_LOAD( "wiza",         0x8000, 0x1000, CRC(f6dea3a6) SHA1(ec0b123fd2e6de6681ca14f35fda249b2c2ec44f) )
	ROM_LOAD( "wizb",         0x9000, 0x1000, CRC(f27fb5a8) SHA1(3ea384a1064302709d97fc16b347d3c012e90ac7) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5e.cpu",       0x0000, 0x1000, CRC(45059e73) SHA1(c960cd5720bfa21db73e1000fe8be7d5baf2a3a1) )
	ROM_LOAD( "5f.cpu",       0x1000, 0x1000, CRC(d2469717) SHA1(194c8f816e5ff7614b3db4f355223667105738fa) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "7f.cpu",       0x0000, 0x0020, CRC(7549a947) SHA1(4f2c3e7d6c38f0b9a90317f91feb3f86c9a0d0a5) )
	ROM_LOAD( "4a.cpu",       0x0020, 0x0100, CRC(5fdca536) SHA1(3a09b29374031aaa3722932aff974a467b3bb201) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( theglobp )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "glob.u2",      0x0000, 0x2000, CRC(829d0bea) SHA1(89f52b459a03fb40b9bbd97ac8a292f7ead6faba) )
	ROM_LOAD( "glob.u3",      0x2000, 0x2000, CRC(31de6628) SHA1(35a47dcf34efd74b5b2fda137e06a3dcabd74854) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "glob.5e",      0x0000, 0x1000, CRC(53688260) SHA1(9ce0d1d67d12743b69e8190bf7506b00b2f02955) )
	ROM_LOAD( "glob.5f",      0x1000, 0x1000, CRC(051f59c7) SHA1(e1e1322686997e5bcdac164704b328cce352ae42) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "glob.7f",      0x0000, 0x0020, CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) )
	ROM_LOAD( "glob.4a",      0x0020, 0x0100, CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


//Program roms same as the globp
ROM_START( sprglobp )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "glob.u2",      0x0000, 0x2000, CRC(829d0bea) SHA1(89f52b459a03fb40b9bbd97ac8a292f7ead6faba) )
	ROM_LOAD( "glob.u3",      0x2000, 0x2000, CRC(31de6628) SHA1(35a47dcf34efd74b5b2fda137e06a3dcabd74854) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5e_2532.dat",  0x0000, 0x1000, CRC(1aa16109) SHA1(ddc8606512d7ab7555b84146b9d793f65ad0a75f) )
	ROM_LOAD( "5f_2532.dat",  0x1000, 0x1000, CRC(afe72a89) SHA1(fb17632e2665c3cebc1865ef25fa310cc52725c4) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "glob.7f",      0x0000, 0x0020, CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) )
	ROM_LOAD( "glob.4a",      0x0020, 0x0100, CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END

/* This set is from a modified Pengo board.  Pengo and Pacman are functionally the same.
   The bad sound is probably correct as the sound data is part of the protection. */
ROM_START( sprglbpg )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "ic8.1",      0x0000, 0x1000, CRC(a2df2073) SHA1(14c55186053b080de06cc3691111ede8b2ead231) )
	ROM_LOAD( "ic7.2",      0x1000, 0x1000, CRC(3d2c22d9) SHA1(2f1d27e49850f904d1f2256bfcf00557ed88bb16) )
	ROM_LOAD( "ic15.3",      0x2000, 0x1000, CRC(a252047f) SHA1(9fadbb098b86ee98e1a81da938316b833fc26912) )
	ROM_LOAD( "ic14.4",      0x3000, 0x1000, CRC(7efa81f1) SHA1(583999280623f02dcc318a6c7af5ee6fc46144b8) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ic92.5",  0x0000, 0x2000, CRC(E54F484D) SHA1(4FEB9EC917C2467A5AC531283CB00FE308BE7775) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "ic78.prm",      0x0000, 0x0020, CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) )
	ROM_LOAD( "ic88.prm",      0x0020, 0x0100, CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "ic51.prm",    0x0000, 0x0100, CRC(c29dea27) SHA1(563c9770028fe39188e62630711589d6ed242a66) )
	ROM_LOAD( "ic70.prm"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( beastf )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "bf-u2.bin",    0x0000, 0x2000, CRC(3afc517b) SHA1(5b74bca9e9cd4d8bcf94a340f8f0e53fe1dcfc1d) )
	ROM_LOAD( "bf-u3.bin",    0x2000, 0x2000, CRC(8dbd76d0) SHA1(058c01e87ad583eb99d5043a821e6c68f1b30267) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "beastf.5e",    0x0000, 0x1000, CRC(5654dc34) SHA1(fc2336b951a3ab48d4fc4f36a8dd80e79e8ca1a0) )
	ROM_LOAD( "beastf.5f",    0x1000, 0x1000, CRC(1b30ca61) SHA1(8495d8a280346246f00c4f1dc42ab5a2a02c5863) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "glob.7f",      0x0000, 0x0020, CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) )
	ROM_LOAD( "glob.4a",      0x0020, 0x0100, CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( vanvan )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "van-1.50",     0x0000, 0x1000, CRC(cf1b2df0) SHA1(938b4434c0129cf9151f829901d00e47dca68956) )
	ROM_LOAD( "van-2.51",     0x1000, 0x1000, CRC(df58e1cb) SHA1(5e0fc713b50d46c7650d6564c20882891864cdc5) )
	ROM_LOAD( "van-3.52",     0x2000, 0x1000, CRC(15571e24) SHA1(d259d81fce16e151b32ac81f94a13b7044fdef95) )
	ROM_LOAD( "van-4.53",     0x3000, 0x1000, CRC(b724cbe0) SHA1(5fe1d3b81d07c538c31daf6522b26bbf35cfc512) )
	ROM_LOAD( "van-5.39",     0x8000, 0x1000, CRC(db67414c) SHA1(19eba21dfea24507b386ea1b5ce737c5822b0696) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "van-20.18",    0x0000, 0x1000, CRC(60efbe66) SHA1(ac398f77bfeab3d18ffd496e117825bfbeed4b62) )
	ROM_LOAD( "van-21.19",    0x1000, 0x1000, CRC(5dd53723) SHA1(f75c869ac364f477d532e695347ceb5e281f9efa) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "6331-1.6",     0x0000, 0x0020, CRC(ce1d9503) SHA1(b829bed78c02d9998c1aecb8f6813e90b417a7f2) )
	ROM_LOAD( "6301-1.37",    0x0020, 0x0100, CRC(4b803d9f) SHA1(59b7f2e22c4e0b20ac3b12d88996a6dfeebc5933) )
ROM_END

ROM_START( vanvank )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "van1.bin",	  0x0000, 0x1000, CRC(00f48295) SHA1(703fab63760cadcce042b491d7d1d45301319158) )
	ROM_LOAD( "van-2.51",     0x1000, 0x1000, CRC(df58e1cb) SHA1(5e0fc713b50d46c7650d6564c20882891864cdc5) )
	ROM_LOAD( "van-3.52",     0x2000, 0x1000, CRC(15571e24) SHA1(d259d81fce16e151b32ac81f94a13b7044fdef95) )
	ROM_LOAD( "van4.bin",     0x3000, 0x1000, CRC(f8b37ed5) SHA1(34f844be891dfa5f6a1160de6f428e9dacd618a8) )
	ROM_LOAD( "van5.bin",     0x8000, 0x1000, CRC(b8c1e089) SHA1(c614fb9159210f6cf68f5085bfebd928caded91c) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "van-20.18",    0x0000, 0x1000, CRC(60efbe66) SHA1(ac398f77bfeab3d18ffd496e117825bfbeed4b62) )
	ROM_LOAD( "van-21.19",    0x1000, 0x1000, CRC(5dd53723) SHA1(f75c869ac364f477d532e695347ceb5e281f9efa) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "6331-1.6",     0x0000, 0x0020, CRC(ce1d9503) SHA1(b829bed78c02d9998c1aecb8f6813e90b417a7f2) )
	ROM_LOAD( "6301-1.37",    0x0020, 0x0100, CRC(4b803d9f) SHA1(59b7f2e22c4e0b20ac3b12d88996a6dfeebc5933) )
ROM_END


ROM_START( vanvanb )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "vv1.bin",     0x0000, 0x1000, CRC(cf1b2df0) SHA1(938b4434c0129cf9151f829901d00e47dca68956) )
	ROM_LOAD( "vv2.bin",     0x1000, 0x1000, CRC(80eca6a5) SHA1(e3e711e5c27f5effdae95222a019e427a754d505) )
	ROM_LOAD( "vv3.bin",     0x2000, 0x1000, CRC(15571e24) SHA1(d259d81fce16e151b32ac81f94a13b7044fdef95) )
	ROM_LOAD( "vv4.bin",     0x3000, 0x1000, CRC(b1f04006) SHA1(f0dccce9cc9871ff671e86947512f354ff5f4f13) )
	ROM_LOAD( "vv5.bin",     0x8000, 0x1000, CRC(db67414c) SHA1(19eba21dfea24507b386ea1b5ce737c5822b0696) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "vv20.bin",    0x0000, 0x1000, CRC(eb56cb51) SHA1(fa8c4f61acbbc0ea7b41aff2624dadcc581ddf8c) )
	ROM_LOAD( "vv21.bin",    0x1000, 0x1000, CRC(5dd53723) SHA1(f75c869ac364f477d532e695347ceb5e281f9efa) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "6331-1.6",     0x0000, 0x0020, CRC(ce1d9503) SHA1(b829bed78c02d9998c1aecb8f6813e90b417a7f2) )
	ROM_LOAD( "6301-1.37",    0x0020, 0x0100, CRC(4b803d9f) SHA1(59b7f2e22c4e0b20ac3b12d88996a6dfeebc5933) )
ROM_END


ROM_START( dremshpr )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "red_1.50",	  0x0000, 0x1000, CRC(830c6361) SHA1(a28c517a9b7f509e0dedacea64b9740335315457) )
	ROM_LOAD( "red_2.51",     0x1000, 0x1000, CRC(d22551cc) SHA1(2c513908899b618f0c0a0c3e48c4a4aad90f627e) )
	ROM_LOAD( "red_3.52",     0x2000, 0x1000, CRC(0713a34a) SHA1(37733b557e6afe116f5d3c8bc918f59124a8229d) )
	ROM_LOAD( "red_4.53",     0x3000, 0x1000, CRC(f38bcaaa) SHA1(cdebeaf5b77ac5a8b4668cff97b6351e075b392b) )
	ROM_LOAD( "red_5.39",     0x8000, 0x1000, CRC(6a382267) SHA1(7d6a1c75de8a6eb714ba9a18dd3c497832145bcc) )
	ROM_LOAD( "red_6.40",     0x9000, 0x1000, CRC(4cf8b121) SHA1(04162b41e747dfa442b958bd360e49993c5c4162) )
	ROM_LOAD( "red_7.41",     0xa000, 0x1000, CRC(bd4fc4ba) SHA1(50a5858acde5fd4b3476f5502141e7d492c3af9f) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "red-20.18",    0x0000, 0x1000, CRC(2d6698dc) SHA1(5f5e54fdcff53c6ba783d585cd994cf563c53613) )
	ROM_LOAD( "red-21.19",    0x1000, 0x1000, CRC(38c9ce9b) SHA1(c719bcd77549228e72ad9bcc42f5db0070ec5dca) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "6331-1.6",     0x0000, 0x0020, CRC(ce1d9503) SHA1(b829bed78c02d9998c1aecb8f6813e90b417a7f2) )
	ROM_LOAD( "6301-1.37",    0x0020, 0x0100, CRC(39d6fb5c) SHA1(848f9cd02f90006e8a2aae3693b57ae391cf498b) )
ROM_END


ROM_START( alibaba )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "6e",           0x0000, 0x1000, CRC(38d701aa) SHA1(4e886a4a17f441f6d1d213c4c433b40dd38eefbc) )
	ROM_LOAD( "6f",           0x1000, 0x1000, CRC(3d0e35f3) SHA1(6b9a1fd11db9f521417566ae4c7065151aa272b5) )
	ROM_LOAD( "6h",           0x2000, 0x1000, CRC(823bee89) SHA1(5381a4fcbc9fa97574c6df2978c7500164df75e5) )
	ROM_LOAD( "6k",           0x3000, 0x1000, CRC(474d032f) SHA1(4516a60ec83e3c3388cd56f538f49afc86a50983) )
	ROM_LOAD( "6l",           0x8000, 0x1000, CRC(5ab315c1) SHA1(6f3507ad10432f9123150b8bc1d0fb52372a412b) )
	ROM_LOAD( "6m",           0xa000, 0x0800, CRC(438d0357) SHA1(7caaf668906b76d4947e988c444723b33f8e9726) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5e",           0x0000, 0x0800, CRC(85bcb8f8) SHA1(986170627953582b1e6fbca59e5c15cf8c4de9c7) )
	ROM_LOAD( "5h",           0x0800, 0x0800, CRC(38e50862) SHA1(094d090bd0563f75d6ff1bfe2302cae941a89504) )
	ROM_LOAD( "5f",           0x1000, 0x0800, CRC(b5715c86) SHA1(ed6aee778295b0182d32846b5e41776b5b15420c) )
	ROM_LOAD( "5k",           0x1800, 0x0800, CRC(713086b3) SHA1(a1609bae637207a82920678f05bcc10a5ff096de) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.e7",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.a4",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */

	/* unknown, used for the mystery items ? */
	ROM_REGION( 0x1000, REGION_USER1, 0 )
	ROM_LOAD( "7.p6",         0x000, 0x1000, CRC(d8eb7cbd) SHA1(fe482d36d81de5c5034e18b91bfa6b43111e683e) )	/* 1ST AND 2ND HALF IDENTICAL */
ROM_END


ROM_START( jumpshot )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "6e",           0x0000, 0x1000, CRC(f00def9a) SHA1(465a7f368e61a1e6614d6eab0fa2c6319920eaa5) )
	ROM_LOAD( "6f",           0x1000, 0x1000, CRC(f70deae2) SHA1(a8a8369e865b62cb9ed66d3de2396c6a5fced549) )
	ROM_LOAD( "6h",           0x2000, 0x1000, CRC(894d6f68) SHA1(8693ffc29587cdd1be0b42cede53f8f450a2c7fa) )
	ROM_LOAD( "6j",           0x3000, 0x1000, CRC(f15a108a) SHA1(db5c8394f688c6f889cadddeeae4fbca63c29a4c) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(d9fa90f5) SHA1(3c37fe077a77baa802230dddbc4bb2c05985d2bb) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(2ec711c1) SHA1(fcc3169f48eb7d4af533ad0169701e4230ff5a1f) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "prom.7f",      0x0000, 0x0020, CRC(872b42f3) SHA1(bbcd392ba3d2a5715e92fa0f7a7cf1e7e6e655a2) )
	ROM_LOAD( "prom.4a",      0x0020, 0x0100, CRC(0399f39f) SHA1(e98f08da4666cab44e01acb760a1bd2fc858bc0d) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( jumpshtp )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "js6e.bin",           0x0000, 0x1000, CRC(acc5e15e) SHA1(c9516f2d0862b29a3efe19eb13ab68decd670ca8) )
	ROM_LOAD( "js6f.bin",           0x1000, 0x1000, CRC(62b48ba4) SHA1(a17d8ca68de6116822cf4eff70eada04a3fbb4c3) )
	ROM_LOAD( "js6h.bin",           0x2000, 0x1000, CRC(7c9b5e30) SHA1(44c7694b8bd774550ec865d133f5660b90350428) )
	ROM_LOAD( "js6j.bin",           0x3000, 0x1000, CRC(9f0c39f6) SHA1(8714c9b0853206ec5d79155b4310195b46fafbf6) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(d9fa90f5) SHA1(3c37fe077a77baa802230dddbc4bb2c05985d2bb) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(2ec711c1) SHA1(fcc3169f48eb7d4af533ad0169701e4230ff5a1f) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "prom.7f",      0x0000, 0x0020, CRC(872b42f3) SHA1(bbcd392ba3d2a5715e92fa0f7a7cf1e7e6e655a2) )
	ROM_LOAD( "prom.4a",      0x0020, 0x0100, CRC(0399f39f) SHA1(e98f08da4666cab44e01acb760a1bd2fc858bc0d) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */
ROM_END


ROM_START( shootbul )
	ROM_REGION( 0x10000, REGION_CPU1, 0 ) /* 64k for code */
	ROM_LOAD( "sb6e.cpu",     0x0000, 0x1000, CRC(25daa5e9) SHA1(8257de5e0e62235d05d74b53e5fc716e85cb05b9) )
	ROM_LOAD( "sb6f.cpu",     0x1000, 0x1000, CRC(92144044) SHA1(905a354a806da47ab40577171acdac7db635d102) )
	ROM_LOAD( "sb6h.cpu",     0x2000, 0x1000, CRC(43b7f99d) SHA1(6372763fbbca3581376204c5e58ceedd3f47fc60) )
	ROM_LOAD( "sb6j.cpu",     0x3000, 0x1000, CRC(bc4d3bbf) SHA1(2fa15b339166b9a5bf711b58a1705bc0b9e528e2) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "sb5e.cpu",     0x0000, 0x1000, CRC(07c6c5aa) SHA1(cbe99ece795f29fdeef374cbf9b1f45ff065e803) )
	ROM_LOAD( "sb5f.cpu",     0x1000, 0x1000, CRC(eaec6837) SHA1(ff21b0fd5381afb1ba7f5920132006ee8e6d10eb) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "7f.rom",       0x0000, 0x0020, CRC(ec578b98) SHA1(196da49cc260f967ec5f01bc3c75b11077c85998) )
	ROM_LOAD( "4a.rom",       0x0020, 0x0100, CRC(81a6b30f) SHA1(60c767fd536c325151a2b759fdbce4ba41e0c78f) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END


ROM_START( acitya )
	ROM_REGION( 0x20000, REGION_CPU1, 0 ) /* 64k for code */
	ROM_LOAD( "aca_u2.bin",   0x0000, 0x2000, CRC(261c2fdc) SHA1(b4e7e6c8d8e401c7e4673213074802a73b9886a2) )
	ROM_LOAD( "aca_u3.bin",   0x2000, 0x2000, CRC(05fab4ca) SHA1(5172229eda25920eeaa6d9f610f2bcfa674979b7) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "aca_5e.bin",   0x0000, 0x1000, CRC(7f2dd2c9) SHA1(aa7ea70355904989b99d568d1e055e8272cfa8ca) )
	ROM_RELOAD( 0x1000, 0x1000 ) /* Not Used?? */

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "aca_7f.bin",   0x0000, 0x0020, CRC(133bb744) SHA1(da4074f3ea30202973f0b6c9ad05a992bb44eafd) )
	ROM_LOAD( "aca_4a.bin",   0x0020, 0x0100, CRC(8e29208f) SHA1(a30a405fbd43d27a8d403f6c3545178564dede5d) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END

ROM_START( bwcasino )
	ROM_REGION( 0x20000, REGION_CPU1, 0 ) /* 64k for code */
	ROM_LOAD( "bwc_u2.bin",   0x0000, 0x2000, CRC(e2eea868) SHA1(9e9dae02ab746ef48981f42a75c192c5aae0ffee) )
	ROM_LOAD( "bwc_u3.bin",   0x2000, 0x2000, CRC(a935571e) SHA1(ab4f53be2544593fc8eb4c4bcccdec4191c0c626) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "bwc_5e.bin",   0x0000, 0x1000, CRC(e334c01e) SHA1(cc6e50e3cf51eb8b7b27aa7351733954da8128ff) )
	ROM_RELOAD( 0x1000, 0x1000 ) /* Not Used?? */

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "aca_7f.bin",   0x0000, 0x0020, CRC(133bb744) SHA1(da4074f3ea30202973f0b6c9ad05a992bb44eafd) )
	ROM_LOAD( "aca_4a.bin",   0x0020, 0x0100, CRC(8e29208f) SHA1(a30a405fbd43d27a8d403f6c3545178564dede5d) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END


ROM_START( newpuc2 )
	ROM_REGION( 0x10000, REGION_CPU1,0 )     /* 64k for code */
	ROM_LOAD( "6e.cpu", 0x0000, 0x0800, CRC(69496a98) SHA1(2934051d6305cc3654951bc1aacf2b8902f463fe) )
	ROM_LOAD( "6k.cpu", 0x0800, 0x0800, CRC(158fc01c) SHA1(2f7a1e24d259fdc716ef8e7354a87780595f3c4e) )
	ROM_LOAD( "6f.cpu", 0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "6m.cpu", 0x1800, 0x0800, CRC(70810ccf) SHA1(3941678606aab1e53356a6781e24d84e83cc88ce) )
	ROM_LOAD( "6h.cpu", 0x2000, 0x0800, CRC(81719de8) SHA1(e886d04ac0e20562a4bd2df7676bdf9aa98665d7) )
	ROM_LOAD( "6n.cpu", 0x2800, 0x0800, CRC(3f250c58) SHA1(53bf2270c26f10f7e97960cd4c96e09e16b9bdf3) )
	ROM_LOAD( "6j.cpu", 0x3000, 0x0800, CRC(e6675736) SHA1(85d0bb79bc96acbc67fcb70ff4d453c870a6c8ea) )
	ROM_LOAD( "6p.cpu", 0x3800, 0x0800, CRC(1f81e765) SHA1(442d8a82e79ae842f1ffb46369c632c1d0b83161) )

	ROM_REGION( 0x2000, REGION_GFX1 , ROMREGION_DISPOSE)
	ROM_LOAD( "5e.cpu", 0x0000, 0x0800, CRC(2066a0b7) SHA1(6d4ccc27d6be185589e08aa9f18702b679e49a4a) )
	ROM_LOAD( "5h.cpu", 0x0800, 0x0800, CRC(777c70d3) SHA1(ed5ccbeb1102ec9f837577de3aa51317c32520d6) )
	ROM_LOAD( "5f.cpu", 0x1000, 0x0800, CRC(ca8c184c) SHA1(833aa845824ed80777b62f03df36a920ad7c3656) )
	ROM_LOAD( "5j.cpu", 0x1800, 0x0800, CRC(7dc75a81) SHA1(d3fe1cad3b594052d8367685febb2335b0ad62f4) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   /*timing - not used */
ROM_END


ROM_START( newpuc2b )
	ROM_REGION( 0x10000, REGION_CPU1,0 )     /* 64k for code */
	ROM_LOAD( "np2b1.bin", 0x0000, 0x0800, CRC(9d027c4a) SHA1(88e094880057451a75cdc2ce9477403021813982) )
	ROM_LOAD( "6k.cpu", 0x0800, 0x0800, CRC(158fc01c) SHA1(2f7a1e24d259fdc716ef8e7354a87780595f3c4e) )
	ROM_LOAD( "6f.cpu", 0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "6m.cpu", 0x1800, 0x0800, CRC(70810ccf) SHA1(3941678606aab1e53356a6781e24d84e83cc88ce) )
	ROM_LOAD( "np2b3.bin", 0x2000, 0x0800, CRC(f5e4b2b1) SHA1(68464f61cc50931f6cd4bb493dd703c139500825) )
	ROM_LOAD( "6n.cpu", 0x2800, 0x0800, CRC(3f250c58) SHA1(53bf2270c26f10f7e97960cd4c96e09e16b9bdf3) )
	ROM_LOAD( "np2b4.bin", 0x3000, 0x0800, CRC(f068e009) SHA1(a30763935e116559d535654827230bb21a5734bb) )
	ROM_LOAD( "np2b8.bin", 0x3800, 0x0800, CRC(1fadcc2f) SHA1(2d636cfc2b52b671ac5a26a03b1195e2cf8d4718) )

	ROM_REGION( 0x2000, REGION_GFX1 , ROMREGION_DISPOSE)
	ROM_LOAD( "5e.cpu", 0x0000, 0x0800, CRC(2066a0b7) SHA1(6d4ccc27d6be185589e08aa9f18702b679e49a4a) )
	ROM_LOAD( "5h.cpu", 0x0800, 0x0800, CRC(777c70d3) SHA1(ed5ccbeb1102ec9f837577de3aa51317c32520d6) )
	ROM_LOAD( "5f.cpu", 0x1000, 0x0800, CRC(ca8c184c) SHA1(833aa845824ed80777b62f03df36a920ad7c3656) )
	ROM_LOAD( "5j.cpu", 0x1800, 0x0800, CRC(7dc75a81) SHA1(d3fe1cad3b594052d8367685febb2335b0ad62f4) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   /*timing - not used */
ROM_END

ROM_START( nmouse )
	ROM_REGION( 0x10000, REGION_CPU1,0 )     /* 64k for code */
	ROM_LOAD( "naumouse.d7", 0x0000, 0x0800, CRC(e447ecfa) SHA1(45bce93f4a4e1c9994fb6b0c81691a14cae43ae5) )
	ROM_LOAD( "naumouse.d6", 0x0800, 0x0800, CRC(2e6f13d9) SHA1(1278bd1ddd84ac5b956cb4d25c151871fab2b1d9) )
	ROM_LOAD( "naumouse.e7", 0x1000, 0x0800, CRC(44a80f97) SHA1(d06ffd96c72c3c8a3c71df564e8f5f9fb289b398) )
	ROM_LOAD( "naumouse.e6", 0x1800, 0x0800, CRC(9c7a46bd) SHA1(04771a99295fc6d3c41807e2c4437ff4e7e4ba4a) )
	ROM_LOAD( "naumouse.h7", 0x2000, 0x0800, CRC(5bc94c5d) SHA1(9238add33bbde151532b7ce3917566d9b4f67c62) )
	ROM_LOAD( "naumouse.h6", 0x2800, 0x0800, CRC(1af29e22) SHA1(628291aa97f5f88793f624af66a0c2b021328ef9) )
	ROM_LOAD( "naumouse.j7", 0x3000, 0x0800, CRC(cc3be185) SHA1(92fdc87256d16c4e400da83e3ca2786012766767) )
	ROM_LOAD( "naumouse.j6", 0x3800, 0x0800, CRC(66b3e5dc) SHA1(0ca7e67ef0ff908bb9953399f024e8b1aaf74e55) )

	ROM_REGION( 0x2000, REGION_GFX1 , ROMREGION_DISPOSE)
	ROM_LOAD( "naumouse.d5", 0x0000, 0x0800, CRC(2ea7cc3f) SHA1(ffeca1c382a7ae0cd898eab2905a0e8e96b95bee) )
	ROM_LOAD( "naumouse.h5", 0x0800, 0x0800, CRC(0511fcea) SHA1(52a498ca024b5c758ad0c978d3f67cdbbf2c56d3) )
	ROM_LOAD( "naumouse.e5", 0x1000, 0x0800, CRC(f5a627cd) SHA1(2b8bc6d29e2aead924423a232c130151c8a8ebe5) )
	ROM_LOAD( "naumouse.j5", 0x1800, 0x0800, CRC(65f2580e) SHA1(769905837b98736ef2bfcaafa7820083dad80c57) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "naumouse.a4", 0x0020, 0x0100, CRC(d8772167) SHA1(782fa53f0de7262924a92d75f12a42bc4e44c812) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   /*timing - not used */
ROM_END

ROM_START( nmouseb )
	ROM_REGION( 0x10000, REGION_CPU1,0 )     /* 64k for code */
	ROM_LOAD( "naumouse.d7", 0x0000, 0x0800, CRC(e447ecfa) SHA1(45bce93f4a4e1c9994fb6b0c81691a14cae43ae5) )
	ROM_LOAD( "naumouse.d6", 0x0800, 0x0800, CRC(2e6f13d9) SHA1(1278bd1ddd84ac5b956cb4d25c151871fab2b1d9) )
	ROM_LOAD( "naumouse.e7", 0x1000, 0x0800, CRC(44a80f97) SHA1(d06ffd96c72c3c8a3c71df564e8f5f9fb289b398) )
	ROM_LOAD( "naumouse.e6", 0x1800, 0x0800, CRC(9c7a46bd) SHA1(04771a99295fc6d3c41807e2c4437ff4e7e4ba4a) )
	ROM_LOAD( "snatch2.bin", 0x2000, 0x0800, CRC(405aa389) SHA1(687c82d94309c4ed83b72d656dbe7068b1de1b44) )
	ROM_LOAD( "snatch6.bin", 0x2800, 0x0800, CRC(f58e7df4) SHA1(a0853374a2a8c3ab572154d12e2e6297c97bd8b9) )
	ROM_LOAD( "snatch3.bin", 0x3000, 0x0800, CRC(06fb18ec) SHA1(ad57ffdb0fc5acdddeb85c4ce3ad618124fd7a6d) )
	ROM_LOAD( "snatch7.bin", 0x3800, 0x0800, CRC(d187b82b) SHA1(db739d5894a7fbfbc2e384ee1bdfe170935b2df7) )

	ROM_REGION( 0x2000, REGION_GFX1 , ROMREGION_DISPOSE)
	ROM_LOAD( "naumouse.d5",  0x0000, 0x0800, CRC(2ea7cc3f) SHA1(ffeca1c382a7ae0cd898eab2905a0e8e96b95bee) )
	ROM_LOAD( "naumouse.h5",  0x0800, 0x0800, CRC(0511fcea) SHA1(52a498ca024b5c758ad0c978d3f67cdbbf2c56d3) )
	ROM_LOAD( "naumouse.e5",  0x1000, 0x0800, CRC(f5a627cd) SHA1(2b8bc6d29e2aead924423a232c130151c8a8ebe5) )
	ROM_LOAD( "snatch11.bin", 0x1800, 0x0800, CRC(330230a5) SHA1(3de4e454dd51b2ef05b5e1c74c8d12f8cb3f42ef) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "naumouse.a4", 0x0020, 0x0100, CRC(d8772167) SHA1(782fa53f0de7262924a92d75f12a42bc4e44c812) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   /*timing - not used */
ROM_END

ROM_START( woodpek )
	ROM_REGION( 0x10000, REGION_CPU1,0 )     /* 64k for code */
	/* roms dumped from epoxy block */
	ROM_LOAD( "f.bin", 0x0000, 0x1000, CRC(37ea66ca) SHA1(1779e2af8ffc72ec454a401cf6fa93e77e28576a) )
	ROM_LOAD( "i.bin", 0x8000, 0x1000, CRC(cd115dba) SHA1(51dfa1966fa391654622cd4ffdd09007ec38ea02) )
	ROM_LOAD( "e.bin", 0x9000, 0x1000, CRC(d40b2321) SHA1(0418cb772e24b67fd1d04e06daed33e766c8bc3d) )
	ROM_LOAD( "g.bin", 0xa000, 0x1000, CRC(024092f4) SHA1(4b16a3ff101397af64fc89d9f93bbdb939c8e699) )
	ROM_LOAD( "h.bin", 0xb000, 0x1000, CRC(18ef0fc8) SHA1(4cf3854adbcdd4ca2d855c48acff39fce5be48f7) )

	ROM_REGION( 0x2000, REGION_GFX1 , ROMREGION_DISPOSE)
	ROM_LOAD( "a.5e",  0x0000, 0x0800, CRC(15a87f62) SHA1(df6a6594ea8c6957e420b95e25ca33a9add13c09) )
	ROM_LOAD( "c.5h",  0x0800, 0x0800, CRC(ab4abd88) SHA1(04199a127556159878d719599d57a3548eb14a3c) )
	ROM_LOAD( "b.5f",  0x1000, 0x0800, CRC(5b9ba95b) SHA1(6d963da936c26830a614b69c663fc1e20b70f9dc) )
	ROM_LOAD( "d.5j",  0x1800, 0x0800, CRC(d7b80a45) SHA1(8f4ef319b960ae0e2cb30910b7efe6c0691df2bb) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "pr.8h", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "pr.4a", 0x0020, 0x0100, CRC(d8772167) SHA1(782fa53f0de7262924a92d75f12a42bc4e44c812) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "pr.1k", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "pr.3k", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   /*timing - not used */
ROM_END

ROM_START( woodpeka )
	ROM_REGION( 0x10000, REGION_CPU1,0 )     /* 64k for code */
	/* roms dumped from epoxy block */
	ROM_LOAD( "0", 0x0000, 0x1000, CRC(b5ee8bca) SHA1(b9a07dafa1b5ac26e28fd6520506c22b12881bc4) )
	ROM_LOAD( "1", 0x8000, 0x1000, NO_DUMP ) /* a rom is missing */
	ROM_LOAD( "2", 0x9000, 0x1000, CRC(07ea534e) SHA1(d93a9c35be21558b553ae8234b7d7e6e7e7e07f0) )
	ROM_LOAD( "3", 0xa000, 0x1000, CRC(a3a3253a) SHA1(e623bf11063570b7a8617a4590a6050cb73f61a2) )
	ROM_LOAD( "4", 0xb000, 0x1000, BAD_DUMP CRC(6c50546b) SHA1(1ca1c70a1722172036b30f99d7f6bf005dca9b79) )

	ROM_REGION( 0x2000, REGION_GFX1 , ROMREGION_DISPOSE)
	ROM_LOAD( "10.5f", 0x0000, 0x1000, CRC(0bf52102) SHA1(dfd8bb56e25b5599a7fdc9d7db8f9f5f2d7c4b03) )
	ROM_LOAD( "11.5h", 0x1000, 0x1000, CRC(0ed8def8) SHA1(542a6615b45776104f3731a34a899850bb40b5e0) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "pr.8h", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "pr.4a", 0x0020, 0x0100, CRC(d8772167) SHA1(782fa53f0de7262924a92d75f12a42bc4e44c812) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "pr.1k", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "pr.3k", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   /*timing - not used */
ROM_END

ROM_START( bigbucks )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "p.rom",        0x0000, 0x4000, CRC(eea6c1c9) SHA1(eaea4ffbcdfbb38364887830fd00ac87fe838006) )
	ROM_LOAD( "m.rom",        0x8000, 0x2000, CRC(bb8f7363) SHA1(11ebdb1a3c589515240d006646f2fb3ead06bdcf) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5e.cpu",       0x0000, 0x1000, CRC(18442c37) SHA1(fac445d15731532364315852492b48470039c0ca) )
	ROM_RELOAD( 0x1000, 0x1000 ) /* Not Used?? */

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )	/* timing - not used */

	ROM_REGION( 0x60000, REGION_USER1, 0 )	/* Question ROMs */
	ROM_LOAD( "rom1.rom",     0x00000, 0x8000, CRC(90b7785f) SHA1(7fc5aa2be868b87ffb9e957c204dabf1508e212e) )
	ROM_LOAD( "rom2.rom",     0x08000, 0x8000, CRC(60172d77) SHA1(92cb2312c6f3395f7ddb45e58695dd000d6ec756) )
	ROM_LOAD( "rom3.rom",     0x10000, 0x8000, CRC(a2207320) SHA1(18ad94b62e7e611ab8a1cbf2d2c6576b8840da2f) )
	ROM_LOAD( "rom4.rom",     0x18000, 0x8000, CRC(5a74c1f9) SHA1(0c55a27a492099ac98daefe0c199761ab1ccce82) )
	ROM_LOAD( "rom5.rom",     0x20000, 0x8000, CRC(93bc1080) SHA1(53e40b8bbc82b3be044f8a39b71fbb811e9263d8) )
	ROM_LOAD( "rom6.rom",     0x28000, 0x8000, CRC(eea2423f) SHA1(34de5495061be7d498773f9a723052c4f13d4a0c) )
	ROM_LOAD( "rom7.rom",     0x30000, 0x8000, CRC(96694055) SHA1(64ebbd85c2a60936f60345b5d573cd9eda196d3f) )
	ROM_LOAD( "rom8.rom",     0x38000, 0x8000, CRC(e68ebf8e) SHA1(cac17ac0231a0526e7f4a58bcb2cae3d05727ee6) )
	ROM_LOAD( "rom9.rom",     0x40000, 0x8000, CRC(fd20921d) SHA1(eedf93841b5ebe9afc4184e089d6694bbdb64445) )
	ROM_LOAD( "rom10.rom",    0x48000, 0x8000, CRC(5091b951) SHA1(224b65795d11599cdbd78e984ac2c71e8847041c) )
	ROM_LOAD( "rom11.rom",    0x50000, 0x8000, CRC(705128db) SHA1(92d45bfd09f61a1a3ac46c2e0ec1f634f04cf049) )
	ROM_LOAD( "rom12.rom",    0x58000, 0x8000, CRC(74c776e7) SHA1(03860d90461b01df4b734b784dddb20843ba811a) )
ROM_END


ROM_START( drivfrcp )
	ROM_REGION( 0x8000, REGION_CPU1, 0 )	/* 32k for code */
	ROM_LOAD( "drivforc.1",   0x0000, 0x1000, CRC(10b59d27) SHA1(fa09f3b95319a3487fa54b72198f41211663e087) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )

	ROM_REGION( 0x8000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "drivforc.2",   0x0000, 0x1000, CRC(56331cb5) SHA1(520f2a18ebbdfb093c8e4d144749a3f5cbce19bf) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x1000, 0x1000 )
	ROM_CONTINUE(             0x3000, 0x1000 )
	ROM_RELOAD(               0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_CONTINUE(             0x5000, 0x1000 )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "drivforc.pr1", 0x0000, 0x0020, CRC(045aa47f) SHA1(ea9034f441937df43a7c0bdb502165fb27d06635) )
	ROM_LOAD( "drivforc.pr2", 0x0020, 0x0100, CRC(9e6d2f1d) SHA1(7bcbcd4c0a40264c3b0667fc6a39ed4f2a86cafe) )
ROM_END


ROM_START( 8bpm )
	ROM_REGION( 0x8000, REGION_CPU1, 0 )	/* 32k for code */
	ROM_LOAD( "8bpmp.bin",    0x0000, 0x1000, CRC(b4f7eba7) SHA1(9b15543895c70f5ee2b4f91b8af78a884453e4f1) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )

	ROM_REGION( 0x8000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "8bpmc.bin",    0x0000, 0x1000, CRC(1c894a6d) SHA1(04e5c548290095d1d0f873b6c2e639e6dbe8ff35) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x1000, 0x1000 )
	ROM_CONTINUE(             0x3000, 0x1000 )
	ROM_RELOAD(               0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_CONTINUE(             0x5000, 0x1000 )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "8bpm.7f",      0x0000, 0x0020, CRC(4cf54241) SHA1(8d1db311941b8f821f949119d5ed2998a2fee80f) )
	ROM_LOAD( "8bpm.4a",      0x0020, 0x0100, CRC(618505a0) SHA1(6f7d4d54706b49d58427a60c2e2a48bd26d160d4) )
ROM_END


ROM_START( porky )
	ROM_REGION( 0x8000, REGION_CPU1, 0 )	/* 32k for code */
	ROM_LOAD( "pp",           0x0000, 0x1000, CRC(00592624) SHA1(41e554178a89b95bed1f570fab28e2a04f7a68d6) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )

	/* what is it used for ? */
	ROM_REGION( 0x4000, REGION_USER1, 0 )
	ROM_LOAD( "ps",           0x0000, 0x4000, CRC(2efb9861) SHA1(8c5a23ed15bd985af78a54d2121fe058e53703bb) )

	ROM_REGION( 0x8000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pc",           0x0000, 0x1000, CRC(a20e3d39) SHA1(3762289a495d597d6b9540ea7fa663128a9d543c) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x1000, 0x1000 )
	ROM_CONTINUE(             0x3000, 0x1000 )
	ROM_RELOAD(               0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_CONTINUE(             0x5000, 0x1000 )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "7f",           0x0000, 0x0020, CRC(98bce7cc) SHA1(e461862ccaf7526421631ac6ebb9b09cd0bc9909) )
	ROM_LOAD( "4a",           0x0020, 0x0100, CRC(30fe0266) SHA1(5081a19ceaeb937ee1378f3374e9d5949d17c3e8) )
ROM_END


ROM_START( rocktrv2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for code */
	ROM_LOAD( "1.aux",        0x0000, 0x4000, CRC(d182947b) SHA1(b778658386b2ed7c9f518cf20d7805ea62ae727b) )
	ROM_LOAD( "2.aux",        0x6000, 0x2000, CRC(27a7461d) SHA1(0cbd4a03dcff352fbd6b9a9009dc908e34553ee2) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5e.cpu",       0x0000, 0x1000, CRC(0a6cc43b) SHA1(a773bf3dda326797d63ceb908ad4d48f516bcea0) )
	ROM_RELOAD( 0x1000, 0x1000 ) /* Not Used?? */

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "7f.cpu",       0x0000, 0x0020, CRC(7549a947) SHA1(4f2c3e7d6c38f0b9a90317f91feb3f86c9a0d0a5) )
	ROM_LOAD( "4a.cpu",       0x0020, 0x0100, CRC(ddd5d88e) SHA1(f28e1d90bb495001c30c63b0ef2eec45de568174) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */

	ROM_REGION( 0x40000, REGION_USER1, 0 )	/* Question ROMs */
	ROM_LOAD( "3.aux",        0x00000, 0x4000, CRC(5b117ca6) SHA1(08d625312a751b99e132b90dcf8274d0ff2aecf2) )
	ROM_LOAD( "4.aux",        0x04000, 0x4000, CRC(81bfd4c3) SHA1(300cb4a38d3a1234bfc793f0574527033697f5a2) )
	ROM_LOAD( "5.aux",        0x08000, 0x4000, CRC(e976423c) SHA1(53a7f100943313014285ce09c03bd3eabd1388b0) )
	ROM_LOAD( "6.aux",        0x0c000, 0x4000, CRC(425946bf) SHA1(c8b0ba85bbba2f2c33f4ba069bf2fbb9692281d8) )
	ROM_LOAD( "7.aux",        0x10000, 0x4000, CRC(7056fc8f) SHA1(99c18ba4cd4d45531066069d2fd5018177072d5b) )
	ROM_LOAD( "8.aux",        0x14000, 0x4000, CRC(8b86464f) SHA1(7827df4c763fe078d3844eafab728e9400275049) )
	ROM_LOAD( "9.aux",        0x18000, 0x4000, CRC(17d8eba4) SHA1(806593824868e266c776e2e49cebb60dd6f8302e) )
	ROM_LOAD( "10.aux",       0x1c000, 0x4000, CRC(398c8eb4) SHA1(2cbbb11e255b84a54621f5fccfa8354bf925f1df) )
	ROM_LOAD( "11.aux",       0x20000, 0x4000, CRC(7f376424) SHA1(72ba5b01053c0c568562ba7a1257252c47736a3c) )
	ROM_LOAD( "12.aux",       0x24000, 0x4000, BAD_DUMP CRC(8d5bbf81) SHA1(0ebc9afbe6df6d60cf8797e246dda45694dca89e) )
	ROM_LOAD( "13.aux",       0x28000, 0x4000, CRC(99fe2c21) SHA1(9ff29cb2b74a16f5249677172b9d96e11241032e) )
	ROM_LOAD( "14.aux",       0x2c000, 0x4000, CRC(df4cf5e7) SHA1(1228a31b9053ade416a33f699f3f5513d1e47b24) )
	ROM_LOAD( "15.aux",       0x30000, 0x4000, CRC(2a32de26) SHA1(5892d4aea590d109339a66d15ebedaa04629fa7e) )
	ROM_LOAD( "16.aux",       0x34000, 0x4000, CRC(fcd42187) SHA1(e99e1f281eff2f6f42440f30bcb7a5efe34590fd) )
	ROM_LOAD( "17.aux",       0x38000, 0x4000, CRC(24d5c388) SHA1(f7039d84b3cbf00884e87ea7221f1b608a7d879e) )
	ROM_LOAD( "18.aux",       0x3c000, 0x4000, CRC(feb195fd) SHA1(5677d31e526cc7752254e9af0d694f05bc6bc907) )
ROM_END

ROM_START( mspactwn )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for encrypted code */
	ROM_LOAD( "m27256.bin",  0x0000, 0x4000, CRC(77a99184) SHA1(9dcb1a1b78994aa401d653bec571cb3e6f9d900b) )
	ROM_CONTINUE(0x8000,0x4000)

	ROM_REGION( 0x2000, REGION_GFX1, 0 )
	ROM_LOAD( "4__2716.5d",  0x0000, 0x0800, CRC(483c1d1c) SHA1(d3b967c6a71cf02b825d800f56d5268f2e0e60eb) )
	ROM_LOAD( "2__2716.5g",  0x0800, 0x0800, CRC(c08d73a2) SHA1(072e57641ac5ae3c47b4f8d9c55e3da5b35489ea) )
	ROM_LOAD( "3__2516.5f",  0x1000, 0x0800, CRC(22b0188a) SHA1(a9ed9ca8b36a60081fd364abc9bc23963932cc0b) )
	ROM_LOAD( "1__2516.5j",  0x1800, 0x0800, CRC(0a8c46a0) SHA1(e38e9e3258ab26fcbc6fdf258844e364f4b165ab) )

	ROM_REGION( 0x0120, REGION_PROMS, 0 )
	ROM_LOAD( "mb7051.8h",   0x0000, 0x0020, CRC(ff344446) SHA1(45eb37533da8912645a089b014f3b3384702114a) )
	ROM_LOAD( "82s129.4a",   0x0020, 0x0100, CRC(a8202d0d) SHA1(2a615211c33f3ef75af14e4bbedd2a700100be29) )

	ROM_REGION( 0x0200, REGION_SOUND1, 0 )	/* Sound PROMs */
	ROM_LOAD( "82s126.1m",   0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",   0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
ROM_END

/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static void maketrax_rom_decode(void)
{
	UINT8 *decrypted = auto_malloc(0x4000);
	UINT8 *rom = memory_region(REGION_CPU1);

	/* patch protection using a copy of the opcodes so ROM checksum */
	/* tests will not fail */
	memory_set_decrypted_region(0, 0x0000, 0x3fff, decrypted);

	memcpy(decrypted,rom,0x4000);

	decrypted[0x0415] = 0xc9;
	decrypted[0x1978] = 0x18;
	decrypted[0x238e] = 0xc9;
	decrypted[0x3ae5] = 0xe6;
	decrypted[0x3ae7] = 0x00;
	decrypted[0x3ae8] = 0xc9;
	decrypted[0x3aed] = 0x86;
	decrypted[0x3aee] = 0xc0;
	decrypted[0x3aef] = 0xb0;
}

static DRIVER_INIT( maketrax )
{
	/* set up protection handlers */
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x5080, 0x50bf, 0, 0, maketrax_special_port2_r);
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x50c0, 0x50ff, 0, 0, maketrax_special_port3_r);

	maketrax_rom_decode();
}

static void korosuke_rom_decode(void)
{
	UINT8 *decrypted = auto_malloc(0x4000);
	UINT8 *rom = memory_region(REGION_CPU1);

	/* patch protection using a copy of the opcodes so ROM checksum */
	/* tests will not fail */
	memory_set_decrypted_region(0, 0x0000, 0x3fff, decrypted);

	memcpy(decrypted,rom,0x4000);

	decrypted[0x044c] = 0xc9;
	decrypted[0x1973] = 0x18;
	decrypted[0x238c] = 0xc9;
	decrypted[0x3ae9] = 0xe6;	/* not changed */
	decrypted[0x3aeb] = 0x00;
	decrypted[0x3aec] = 0xc9;
	decrypted[0x3af1] = 0x86;
	decrypted[0x3af2] = 0xc0;
	decrypted[0x3af3] = 0xb0;
}

static DRIVER_INIT( korosuke )
{
	/* set up protection handlers */
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x5080, 0x5080, 0, 0, korosuke_special_port2_r);
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x50c0, 0x50ff, 0, 0, korosuke_special_port3_r);

	korosuke_rom_decode();
}

static DRIVER_INIT( ponpoko )
{
	/* The gfx data is swapped wrt the other Pac-Man hardware games. */
	/* Here we revert it to the usual format. */

	int i, j;
	UINT8 *RAM, temp;
	int length = memory_region_length(REGION_GFX1)/2;

	/* Characters */
	RAM = memory_region(REGION_GFX1);
	for (i = 0;i < length;i += 0x10)
	{
		for (j = 0; j < 8; j++)
		{
			temp          = RAM[i+j+0x08];
			RAM[i+j+0x08] = RAM[i+j+0x00];
			RAM[i+j+0x00] = temp;
		}
	}

	/* Sprites */
	RAM = memory_region(REGION_GFX1)+length;
	for (i = 0;i < length;i += 0x20)
	{
		for (j = 0; j < 8; j++)
		{
			temp          = RAM[i+j+0x18];
			RAM[i+j+0x18] = RAM[i+j+0x10];
			RAM[i+j+0x10] = RAM[i+j+0x08];
			RAM[i+j+0x08] = RAM[i+j+0x00];
			RAM[i+j+0x00] = temp;
		}
	}
}

static void eyes_decode(UINT8 *data)
{
	int j;
	UINT8 swapbuffer[8];

	for (j = 0; j < 8; j++)
	{
		swapbuffer[j] = data[BITSWAP16(j,15,14,13,12,11,10,9,8,7,6,5,4,3,0,1,2)];
	}

	for (j = 0; j < 8; j++)
	{
		data[j] = BITSWAP8(swapbuffer[j],7,4,5,6,3,2,1,0);
	}
}

static DRIVER_INIT( eyes )
{
	int i;
	UINT8 *RAM;

	/* CPU ROMs */

	/* Data lines D3 and D5 swapped */
	RAM = memory_region(REGION_CPU1);
	for (i = 0; i < 0x4000; i++)
	{
		RAM[i] = BITSWAP8(RAM[i],7,6,3,4,5,2,1,0);
	}


	/* Graphics ROMs */

	/* Data lines D4 and D6 and address lines A0 and A2 are swapped */
	RAM = memory_region(REGION_GFX1);
	for (i = 0;i < memory_region_length(REGION_GFX1);i += 8)
		eyes_decode(&RAM[i]);
}

static DRIVER_INIT( woodpek )
{
	int i;
	UINT8 *RAM;

	/* Graphics ROMs */

	/* Data lines D4 and D6 and address lines A0 and A2 are swapped */
	RAM = memory_region(REGION_GFX1);
	for (i = 0;i < memory_region_length(REGION_GFX1);i += 8)
		eyes_decode(&RAM[i]);
}

static DRIVER_INIT( pacplus )
{
	pacplus_decode();
}

static DRIVER_INIT( jumpshot )
{
	jumpshot_decode();
}

static DRIVER_INIT( 8bpm )
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	int i;

	/* Data lines D0 and D6 swapped */
	for( i = 0; i < 0x8000; i++ )
	{
		RAM[i] = BITSWAP8(RAM[i],7,0,5,4,3,2,1,6);
	}
}

static DRIVER_INIT( porky )
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	int i;

	/* Data lines D0 and D4 swapped */
	for(i = 0; i < 0x8000; i++)
	{
		RAM[i] = BITSWAP8(RAM[i],7,6,5,0,3,2,1,4);
	}

}

static DRIVER_INIT( rocktrv2 )
{
	/* hack to pass the rom check for the bad rom */
	UINT8 *ROM = memory_region(REGION_CPU1);

	ROM[0x7ffe] = 0xa7;
	ROM[0x7fee] = 0x6d;
}

/* The encrpytion is provided by a 74298 sitting on top of the rom at 6f.
The select line is tied to a2; a0 and a1 of the eprom are are left out of
socket and run through the 74298.  Clock is tied to system clock.  */
static DRIVER_INIT( mspacmbe )
{
	UINT8 temp;
	UINT8 *RAM = memory_region(REGION_CPU1);
	int i;

	/* Address lines A1 and A0 swapped if A2=0 */
	for(i = 0x1000; i < 0x2000; i+=4)
	{
	    if (!(i & 8))
	    {
	          temp = RAM[i+1];
	          RAM[i+1] = RAM[i+2];
	          RAM[i+2] = temp;
	    };
	}
}


/*************************************
 *
 *  Game drivers
 *
 *************************************/

/*          rom       parent    machine   inp       init */
GAME( 1980, puckman,  0,        pacman,   pacman,   0,        ROT90,  "Namco", "PuckMan (Japan set 1, Probably Bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1980, puckmana, puckman,  pacman,   pacman,   0,        ROT90,  "Namco", "PuckMan (Japan set 2)", GAME_SUPPORTS_SAVE )
GAME( 1980, puckmanf, puckman,  pacman,   pacman,   0,        ROT90,  "Namco", "PuckMan (Japan set 1 with speedup hack)", GAME_SUPPORTS_SAVE )
GAME( 1980, puckmanh, puckman,  pacman,   pacman,   0,        ROT90,  "hack", "Puckman (Falcom?)", GAME_SUPPORTS_SAVE )
GAME( 1980, pacman,   puckman,  pacman,   pacman,   0,        ROT90,  "[Namco] (Midway license)", "Pac-Man (Midway)", GAME_SUPPORTS_SAVE )
GAME( 1980, pacmanf,  puckman,  pacman,   pacman,   0,        ROT90,  "[Namco] (Midway license)", "Pac-Man (Midway, with speedup hack)", GAME_SUPPORTS_SAVE )
GAME( 1981, puckmod,  puckman,  pacman,   pacman,   0,        ROT90,  "Namco", "PuckMan (Japan set 3)", GAME_SUPPORTS_SAVE )
GAME( 1981, pacmod,   puckman,  pacman,   pacman,   0,        ROT90,  "[Namco] (Midway license)", "Pac-Man (Midway, harder)", GAME_SUPPORTS_SAVE )
GAME( 1980, newpuc2,  puckman,  pacman,   pacman,   0,        ROT90,  "hack", "Newpuc2", GAME_SUPPORTS_SAVE )
GAME( 1980, newpuc2b, puckman,  pacman,   pacman,   0,        ROT90,  "hack", "Newpuc2 (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1980, newpuckx, puckman,  pacman,   pacman,   0,        ROT90,  "hack", "New Puck-X", GAME_SUPPORTS_SAVE )
GAME( 1981, pacheart, puckman,  pacman,   pacman,   0,        ROT90,  "hack", "Pac-Man (Hearts)", GAME_SUPPORTS_SAVE )
GAME( 1981, hangly,   puckman,  pacman,   pacman,   0,        ROT90,  "hack", "Hangly-Man (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1981, hangly2,  puckman,  pacman,   pacman,   0,        ROT90,  "hack", "Hangly-Man (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1981, hangly3,  puckman,  pacman,   pacman,   0,        ROT90,  "hack", "Hangly-Man (set 3)", GAME_SUPPORTS_SAVE )
GAME( 1981, piranhah, puckman,  pacman,   mspacman, 0,        ROT90,  "hack", "Piranha (hack)", GAME_SUPPORTS_SAVE )
GAME( 1981, crush,    0,        pacman,   maketrax, maketrax, ROT90,  "Kural Samno Electric", "Crush Roller (Kural Samno)", GAME_SUPPORTS_SAVE )
GAME( 1981, crush2,   crush,    pacman,   maketrax, 0,        ROT90,  "Kural Esco Electric", "Crush Roller (Kural Esco - bootleg?)", GAME_SUPPORTS_SAVE )
GAME( 1981, crush3,   crush,    pacman,   maketrax, eyes,     ROT90,  "Kural Electric", "Crush Roller (Kural - bootleg?)", GAME_SUPPORTS_SAVE )
GAME( 19??, crush4,   crush,    crush4,   crush4,   0,        ROT90,  "Kural TWT", "Crush Roller (Kural TWT)", GAME_SUPPORTS_SAVE )
GAME( 1981, maketrax, crush,    pacman,   maketrax, maketrax, ROT270, "[Kural] (Williams license)", "Make Trax (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1981, maketrxb, crush,    pacman,   maketrax, maketrax, ROT270, "[Kural] (Williams license)", "Make Trax (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1981, korosuke, crush,    pacman,   korosuke, korosuke, ROT90,  "Kural Electric", "Korosuke Roller", GAME_SUPPORTS_SAVE )
GAME( 1981, mbrush,   crush,    pacman,   mbrush,   0,        ROT90,  "bootleg", "Magic Brush", GAME_SUPPORTS_SAVE )
GAME( 1981, paintrlr, crush,    pacman,   paintrlr, 0,        ROT90,  "bootleg", "Paint Roller", GAME_SUPPORTS_SAVE )
GAME( 1982, pacplus,  0,        pacman,   pacman,   pacplus,  ROT90,  "[Namco] (Midway license)", "Pac-Man Plus", GAME_SUPPORTS_SAVE )
GAME( 1982, joyman,   puckman,  pacman,   pacman,   0,        ROT90,  "hack", "Joyman", GAME_SUPPORTS_SAVE )
GAME( 1982, ctrpllrp, puckman,  pacman,   pacman,   0,        ROT90,  "hack", "Caterpillar Pacman Hack", GAME_SUPPORTS_SAVE )
GAME( 1982, eyes,     0,        pacman,   eyes,     eyes,     ROT90,  "Digitrex Techstar (Rock-ola license)", "Eyes (Digitrex Techstar)", GAME_SUPPORTS_SAVE )
GAME( 1982, eyes2,    eyes,     pacman,   eyes,     eyes,     ROT90,  "Techstar (Rock-ola license)", "Eyes (Techstar)", GAME_SUPPORTS_SAVE )
GAME( 1982, eyeszac,  eyes,     pacman,   eyes,     0,        ROT90,  "Zaccaria / bootleg", "Eyes (Zaccaria)", GAME_SUPPORTS_SAVE )
GAME( 1983, mrtnt,    0,        pacman,   mrtnt,    eyes,     ROT90,  "Telko", "Mr. TNT", GAME_SUPPORTS_SAVE )
GAME( 1983, gorkans,  mrtnt,    pacman,   mrtnt,    0,        ROT90,  "Techstar", "Gorkans", GAME_SUPPORTS_SAVE )
GAME( 1983, eggor,    0,        pacman,   mrtnt,    eyes,     ROT90,  "Telko", "Eggor", GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1985, jumpshot, 0,        pacman,   jumpshot, jumpshot, ROT90,  "Bally Midway", "Jump Shot", GAME_SUPPORTS_SAVE )
GAME( 1985, jumpshtp, jumpshot, pacman,   jumpshtp, jumpshot, ROT90,  "Bally Midway", "Jump Shot Engineering Sample", GAME_SUPPORTS_SAVE )
GAME( 1985, shootbul, 0,        pacman,   shootbul, jumpshot, ROT90,  "Bally Midway", "Shoot the Bull", GAME_SUPPORTS_SAVE )
GAME( 1981, piranha,  puckman,  piranha,  mspacman, eyes,     ROT90,  "GL (US Billiards License)", "Piranha", GAME_SUPPORTS_SAVE )
GAME( 1981, piranhao, puckman,  piranha,  mspacman, eyes,     ROT90,  "GL (US Billiards License)", "Piranha (older)", GAME_SUPPORTS_SAVE )
GAME( 1981, abscam,   puckman,  piranha,  mspacman, eyes,     ROT90,  "GL (US Billiards License)", "Abscam", GAME_SUPPORTS_SAVE )
GAME( 1981, nmouse,   0	     ,  nmouse ,  nmouse,   eyes,     ROT90,  "Amenip (Palcom Queen River)", "Naughty Mouse (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1981, nmouseb,  nmouse ,  nmouse ,  nmouse,   eyes,     ROT90,  "Amenip Nova Games Ltd.", "Naughty Mouse (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1981, mspacman, 0,        mspacman, mspacman, 0,        ROT90,  "Midway", "Ms. Pac-Man", GAME_SUPPORTS_SAVE )
GAME( 1981, mspacmnf, mspacman, mspacman, mspacman, 0,        ROT90,  "Midway", "Ms. Pac-Man (with speedup hack)", GAME_SUPPORTS_SAVE )
GAME( 1981, mspacmat, mspacman, mspacman, mspacman, 0,        ROT90,  "hack", "Ms. Pac Attack", GAME_SUPPORTS_SAVE )
GAME( 1981, woodpek,  0,        woodpek,  woodpek,  woodpek,  ROT90,  "Amenip (Palcom Queen River)", "Woodpecker (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1981, woodpeka, woodpek,  woodpek,  woodpek,  woodpek,  ROT90,  "Amenip", "Woodpecker (set 2)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1981, mspacmab, mspacman, woodpek,  mspacman, 0,        ROT90,  "bootleg", "Ms. Pac-Man (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1981, mspacmbe, mspacman, woodpek,  mspacman, mspacmbe, ROT90,  "bootleg", "Ms. Pac-Man (bootleg, (encrypted))", GAME_SUPPORTS_SAVE )
GAME( 1981, pacgal,   mspacman, woodpek,  mspacman, 0,        ROT90,  "hack", "Pac-Gal", GAME_SUPPORTS_SAVE )
GAME( 1981, mspacpls, mspacman, woodpek,  mspacpls, 0,        ROT90,  "hack", "Ms. Pac-Man Plus", GAME_SUPPORTS_SAVE )
GAME( 1982, ponpoko,  0,        woodpek,  ponpoko,  ponpoko,  ROT0,   "Sigma Enterprises Inc.", "Ponpoko", GAME_SUPPORTS_SAVE )
GAME( 1982, ponpokov, ponpoko,  woodpek,  ponpoko,  ponpoko,  ROT0,   "Sigma Enterprises Inc. (Venture Line license)", "Ponpoko (Venture Line)", GAME_SUPPORTS_SAVE )
GAME( 1985, lizwiz,   0,        woodpek,  lizwiz,   0,        ROT90,  "Techstar (Sunn license)", "Lizard Wizard", GAME_SUPPORTS_SAVE )
GAME( 1982, alibaba,  0,        alibaba,  alibaba,  0,        ROT90,  "Sega", "Ali Baba and 40 Thieves", GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE )
GAME( 1982, dremshpr, 0,        dremshpr, dremshpr, 0,        ROT270, "Sanritsu", "Dream Shopper", GAME_SUPPORTS_SAVE )
GAME( 1983, vanvan,   0,        vanvan,   vanvan,   0,        ROT270, "Sanritsu", "Van-Van Car", GAME_SUPPORTS_SAVE )
GAME( 1983, vanvank,  vanvan,   vanvan,   vanvank,  0,        ROT270, "Karateco", "Van-Van Car (Karateco)", GAME_SUPPORTS_SAVE )
GAME( 1983, vanvanb,  vanvan,   vanvan,   vanvank,  0,        ROT270, "Karateco", "Van-Van Car (set 3)", GAME_SUPPORTS_SAVE )
GAME( 1983, bwcasino, 0,        acitya,   bwcasino, 0,        ROT90,  "Epos Corporation", "Boardwalk Casino", GAME_SUPPORTS_SAVE )
GAME( 1983, acitya,   bwcasino, acitya,   acitya,   0,        ROT90,  "Epos Corporation", "Atlantic City Action", GAME_SUPPORTS_SAVE )
GAME( 1983, theglobp, suprglob, theglobp, theglobp, 0,        ROT90,  "Epos Corporation", "The Glob (Pac-Man hardware)", GAME_SUPPORTS_SAVE )
GAME( 1983, sprglobp, suprglob, theglobp, theglobp, 0,        ROT90,  "Epos Corporation", "Super Glob (Pac-Man hardware)", GAME_SUPPORTS_SAVE )
GAME( 1983, sprglbpg, suprglob, pacman,   theglobp, 0,        ROT90,  "Bootleg", "Super Glob (Pac-Man hardware) German", GAME_SUPPORTS_SAVE )
GAME( 1984, beastf,   suprglob, theglobp, theglobp, 0,        ROT90,  "Epos Corporation", "Beastie Feastie", GAME_SUPPORTS_SAVE )
GAME( 1984, drivfrcp, 0,        drivfrcp, drivfrcp, 0,        ROT90,  "Shinkai Inc. (Magic Eletronics Inc. licence)", "Driving Force (Pac-Man conversion)", GAME_SUPPORTS_SAVE )
GAME( 1985, 8bpm,     8ballact, 8bpm,     8bpm,     8bpm,     ROT90,  "Seatongrove Ltd (Magic Eletronics USA licence)", "Eight Ball Action (Pac-Man conversion)", GAME_SUPPORTS_SAVE )
GAME( 1985, porky,    0,        porky,    porky,    porky,    ROT90,  "Shinkai Inc. (Magic Eletronics Inc. licence)", "Porky", GAME_NO_SOUND )
GAME( 1986, rocktrv2, 0,        rocktrv2, rocktrv2, rocktrv2, ROT90,  "Triumph Software Inc.", "MTV Rock-N-Roll Trivia (Part 2)", GAME_SUPPORTS_SAVE )
GAME( 1986, bigbucks, 0,        bigbucks, bigbucks, 0,        ROT90,  "Dynasoft Inc.", "Big Bucks", GAME_SUPPORTS_SAVE )
GAME( 1995, mschamp,  mspacman, mschamp,  mschamp,  0,        ROT90,  "hack", "Ms. Pacman Champion Edition / Super Zola Pac Gal", GAME_SUPPORTS_SAVE )

/* Simultaneous 2 player hack of Ms Pac-Man */
GAME( 1992, mspactwn, 0,       mspactwin, mspactwin, mspactwin,      ROT90,  "SUSILU", "Ms. Pac-Man Twin (Argentina)", GAME_SUPPORTS_SAVE )
