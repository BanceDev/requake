#include "discord_activity.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static struct IDiscordActivityManager *global_man;

void init_discord_activity(struct IDiscordActivityManager *manager) {
  struct DiscordActivity activity;
  memset(&activity, 0, sizeof(activity));

  strcpy(activity.name, "reQuake");
  strcpy(activity.state, "Playing");
  strcpy(activity.details, "Main Menu");

  activity.timestamps.start = time(NULL);

  struct DiscordActivityAssets *assets = &activity.assets;
  strcpy(assets->large_image, "default");
  strcpy(assets->large_text, "reQuake");

  manager->update_activity(manager, &activity, NULL, NULL);
  global_man = manager;
}

void menu_discord_activity(void) {
  if (global_man == NULL)
    return;

  struct DiscordActivity activity;
  memset(&activity, 0, sizeof(activity));

  strcpy(activity.name, "reQuake");
  strcpy(activity.state, "Playing");
  strcpy(activity.details, "Main Menu");

  struct DiscordActivityAssets *assets = &activity.assets;
  strcpy(assets->large_image, "default");
  strcpy(assets->large_text, "reQuake");

  global_man->update_activity(global_man, &activity, NULL, NULL);
}

void map_discord_activity(const char *mode, const char *map) {
  if (global_man == NULL)
    return;

  struct DiscordActivity activity;
  memset(&activity, 0, sizeof(activity));
  char mapstate[256];
  snprintf(mapstate, sizeof(mapstate), "Playing %s on %s", mode, map);
  strcpy(activity.name, "reQuake");
  strcpy(activity.state, "Playing");
  strcpy(activity.details, mapstate);

  struct DiscordActivityAssets *assets = &activity.assets;
  strcpy(assets->large_image, map);
  strcpy(assets->large_text, map);
  strcpy(assets->small_image, "requake");
  strcpy(assets->small_text, "reQuake");

  global_man->update_activity(global_man, &activity, NULL, NULL);
}
