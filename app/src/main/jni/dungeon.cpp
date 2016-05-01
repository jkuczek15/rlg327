#include <stdio.h>
#include <stdint.h>
#include <endian.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>

#include "dungeon.h"
#include "utils.h"
#include "heap.h"
#include "pc.h"
#include "npc.h"

using namespace std;

dungeon_t *dungeon_UI::the_dungeon;

typedef struct corridor_path {
  heap_node_t *hn;
  uint8_t pos[2];
  uint8_t from[2];
  /* Because paths can meander about the dungeon, they can be *
   * significantly longer than DUNGEON_X.                     */
  int32_t cost;
} corridor_path_t;

static int32_t corridor_path_cmp(const void *key, const void *with) {
  return ((corridor_path_t *) key)->cost - ((corridor_path_t *) with)->cost;
}

char *dungeon_UI::dungeon_text(){
    int x, y;
    std::string rs = "";

    for(y = 0; y < DUNGEON_Y; y++){
        for(x = 0; x < DUNGEON_X; x++){
            rs += the_dungeon->character_map[y][x];
        }
        rs += '\n';
    }

    if(dungeon_UI::instance()->get_dungeon()->game_over == 1){
        rs = "\n                                       o\n"
               "                                      $\"\"$o\n"
               "                                     $\"  $$\n"
               "                                      $$$$\n"
               "                                      o \"$o\n"
               "                                     o\"  \"$\n"
               "                oo\"$$$\"  oo$\"$ooo   o$    \"$    ooo\"$oo  $$$\"o\n"
               "   o o o o    oo\"  o\"      \"o    $$o$\"     o o$\"\"  o$      \"$  "
               "\"oo   o o o o\n"
               "   \"$o   \"\"$$$\"   $$         $      \"   o   \"\"    o\"         $"
               "   \"o$$\"    o$$\n"
               "     \"\"o       o  $          $\"       $$$$$       o          $  ooo"
               "     o\"\"\n"
               "        \"o   $$$$o $o       o$        $$$$$\"       $o        \" $$$$"
               "   o\"\n"
               "         \"\"o $$$$o  oo o  o$\"         $$$$$\"        \"o o o o\"  "
               "\"$$$  $\n"
               "           \"\" \"$\"     \"\"\"\"\"            \"\"$\"            \""
               "\"\"      \"\"\" \"\n"
               "            \"oooooooooooooooooooooooooooooooooooooooooooooooooooooo$\n"
               "             \"$$$$\"$$$$\" $$$$$$$\"$$$$$$ \" \"$$$$$\"$$$$$$\"  $$$\""
               "\"$$$$\n"
               "              $$$oo$$$$   $$$$$$o$$$$$$o\" $$$$$$$$$$$$$$ o$$$$o$$$\"\n"
               "              $\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\""
               "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"$\n"
               "              $\"                                                 \"$\n"
               "              $\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\""
               "$\"$\"$\"$\"$\"$\"$\"$\n"
               "                                   You win!\n\n";
    }else if(dungeon_UI::instance()->get_dungeon()->game_over == 2){
        rs = "\n\n\n\n                /\"\"\"\"\"/\"\"\"\"\"\"\".\n"
               "               /     /         \\             __\n"
               "              /     /           \\            ||\n"
               "             /____ /   Rest in   \\           ||\n"
               "            |     |    Pieces     |          ||\n"
               "            |     |               |          ||\n"
               "            |     |   A. Luser    |          ||\n"
               "            |     |               |          ||\n"
               "            |     |     * *   * * |         _||_\n"
               "            |     |     *\\/* *\\/* |        | TT |\n"
               "            |     |     *_\\_  /   ...\"\"\"\"\"\"| |"
               "| |.\"\"....\"\"\"\"\"\"\"\".\"\"\n"
               "            |     |         \\/..\"\"\"\"\"...\"\"\""
               "\\ || /.\"\"\".......\"\"\"\"...\n"
               "            |     |....\"\"\"\"\"\"\"........\"\"\"\"\""
               "\"^^^^\".......\"\"\"\"\"\"\"\"..\"\n"
               "            |......\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"......"
               "..\"\"\"\"\"....\"\"\"\"\"..\"\"...\"\"\".\n\n"
               "            You're dead.  Better luck in the next life.\n\n\n";
    }

    return &(rs[0]);
}

