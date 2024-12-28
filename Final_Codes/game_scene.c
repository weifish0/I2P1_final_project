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
ALLEGRO_BITMAP* speed_image;
ALLEGRO_BITMAP* shop_icon;

const int info_chunk_dx = 60;
const int info_chunk_dy = 40;
const int vertical_blank = 70;
const int horizontal_blank = 50;

int shop_dx;
int shop_dy;
int shop_icon_width;
int shop_icon_height;
bool shop_hover = false;
bool shop_opening = false;
static Button shop_back_btn;
int shop_item1_dx;
int shop_item1_dy;
int shop_item2_dx;
int shop_item2_dy;
int shop_item_width = 160;
int shop_time_lock = 0;

bool item1_hovered = false;
bool item2_hovered = false;

ALLEGRO_SAMPLE* buy_audio;

static void init_lose_scene(void){
    lose_image = al_load_bitmap("Assets/panda_lose.png");
    change_bgm("Assets/audio/lose.wav");
}

static void update_lose_scene(void){
    if(keyState[ALLEGRO_KEY_ENTER]){
        change_scene(create_menu_scene());
    }
}

static void draw_lose_scene(void){
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
    win_image = al_load_bitmap("Assets/panda_win.png");
    change_bgm("Assets/audio/win.wav");
}

static void update_win_scene(void){
    if(keyState[ALLEGRO_KEY_ENTER]){
        change_scene(create_menu_scene());
    }
}

