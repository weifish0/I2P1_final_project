#include <allegro5/allegro.h>
#include <string.h>
#include "menu_scene.h"
#include "game_scene.h"
#include "setting_scene.h"
#include "loading_scene.h"
#include "utility.h"
#include "UI.h"
#include "game.h"

static Button settingButton;

static int scene_time_lock;

static Button leaderboardButton;

#define MAX_INPUT_LEN 32
#define MAX_LEADERS 10 // 排行榜顯示最多10人

typedef struct {
    char name[MAX_INPUT_LEN];
    int coins;
} Leader;

static char player_name[MAX_INPUT_LEN + 10] = ""; // 玩家輸入
static int type_timer = 10;
static Leader leaderboard[MAX_LEADERS];
static int leaderboard_size = 0;
bool score_submitted = false;

static void init(void) {
    settingButton = button_create(SCREEN_W / 2 - 200, 600, 400, 100, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
    leaderboardButton = button_create(SCREEN_W / 2 - 200, 500, 400, 100, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");

    scene_time_lock = 60;
    change_bgm("Assets/audio/menu_bgm.wav");
}

static void update(void) {
    if(scene_time_lock){
        scene_time_lock--;
    }
    update_button(&settingButton);
    update_button(&leaderboardButton);

    if (keyState[ALLEGRO_KEY_ENTER] && !scene_time_lock) {
        score_submitted = false;
        change_scene(create_loading_scene());
    }

    /*
        [TODO HACKATHON 4-3] 
        
        Change scene to setting scene when the button is pressed
    */
    if (mouseState.buttons && settingButton.hovered) {
        change_scene(create_setting_scene());
    }
    if(mouseState.buttons && leaderboardButton.hovered){
        change_scene(create_leaderboard_scene(-1));
    }
}

static void draw(void) {
    // Title Text
    al_draw_text(
        TITLE_FONT,
        al_map_rgb(146, 161, 185),
        SCREEN_W / 2,
        225,
        ALLEGRO_ALIGN_CENTER,
        "NTHU-RPG ADVENTURE"
    );
    al_draw_text(
        TITLE_FONT,
        al_map_rgb(199, 207, 221),
        SCREEN_W / 2,
        220,
        ALLEGRO_ALIGN_CENTER,
        "NTHU-RPG ADVENTURE"
    );

    al_draw_text(
        P2_FONT,
        al_map_rgb(255, 255, 255),
        SCREEN_W / 2,
        350,
        ALLEGRO_ALIGN_CENTER,
        "PRESS [ENTER] TO PLAY"
    );


    // leaderboardButton
    draw_button(leaderboardButton);
    // button text
    al_draw_text(
        P2_FONT,
        al_map_rgb(66, 76, 110),
        SCREEN_W / 2,
        500 + 28 + leaderboardButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "LEADERBOARD"
    );
    al_draw_text(
        P2_FONT,
        al_map_rgb(225, 225, 225),
        SCREEN_W / 2,
        500 + 31 + leaderboardButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "LEADERBOARD"
    );


    // settingButton
    draw_button(settingButton);
    al_draw_text(
        P2_FONT,
        al_map_rgb(66, 76, 110),
        SCREEN_W / 2,
        600 + 28 + settingButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "SETTING"
    );
    al_draw_text(
        P2_FONT,
        al_map_rgb(225, 225, 225),
        SCREEN_W / 2,
        600 + 31 + settingButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "SETTING"
    );
}

static void destroy(void) {
    destroy_button(&settingButton);
    destroy_button(&leaderboardButton);
}


Scene create_menu_scene(void) {
    Scene scene;
    memset(&scene, 0, sizeof(Scene));

    scene.name = "menu";
    scene.init = &init;
    scene.draw = &draw;
    scene.update = &update;
    scene.destroy = &destroy;

    return scene;
}


int player_coins;

// **初始化排行榜**
static void init_leaderboard(void) {
    type_timer = 10; // 初始化計時器
    player_name[0] = '\0'; // 清空玩家輸入數據
    leaderboard_size = 0;
    scene_time_lock = 60;

    // **讀取排行榜檔案**
    FILE *file = fopen("leaderboard.txt", "r");
    if (file) {
        char line[64];
        while (fgets(line, sizeof(line), file)) {
            char *comma = strchr(line, ',');
            char *semicolon = strchr(line, ';');
            if (comma && semicolon) {
                *comma = '\0';     // 分隔名字
                *semicolon = '\0'; // 分隔金幣數量
                strncpy(leaderboard[leaderboard_size].name, line, MAX_INPUT_LEN);
                leaderboard[leaderboard_size].coins = atoi(comma + 1);
                leaderboard_size++;
                if (leaderboard_size >= MAX_LEADERS) {
                    break;
                }
            }
        }
        fclose(file);
    }

    // **按金幣數量排序**
    for (int i = 0; i < leaderboard_size - 1; i++) {
        for (int j = i + 1; j < leaderboard_size; j++) {
            if (leaderboard[j].coins > leaderboard[i].coins) {
                Leader temp = leaderboard[i];
                leaderboard[i] = leaderboard[j];
                leaderboard[j] = temp;
            }
        }
    }
}

// **繪製排行榜畫面**
static void draw_leaderboard(void) {
    al_draw_text(
        P2_FONT,
        al_map_rgb(255, 255, 255),
        SCREEN_W / 2,
        750,
        ALLEGRO_ALIGN_CENTER,
        "PRESS [ESC] TO GO BACK TO MENU"
    );

    // **顯示排行榜**
    for (int i = 0; i < leaderboard_size; i++) {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%d.  %-15s  %5d Coins", i + 1, leaderboard[i].name, leaderboard[i].coins);
        al_draw_text(
            P1_FONT,
            al_map_rgb(255, 255, 255),
            50,
            80 + i * 50,
            ALLEGRO_ALIGN_LEFT,
            buffer
        );
    }

    if(player_coins != -1){
        // **顯示輸入框**
        al_draw_filled_rectangle(
            SCREEN_W / 2 - 200, 600,
            SCREEN_W / 2 + 200, 650,
            al_map_rgb(0, 0, 0)
        );
        al_draw_rectangle(
            SCREEN_W / 2 - 200, 600,
            SCREEN_W / 2 + 200, 650,
            al_map_rgb(255, 255, 255),
            2
        );
        // **顯示玩家輸入的文字**
        al_draw_text(
            P2_FONT,
            al_map_rgb(255, 255, 255),
            SCREEN_W / 2,
            610,
            ALLEGRO_ALIGN_CENTER,
            player_name
        );
        al_draw_textf(P2_FONT, al_map_rgb(255,155,0), SCREEN_W / 2, 680, 0, "Coins: %d", player_coins);
    }
}

// **更新排行榜邏輯**
static void update_leaderboard(void) {
    if(scene_time_lock != 0){
        scene_time_lock--;
    }

    if (keyState[ALLEGRO_KEY_ESCAPE]) {
        change_scene(create_menu_scene());
        return;
    }
    if(player_coins == -1){
        return;
    }

    if (type_timer > 0) {
        type_timer--;
    }

    if (type_timer == 0) {
        for (int key = ALLEGRO_KEY_A; key <= ALLEGRO_KEY_Z; key++) {
            if (keyState[key] && strlen(player_name) < MAX_INPUT_LEN - 1) {
                char c = 'a' + (key - ALLEGRO_KEY_A);
                strncat(player_name, &c, 1);
                type_timer = 10;
                break;
            }
        }

        if (keyState[ALLEGRO_KEY_SPACE] && strlen(player_name) < MAX_INPUT_LEN - 1) {
            char c = ' ';
            strncat(player_name, &c, 1);
            type_timer = 10;
        }

        if (keyState[ALLEGRO_KEY_BACKSPACE] && strlen(player_name) > 0) {
            player_name[strlen(player_name) - 1] = '\0';
            type_timer = 10;
        }

        if (keyState[ALLEGRO_KEY_ENTER] && !scene_time_lock && strlen(player_name)>0 && !score_submitted) {
            // **儲存新玩家數據**
            FILE *file = fopen("leaderboard.txt", "a");
            if (file) {
                fprintf(file, "%s,%d;\n", player_name, player_coins);
                fclose(file);
            }
            player_name[0] = '\0';
            type_timer = 10;
            score_submitted = true;
            init_leaderboard(); // 重新加載排行榜
        }
    }
}

// **清除資源**
static void destroy_leaderboard(void) {}


Scene create_leaderboard_scene(int coins_num){
    player_coins = coins_num;
    Scene scene;
    memset(&scene, 0, sizeof(Scene));

    scene.name = "leaderboard";
    scene.init = &init_leaderboard;
    scene.draw = &draw_leaderboard;
    scene.update = &update_leaderboard;
    scene.destroy = &destroy_leaderboard;

    return scene;
}
