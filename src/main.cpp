#include <sdl2/sdl.h>
#include <sdl2/sdl_mixer.h>
#include <iostream>
#include <random>

SDL_Renderer* renderer;

constexpr int RENDER_WIDTH = 640;
constexpr int RENDER_HEIGHT = 480;

std::mt19937 mt {std::random_device{}()};
std::uniform_int_distribution range(0, RENDER_HEIGHT);
std::uniform_int_distribution binary_range(0, 1);

int p1_score = 0;
int p2_score = 0;

enum Vertical_Direction {
    UP = -1,
    DOWN = 1
};

enum Horizontal_Direction {
    LEFT = -1,
    RIGHT = 1
};

struct Ball {
    float x = RENDER_WIDTH/2;
    float y = RENDER_HEIGHT/2;
    const float w = 0.0125f * RENDER_WIDTH;
    const float h = 0.0167f * RENDER_HEIGHT;
    Horizontal_Direction horizontal_direction;
    Vertical_Direction vertical_direction;
    const float velocity = RENDER_HEIGHT;
    float angle = 1.0f;
    float wait = 0.0f;
};

struct Paddle {
    const float x;
    float y = RENDER_HEIGHT/2 - (0.0583f * RENDER_HEIGHT)/2;
    const float w = 0.0125f * RENDER_WIDTH;
    const float h = 0.0583f * RENDER_HEIGHT;
    const float velocity = RENDER_HEIGHT*2;
};

void reset_ball(Ball *ball)
{
    ball->x = RENDER_WIDTH/2;
    ball->y = static_cast<float>(range(mt));
    ball->angle = 1;
    ball->wait = 1.5f;
    (binary_range(mt) == 1) ? ball->vertical_direction = DOWN : ball->vertical_direction = UP;
    (binary_range(mt) == 1) ? ball->horizontal_direction = LEFT : ball->horizontal_direction = RIGHT;
}

bool check_collision(Ball *ball, Paddle *paddle)
{
    SDL_FRect ball_rect = {ball->x, ball->y, ball->w, ball->h};
    SDL_FRect paddle_rect = {paddle->x, paddle->y, paddle->w, paddle->h};
    return SDL_HasIntersectionF(&ball_rect, &paddle_rect);
}

void draw_0(float x_offset, float y_offset);
void draw_1(float x_offset, float y_offset);
void draw_2(float x_offset, float y_offset);
void draw_3(float x_offset, float y_offset);
void draw_4(float x_offset, float y_offset);
void draw_5(float x_offset, float y_offset);
void draw_6(float x_offset, float y_offset);
void draw_7(float x_offset, float y_offset);
void draw_8(float x_offset, float y_offset);
void draw_9(float x_offset, float y_offset);

