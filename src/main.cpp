#include <config_parser.hpp>
#include <sdl2/sdl.h>
#include <sdl2/sdl_mixer.h>
#include <sdl2/sdl_image.h>
#include <iostream>
#include <random>
#include <unordered_map>

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

constexpr int RENDER_WIDTH = 640;
constexpr int RENDER_HEIGHT = 480;
SDL_Renderer* renderer;

std::mt19937 rng {std::random_device{}()};

enum Vertical_Direction {
    UP = -1,
    DOWN = 1
};

enum Horizontal_Direction {
    LEFT = -1,
    RIGHT = 1
};

struct Ball {
    float x = RENDER_WIDTH / 2;
    float y = RENDER_HEIGHT / 2;
    const float w = 0.0125f * RENDER_WIDTH;
    const float h = 0.0167f * RENDER_HEIGHT;
    Horizontal_Direction hor = std::uniform_int_distribution{0, 1}(rng) ? LEFT : RIGHT;
    Vertical_Direction ver = std::uniform_int_distribution{0, 1}(rng) ? UP : DOWN;
    const float velocity = RENDER_HEIGHT * 1.2;
    float acceleration = 0.0f;
    float angle = 1.0f;
    float wait = 0.0f;
};

void reset_ball(Ball* ball)
{
    ball->x = RENDER_WIDTH / 2;
    ball->y = static_cast<float>(std::uniform_int_distribution{0, RENDER_HEIGHT}(rng));
    ball->hor = std::uniform_int_distribution{0, 1}(rng) ? LEFT : RIGHT;
    ball->ver = std::uniform_int_distribution{0, 1}(rng) ? UP : DOWN;
    ball->acceleration = 0.0f;
    ball->angle = 1.0f;
    ball->wait = 1.5f;
}

struct Paddle {
    const float x;
    float y = RENDER_HEIGHT / 2 - (0.0583f * RENDER_HEIGHT) / 2;
    const float w = 0.0125f * RENDER_WIDTH;
    const float h = 0.0583f * RENDER_HEIGHT;
    const float velocity = RENDER_HEIGHT * 2;
};

bool check_collision(Ball* ball, Paddle* paddle)
{
    SDL_FRect ball_rect = {ball->x, ball->y, ball->w, ball->h};
    SDL_FRect paddle_rect = {paddle->x, paddle->y, paddle->w, paddle->h};
    return SDL_HasIntersectionF(&ball_rect, &paddle_rect);
}

float calculate_angle(Ball* ball, Paddle* paddle)
{
    const float MAX_ANGLE = (0.04f * paddle->h);
    float tmp_angle = -((paddle->y + ((paddle->h-1)/2)) - ball->y) / ((paddle->h-1)/2);
    if(tmp_angle > MAX_ANGLE)
    {
        tmp_angle = MAX_ANGLE;
    }
    else if(tmp_angle < -MAX_ANGLE)
    {
        tmp_angle = -MAX_ANGLE;
    }
    return tmp_angle;
}

