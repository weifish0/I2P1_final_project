#include "player.h"
#include "game.h"
#include "utility.h"
#include "map.h"

#include <math.h>

static bool isCollision(Player* player, Map* map);

Player create_player(char * path, int row, int col){
    Player player;
    memset(&player, 0, sizeof(player));
    
    player.coord = (Point){
        col * TILE_SIZE,
        row * TILE_SIZE
    };
    
    player.speed = 4;
    player.health = 50;
    player.status = PLAYER_IDLE;
    
    player.image = al_load_bitmap(path);
    if(!player.image){
        game_abort("Error Load Bitmap with path : %s", path);
    }
    
    return player;
}

void update_player(Player * player, Map* map){

    Point original = player->coord;
    
    // Knockback effect
    if(player->knockback_CD > 0){

        player->knockback_CD--;
        int next_x = player->coord.x + player->speed * cos(player->knockback_angle);
        int next_y = player->coord.y + player->speed * sin(player->knockback_angle);
        Point next;
        next = (Point){next_x, player->coord.y};
        
        if(!isCollision(player, map)){
            player->coord = next;
        }
        
        next = (Point){player->coord.x, next_y};
        if(!isCollision(player, map)){
            player->coord = next;
        }
    }
    
    if(player->status == PLAYER_DYING){
    }
    else if(player->status == PLAYER_DEAD){
        return;
    }
    else if (keyState[ALLEGRO_KEY_W]) {
        player->coord.y -= player->speed;  // Move up
        player->direction = UP;
        player->status = PLAYER_WALKING;
    }
    else if (keyState[ALLEGRO_KEY_S]) {
        player->coord.y += player->speed;  // Move down
        player->direction = DOWN;
        player->status = PLAYER_WALKING;
    }
    else if (keyState[ALLEGRO_KEY_A]) {
        player->coord.x -= player->speed;  // Move left
        player->direction = LEFT;
        player->status = PLAYER_WALKING;
    }
    else if (keyState[ALLEGRO_KEY_D]) {
        player->coord.x += player->speed;  // Move right
        player->direction = RIGHT;
        player->status = PLAYER_WALKING;
    }
    else{
        player->status = PLAYER_IDLE;
    }

    // if Collide, snap to the grid to make it pixel perfect
    if(isCollision(player, map)){
        player->coord.y = round((float)original.y / (float)TILE_SIZE) * TILE_SIZE;
    }
    
    if(isCollision(player, map)){
        player->coord.x = round((float)original.x / (float)TILE_SIZE) * TILE_SIZE;
    }
    
    /*
        [TODO Homework] 
        
        Calculate the animation tick to draw animation later
    */
    if (player->status == PLAYER_IDLE){
        player->animation_tick = (player->animation_tick+1) % 16;
    }
    else if(player->status == PLAYER_WALKING){
        player->animation_tick = (player->animation_tick+1) % 32;
    }
    else if(player->status == PLAYER_DYING){
        player->animation_tick = (player->animation_tick+1) % 32;
    }
}

void draw_player(Player * player, Point cam){
    int dy = player->coord.y - cam.y; // destiny y axis
    int dx = player->coord.x - cam.x; // destiny x axis
    
    /*
        [TODO Homework] 
        
        Draw Animation of Dying, Walking, and Idle
    */
    int offset = 32 * (player->animation_tick / 8);
    int tint_red = player->knockback_CD > 0 ? 0 : 255;
    int flag = 0;
    if (player->status == PLAYER_IDLE) {
        // 閒置狀態的動畫
        al_draw_tinted_scaled_bitmap(player->image, al_map_rgb(255, tint_red, tint_red),
            offset, 0, 32, 32,
            dx, dy, TILE_SIZE, TILE_SIZE,
            flag);
    }
    else if (player->status == PLAYER_DYING) {
        // 死亡狀態的動畫，這裡用圓形來表示死亡動畫
        al_draw_tinted_scaled_bitmap(player->image, al_map_rgb(255, tint_red, tint_red),
            offset, 64, 32, 32,
            dx, dy, TILE_SIZE, TILE_SIZE,
            flag);
        if(player->animation_tick == 31){
            player->status = PLAYER_DEAD;
        }
    }
    else if (player->status == PLAYER_WALKING){
        al_draw_tinted_scaled_bitmap(player->image, al_map_rgb(255, tint_red, tint_red),
            offset, 32, 32, 32,
            dx, dy, TILE_SIZE, TILE_SIZE,
            flag);
    }

    
#ifdef DRAW_HITBOX
    al_draw_rectangle(
        dx, dy, dx + TILE_SIZE, dy + TILE_SIZE,
        al_map_rgb(255, 0, 0), 1
    );
#endif
    
}

void delete_player(Player * player){
    al_destroy_bitmap(player->image);
}

static bool isCollision(Player* player, Map* map){

    if (player->coord.x < 0 ||
        player->coord.y < 0 ||
        (player->coord.x + TILE_SIZE - 1) / TILE_SIZE >= map->col ||
        (player->coord.y + TILE_SIZE - 1) / TILE_SIZE >= map->row)
        return true;

    /*

        [TODO HACKATHON 2-1] 
    
        Check every corner of enemy if it's collide or not

        We check every corner if it's collide with the wall/hole or not
        You can activate DRAW_HITBOX in utility.h to see how the collision work

        if(!isWalkable(map->map[...][...])) return true;
        if(!isWalkable(map->map[...][...])) return true;
        if(!isWalkable(map->map[...][...])) return true;
        if(!isWalkable(map->map[...][...])) return true;

    */
    // Check every corner of the player (player's bounding box)
    int player_left = player->coord.x / TILE_SIZE;
    int player_right = (player->coord.x + TILE_SIZE - 1) / TILE_SIZE;
    int player_top = player->coord.y / TILE_SIZE;
    int player_bottom = (player->coord.y + TILE_SIZE - 1) / TILE_SIZE;

    // Check the four corners (top-left, top-right, bottom-left, bottom-right)
    if (!isWalkable(map->map[player_top][player_left])) return true;
    if (!isWalkable(map->map[player_top][player_right])) return true;
    if (!isWalkable(map->map[player_bottom][player_left])) return true;
    if (!isWalkable(map->map[player_bottom][player_right])) return true;
    
    return false;
}

void hitPlayer(Player * player, Point enemy_coord, int damage){
    if(player->knockback_CD == 0){
        float dY = player->coord.y - enemy_coord.y;
        float dX = player->coord.x - enemy_coord.x;
        float angle = atan2(dY, dX);

        /*
            [TODO Homework]
            
            Game Logic when the player get hit or die

            player->knockback_angle = angle;
            player->knockback_CD = 32;

            player->health = ...
            if(player->health <= 0){

                player->health = 0;
                player->status = ...

                ...
            }
        */
        player->knockback_angle = angle;
        player->knockback_CD = 32;
        player->health -= damage;
        if(player->health <= 0){
            player->health = 0;
            player->animation_tick = 0;
            player->status = PLAYER_DYING;
        }

    }
}
