#include "dungeon.h"

dungeon_UI *dungeon_UI::dui = 0;

dungeon_UI *dungeon_UI::instance()
{
  if (!dui) {
    dui = new dungeon_UI();
  }

  return dui;
}