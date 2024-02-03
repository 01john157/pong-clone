#include <SDL2/sdl.h>
#include <iostream>
#include <random>

struct Ball
{
    SDL_FRect ball_rect;
    bool up;
    bool right;
};

int main(int argc, char* args[])
{
    int internal_width = 100;
    int internal_height = 75;

    float paddle_width = 2;
    float paddle_height = 15;
    
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

    // Window
    SDL_Window* window = SDL_CreateWindow("pong-clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    // Renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
    SDL_RenderSetLogicalSize(renderer, internal_width, internal_height);

    float fps_cap = (1.0f / 120.0f) * 1000.0f;

    SDL_Event event;

    Uint64 previous_time = SDL_GetPerformanceCounter();

    std::mt19937 mt{};
    std::uniform_int_distribution range{0, internal_height};
    
    SDL_FRect ball_rect;
    ball_rect.x = static_cast<float>(internal_width / 2);
    ball_rect.y = static_cast<float>(internal_height / 2);
    ball_rect.w = 2;
    ball_rect.h = 2;
    Ball ball = {ball_rect, false, true}; 

    SDL_FRect p1;
    p1.x = paddle_width;
    p1.y = static_cast<float>((internal_height / 2)) - static_cast<float>(paddle_height / 2);
    p1.w = paddle_width;
    p1.h = paddle_height;

    SDL_FRect p2;
    p2.x = 98 - paddle_width;
    p2.y = static_cast<float>((internal_height / 2)) - static_cast<float>(paddle_height / 2);
    p2.w = paddle_width;
    p2.h = paddle_height;


    // Loop
    bool running = true;
    while(running)
    {
        Uint64 start_time = SDL_GetPerformanceCounter();

        bool p1_up = false;
        bool p1_down = false;
        bool p2_up = false;
        bool p2_down = false;


        // Event Handling
        const Uint8* keys = SDL_GetKeyboardState(NULL);

        while(SDL_PollEvent(&event) != 0){
            if(event.type == SDL_QUIT)
            {
                running = false;
            }
            else if(event.type == SDL_KEYDOWN)
            {
                if(keys[SDL_SCANCODE_W])
                {
                    p1_up = true;
                }
                if(keys[SDL_SCANCODE_S])
                {
                    p1_down = true;
                }
                if(keys[SDL_SCANCODE_UP])
                {
                    p2_up = true;
                }
                if(keys[SDL_SCANCODE_DOWN])
                {
                    p2_down = true;
                }
            }
        }
        //


        // Logic
        Uint64 current_time = SDL_GetPerformanceCounter();
        float dT = static_cast<float>(current_time - previous_time) / static_cast<float>(SDL_GetPerformanceFrequency());

        // p1
        if(p1_up)
        {
            if(p1.y > 0)
            {
                p1.y -= 400 * dT;
            } 
        }
        if(p1_down)
        {
            if((p1.y + p1.h) < static_cast<float>(internal_height))
            {
                p1.y += 400 * dT;
            }
        }

        // p2
        if(p2_up)
        {
            if(p2.y > 0)
            {
                p2.y -= 400 * dT;
            }
        }
        if(p2_down)
        {
            if((p2.y + p2.h) < static_cast<float>(internal_height))
            {
                p2.y += 400 * dT;
            }
        }

        // if ball goes off screen, reset position and swap directions
        if(ball.ball_rect.x < 0)
        {
            ball.ball_rect.x = static_cast<float>(internal_width / 2);
            ball.ball_rect.y = static_cast<float>(range(mt));
            ball.right = true;
        }
        else if((ball.ball_rect.x + ball.ball_rect.w) > static_cast<float>(internal_width))
        {
            ball.ball_rect.x = static_cast<float>(internal_width / 2);
            ball.ball_rect.y = static_cast<float>(range(mt));
            ball.right = false;
        }

        // if ball touches paddle, reverse direction
        if(ball.ball_rect.x < p1.x && (ball.ball_rect.y >= p1.y && ball.ball_rect.y <= (p1.y + p1.h)))
        {
            ball.right = true;
        }
        else if((ball.ball_rect.x + ball.ball_rect.w) > p2.x && (ball.ball_rect.y >= p2.y && ball.ball_rect.y <= (p2.y + p2.h)))
        {
            ball.right = false;
        }

        // if ball touches top of screen, reverse direction
        if((ball.ball_rect.y + ball.ball_rect.h) > static_cast<float>(internal_height))
        {
            ball.up = true;
        }
        else if(ball.ball_rect.y < 0){
            ball.up = false;
        }

        // move ball
        if(ball.right)
        {
            ball.ball_rect.x += 50 * dT;
        }
        else{
            ball.ball_rect.x -= 50 * dT;
        }
        if(ball.up)
        {
            ball.ball_rect.y -= 50 * dT;
        }
        else{
            ball.ball_rect.y += 50 * dT;
        }

        previous_time = current_time;
        //


        // Rendering
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRectF(renderer, &p1);
        SDL_RenderDrawRectF(renderer, &p2);
        SDL_RenderDrawRectF(renderer, &ball.ball_rect);

        SDL_RenderPresent(renderer);
        //
        

        Uint64 end_time = SDL_GetPerformanceCounter();
        float elapsed = static_cast<float>(end_time - start_time) / static_cast<float>(SDL_GetPerformanceFrequency());

        SDL_Delay(static_cast<Uint32>(fps_cap - elapsed));
    }

    SDL_Quit();
    
    return 0;
}