static void dijkstra_corridor(dungeon_t *d, pair_t from, pair_t to)
{
  static corridor_path_t path[DUNGEON_Y][DUNGEON_X], *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint32_t x, y;

  if (!initialized) {
    for (y = 0; y < DUNGEON_Y; y++) {
      for (x = 0; x < DUNGEON_X; x++) {
        path[y][x].pos[dim_y] = y;
        path[y][x].pos[dim_x] = x;
      }
    }
    initialized = 1;
  }

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      path[y][x].cost = INT_MAX;
    }
  }

  path[from[dim_y]][from[dim_x]].cost = 0;

  heap_init(&h, corridor_path_cmp, NULL);

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if (mapxy(x, y) != ter_wall_immutable) {
        path[y][x].hn = heap_insert(&h, &path[y][x]);
      } else {
        path[y][x].hn = NULL;
      }
    }
  }

  while ((p = (corridor_path_t *) heap_remove_min(&h))) {
    p->hn = NULL;

    if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
      for (x = to[dim_x], y = to[dim_y];
           (x != from[dim_x]) || (y != from[dim_y]);
           p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
        if (mapxy(x, y) != ter_floor_room) {
          mapxy(x, y) = ter_floor_hall;
          hardnessxy(x, y) = 0;
        }
      }
      heap_delete(&h);
      return;
    }
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost >
         p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_x] != from[dim_x]) ? 48  : 0))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost =
        p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_x] != from[dim_x]) ? 48  : 0);
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
                                           [p->pos[dim_x]    ].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
         p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_y] != from[dim_y]) ? 48  : 0))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost =
        p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_y] != from[dim_y]) ? 48  : 0);
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] - 1].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
         p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_y] != from[dim_y]) ? 48  : 0))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost =
        p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_y] != from[dim_y]) ? 48  : 0);
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] + 1].hn);
    }
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
         p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_x] != from[dim_x]) ? 48  : 0))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost =
        p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_x] != from[dim_x]) ? 48  : 0);
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
                                           [p->pos[dim_x]    ].hn);
    }
  }
}

static int empty_dungeon(dungeon_t *d)
{
  uint8_t x, y;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      mapxy(x, y) = ter_wall;
      hardnessxy(x, y) = rand_range(1, 254);
      if (y == 0 || y == DUNGEON_Y - 1 ||
          x == 0 || x == DUNGEON_X - 1) {
        mapxy(x, y) = ter_wall_immutable;
        hardnessxy(x, y) = 255;
      }
        charxy(x, y) = NULL;
        objxy(x, y) = NULL;
    }
  }

  return 0;
}

static int make_rooms(dungeon_t *d)
{
  uint32_t i;

  for (i = MIN_ROOMS; i < MAX_ROOMS && rand_under(6, 8); i++)
    ;

  d->num_rooms = i;
  d->rooms = (room_t *) malloc(sizeof (*d->rooms) * d->num_rooms);

  for (i = 0; i < d->num_rooms; i++) {
    d->rooms[i].size[dim_x] = ROOM_MIN_X;
    d->rooms[i].size[dim_y] = ROOM_MIN_Y;
    while (rand_under(3, 4) && d->rooms[i].size[dim_x] < ROOM_MAX_X) {
      d->rooms[i].size[dim_x]++;
    }
    while (rand_under(3, 4) && d->rooms[i].size[dim_y] < ROOM_MAX_Y) {
      d->rooms[i].size[dim_y]++;
    }
    /* Initially, every room is connected only to itself. */
    d->rooms[i].connected = i;
  }

  return 0;
}

static int connect_two_rooms(dungeon_t *d, room_t *r1, room_t *r2)
{
  pair_t e1, e2;

  e1[dim_y] = rand_range(r1->position[dim_y],
                         r1->position[dim_y] + r1->size[dim_y] - 1);
  e1[dim_x] = rand_range(r1->position[dim_x],
                         r1->position[dim_x] + r1->size[dim_x] - 1);
  e2[dim_y] = rand_range(r2->position[dim_y],
                         r2->position[dim_y] + r2->size[dim_y] - 1);
  e2[dim_x] = rand_range(r2->position[dim_x],
                         r2->position[dim_x] + r2->size[dim_x] - 1);

  /*  return connect_two_points_recursive(d, e1, e2);*/
  dijkstra_corridor(d, e1, e2);
  return 0;
}

void dungeon_UI::gen_character_map(){
    int x, y;

    for(y = 0; y < DUNGEON_Y; y++){
            for(x = 0; x < DUNGEON_X; x++){
                if (the_dungeon->charmap[y][x]) {
                    the_dungeon->character_map[y][x] = character_get_symbol(the_dungeon->charmap[y][x]);
                }else if (the_dungeon->objmap[y][x]){
                    the_dungeon->character_map[y][x] = the_dungeon->objmap[y][x]->get_symbol();
                }else{
                     switch(the_dungeon->map[y][x]){
                        case ter_wall:
                            the_dungeon->character_map[y][x] = ' ';
                            break;
                        case ter_wall_immutable:
                            the_dungeon->character_map[y][x] = '$';
                            break;
                        case ter_floor_room:
                            the_dungeon->character_map[y][x] = '.';
                            break;
                        case ter_floor_hall:
                            the_dungeon->character_map[y][x] = '#';
                            break;
                        case ter_stairs_up:
                            the_dungeon->character_map[y][x] = '<';
                            break;
                        case ter_stairs_down:
                            the_dungeon->character_map[y][x] = '>';
                            break;
                    }
                }
            }
        }
}

