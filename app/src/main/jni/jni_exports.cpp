#include <string.h>
#include <jni.h>
#include "dungeon.h"
#include "pc.h"
#include "npc.h"
#include "move.h"

#ifdef __cplusplus
extern "C"
{
#endif

   dungeon_t d;

    JNIEXPORT jstring JNICALL
      Java_com_example_hellojni_HelloJni_dungeonText
      (JNIEnv *env, jobject obj)
      {
        std::string toReturn = dungeon_UI::instance()->dungeon_text();
        return env->NewStringUTF(&(toReturn[0]));
      }

    JNIEXPORT void JNICALL
      Java_com_example_hellojni_HelloJni_createDungeon
      (JNIEnv *env, jobject obj)
      {
        d.max_monsters = 10;
        init_dungeon(&d);
        generate_monster_descriptions(&d);
        generate_object_descriptions(&d);
        gen_dungeon(&d);
        config_pc(&d);
        gen_monsters(&d, d.max_monsters, 0);
        gen_objects(&d, 6);
        d.combat_message = "Welcome to RLG 327!";
        dungeon_UI::instance()->set_dungeon(&d);
        dungeon_UI::instance()->gen_character_map();
      }

    JNIEXPORT void JNICALL
      Java_com_example_hellojni_HelloJni_handleInput1
      (JNIEnv *env, jobject obj)
      {
         move_pc(dungeon_UI::instance()->get_dungeon(), 1);
      }

    JNIEXPORT void JNICALL
      Java_com_example_hellojni_HelloJni_handleInput2
      (JNIEnv *env, jobject obj)
      {
         move_pc(dungeon_UI::instance()->get_dungeon(), 2);
      }

    JNIEXPORT void JNICALL
      Java_com_example_hellojni_HelloJni_handleInput3
      (JNIEnv *env, jobject obj)
      {
         move_pc(dungeon_UI::instance()->get_dungeon(), 3);
      }

    JNIEXPORT void JNICALL
      Java_com_example_hellojni_HelloJni_handleInput4
      (JNIEnv *env, jobject obj)
      {
         move_pc(dungeon_UI::instance()->get_dungeon(), 4);
      }

    JNIEXPORT void JNICALL
      Java_com_example_hellojni_HelloJni_handleInput5
      (JNIEnv *env, jobject obj)
      {
         move_pc(dungeon_UI::instance()->get_dungeon(), 5);
      }

    JNIEXPORT void JNICALL
      Java_com_example_hellojni_HelloJni_handleInput6
      (JNIEnv *env, jobject obj)
      {
         move_pc(dungeon_UI::instance()->get_dungeon(), 6);
      }

    JNIEXPORT void JNICALL
      Java_com_example_hellojni_HelloJni_handleInput7
      (JNIEnv *env, jobject obj)
      {
         move_pc(dungeon_UI::instance()->get_dungeon(), 7);
      }

    JNIEXPORT void JNICALL
      Java_com_example_hellojni_HelloJni_handleInput8
      (JNIEnv *env, jobject obj)
      {
         move_pc(dungeon_UI::instance()->get_dungeon(), 8);
      }

    JNIEXPORT void JNICALL
      Java_com_example_hellojni_HelloJni_handleInput9
      (JNIEnv *env, jobject obj)
      {
         move_pc(dungeon_UI::instance()->get_dungeon(), 9);
      }

    JNIEXPORT void JNICALL
      Java_com_example_hellojni_HelloJni_handleInputStairsUp
      (JNIEnv *env, jobject obj)
      {
         d.max_monsters = 5;
         d.combat_message = "";
         move_pc(dungeon_UI::instance()->get_dungeon(), '<');
         dungeon_UI::instance()->gen_character_map();
      }

    JNIEXPORT void JNICALL
      Java_com_example_hellojni_HelloJni_handleInputStairsDown
      (JNIEnv *env, jobject obj)
      {
         d.max_monsters = 5;
         d.combat_message = "";
         move_pc(dungeon_UI::instance()->get_dungeon(), '>');
         dungeon_UI::instance()->gen_character_map();
      }

    JNIEXPORT void JNICALL
      Java_com_example_hellojni_HelloJni_handleInputRestart
      (JNIEnv *env, jobject obj)
      {
         dungeon_UI::instance()->get_dungeon()->game_over = 0;
         d.max_monsters = 10;
         init_dungeon(&d);
         generate_monster_descriptions(&d);
         generate_object_descriptions(&d);
         gen_dungeon(&d);
         config_pc(&d);
         gen_monsters(&d, d.max_monsters, 0);
         gen_objects(&d, 6);
         d.combat_message = "Welcome to RLG 327!";
         dungeon_UI::instance()->set_dungeon(&d);
         dungeon_UI::instance()->gen_character_map();
      }

    JNIEXPORT int JNICALL
      Java_com_example_hellojni_HelloJni_getHp
      (JNIEnv *env, jobject obj)
      {
         return dungeon_UI::instance()->get_dungeon()->pc->hp;
      }

     JNIEXPORT jstring JNICALL
      Java_com_example_hellojni_HelloJni_getCombatMessage
      (JNIEnv *env, jobject obj)
      {
        std::string toReturn = dungeon_UI::instance()->get_dungeon()->combat_message;
        return env->NewStringUTF(&(toReturn[0]));
      }

     JNIEXPORT void JNICALL
       Java_com_example_hellojni_HelloJni_doMoves
        (JNIEnv *env, jobject obj)
      {
         do_moves(dungeon_UI::instance()->get_dungeon());
         check_game_over(dungeon_UI::instance()->get_dungeon());
      }

#ifdef __cplusplus
}
#endif
