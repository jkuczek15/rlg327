#include "move.h"

#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "dungeon.h"
#include "heap.h"
#include "move.h"
#include "npc.h"
#include "pc.h"
#include "character.h"
#include "utils.h"
#include "path.h"
#include "io.h"

int do_combat(dungeon_t *d, character *atk, character *def)
{
  char attack_message[40] = "You hit a monster for damage: ";
  char defense_message[40] = "The monster hits you for damage: ";
  char int_string[6];
  int damage;

  if(atk != d->pc && def != d->pc){
    return -1;
  }

  if(atk == d->pc && d->equipment[0]){
    damage = atk->damage->roll() + d->equipment[0]->damage.roll();
    def->hp = def->hp - damage;
    sprintf(int_string, "%d", damage);
    strcat(attack_message, int_string);
    d->combat_message = attack_message;
  }else if(atk == d->pc){
    damage = atk->damage->roll();
    def->hp = def->hp - damage;
    sprintf(int_string, "%d", damage);
    strcat(attack_message, int_string);
    d->combat_message = attack_message;
  }else if(def == d->pc){
    damage = atk->damage->roll();
    def->hp = def->hp - damage;
    sprintf(int_string, "%d", damage);
    strcat(defense_message, int_string);
    d->combat_message = defense_message;
  }

  if(def->hp <= 0){
    character_die(def);
    if (def != d->pc) {
      d->num_monsters--;
      d->combat_message = "You beat up a monster.";
      return 0;
    }
    return 0;
  }
  return -1;
}

int add_to_inventory(dungeon *d, object *o){
  int i;

  for(i = 0; i < 10; i++){
    if(!d->inventory[i]){
      d->inventory[i] = o;
      return 0;
    }
  }
  return -1;
}

void move_character(dungeon_t *d, character *c, pair_t next)
{
  int rs;
  pair_t p;

  if (charpair(next) &&
      ((next[dim_y] != character_get_y(c)) ||
       (next[dim_x] != character_get_x(c)))) {
    d->combat_message = "";
    rs = do_combat(d, c, charpair(next));
  }
  /* No character in new position. */

  if(rs == 0 || !charpair(next)){
    d->charmap[character_get_y(c)][character_get_x(c)] = NULL;
    character_set_y(c, next[dim_y]);
    character_set_x(c, next[dim_x]);
    d->charmap[character_get_y(c)][character_get_x(c)] = c;
  }

  if (c == d->pc) {
    pc_set_speed(d, c);
  }
}

void do_moves(dungeon_t *d)
{
  pair_t next;
  character *c;

  /* Remove the PC when it is PC turn.  Replace on next call.  This allows *
   * use to completely uninit the heap when generating a new level without *
   * worrying about deleting the PC.                                       */

  if (pc_is_alive(d)) {
    heap_insert(&d->next_turn, d->pc);
  }

  while (pc_is_alive(d) && ((c = ((character *)
                                  heap_remove_min(&d->next_turn))) != d->pc)) {
    if (!character_is_alive(c)) {
      if (d->charmap[character_get_y(c)][character_get_x(c)] == c) {
        d->charmap[character_get_y(c)][character_get_x(c)] = NULL;
      }
      if (c != d->pc) {
        character_delete(c);
      }
      continue;
    }

    character_next_turn(c);

    npc_next_pos(d, c, next);
    move_character(d, c, next);

    heap_insert(&d->next_turn, c);
  }

  if (pc_is_alive(d) && c == d->pc) {
    character_next_turn(c);

    if(d->objmap[character_get_y(c)][character_get_x(c)]){
      if(add_to_inventory(d, d->objmap[character_get_y(c)][character_get_x(c)]) == 0){
        d->objmap[character_get_y(c)][character_get_x(c)] = NULL;
      }
    }

  }
}

void dir_nearest_wall(dungeon_t *d, character *c, pair_t dir)
{
  dir[dim_x] = dir[dim_y] = 0;

  if (character_get_x(c) != 1 && character_get_x(c) != DUNGEON_X - 2) {
    dir[dim_x] = (character_get_x(c) > DUNGEON_X - character_get_x(c) ? 1 : -1);
  }
  if (character_get_y(c) != 1 && character_get_y(c) != DUNGEON_Y - 2) {
    dir[dim_y] = (character_get_y(c) > DUNGEON_Y - character_get_y(c) ? 1 : -1);
  }
}

uint32_t in_corner(dungeon_t *d, character *c)
{
  uint32_t num_immutable;

  num_immutable = 0;

  num_immutable += (mapxy(character_get_x(c) - 1,
                          character_get_y(c)    ) == ter_wall_immutable);
  num_immutable += (mapxy(character_get_x(c) + 1,
                          character_get_y(c)    ) == ter_wall_immutable);
  num_immutable += (mapxy(character_get_x(c)    ,
                          character_get_y(c) - 1) == ter_wall_immutable);
  num_immutable += (mapxy(character_get_x(c)    ,
                          character_get_y(c) + 1) == ter_wall_immutable);

  return num_immutable > 1;
}

static void new_dungeon_level(dungeon_t *d, uint32_t dir)
{
  /* Eventually up and down will be independantly meaningful. *
   * For now, simply generate a new dungeon.                  */

  switch (dir) {
  case '<':
  case '>':
    new_dungeon(d);
    break;
  default:
    break;
  }
}

uint32_t move_pc(dungeon_t *d, uint32_t dir)
{
  pair_t next;
  uint32_t was_stairs = 0;

  next[dim_y] = character_get_y(d->pc);
  next[dim_x] = character_get_x(d->pc);

  switch (dir) {
  case 1:
  case 2:
  case 3:
    next[dim_y]++;
    break;
  case 4:
  case 5:
  case 6:
    break;
  case 7:
  case 8:
  case 9:
    next[dim_y]--;
    break;
  }
  switch (dir) {
  case 1:
  case 4:
  case 7:
    next[dim_x]--;
    break;
  case 2:
  case 5:
  case 8:
    break;
  case 3:
  case 6:
  case 9:
    next[dim_x]++;
    break;
  case '<':
    if (mappair(character_get_pos(d->pc)) == ter_stairs_up) {
      was_stairs = 1;
      new_dungeon_level(d, '<');
    }
    break;
  case '>':
    if (mappair(character_get_pos(d->pc)) == ter_stairs_down) {
      was_stairs = 1;
      new_dungeon_level(d, '>');
    }
    break;
  }

  if (was_stairs) {
    return 0;
  }

  if ((dir != '>') && (dir != '<') && (mappair(next) >= ter_floor)) {
    move_character(d, d->pc, next);
    dijkstra(d);
    dijkstra_tunnel(d);

    return 0;
  }


  return 1;
}
