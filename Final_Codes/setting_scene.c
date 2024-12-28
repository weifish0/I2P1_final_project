#include <allegro5/allegro.h>
#include "setting_scene.h"
#include "menu_scene.h"
#include "utility.h"
#include "UI.h"
#include "game.h"

static Button backButton;

static void init(void) {
    backButton = button_create(SCREEN_W / 2 - 200, 700, 400, 100, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
}

// 滑動條的基本參數
const int SLIDER_X = 200; // 滑動條的起始 X 座標
const int SLIDER_Y = 300; // 滑動條的起始 Y 座標
const int SLIDER_WIDTH = 400; // 滑動條的寬度
const int SLIDER_HEIGHT = 20; // 滑動條的高度
const int SLIDER_HANDLE_RADIUS = 20; // 滑塊的半徑

bool is_dragging = false; // 滑塊是否被拖動

static void update_slider(void) {
    int mouse_x = mouseState.x;
    int mouse_y = mouseState.y;

    int handle_x = SLIDER_X + BGM_VOLUME * SLIDER_WIDTH;

    if (mouseState.buttons) { // 偵測左鍵按下
        if (is_dragging || 
            (mouse_x >= handle_x - SLIDER_HANDLE_RADIUS && mouse_x <= handle_x + SLIDER_HANDLE_RADIUS &&
             mouse_y >= SLIDER_Y - SLIDER_HANDLE_RADIUS && mouse_y <= SLIDER_Y + SLIDER_HANDLE_RADIUS)) {
            is_dragging = true;

            // 更新滑塊位置，限制在滑動條範圍內
            float new_value = (float)(mouse_x - SLIDER_X) / SLIDER_WIDTH;
            BGM_VOLUME = (new_value < 0) ? 0 : (new_value > 1 ? 1 : new_value);

            // 更新音量值
            change_bgm("Assets/audio/menu_bgm.wav");
            SFX_VOLUME = BGM_VOLUME;
        }
    } else {
        is_dragging = false; // 左鍵鬆開時停止拖動
    }
}

static void update(void) {
    update_button(&backButton);
    update_slider();

    if (mouseState.buttons && backButton.hovered == true) {
        change_scene(create_menu_scene());
    }
}

static void draw(void) {
    // button
    draw_button(backButton);
    //button text
    al_draw_text(
        P2_FONT,
        al_map_rgb(66, 76, 110),
        SCREEN_W / 2,
        700 + 28 + backButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "BACK"
    );
    al_draw_text(
        P2_FONT,
        al_map_rgb(225, 225, 225),
        SCREEN_W / 2,
        700 + 31 + backButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "BACK"
    );
    // 繪製滑動條背景
    al_draw_filled_rectangle(
        SLIDER_X, SLIDER_Y - SLIDER_HEIGHT / 2,
        SLIDER_X + SLIDER_WIDTH, SLIDER_Y + SLIDER_HEIGHT / 2,
        al_map_rgb(200, 200, 200)
    );

    // 繪製滑塊
    int handle_x = SLIDER_X + BGM_VOLUME * SLIDER_WIDTH;
    al_draw_filled_circle(handle_x, SLIDER_Y, SLIDER_HANDLE_RADIUS, al_map_rgb(255, 100, 100));
}

static void destroy(void) {
    destroy_button(&backButton);
}

Scene create_setting_scene(void) {
    Scene scene;
    memset(&scene, 0, sizeof(Scene));

    scene.name = "setting";
    scene.init = &init;
    scene.draw = &draw;
    scene.update = &update;
    scene.destroy = &destroy;

    return scene;
}
