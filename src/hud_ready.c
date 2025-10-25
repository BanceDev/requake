/*
Copyright (C) 2025 Lance Borden

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

$Id: hud_mapvote.c,v 1.0 2025-09-25 Bance $
*/

#include "quakedef.h"
#include "hud.h"
#include "keys.h"

static char ready_str[32];
static char break_str[32];

void SCR_HUD_DrawReady(hud_t *hud)
{
    if (!cl.standby) {
        return;
    }

	int numchars = strlen(ready_str);
    int x, y;
    cvar_t *v_scale = HUD_FindVar(hud, "scale");
    float scale = bound(0.1f, v_scale->value, 10.0f);

    if (!HUD_PrepareDraw(hud, scale * numchars * 8, scale * 8, &x, &y))
        return;

    if (is_ready) {
		Draw_SStringAlpha(x, y, break_str, scale, 1.0f, 0);
	} else {
		Draw_SStringAlpha(x, y, ready_str, scale, 1.0f, 0);
	}
}


void Ready_HudInit(void)
{
	int r_key = Key_FindBinding("toggleready");
	int b_key = Key_FindBinding("toggleready");
	if (r_key == -1) {
		r_key = Key_FindBinding("ready");
		b_key = Key_FindBinding("break");
	}

    sprintf(ready_str, "Press %s To Ready Up.", Key_KeynumToString(r_key));
    sprintf(break_str, "Press %s To Unready.", Key_KeynumToString(b_key));

    HUD_Register(
        "readyup", NULL, "Ready up reminder HUD element.",
        0, ca_active, 0, SCR_HUD_DrawReady,
        "1", "screen", "center", "top", "0", "150", "0", "0 0 0 0", NULL,
        "scale", "1",
        NULL
    );
}