static int connect_rooms(dungeon_t *d)
{
  uint32_t i;

  for (i = 1; i < d->num_rooms; i++) {
    connect_two_rooms(d, d->rooms + i - 1, d->rooms + i);
  }

  return 0;
}

static void place_stairs(dungeon_t *d)
{
  pair_t p;
  do {
    while ((p[dim_y] = rand_range(1, DUNGEON_Y - 2)) &&
           (p[dim_x] = rand_range(1, DUNGEON_X - 2)) &&
           ((mappair(p) < ter_floor)                 ||
            (mappair(p) > ter_stairs)))
      ;
    mappair(p) = ter_stairs_down;
  } while (rand_under(1, 3));
  do {
    while ((p[dim_y] = rand_range(1, DUNGEON_Y - 2)) &&
           (p[dim_x] = rand_range(1, DUNGEON_X - 2)) &&
           ((mappair(p) < ter_floor)                 ||
            (mappair(p) > ter_stairs)))

      ;
    mappair(p) = ter_stairs_up;
  } while (rand_under(1, 4));
}

static int place_rooms(dungeon_t *d)
{
  pair_t p;
  uint32_t i;
  int success;
  room_t *r;

  for (success = 0; !success; ) {
    success = 1;
    for (i = 0; success && i < d->num_rooms; i++) {
      r = d->rooms + i;
      r->position[dim_x] = 1 + rand() % (DUNGEON_X - 2 - r->size[dim_x]);
      r->position[dim_y] = 1 + rand() % (DUNGEON_Y - 2 - r->size[dim_y]);
      for (p[dim_y] = r->position[dim_y] - 1;
           success && p[dim_y] < r->position[dim_y] + r->size[dim_y] + 1;
           p[dim_y]++) {
        for (p[dim_x] = r->position[dim_x] - 1;
             success && p[dim_x] < r->position[dim_x] + r->size[dim_x] + 1;
             p[dim_x]++) {
          if (mappair(p) >= ter_floor) {
            success = 0;
            empty_dungeon(d);
          } else if ((p[dim_y] != r->position[dim_y] - 1)              &&
                     (p[dim_y] != r->position[dim_y] + r->size[dim_y]) &&
                     (p[dim_x] != r->position[dim_x] - 1)              &&
                     (p[dim_x] != r->position[dim_x] + r->size[dim_x])) {
            mappair(p) = ter_floor_room;
            hardnesspair(p) = 0;
          }
        }
      }
    }
  }

  return 0;
}

int gen_dungeon(dungeon_t *d)
{
  srand(time(NULL));
  empty_dungeon(d);

  do {
    make_rooms(d);
  } while (place_rooms(d));
  connect_rooms(d);
  place_stairs(d);

  return 0;
}

void delete_dungeon(dungeon_t *d)
{
  uint8_t y, x;

  free(d->rooms);
  heap_delete(&d->next_turn);
  memset(d->charmap, 0, sizeof (d->charmap));
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if (d->objmap[y][x]) {
        delete(d->objmap[y][x]);
        d->objmap[y][x] = NULL;
      }
    }
  }
}

void init_dungeon(dungeon_t *d)
{
  empty_dungeon(d);

  memset(&d->next_turn, 0, sizeof (d->next_turn));
  heap_init(&d->next_turn, compare_characters_by_next_turn, character_delete);
}

void new_dungeon(dungeon_t *d)
{
  uint32_t sequence_number;

  sequence_number = d->character_sequence_number;

  delete_dungeon(d);

  init_dungeon(d);
  gen_dungeon(d);
  d->character_sequence_number = sequence_number;

  place_pc(d);
  d->charmap[character_get_y(d->pc)][character_get_x(d->pc)] = d->pc;

  /* Need to add a mechanism to decide on a number of monsters.  --nummon  *
   * is just for testing, and if we're generating new dungeon levels, then *
   * presumably we've already done that testing.  We'll just put 10 in for *
   * now.                                                                  */
  gen_monsters(d, d->max_monsters, character_get_next_turn(d->pc));
  gen_objects(d, d->max_objects);
}

void dungeon_UI::set_dungeon(dungeon_t *d){
    the_dungeon = d;
}

dungeon_t* dungeon_UI::get_dungeon(){
    return the_dungeon;
}

void check_game_over(dungeon_t *d){
    if(pc_is_alive(d) && !dungeon_has_npcs(d)){
            d->game_over = 1;
            d->combat_message = "";
     }else{
        dungeon_UI::instance()->gen_character_map();
     }

    if(!pc_is_alive(dungeon_UI::instance()->get_dungeon())){
         d->game_over = 2;
         d->combat_message = "";
     }
}