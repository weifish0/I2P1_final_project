#include <string.h>
#include "game_scene.h"
#include "menu_scene.h"
#include "loading_scene.h"
#include "game.h"
#include "player.h"
#include "map.h"
#include "enemy.h"
#include "weapon.h"
#include "UI.h"

#include <math.h>

Player player; // Player
Map map; // Map
enemyNode * enemyList; // Enemy List
BulletNode * bulletList; // Bullet List

// Weapon
Weapon weapon; 
Weapon normal_gun; 
Weapon sniper;
int weapon_id;
int weapon_time_lock;

int coins_obtained = 0;

ALLEGRO_BITMAP* lose_image;
ALLEGRO_BITMAP* win_image;
ALLEGRO_BITMAP* heart_image;
ALLEGRO_BITMAP* coin_image;

static void init_lose_scene(void){
    change_bgm("Assets/audio/lose.wav");
}

static void update_lose_scene(void){
    if(keyState[ALLEGRO_KEY_ENTER]){
        change_scene(create_menu_scene());
    }
}

static void draw_lose_scene(void){
    lose_image = al_load_bitmap("Assets/panda_lose.png");
    float scale_factor = 5;
    al_clear_to_color(al_map_rgb(0, 0, 0));

    if (!lose_image) {
        game_abort("Failed to load lose screen image!");
    }
    else{
        int img_width = al_get_bitmap_width(lose_image);
        int img_height = al_get_bitmap_height(lose_image);

        // 計算放大後的寬高
        int scaled_width = img_width * scale_factor;
        int scaled_height = img_height * scale_factor;

        // 計算圖片在畫面上的中心位置
        int draw_x = (SCREEN_W - scaled_width) / 2;
        int draw_y = (SCREEN_H - scaled_height) / 3;

        // 使用 al_draw_scaled_bitmap 繪製放大圖片
        al_draw_scaled_bitmap(
            lose_image,
            0, 0, img_width, img_height, // 原圖片來源的區域
            draw_x, draw_y, scaled_width, scaled_height, // 繪製的目標區域
            0 // 無額外標誌
        );
    }
    
    al_draw_text(TITLE_FONT, al_map_rgb(200, 0, 0), SCREEN_W / 2, SCREEN_H / 2 + 100, ALLEGRO_ALIGN_CENTER, "You Lose");
    al_draw_text(P3_FONT, al_map_rgb(255, 155, 0), SCREEN_W / 2, SCREEN_H / 2 + 200, ALLEGRO_ALIGN_CENTER, "Press Enter to Go Back to Menu");
}

static void destroy_lose_scene(void){
    coins_obtained = 0;
    al_destroy_bitmap(lose_image);
}

static void init_win_scene(void){
    change_bgm("Assets/audio/win.wav");
}

static void update_win_scene(void){
    if(keyState[ALLEGRO_KEY_ENTER]){
        change_scene(create_menu_scene());
    }
}

static void draw_win_scene(void){
    win_image = al_load_bitmap("Assets/panda_win.png");
    float scale_factor = 5;
    al_clear_to_color(al_map_rgb(0, 0, 0));

    if (!win_image) {
        game_abort("Failed to load win screen image!");
    }
    else{
        int img_width = al_get_bitmap_width(win_image);
        int img_height = al_get_bitmap_height(win_image);

        // 計算放大後的寬高
        int scaled_width = img_width * scale_factor;
        int scaled_height = img_height * scale_factor;

        // 計算圖片在畫面上的中心位置
        int draw_x = (SCREEN_W - scaled_width) / 2;
        int draw_y = (SCREEN_H - scaled_height) / 3;

        // 使用 al_draw_scaled_bitmap 繪製放大圖片
        al_draw_scaled_bitmap(
            win_image,
            0, 0, img_width, img_height, // 原圖片來源的區域
            draw_x, draw_y, scaled_width, scaled_height, // 繪製的目標區域
            0 // 無額外標誌
        );
    }
    al_draw_text(TITLE_FONT, al_map_rgb(0, 200, 0), SCREEN_W / 2, SCREEN_H / 2 + 100, ALLEGRO_ALIGN_CENTER, "You Win");
    al_draw_text(P3_FONT, al_map_rgb(255, 155, 0), SCREEN_W / 2, SCREEN_H / 2 + 200, ALLEGRO_ALIGN_CENTER, "Press Enter to Go Back to Menu");
}

static void destroy_win_scene(void){
    coins_obtained = 0;
    al_destroy_bitmap(win_image);
}

static void init(void){
    
    initEnemy();
    
    map = create_map("Assets/map0.txt", 0);

    player = create_player("Assets/panda2.png", map.Spawn.x, map.Spawn.y);

    enemyList = createEnemyList();
    bulletList = createBulletList();

    normal_gun = create_weapon("Assets/guns.png", "Assets/yellow_bullet.png", "Assets/audio/shooting.wav", 16, 8, 10);
    sniper = create_weapon("Assets/sniper.png", "Assets/orange_bullet.png", "Assets/audio/sniper.wav", 32, 14, 30);

    weapon = normal_gun;
    weapon_id = 0;
    
    for(int i=0; i<map.EnemySpawnSize; i++){
        Enemy enemy = createEnemy(map.EnemySpawn[i].x, map.EnemySpawn[i].y, map.EnemyCode[i]);
        insertEnemyList(enemyList, enemy);
    }

    game_log("coord x:%d \n coords y:%d \n", map.Spawn.x, map.Spawn.y);
    change_bgm("Assets/audio/game_bgm.wav");
}

