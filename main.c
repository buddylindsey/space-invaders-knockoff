#include <SDL.h>
#include <stdbool.h>


const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;
const float TIME_STEP = 1.0f / FPS;

typedef struct {
    float health;
    SDL_Rect visual;
    char *name;
    bool visible;
} GameObject;

typedef struct {
    GameObject player;
    GameObject enemies[3][10];
    GameObject bullets[3];
    bool running;
    SDL_Renderer* renderer;
} GameState;


GameObject create_game_object(char *name, int x, int y, int width, int height) {
    SDL_Rect visual = { x, y, width, height };
    GameObject object = { 100.0f, visual, name, true};
    object.health = 100.0f;
    object.visual = visual;
    object.visible = true;

    // Dynamically allocate memory for the name
    object.name = malloc(strlen(name) + 1); // +1 for the null terminator
    if (object.name != NULL) {
        strcpy(object.name, name);
    }

    return object;
}

void move_left(GameObject *object, int step) {
    if(object->visual.x <= 0)
        return;
    object->visual.x -= step;
}

void move_right(GameObject *object, int step) {
    if(object->visual.x >= WINDOW_WIDTH-object->visual.w)
        return;
    object->visual.x += step;
}

void draw_game_object(SDL_Renderer* renderer, GameObject object) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set draw color (white)
    SDL_RenderFillRect(renderer, &object.visual); // Draw a filled rectangle with the draw color
}

void render_scene(GameState *game_state) {

    SDL_SetRenderDrawColor(game_state->renderer, 0, 0, 0, 255); // Set draw color (black)
    SDL_RenderClear(game_state->renderer); // Clear the screen with the draw color

    for (int i = 0; i < 1; i++) {
        draw_game_object(game_state->renderer, game_state->player);
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            draw_game_object(game_state->renderer, game_state->bullets[i]);
        }
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            draw_game_object(game_state->renderer, game_state->enemies[i][j]);
        }
    }
}

void create_enemies(GameState *game_state) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            game_state->enemies[i][j] = create_game_object("enemy", 50 + j*50, 50 + i*50, 20, 20);
        }
    }
}

void move_bullet(GameObject *bullet) {
    if(bullet->visual.y <= 0 - bullet->visual.h) {
        return;
    }
    bullet->visual.y -= 10;
}

void fire_bullet(GameState *game_state) {
    for (int i = 0; i < 3; i++) {
        if(game_state->bullets[i].visual.y <= 0){
            int x = game_state->player.visual.x + game_state->player.visual.w/2;
            int y = game_state->player.visual.y;
            game_state->bullets[i] = create_game_object("bullet", x, y, 5, 5);
            return;
        }
    }
}

bool enemy_move_left = true;
float enemy_speed = 1;

void destroy_enemy(GameState *game_state, int i, int j) {
    game_state->enemies[i][j].visual.x = -1000;
    game_state->enemies[i][j].visual.y = -1000;
    game_state->enemies[i][j].visible = false;
}

void enemy_bullet_collision(GameState *game_state) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 3; k++) {
                int bullet_x = game_state->bullets[k].visual.x;
                int bullet_y = game_state->bullets[k].visual.y;

                int enemy_x = game_state->enemies[i][j].visual.x;
                int enemy_y = game_state->enemies[i][j].visual.y;
                int enemy_w = game_state->enemies[i][j].visual.w;
                int enemy_h = game_state->enemies[i][j].visual.h;

                if(bullet_x >= enemy_x && bullet_x <= enemy_x + enemy_w &&
                   bullet_y >= enemy_y && bullet_y <= enemy_y + enemy_h) {
                    destroy_enemy(game_state, i, j);
                    game_state->bullets[k].visual.y = -1000;
                }
            }
        }
    }
}

void move_enemies_down(GameState *game_state) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            game_state->enemies[i][j].visual.y += 20;
            enemy_speed += 0.01;
        }
    }
}

void move_enemies(GameState *game_state) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            bool enemy_visible = game_state->enemies[i][j].visible;
            int enemy_x = game_state->enemies[i][j].visual.x;
            int enemy_w = game_state->enemies[i][j].visual.w;

            if (enemy_move_left) {
                if(enemy_visible && enemy_x <= 0) {
                    enemy_move_left = false;
                    move_enemies_down(game_state);
                }
                game_state->enemies[i][j].visual.x -= enemy_speed;
            }
            if (!enemy_move_left) {
                if(enemy_visible && enemy_x >= WINDOW_WIDTH-enemy_w) {
                    enemy_move_left = true;
                    move_enemies_down(game_state);
                }
                game_state->enemies[i][j].visual.x += enemy_speed;
            }
        }
    }
}

void destroy_game_object(GameObject *object) {
    // Free the dynamically allocated name
    if (object->name != NULL) {
        free(object->name);
        object->name = NULL; // Avoid dangling pointer
    }
}

GameState initialize_game_state(SDL_Window* window) {
    GameState game_state;
    game_state.player = create_game_object("ship", 20, WINDOW_HEIGHT-50, 20, 20);
    game_state.running = true;
    game_state.renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    create_enemies(&game_state);
    return game_state;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO); // Initialize SDL2

    SDL_Window* window = SDL_CreateWindow(
        "Space Invaders",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        WINDOW_WIDTH,                      // width, in pixels
        WINDOW_HEIGHT,                     // height, in pixels
        0                                  // flags
    );

    GameState game_state = initialize_game_state(window);

    SDL_Event event;

    // Variables for frame timing
    Uint32 previousTicks = SDL_GetTicks();
    float lag = 0.0f;

    while (game_state.running) {
        Uint32 currentTicks = SDL_GetTicks();
        Uint32 elapsedTicks = currentTicks - previousTicks;
        previousTicks = currentTicks;
        float deltaTime = elapsedTicks / 1000.0f;

        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                game_state.running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    game_state.running = false;
                }
                if (event.key.keysym.sym == SDLK_LEFT) {
                    move_left(&game_state.player, 10);
                }
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    move_right(&game_state.player, 10);
                }
                if (event.key.keysym.sym == SDLK_SPACE) {
                    fire_bullet(&game_state);
                }
            }
            // Additional input handling here (e.g., keyboard events)
        }

        lag += deltaTime;
        while (lag >= TIME_STEP) {
            for (int i = 0; i < 3; i++) {
                move_bullet(&game_state.bullets[i]);
            }

            move_enemies(&game_state);
            enemy_bullet_collision(&game_state);

            lag -= TIME_STEP;
        }

        // Game state update logic here (movement, collisions, etc.)
        render_scene(&game_state);

        // Drawing code here (draw your game objects)
        SDL_RenderPresent(game_state.renderer); // Update the screen with rendering performed since the last call

        Uint32 frameTicks = SDL_GetTicks() - currentTicks;
        if (FRAME_DELAY > frameTicks) {
            SDL_Delay(FRAME_DELAY - frameTicks);
        }
    }

    // Cleanup
    SDL_DestroyRenderer(game_state.renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // Destroy game objects
    destroy_game_object(&game_state.player);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            destroy_game_object(&game_state.enemies[i][j]);
        }
    }
    for (int i = 0; i < 3; i++) {
        destroy_game_object(&game_state.bullets[i]);
    }

    printf("Game exited successfully!\n");

    return 0;
}
