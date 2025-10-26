/*
Copyright (C) 2011 azazello and ezQuake team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "quakedef.h"
#include "teamplay.h"
#include "hud.h"
#include "hud_common.h"

typedef enum
{
	wpNONE = 0,
	wpAXE,
	wpSG,
	wpSSG,
	wpNG,
	wpSNG,
	wpGL,
	wpRL,
	wpLG,
	wpMAX
} weaponName_t;

typedef struct wpType_s {
	int hits;			// hits with this weapon, for SG and SSG this is count of bullets
	int attacks;		// all attacks with this weapon, for SG and SSG this is count of bullets
} wpType_t;

typedef struct ws_player_s {
	int 		client;

	wpType_t	wpn[wpMAX];
} ws_player_t;

static ws_player_t ws_clients[MAX_CLIENTS];

static weaponName_t WS_NameToNum(const char *name)
{
	if ( !strcmp(name, "axe") )
		return wpAXE;
	if ( !strcmp(name, "sg") )
		return wpSG;
	if ( !strcmp(name, "ssg") )
		return wpSSG;
	if ( !strcmp(name, "ng") )
		return wpNG;
	if ( !strcmp(name, "sng") )
		return wpSNG;
	if ( !strcmp(name, "gl") )
		return wpGL;
	if ( !strcmp(name, "rl") )
		return wpRL;
	if ( !strcmp(name, "lg") )
		return wpLG;

	return wpNONE;
}

// reads weapon stats string from server/demo
void Parse_WeaponStats(char *s)
{
	int		client, arg;
	weaponName_t wp;

	Cmd_TokenizeString( s );

	arg = 1;

	client = atoi( Cmd_Argv( arg++ ) );

	if (client < 0 || client >= MAX_CLIENTS)
	{
		Com_DPrintf("Parse_WeaponStats: wrong client %d\n", client);
		return;
	}

	ws_clients[ client ].client = client; // no, its not stupid

	wp = WS_NameToNum( Cmd_Argv( arg++ ) );

	if ( wp == wpNONE )
	{
		Com_DPrintf("Parse_WeaponStats: wrong weapon\n");
		return;
	}

	ws_clients[ client ].wpn[wp].attacks = atoi( Cmd_Argv( arg++ ) );
	ws_clients[ client ].wpn[wp].hits    = atoi( Cmd_Argv( arg++ ) );
}

// called when new map spawned
void SCR_ClearWeaponStats(void)
{
	memset(ws_clients, 0, sizeof(ws_clients));
}

// 
static void SCR_CreateWeaponStatsPlayerText(ws_player_t *ws_cl, char* input_string, char* output_string, int max_length)
{
	char *s, tmp2[MAX_MACRO_STRING];
	int new_length = 0;

	*output_string = '\0';
	if (!ws_cl) {
		return;
	}

	// this limit len of string because TP_ParseFunChars() do not check overflow
	strlcpy(tmp2, input_string , sizeof(tmp2));
	strlcpy(tmp2, TP_ParseFunChars(tmp2, false), sizeof(tmp2));

	for (s = tmp2; *s && new_length < max_length - 1; ++s) {
		int wp;
		qbool percentage = (*s == '%');

		if (*s != '%' && *s != '#') {
			output_string[new_length++] = *s;
			continue;
		}

		if (*s == s[1]) {
			output_string[new_length++] = *s;
			++s;
			continue;
		}

		++s;
		wp = (int)s[0] - '0'; 
		if (wp <= wpNONE || wp >= wpMAX) {
			continue; // unsupported weapon
		}

		if (percentage) {
			float accuracy = (ws_cl->wpn[wp].attacks ? 100.0f * ws_cl->wpn[wp].hits / ws_cl->wpn[wp].attacks : 0);

			new_length += snprintf(output_string + new_length, max_length - new_length - 1, "%.1f", accuracy);
		}
		else {
			new_length += snprintf(output_string + new_length, max_length - new_length - 1, "%d", ws_cl->wpn[wp].hits);
		}
	}
	output_string[new_length] = '\0';
}

void SCR_HUD_WeaponStats(hud_t *hud)
{
	char content[128];
	int x, y;
	int i;
	int alignment = 0;

	static cvar_t
		*hud_weaponstats_format = NULL,
		*hud_weaponstats_textalign,
		*hud_weaponstats_scale,
		*hud_weaponstats_proportional;

	if (hud_weaponstats_format == NULL) {
		// first time
		hud_weaponstats_format = HUD_FindVar(hud, "format");
		hud_weaponstats_textalign = HUD_FindVar(hud, "textalign");
		hud_weaponstats_scale = HUD_FindVar(hud, "scale");
		hud_weaponstats_proportional = HUD_FindVar(hud, "proportional");
	}

	if (!strcmp(hud_weaponstats_textalign->string, "right")) {
		alignment = 1;
	}
	else if (!strcmp(hud_weaponstats_textalign->string, "center")) {
		alignment = 2;
	}

	i = (cl.spectator ? Cam_TrackNum() : cl.playernum);
	if (i < 0 || i >= MAX_CLIENTS) {
		HUD_PrepareDraw(hud, 0, 0, &x, &y);
		return;
	}

	SCR_CreateWeaponStatsPlayerText(&ws_clients[i], hud_weaponstats_format->string, content, sizeof(content));

	SCR_HUD_MultiLineString(hud, content, false, alignment, hud_weaponstats_scale->value, hud_weaponstats_proportional->integer);
}

void OnChange_scr_weaponstats (cvar_t *var, char *value, qbool *cancel)
{
	extern void CL_UserinfoChanged (char *key, char *value);

	// do not allow set "wpsx" to "0" instead set it to ""
	CL_UserinfoChanged("wpsx", strcmp(value, "0") ? value : "");
}

static void SCR_MvdWeaponStatsOn_f(void)
{
	cvar_t* show = Cvar_Find("hud_weaponstats_show");
	if (show) {
		Cvar_Set (show, "1");
	}
}

static void SCR_MvdWeaponStatsOff_f(void)
{
	cvar_t* show = Cvar_Find("hud_weaponstats_show");
	if (show) {
		Cvar_Set (show, "0");
	}
}

void WeaponStats_HUDInit(void)
{
	cvar_t* show_cvar;

	HUD_Register(
		"weaponstats", NULL, "Weapon stats",
		0, ca_active, 0, SCR_HUD_WeaponStats,
		"0", "screen", "center", "bottom", "0", "0", "0", "0 0 0", NULL,
		"format", "&c990sg&r:%2 &c099ssg&r:%3 &c900rl&r:#7 &c009lg&r:%8",
		"textalign", "center",
		"scale", "1",
		"proportional", "0",
		NULL
	);

	show_cvar = Cvar_Find("hud_weaponstats_show");
	if (show_cvar) {
		show_cvar->OnChange = OnChange_scr_weaponstats;
	}
}

void WeaponStats_CommandInit(void)
{
	if (!host_initialized) {
		Cmd_AddCommand("+cl_wp_stats", SCR_MvdWeaponStatsOn_f);
		Cmd_AddCommand("-cl_wp_stats", SCR_MvdWeaponStatsOff_f);
	}
}

// Structure to track hit detection
typedef struct hit_detector_s {
	int last_hits[wpMAX];
	double hit_time;
	qbool hit_active;
} hit_detector_t;

static hit_detector_t hit_detector;

// Call this function periodically to check for new hits
static void HIT_DetectHit(ws_player_t *ws_cl)
{
	int wp;
	qbool new_hit = false;

	if (!ws_cl) {
		return;
	}

	// Check each weapon for increased hit count
	for (wp = wpAXE; wp < wpMAX; wp++) {
		if (ws_cl->wpn[wp].hits > hit_detector.last_hits[wp]) {
			new_hit = true;
			hit_detector.last_hits[wp] = ws_cl->wpn[wp].hits;
		}
	}

	// If new hit detected, activate indicator and record time
	if (new_hit) {
		hit_detector.hit_active = true;
		hit_detector.hit_time = cls.realtime;
	}
}

// Called when new map spawned
void SCR_ClearHitIndicator(void)
{
	memset(&hit_detector, 0, sizeof(hit_detector));
}

void SCR_HUD_HitIndicator(hud_t *hud)
{
	int x, y;
	int i;
	double current_time = cls.realtime;
	double time_since_hit;
	int alpha = 255;
	mpic_t *pic;

	static cvar_t
		*hud_hitindicator_scale = NULL,
		*hud_hitindicator_duration,
		*hud_hitindicator_fade;

	if (hud_hitindicator_scale == NULL) {
		// first time - cache cvars
		hud_hitindicator_scale = HUD_FindVar(hud, "scale");
		hud_hitindicator_duration = HUD_FindVar(hud, "duration");
		hud_hitindicator_fade = HUD_FindVar(hud, "fade");
	}

	// Get current player index
	i = (cl.spectator ? Cam_TrackNum() : cl.playernum);
	if (i < 0 || i >= MAX_CLIENTS) {
		HUD_PrepareDraw(hud, 0, 0, &x, &y);
		return;
	}

	// Check for new hits
	HIT_DetectHit(&ws_clients[i]);

	// Calculate time since last hit
	time_since_hit = current_time - hit_detector.hit_time;

	// Check if we should still show the indicator
	if (!hit_detector.hit_active || time_since_hit > hud_hitindicator_duration->value) {
		hit_detector.hit_active = false;
		HUD_PrepareDraw(hud, 0, 0, &x, &y);
		return;
	}

	// Load the picture
	pic = Draw_CachePic(CACHEPIC_DMG);
	if (!pic) {
		HUD_PrepareDraw(hud, 0, 0, &x, &y);
		return;
	}

	// Prepare draw area with scaled picture dimensions
	HUD_PrepareDraw(hud, pic->width * hud_hitindicator_scale->value, 
		pic->height * hud_hitindicator_scale->value, &x, &y);

	// Apply fade effect if enabled
	if (hud_hitindicator_fade->integer) {
		float fade_ratio = 1.0f - (time_since_hit / hud_hitindicator_duration->value);
		alpha = (int)(255 * fade_ratio);
		alpha = bound(0, alpha, 255);
	}

	// Set alpha for drawing if fading
	if (alpha < 255) {
		Draw_AlphaFillRGB(0, 0, 0, 0, alpha);
	}

	// Draw the picture
	Draw_Pic(x, y, pic);

	// Reset alpha if it was changed
	if (alpha < 255) {
		Draw_AlphaFillRGB(0, 0, 0, 0, 255);
	}
}

void HitIndicator_HUDInit(void)
{
	// Clear hit detector state
	memset(&hit_detector, 0, sizeof(hit_detector));

	HUD_Register(
		"hitindicator", NULL, "Hit indicator - shows when you hit an opponent. Must have weaponstats enabled.",
		HUD_PLUSMINUS, ca_active, 0, SCR_HUD_HitIndicator,
		"0", "screen", "center", "center", "0", "0", "0", "0 0 0", NULL,
		"duration", "0.1",
		"scale", "1",
		"fade", "1",
		NULL
	);
}
