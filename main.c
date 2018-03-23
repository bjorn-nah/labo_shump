#include <genesis.h>

#include "gfx.h"
#include "sprite.h"

#define MAX_SPEED       FIX32(8)
#define ACCEL           FIX32(1)
#define DE_ACCEL        FIX32(0.5)

#define MIN_POSX        FIX32(10)
#define MAX_POSX        FIX32(300)
#define MIN_POSY        FIX32(10)
#define MAX_POSY        FIX32(224)

#define MAX_BULLET      10
#define MAX_ENEMY       10

#define BULLET_SPEED	2


// forward
static void handleInput();
static void joyEvent(u16 joy, u16 changed, u16 state);

static void updatePhysic();
static void updateCamera(u16 x);

// sprites structure (pointer of Sprite)
Sprite* sprites[1+MAX_BULLET+MAX_ENEMY];
u16 bullet_ls[3][MAX_BULLET];	// X, Y, State

fix32 camposx;
fix32 camposy;
fix32 posx;
fix32 posy;
fix32 movx;
fix32 movy;
s16 xorder;
s16 yorder;

fix32 enemyPosx[2];
fix32 enemyPosy[2];
s16 enemyXorder[2];

int main()
{
    u16 palette[64];
    u16 ind;
	u16 i;
	u16 counter;

    // disable interrupt when accessing VDP
    SYS_disableInts();
    // initialization
    VDP_setScreenWidth320();


    // init sprites engine
    SPR_init(16, 256, 256);

    // set all palette to black
    VDP_setPaletteColors(0, (u16*) palette_black, 64);

    // load background
	
    ind = TILE_USERINDEX;
    VDP_drawImageEx(PLAN_B, &plan_b, TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
    ind += plan_b.tileset->numTile;
    VDP_drawImageEx(PLAN_A, &plan_a, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
    ind += plan_a.tileset->numTile;
	

    // VDP process done, we can re enable interrupts
    SYS_enableInts();

    camposx = -1;
    camposy = -1;
    posx = FIX32(48);
    posy = FIX32(48);
    movx = FIX32(0);
    movy = FIX32(0);
    xorder = 0;
    yorder = 0;
	counter = 0;


    // init scrolling
	updateCamera(counter);
	
	// init bullet
	for(i = 0; i < MAX_BULLET; i++)
	{
		bullet_ls[2][i] = 0;
		sprites[1+i] = SPR_addSprite(&bullet, fix32ToInt(posx), fix32ToInt(posy), TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
		SPR_setVisibility(sprites[i+1],HIDDEN);
	}

    // init ship sprite
    sprites[0] = SPR_addSprite(&labo_ship, fix32ToInt(posx), fix32ToInt(posy), TILE_ATTR(PAL2, TRUE, FALSE, FALSE));

    SPR_update();

    // prepare palettes
    memcpy(&palette[0], plan_a.palette->data, 16 * 2);
    memcpy(&palette[16], plan_b.palette->data, 16 * 2);

    memcpy(&palette[32], labo_ship.palette->data, 16 * 2);
	memcpy(&palette[48], bullet.palette->data, 16 * 2);

    // fade in
    VDP_fadeIn(0, (4 * 16) - 1, palette, 20, FALSE);

    JOY_setEventHandler(joyEvent);

    while(TRUE)
    {
		counter += 1;
        handleInput();

        updatePhysic();

        // update sprites
        SPR_update();
		
		updateCamera(counter);

        VDP_waitVSync();
    }

    return 0;
}

static void updatePhysic()
{
	u16 i;

    // ship physic
    if (xorder > 0)
    {
        movx += ACCEL;
        // going opposite side, quick breaking
        if (movx < 0) movx += ACCEL;

        if (movx >= MAX_SPEED) movx = MAX_SPEED;
    }
    else if (xorder < 0)
    {
        movx -= ACCEL;
        // going opposite side, quick breaking
        if (movx > 0) movx -= ACCEL;

        if (movx <= -MAX_SPEED) movx = -MAX_SPEED;
    }
	else
    {
        //movx = 0;
		if (movx > 0) movx -= DE_ACCEL;
		if (movx < 0) movx += DE_ACCEL;
    } 
	if (yorder > 0)
    {
        movy += ACCEL;
        // going opposite side, quick breaking
        if (movy < 0) movy += ACCEL;

        if (movy >= MAX_SPEED) movy = MAX_SPEED;
    }
    else if (yorder < 0)
    {
        movy -= ACCEL;
        // going opposite side, quick breaking
        if (movy > 0) movy -= ACCEL;

        if (movy <= -MAX_SPEED) movy = -MAX_SPEED;
    }
    else
    {
        //movy = 0;
		if (movy > 0) movy -= DE_ACCEL;
		if (movy < 0) movy += DE_ACCEL;
    } 

    posx += movx;
    posy += movy;

    if (posx >= MAX_POSX)
    {
        posx = MAX_POSX;
        movx = 0;
    }
    else if (posx <= MIN_POSX)
    {
        posx = MIN_POSX;
        movx = 0;
    }

	if (posy >= MAX_POSY)
    {
        posy = MAX_POSY;
        movy = 0;
    }
    else if (posy<= MIN_POSY)
    {
        posy = MIN_POSY;
        movy = 0;
    }
	
	// shoot motor
	for(i = 0; i < MAX_BULLET; i++)
		{
			if(bullet_ls[0][i] > fix32ToInt(MAX_POSX))
			{
				SPR_setVisibility(sprites[i+1],HIDDEN);
				bullet_ls[2][i] = 0;
			}
			if(bullet_ls[2][i] == 1)
			{
				bullet_ls[0][i] += BULLET_SPEED;
				SPR_setPosition(sprites[i+1], bullet_ls[0][i], bullet_ls[1][i]);
			}
		}

    // set sprites position
    SPR_setPosition(sprites[0], fix32ToInt(posx), fix32ToInt(posy));

}

static void updateCamera(u16 x)
{
    VDP_setHorizontalScroll(PLAN_A, -x);
    VDP_setHorizontalScroll(PLAN_B, -x >> 3);
	VDP_setVerticalScroll(PLAN_A, 0);
    VDP_setVerticalScroll(PLAN_B, 0);
}

static void handleInput()
{
    u16 value = JOY_readJoypad(JOY_1);

    if (value & BUTTON_UP) yorder = -1;
    else if (value & BUTTON_DOWN) yorder = +1;
    else yorder = 0;

    if (value & BUTTON_LEFT) xorder = -1;
    else if (value & BUTTON_RIGHT) xorder = +1;
    else xorder = 0;
}

static void joyEvent(u16 joy, u16 changed, u16 state)
{
	u16 i;
    // START button state changed
    if (changed & BUTTON_START)
    {

    }

    if (changed & state & (BUTTON_A | BUTTON_B | BUTTON_C))
    {
		// shoot
		for(i = 0; i < MAX_BULLET; i++)
		{
			if(bullet_ls[2][i] == 0)
			{
				bullet_ls[0][i] = fix32ToInt(posx)+32;
				bullet_ls[1][i] = fix32ToInt(posy)+8;
				bullet_ls[2][i] = 1;
				SPR_setVisibility(sprites[i+1],VISIBLE);
				break;
			}
		}
        // if (movy == 0)
        // {
            // movy = JUMP_SPEED;
        // }
    }
}