static void draw_win_scene(void){
    float scale_factor = 5;
    al_clear_to_color(al_map_rgb(0, 0, 0));

    if (!win_image) {
        game_abort("Failed to load win scene image!");
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

    normal_gun = create_weapon("Assets/guns.png", "Assets/yellow_bullet.png", "Assets/audio/shooting.wav", 6, 8, 15);
    sniper = create_weapon("Assets/sniper.png", "Assets/orange_bullet.png", "Assets/audio/sniper.wav", 36, 10, 100);

    weapon = normal_gun;
    weapon_id = 0;
    
    for(int i=0; i<map.EnemySpawnSize; i++){
        Enemy enemy = createEnemy(map.EnemySpawn[i].x, map.EnemySpawn[i].y, map.EnemyCode[i]);
        insertEnemyList(enemyList, enemy);
    }

    heart_image = al_load_bitmap("Assets/heart.png");
    coin_image = al_load_bitmap("Assets/coin_icon.png");
    speed_image = al_load_bitmap("Assets/speed_icon.png");

    shop_icon = al_load_bitmap("Assets/shop_icon.png");
    shop_icon_width = 96;
    shop_icon_height = 96;
    shop_dx = SCREEN_W - shop_icon_width/2 - info_chunk_dx;
    shop_dy = info_chunk_dy - 15;
    shop_item1_dx = 200;
    shop_item1_dy = 350;
    shop_item2_dx = 500;
    shop_item2_dy = 350;

    buy_audio = al_load_sample("Assets/audio/buy_audio.wav");

    shop_back_btn = button_create(SCREEN_W / 2 - 150, 650, 300, 100, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");

    game_log("coord x:%d \n coords y:%d \n", map.Spawn.x, map.Spawn.y);
    change_bgm("Assets/audio/game_bgm.wav");
}

static void mouse_detection(void){
    int mouse_x = mouseState.x;
    int mouse_y = mouseState.y;

    if(shop_time_lock){
        shop_time_lock--;
    }

    // shop_icon mouse detection
    if(mouse_x >= shop_dx && mouse_x <= shop_dx+shop_icon_width &&
       mouse_y >= shop_dy && mouse_y <= shop_dy+shop_icon_height && 
       !shop_opening){
        shop_hover = true;

        if(mouseState.buttons){ // 左鍵按下
            shop_opening = true;
        }
    }
    else{
        shop_hover = false;
    }

    // item1
    if(shop_opening &&
       mouse_x >= shop_item1_dx && mouse_x <= shop_item1_dx+shop_item_width &&
       mouse_y >= shop_item1_dy && mouse_y <= shop_item1_dy+shop_item_width){
        item1_hovered = true;
        if(mouseState.buttons){
            if(coins_obtained >= 1 && !shop_time_lock){
                al_play_sample(buy_audio, SFX_VOLUME, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                coins_obtained -= 1;
                player.health += 10;
                shop_time_lock = 30;
            }
        }
    }
    else{
        item1_hovered = false;
    }

    // item2
    if(shop_opening &&
       mouse_x >= shop_item2_dx && mouse_x <= shop_item2_dx+shop_item_width &&
       mouse_y >= shop_item2_dy && mouse_y <= shop_item2_dy+shop_item_width){
        item2_hovered = true;
        if(mouseState.buttons){
            if(coins_obtained >= 2 && !shop_time_lock){
                al_play_sample(buy_audio, SFX_VOLUME, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                coins_obtained -= 2;
                player.speed += 1;
                shop_time_lock = 30;
            }
        }
    }
    else{
        item2_hovered = false;
    }

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
    if(!map.win_game){
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
    update_map(&map, player.coord, &coins_obtained, &player.health, &player.speed);

    // shop back btn
    update_button(&shop_back_btn);
    if(mouseState.buttons && shop_back_btn.hovered){
        shop_opening = false;
    }
    
    mouse_detection();
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

    if(shop_opening){
        // 繪製黑色遮罩
        al_draw_filled_rectangle(
            0, 0, SCREEN_W, SCREEN_H, // 遮罩覆蓋整個螢幕
            al_map_rgba_f(0, 0, 0, 0.5) // RGBA 顏色，最後一個參數為透明度
        );

        // shop title
        al_draw_text(TITLE_FONT, al_map_rgb(235,121,12), SCREEN_W/2 - 64, SCREEN_H/2 - 250, 0, "Shop");

        // button
        draw_button(shop_back_btn);
        // button text
        al_draw_text(
            P2_FONT,
            al_map_rgb(66, 76, 110),
            SCREEN_W / 2,
            650 + 28 + shop_back_btn.hovered * 11,
            ALLEGRO_ALIGN_CENTER,
            "GO BACK"
        );
        al_draw_text(
            P2_FONT,
            al_map_rgb(225, 225, 225),
            SCREEN_W / 2,
            650 + 31 + shop_back_btn.hovered * 11,
            ALLEGRO_ALIGN_CENTER,
            "GO BACK"
        );

        // item1
        if(item1_hovered){
            al_draw_tinted_scaled_bitmap(
                heart_image, al_map_rgb(150, 150, 150),
                0, 0, 32, 32, // 原圖片來源的區域
                shop_item1_dx, shop_item1_dy, shop_item_width, shop_item_width, // 繪製的目標區域
                0 // 無額外標誌
            );
        }
        else{
            al_draw_tinted_scaled_bitmap(
                heart_image, al_map_rgb(255, 255, 255),
                0, 0, 32, 32, // 原圖片來源的區域
                shop_item1_dx, shop_item1_dy, shop_item_width, shop_item_width, // 繪製的目標區域
                0 // 無額外標誌
            );
        }
        al_draw_text(
            P1_FONT,
            al_map_rgb(225, 225, 225),
            shop_item1_dx + shop_item_width/2,
            shop_item1_dy + shop_item_width + 30,
            ALLEGRO_ALIGN_CENTER,
            "$ 1"
        );


        // item2
        if(item2_hovered){
            al_draw_tinted_scaled_bitmap(
                speed_image, al_map_rgb(150, 150, 150),
                0, 0, 32, 32, // 原圖片來源的區域
                shop_item2_dx, shop_item2_dy, shop_item_width, shop_item_width, // 繪製的目標區域
                0 // 無額外標誌
            );
        }
        else{
            al_draw_tinted_scaled_bitmap(
                speed_image, al_map_rgb(255, 255, 255),
                0, 0, 32, 32, // 原圖片來源的區域
                shop_item2_dx, shop_item2_dy, shop_item_width, shop_item_width, // 繪製的目標區域
                0 // 無額外標誌
            );
        }
        al_draw_text(
            P1_FONT,
            al_map_rgb(225, 225, 225),
            shop_item2_dx + shop_item_width/2,
            shop_item2_dy + shop_item_width + 30,
            ALLEGRO_ALIGN_CENTER,
            "$ 2"
        );

    }

    if (!heart_image) {
        game_abort("Failed to load heart icon!");
    }
    else{
        float scale_factor = 3;

        // 計算放大後的寬高
        int scaled_width = 32 * scale_factor;
        int scaled_height = 32 * scale_factor;

        int draw_dx = (info_chunk_dx - scaled_width/2);
        int draw_dy = (info_chunk_dy - scaled_height/2) + 10;

        // 使用 al_draw_scaled_bitmap 繪製放大圖片
        al_draw_scaled_bitmap(
            heart_image,
            0, 0, 32, 32, // 原圖片來源的區域
            draw_dx, draw_dy, scaled_width, scaled_height, // 繪製的目標區域
            0 // 無額外標誌
        );
    }
    
    if (!coin_image) {
        game_abort("Failed to load coin icon!");
    }
    else{
        float scale_factor = 5;

        // 計算放大後的寬高
        int scaled_width = 16 * scale_factor;
        int scaled_height = 16 * scale_factor;

        int draw_dx = info_chunk_dx - scaled_width/2;
        int draw_dy = info_chunk_dy - scaled_height/2 + vertical_blank + 10;

        // 使用 al_draw_scaled_bitmap 繪製放大圖片
        al_draw_scaled_bitmap(
            coin_image,
            0, 0, 16, 16, // 原圖片來源的區域
            draw_dx, draw_dy, scaled_width, scaled_height, // 繪製的目標區域
            0 // 無額外標誌
        );
    }

    if (!speed_image) {
        game_abort("Failed to load speed icon!");
    }
    else{
        float scale_factor = 1.5;

        // 計算放大後的寬高
        int scaled_width = 32 * scale_factor;
        int scaled_height = 32 * scale_factor;

        int draw_dx = (info_chunk_dx - scaled_width/2);
        int draw_dy = info_chunk_dy - scaled_height/2 + 2*vertical_blank + 10;

        // 使用 al_draw_scaled_bitmap 繪製放大圖片
        al_draw_scaled_bitmap(
            speed_image,
            0, 0, 32, 32, // 原圖片來源的區域
            draw_dx, draw_dy, scaled_width, scaled_height, // 繪製的目標區域
            0 // 無額外標誌
        );
    }

    al_draw_textf(P2_FONT, al_map_rgb(255,255,255), info_chunk_dx + horizontal_blank, info_chunk_dy, 0, "%03d", player.health);
    al_draw_textf(P2_FONT, al_map_rgb(255,255,255), info_chunk_dx + horizontal_blank, info_chunk_dy + vertical_blank, 0, "%03d", coins_obtained);
    al_draw_textf(P2_FONT, al_map_rgb(255,255,255), info_chunk_dx + horizontal_blank, info_chunk_dy + 2*vertical_blank, 0, "%03d", player.speed);

    if(shop_hover){
        al_draw_tinted_scaled_bitmap(
            shop_icon, al_map_rgb(150, 150, 150),
            0, 0, 64, 64, // 原圖片來源的區域
            shop_dx, shop_dy, shop_icon_width, shop_icon_height, // 繪製的目標區域
            0 // 無額外標誌
        );
    }
    else{
        if(!shop_opening){
            al_draw_tinted_scaled_bitmap(
                shop_icon, al_map_rgb(255, 255, 255),
                0, 0, 64, 64, // 原圖片來源的區域
                shop_dx, shop_dy, shop_icon_width, shop_icon_height, // 繪製的目標區域
                0 // 無額外標誌
            );
        }
    }
}

static void destroy(void){
    delete_player(&player);
    delete_weapon(&normal_gun);
    delete_weapon(&sniper);
    destroy_map(&map);
    destroyBulletList(bulletList);
    destroyEnemyList(enemyList);
    terminateEnemy();

    al_destroy_bitmap(heart_image);
    al_destroy_bitmap(coin_image);
    al_destroy_bitmap(speed_image);
    al_destroy_bitmap(shop_icon);

    al_destroy_sample(buy_audio);

    destroy_button(&shop_back_btn);
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