int main(int argc, char* args[])
{
    std::unordered_map<std::string, int> config = parse_config_file("./data/config.txt");

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER);
    IMG_Init(IMG_INIT_PNG);

    // initialise window
    bool fullscreen;
    Uint32 fullscreen_mode = SDL_WINDOW_FULLSCREEN_DESKTOP;
    Uint32 window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN;
    switch(config.at("WINDOW_MODE"))
    {
        case 2: fullscreen_mode = SDL_WINDOW_FULLSCREEN; window_flags += fullscreen_mode; fullscreen = true; break;
        case 1: fullscreen_mode = SDL_WINDOW_FULLSCREEN_DESKTOP; window_flags += fullscreen_mode; fullscreen = true; break;
        case 0: fullscreen = false; break;
    }
    SDL_Window* window = SDL_CreateWindow("pong-clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, config.at("WINDOW_RESOLUTION_X"), config.at("WINDOW_RESOLUTION_Y"), window_flags);
    SDL_SetWindowIcon(window, IMG_Load("./assets/window_icon.png"));

    // initialise renderer
    Uint32 renderer_flags = SDL_RENDERER_ACCELERATED;
    switch(config.at("VSYNC"))
    {
        case 1: renderer_flags += SDL_RENDERER_PRESENTVSYNC; break;
        case 0: break;
    }
    renderer = SDL_CreateRenderer(window, -1, renderer_flags);
    SDL_RenderSetLogicalSize(renderer, RENDER_WIDTH, RENDER_HEIGHT);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_RenderSetIntegerScale(renderer, static_cast<SDL_bool>(config.at("SCALING_MODE")));
    
    // initialise audio
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Chunk* ball_hit_paddle = Mix_LoadWAV("./assets/audio/ball_hit_paddle.mp3");
    Mix_Chunk* ball_hit_wall = Mix_LoadWAV("./assets/audio/ball_hit_wall.mp3");
    Mix_Chunk* score = Mix_LoadWAV("./assets/audio/score.mp3");

    float game_wait = 0.0f;

    SDL_Event event;

    int p1_score = 0;
    int p2_score = 0;
    constexpr int MAX_SCORE = 11;

    SDL_GameController* controller = SDL_GameControllerOpen(0);
    constexpr int DEADZONE = 6000;

    Ball ball {};
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
        float p1_stick = 1.0f;
        float p2_stick = 1.0f;

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
                        SDL_SetWindowFullscreen(window, fullscreen_mode);
                        fullscreen = true;
                    }
                }
                if(keys[SDL_SCANCODE_F5])
                {
                    game_wait = 2.0f;
                    p1_score = 0;
                    p2_score = 0;
                    paddle_left.y = RENDER_HEIGHT/2 - (0.0583f * RENDER_HEIGHT)/2;
                    paddle_right.y = RENDER_HEIGHT/2 - (0.0583f * RENDER_HEIGHT)/2;
                    reset_ball(&ball);
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
        previous_time = current_time;

        if(game_wait <= 0)
        {
            if(p1_up)
            {   
                ((paddle_left.y - ((paddle_left.velocity * p1_stick) * dT)) <= 0) ? paddle_left.y = 0 : paddle_left.y -= (paddle_left.velocity * p1_stick) * dT;
            }
            if(p1_down)
            {
                (((paddle_left.y + paddle_left.h-1) + ((paddle_left.velocity * p1_stick) * dT)) >= RENDER_HEIGHT) ? paddle_left.y = RENDER_HEIGHT - paddle_left.h : paddle_left.y += (paddle_left.velocity * p1_stick) * dT;
            }
            if(p2_up)
            {   
                ((paddle_right.y - ((paddle_right.velocity * p2_stick) * dT)) <= 0) ? paddle_right.y = 0 : paddle_right.y -= (paddle_right.velocity * p2_stick) * dT;
            }
            if(p2_down)
            {
                (((paddle_right.y + paddle_right.h-1) + ((paddle_right.velocity * p2_stick) * dT)) >= RENDER_HEIGHT) ? paddle_right.y = RENDER_HEIGHT - paddle_right.h : paddle_right.y += (paddle_right.velocity * p2_stick) * dT;
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
                    if((ball.y + ball.velocity * dT) <= 0)
                    {
                        Mix_PlayChannel(-1, ball_hit_wall, 0);
                        ball.ver = DOWN;

                    }
                    else if(((ball.y + ball.h - 1) - ball.velocity * dT) >= RENDER_HEIGHT)
                    {
                        Mix_PlayChannel(-1, ball_hit_wall, 0);
                        ball.ver = UP;
                    }

                    if(check_collision(&ball, &paddle_left))
                    {
                        Mix_PlayChannel(-1, ball_hit_paddle, 0);
                        ball.acceleration += (0.015f * RENDER_HEIGHT);
                        ball.angle = calculate_angle(&ball, &paddle_left);
                        if(ball.angle < 0)
                        {
                            ball.ver = UP;
                            ball.angle = -ball.angle;
                        }
                        else
                        {
                            ball.ver = DOWN;
                        }
                        ball.hor = RIGHT;
                    }
                    else if(check_collision(&ball, &paddle_right))
                    {
                        Mix_PlayChannel(-1, ball_hit_paddle, 0);
                        ball.acceleration += (0.015f * RENDER_HEIGHT);
                        ball.angle = calculate_angle(&ball, &paddle_right);
                        if(ball.angle < 0)
                        {
                            ball.ver = UP;
                            ball.angle = -ball.angle;
                        }
                        else
                        {
                            ball.ver = DOWN;
                        }
                        ball.hor = LEFT;
                    }
                }
                ball.x += static_cast<float>(ball.hor) * (ball.velocity * std::cos(ball.angle) + ball.acceleration) * dT;
                ball.y += static_cast<float>(ball.ver) * (ball.velocity * std::sin(ball.angle) + ball.acceleration) * dT;
            }
        }
        ball.wait -= 1 * dT;
        game_wait -= 1 * dT;

        if(p1_score == MAX_SCORE || p2_score == MAX_SCORE)
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
        /* logic */

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
            case 11: draw_1(0.675f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); draw_1(0.775f*RENDER_WIDTH, 0.0667f*RENDER_HEIGHT); break;
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
        SDL_Delay(static_cast<Uint32>(((1.0f / static_cast<float>(config.at("MAX_FPS"))) * 1000.0f) - frame_time));
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