static void update(void){
    /*
        [TODO Homework]
        
        Change the scene if winning/losing to win/lose scene
    */
    if(weapon_time_lock){
        weapon_time_lock--;
    }

    if(player.status == PLAYER_DEAD){
        change_scene(create_lose_scene());
        return;
    }
    if(coins_obtained == map.total_coins){
        change_scene(create_win_scene());
        return;
    }

    if(keyState[ALLEGRO_KEY_G] && !weapon_time_lock){
        weapon_time_lock = 60;
        if(!weapon_id){
            weapon_id = 1;
            weapon = sniper;
        }
        else{
            weapon_id = 0;
            weapon = normal_gun;
        }
    }

    update_player(&player, &map);

    Point Camera;
    Camera.x = player.coord.x - SCREEN_W / 2;
    Camera.y = player.coord.y - SCREEN_H / 2;

    updateEnemyList(enemyList, &map, &player);
    update_weapon(&weapon, bulletList, player.coord, Camera);
    updateBulletList(bulletList, enemyList, &map);
    update_map(&map, player.coord, &coins_obtained, &player.health);
}

static void draw(void){
    Point Camera;
    Camera.x = player.coord.x - SCREEN_W / 2;
    Camera.y = player.coord.y - SCREEN_H / 2;
    
    // Draw
    draw_map(&map, Camera);
    drawEnemyList(enemyList, Camera);
    drawBulletList(bulletList, Camera);
    draw_player(&player, Camera);
    draw_weapon(&weapon, player.coord, Camera);

    /*
        [TODO Homework]
        
        Draw the UI of Health and Total Coins
    */

    int info_chunk_dx = 60;
    int info_chunk_dy = 50;
    int vertical_blank = 80;
    int horizontal_blank = 50;

    heart_image = al_load_bitmap("Assets/heart.png");
    if (!heart_image) {
        game_abort("Failed to load heart icon!");
    }
    else{
        int heart_width = al_get_bitmap_width(heart_image);
        int heart_height = al_get_bitmap_height(heart_image);
        float scale_factor = 3;

        // 計算放大後的寬高
        int scaled_width = heart_width * scale_factor;
        int scaled_height = heart_height * scale_factor;

        int draw_dx = (info_chunk_dx - scaled_width/2);
        int draw_dy = (info_chunk_dy - scaled_height/2) + 10;

        // 使用 al_draw_scaled_bitmap 繪製放大圖片
        al_draw_scaled_bitmap(
            heart_image,
            0, 0, heart_width, heart_height, // 原圖片來源的區域
            draw_dx, draw_dy, scaled_width, scaled_height, // 繪製的目標區域
            0 // 無額外標誌
        );
    }
    
    coin_image = al_load_bitmap("Assets/coin_icon.png");
    if (!coin_image) {
        game_abort("Failed to load coin icon!");
    }
    else{
        int coin_width = al_get_bitmap_width(coin_image);
        int coin_height = al_get_bitmap_height(coin_image);
        float scale_factor = 5;

        // 計算放大後的寬高
        int scaled_width = coin_width * scale_factor;
        int scaled_height = coin_height * scale_factor;

        int draw_dx = info_chunk_dx - scaled_width/2;
        int draw_dy = info_chunk_dy - scaled_height/2 + vertical_blank + 10;

        // 使用 al_draw_scaled_bitmap 繪製放大圖片
        al_draw_scaled_bitmap(
            coin_image,
            0, 0, coin_width, coin_height, // 原圖片來源的區域
            draw_dx, draw_dy, scaled_width, scaled_height, // 繪製的目標區域
            0 // 無額外標誌
        );
    }

    al_draw_textf(P2_FONT, al_map_rgb(255,255,255), info_chunk_dx + horizontal_blank, info_chunk_dy, 0, "%03d", player.health);
    al_draw_textf(P2_FONT, al_map_rgb(255,255,255), info_chunk_dx + horizontal_blank, info_chunk_dy + vertical_blank, 0, "%03d", coins_obtained);
}

static void destroy(void){
    delete_player(&player);
    delete_weapon(&normal_gun);
    delete_weapon(&sniper);
    destroy_map(&map);
    destroyBulletList(bulletList);
    destroyEnemyList(enemyList);
    terminateEnemy();
}


Scene create_game_scene(void){
    Scene scene;
    memset(&scene, 0, sizeof(Scene));
    
    scene.name = "game";
    scene.init = &init;
    scene.draw = &draw;
    scene.update = &update;
    scene.destroy = &destroy;
    
    return scene;
}

Scene create_lose_scene(void){
    Scene scene;
    memset(&scene, 0, sizeof(Scene));
    
    scene.name = "lose_scene";
    scene.init = &init_lose_scene;
    scene.draw = &draw_lose_scene;
    scene.update = &update_lose_scene;
    scene.destroy = &destroy_lose_scene;
    
    return scene;
}

Scene create_win_scene(void){
    Scene scene;
    memset(&scene, 0, sizeof(Scene));
    
    scene.name = "win_scene";
    scene.init = &init_win_scene;
    scene.draw = &draw_win_scene;
    scene.update = &update_win_scene;
    scene.destroy = &destroy_win_scene;
    
    return scene;
}