int main(int argc, char* args[])
{
    // todo: refactor, split engine from game code
    // todo: parse config file for settings
    // todo: window icon
    // todo: backup keyboard controls
    // todo: reset all keyboard shortcut
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER);
    SDL_Window* window = SDL_CreateWindow("pong-clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 960, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(renderer, RENDER_WIDTH, RENDER_HEIGHT);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_ShowCursor(SDL_DISABLE);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Chunk* ball_hit_paddle = Mix_LoadWAV("./assets/audio/ball_hit_paddle.mp3");
    Mix_Chunk* ball_hit_wall = Mix_LoadWAV("./assets/audio/ball_hit_wall.mp3");
    Mix_Chunk* score = Mix_LoadWAV("./assets/audio/score.mp3");

    bool fullscreen = false;
    float game_wait = 5.0f;

    SDL_Event event;
    SDL_GameController* controller = SDL_GameControllerOpen(0);
    constexpr int DEADZONE = 6000;

    Ball ball;
    (binary_range(mt) == 1) ? ball.vertical_direction = DOWN : ball.vertical_direction = UP;
    (binary_range(mt) == 1) ? ball.horizontal_direction = LEFT : ball.horizontal_direction = RIGHT;
    Paddle paddle_left = {0.05f * RENDER_WIDTH};
    Paddle paddle_right = {0.95f * RENDER_WIDTH - paddle_right.w};

    Uint64 previous_time = SDL_GetPerformanceCounter();

    bool running = true;
    while(running)
    {
        Uint64 frame_start_time = SDL_GetPerformanceCounter();

        bool p1_up = false;
        bool p1_down = false;
        bool p2_up = false;
        bool p2_down = false;
        float p1_stick = 0.0f;
        float p2_stick = 0.0f;

        /* input handling */
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        while(SDL_PollEvent(&event) != 0){
            if(event.type == SDL_QUIT)
            {
                running = false;
            }
            if(event.type == SDL_KEYDOWN)
            {
                if(keys[SDL_SCANCODE_F11])
                {
                    if(fullscreen)
                    {
                        SDL_SetWindowFullscreen(window, 0);
                        fullscreen = false;
                    }
                    else
                    {
                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        fullscreen = true;
                    }
                }
            }
        }

        if((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY)*-1) > DEADZONE)
        {
            p1_up = true;
            p1_stick = static_cast<float>((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY)*-1) / 32768.0f);
        }
        else if(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) > DEADZONE)
        {
            p1_down = true;
            p1_stick = static_cast<float>(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / 32768.0f);
        }

        if((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY)*-1) > DEADZONE)
        {
            p2_up = true;
            p2_stick = static_cast<float>((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY)*-1) / 32768.0f);
        }
        else if(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) > DEADZONE)
        {
            p2_down = true;
            p2_stick = static_cast<float>(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) / 32768.0f);
        }
        /* input handling */

        /* logic */
        Uint64 current_time = SDL_GetPerformanceCounter();
        float dT = static_cast<float>(current_time - previous_time) / static_cast<float>(SDL_GetPerformanceFrequency());

        if(game_wait <= 0)
        {
            if(p1_up)
            {   
                ((paddle_left.y - ((paddle_left.velocity * p1_stick) * dT)) <= 0) ? paddle_left.y = 0 : paddle_left.y -= (paddle_left.velocity * p1_stick) * dT;
            }
            if(p1_down)
            {
                (((paddle_left.y + paddle_left.h -1) + ((paddle_left.velocity * p1_stick) * dT)) >= RENDER_HEIGHT) ? paddle_left.y = RENDER_HEIGHT - paddle_left.h : paddle_left.y += (paddle_left.velocity * p1_stick) * dT;
            }
            if(p2_up)
            {
                ((paddle_right.y - ((paddle_right.velocity * p2_stick) * dT)) <= 0) ? paddle_right.y = 0 : paddle_right.y -= (paddle_right.velocity * p2_stick) * dT; 
            }
            if(p2_down)
            {
                (((paddle_right.y + paddle_right.h - 1) + ((paddle_right.velocity * p2_stick) * dT)) >= RENDER_HEIGHT) ? paddle_right.y = RENDER_HEIGHT - paddle_right.h : paddle_right.y += (paddle_right.velocity * p2_stick) * dT;
            }

            if(ball.wait <= 0)
            {
                if(ball.x < 0)
                {
                    Mix_PlayChannel(-1, score, 0);
                    reset_ball(&ball);
                    p2_score++;
                }
                else if((ball.x + ball.w) > RENDER_WIDTH)
                {
                    Mix_PlayChannel(-1, score, 0);
                    reset_ball(&ball);
                    p1_score++;
                }
                else
                {
                    if(ball.y + ball.velocity * dT <= 0)
                    {
                        Mix_PlayChannel(-1, ball_hit_wall, 0);
                        ball.vertical_direction = DOWN;
                    }
                    else if((ball.y + ball.h - 1) >= RENDER_HEIGHT)
                    {
                        Mix_PlayChannel(-1, ball_hit_wall, 0);
                        ball.vertical_direction = UP;
                    }
                    if(check_collision(&ball, &paddle_left))
                    {
                        Mix_PlayChannel(-1, ball_hit_paddle, 0);
                        ball.angle = -((paddle_left.y + ((paddle_left.h-1)/2)) - ball.y) / ((paddle_left.h-1)/2);
                        if(ball.angle < 0)
                        {
                            ball.vertical_direction = UP;
                            ball.angle = -ball.angle;
                        }
                        else
                        {
                            ball.vertical_direction = DOWN;
                        }
                        ball.horizontal_direction = RIGHT;
                    }
                    else if(check_collision(&ball, &paddle_right))
                    {
                        Mix_PlayChannel(-1, ball_hit_paddle, 0);
                        ball.angle = -((paddle_right.y + ((paddle_right.h-1)/2)) - ball.y) / ((paddle_right.h-1)/2);
                        if(ball.angle < 0)
                        {
                            ball.vertical_direction = UP;
                            ball.angle = -ball.angle;
                        }
                        else
                        {
                            ball.vertical_direction = DOWN;
                        }
                        ball.horizontal_direction = LEFT;
                    }
                }
                ball.x += static_cast<float>(ball.horizontal_direction) * (ball.velocity * std::cos(ball.angle)) * dT;
                ball.y += static_cast<float>(ball.vertical_direction) * (ball.velocity * std::sin(ball.angle)) * dT;
            }
        }
        ball.wait -= 1 * dT;
        game_wait -= 1 * dT;

        if(p1_score == 11 || p2_score == 11)
        {
            if(game_wait <= 0)
            {
                game_wait = 5.0f;
            }
            else if(game_wait > 0 && game_wait <= 0.5f)
            {
                p1_score = 0;
                p2_score = 0;
                paddle_left.y = RENDER_HEIGHT/2 - (0.0583f * RENDER_HEIGHT)/2;
                paddle_right.y = RENDER_HEIGHT/2 - (0.0583f * RENDER_HEIGHT)/2;
            }
        }

        previous_time = current_time;
        /* logic*/

        /* rendering */
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        for(float i = 0; i < 30 * 0.0333f*RENDER_HEIGHT; i+=0.0333f*RENDER_HEIGHT)
        {
            SDL_FRect centre_line = {RENDER_WIDTH/2 - (0.00312f * RENDER_WIDTH)/2, i, 0.00312f * RENDER_WIDTH, 0.0167f * RENDER_HEIGHT};
            SDL_RenderDrawRectF(renderer, &centre_line);
            SDL_RenderFillRectF(renderer, &centre_line);
        }

        switch(p1_score)
        {
            case 0: draw_0(0.275f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 1: draw_1(0.275f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 2: draw_2(0.275f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 3: draw_3(0.275f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 4: draw_4(0.275f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 5: draw_5(0.275f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 6: draw_6(0.275f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 7: draw_7(0.275f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 8: draw_8(0.275f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 9: draw_9(0.275f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 10: draw_1(0.175f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); draw_0(0.275f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 11: draw_1(0.175f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); draw_1(0.275f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
        }
        switch(p2_score)
        {
            case 0: draw_0(0.775f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 1: draw_1(0.775f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 2: draw_2(0.775f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 3: draw_3(0.775f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 4: draw_4(0.775f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 5: draw_5(0.775f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 6: draw_6(0.775f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 7: draw_7(0.775f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 8: draw_8(0.775f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 9: draw_9(0.775f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 10: draw_1(0.675f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); draw_0(0.775f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
            case 11: draw_1(0.675f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); draw_1(0.775f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT);; break;
        }

        if(ball.wait > 0 || game_wait > 0){
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        }
        SDL_FRect ball_rect = {ball.x, ball.y, ball.w, ball.h};
        SDL_RenderDrawRectF(renderer, &ball_rect);
        SDL_RenderFillRectF(renderer, &ball_rect);

        if(game_wait > 0){
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }
        SDL_FRect paddle_left_rect = {paddle_left.x, paddle_left.y, paddle_left.w, paddle_left.h};
        SDL_RenderDrawRectF(renderer, &paddle_left_rect);
        SDL_RenderFillRectF(renderer, &paddle_left_rect);

        SDL_FRect paddle_right_rect = {paddle_right.x, paddle_right.y, paddle_right.w, paddle_right.h};
        SDL_RenderDrawRectF(renderer, &paddle_right_rect);
        SDL_RenderFillRectF(renderer, &paddle_right_rect);

        SDL_RenderPresent(renderer);
        /* rendering */

        Uint64 frame_end_time = SDL_GetPerformanceCounter();
        float frame_time = static_cast<float>(frame_end_time - frame_start_time) / static_cast<float>(SDL_GetPerformanceFrequency());
        //SDL_Delay(static_cast<Uint32>(((1.0f / 120.0f) * 1000.0f) - frame_time));
    }

    SDL_Quit();

    return 0;
}

void draw_0(float x_offset, float y_offset)
{
    SDL_Rect rect1 = {static_cast<int>(x_offset), static_cast<int>(y_offset), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.133f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect1);
    SDL_RenderFillRect(renderer, &rect1);
    SDL_Rect rect2 = {static_cast<int>(x_offset + 0.0125f*RENDER_WIDTH), static_cast<int>(y_offset), static_cast<int>(0.025f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect2);
    SDL_RenderFillRect(renderer, &rect2);
    SDL_Rect rect3 = {static_cast<int>(x_offset + 0.0125f*RENDER_WIDTH), static_cast<int>(y_offset + 0.116f*RENDER_HEIGHT), static_cast<int>(0.025f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect3);
    SDL_RenderFillRect(renderer, &rect3);
    SDL_Rect rect4 = {static_cast<int>(x_offset + 0.0375f*RENDER_WIDTH), static_cast<int>(y_offset), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.133f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect4);
    SDL_RenderFillRect(renderer, &rect4);
}

void draw_1(float x_offset, float y_offset)
{
    SDL_Rect rect1 = {static_cast<int>(x_offset + 0.0375*RENDER_HEIGHT), static_cast<int>(y_offset), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.133f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect1);
    SDL_RenderFillRect(renderer, &rect1);
}

void draw_2(float x_offset, float y_offset)
{
    SDL_Rect rect1 = {static_cast<int>(x_offset), static_cast<int>(y_offset + 0.05f*RENDER_HEIGHT), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.083f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect1);
    SDL_RenderFillRect(renderer, &rect1);
    SDL_Rect rect2 = {static_cast<int>(x_offset), static_cast<int>(y_offset), static_cast<int>(0.0375f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect2);
    SDL_RenderFillRect(renderer, &rect2);
    SDL_Rect rect3 = {static_cast<int>(x_offset + 0.0125f*RENDER_WIDTH), static_cast<int>(y_offset + 0.116f*RENDER_HEIGHT), static_cast<int>(0.0375f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect3);
    SDL_RenderFillRect(renderer, &rect3);
    SDL_Rect rect4 = {static_cast<int>(x_offset + 0.0125f*RENDER_WIDTH), static_cast<int>(y_offset + 0.05f*RENDER_HEIGHT), static_cast<int>(0.025f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect4);
    SDL_RenderFillRect(renderer, &rect4);
    SDL_Rect rect5 = {static_cast<int>(x_offset + 0.0375f*RENDER_WIDTH), static_cast<int>(y_offset), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.0667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect5);
    SDL_RenderFillRect(renderer, &rect5);
}

void draw_3(float x_offset, float y_offset)
{
    SDL_Rect rect1 = {static_cast<int>(x_offset), static_cast<int>(y_offset), static_cast<int>(0.0375f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect1);
    SDL_RenderFillRect(renderer, &rect1);
    SDL_Rect rect2 = {static_cast<int>(x_offset), static_cast<int>(y_offset + 0.116f*RENDER_HEIGHT), static_cast<int>(0.0375f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect2);
    SDL_RenderFillRect(renderer, &rect2);
    SDL_Rect rect3 = {static_cast<int>(x_offset), static_cast<int>(y_offset + 0.05f*RENDER_HEIGHT), static_cast<int>(0.0375f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect3);
    SDL_RenderFillRect(renderer, &rect3);
    SDL_Rect rect4 = {static_cast<int>(x_offset + 0.0375f*RENDER_WIDTH), static_cast<int>(y_offset), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.133f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect4);
    SDL_RenderFillRect(renderer, &rect4);
}

void draw_4(float x_offset, float y_offset)
{
    SDL_Rect rect1 = {static_cast<int>(x_offset), static_cast<int>(y_offset), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.0667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect1);
    SDL_RenderFillRect(renderer, &rect1);
    SDL_Rect rect4 = {static_cast<int>(x_offset + 0.0125f*RENDER_WIDTH), static_cast<int>(y_offset + 0.05f*RENDER_HEIGHT), static_cast<int>(0.025f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect4);
    SDL_RenderFillRect(renderer, &rect4);
    SDL_Rect rect5 = {static_cast<int>(x_offset + 0.0375f*RENDER_WIDTH), static_cast<int>(y_offset), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.133f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect5);
    SDL_RenderFillRect(renderer, &rect5);
}

void draw_5(float x_offset, float y_offset)
{
    SDL_Rect rect1 = {static_cast<int>(x_offset), static_cast<int>(y_offset), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.0667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect1);
    SDL_RenderFillRect(renderer, &rect1);
    SDL_Rect rect2 = {static_cast<int>(x_offset + 0.0125f*RENDER_WIDTH), static_cast<int>(y_offset), static_cast<int>(0.0375f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect2);
    SDL_RenderFillRect(renderer, &rect2);
    SDL_Rect rect3 = {static_cast<int>(x_offset), static_cast<int>(y_offset + 0.116f*RENDER_HEIGHT), static_cast<int>(0.0375f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect3);
    SDL_RenderFillRect(renderer, &rect3);
    SDL_Rect rect4 = {static_cast<int>(x_offset + 0.0125f*RENDER_WIDTH), static_cast<int>(y_offset + 0.05f*RENDER_HEIGHT), static_cast<int>(0.025f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect4);
    SDL_RenderFillRect(renderer, &rect4);
    SDL_Rect rect5 = {static_cast<int>(x_offset + 0.0375f*RENDER_WIDTH), static_cast<int>(y_offset + 0.05*RENDER_HEIGHT), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.083f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect5);
    SDL_RenderFillRect(renderer, &rect5);
}

void draw_6(float x_offset, float y_offset)
{
    SDL_Rect rect1 = {static_cast<int>(x_offset), static_cast<int>(y_offset), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.133f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect1);
    SDL_RenderFillRect(renderer, &rect1);
    SDL_Rect rect3 = {static_cast<int>(x_offset + 0.0125f*RENDER_WIDTH), static_cast<int>(y_offset + 0.116f*RENDER_HEIGHT), static_cast<int>(0.025f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect3);
    SDL_RenderFillRect(renderer, &rect3);
    SDL_Rect rect4 = {static_cast<int>(x_offset + 0.0125f*RENDER_WIDTH), static_cast<int>(y_offset + 0.05f*RENDER_HEIGHT), static_cast<int>(0.025f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect4);
    SDL_RenderFillRect(renderer, &rect4);
    SDL_Rect rect5 = {static_cast<int>(x_offset + 0.0375f*RENDER_WIDTH), static_cast<int>(y_offset + 0.05*RENDER_HEIGHT), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.083f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect5);
    SDL_RenderFillRect(renderer, &rect5);
}

void draw_7(float x_offset, float y_offset)
{
    SDL_Rect rect2 = {static_cast<int>(x_offset), static_cast<int>(y_offset), static_cast<int>(0.0375f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect2);
    SDL_RenderFillRect(renderer, &rect2);
    SDL_Rect rect5 = {static_cast<int>(x_offset + 0.0375f*RENDER_WIDTH), static_cast<int>(y_offset), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.133f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect5);
    SDL_RenderFillRect(renderer, &rect5);
}

void draw_8(float x_offset, float y_offset)
{
    SDL_Rect rect1 = {static_cast<int>(x_offset), static_cast<int>(y_offset), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.133f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect1);
    SDL_RenderFillRect(renderer, &rect1);
    SDL_Rect rect2 = {static_cast<int>(x_offset + 0.0125f*RENDER_WIDTH), static_cast<int>(y_offset), static_cast<int>(0.025f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect2);
    SDL_RenderFillRect(renderer, &rect2);
    SDL_Rect rect3 = {static_cast<int>(x_offset + 0.0125f*RENDER_WIDTH), static_cast<int>(y_offset + 0.116f*RENDER_HEIGHT), static_cast<int>(0.025f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect3);
    SDL_RenderFillRect(renderer, &rect3);
    SDL_Rect rect4 = {static_cast<int>(x_offset + 0.0125f*RENDER_WIDTH), static_cast<int>(y_offset + 0.05f*RENDER_HEIGHT), static_cast<int>(0.025f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect4);
    SDL_RenderFillRect(renderer, &rect4);
    SDL_Rect rect5 = {static_cast<int>(x_offset + 0.0375f*RENDER_WIDTH), static_cast<int>(y_offset), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.133f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect5);
    SDL_RenderFillRect(renderer, &rect5);
}

void draw_9(float x_offset, float y_offset)
{
    SDL_Rect rect1 = {static_cast<int>(x_offset), static_cast<int>(y_offset), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.0667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect1);
    SDL_RenderFillRect(renderer, &rect1);
    SDL_Rect rect2 = {static_cast<int>(x_offset + 0.0125f*RENDER_WIDTH), static_cast<int>(y_offset), static_cast<int>(0.025f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect2);
    SDL_RenderFillRect(renderer, &rect2);
    SDL_Rect rect4 = {static_cast<int>(x_offset + 0.0125f*RENDER_WIDTH), static_cast<int>(y_offset + 0.05f*RENDER_HEIGHT), static_cast<int>(0.025f*RENDER_WIDTH), static_cast<int>(0.01667f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect4);
    SDL_RenderFillRect(renderer, &rect4);
    SDL_Rect rect5 = {static_cast<int>(x_offset + 0.0375f*RENDER_WIDTH), static_cast<int>(y_offset), static_cast<int>(0.0125f*RENDER_WIDTH), static_cast<int>(0.133f*RENDER_HEIGHT)};
    SDL_RenderDrawRect(renderer, &rect5);
    SDL_RenderFillRect(renderer, &rect5);
}