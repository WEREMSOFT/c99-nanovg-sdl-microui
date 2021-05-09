#include <stdio.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"
#include "microui.h"
#include "renderer.h"
#include <assert.h>
#include "atlas.inl"

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_GLContext *context;
} Program;

static int text_width(mu_Font font, const char *text, int len)
{
    if (len == -1)
    {
        len = strlen(text);
    }
    return r_get_text_width(text, len);
}

static int text_height(mu_Font font)
{
    return r_get_text_height();
}

static const char button_map[256] = {
    [SDL_BUTTON_LEFT & 0xff] = MU_MOUSE_LEFT,
    [SDL_BUTTON_RIGHT & 0xff] = MU_MOUSE_RIGHT,
    [SDL_BUTTON_MIDDLE & 0xff] = MU_MOUSE_MIDDLE,
};

static const char key_map[256] = {
    [SDLK_LSHIFT & 0xff] = MU_KEY_SHIFT,
    [SDLK_RSHIFT & 0xff] = MU_KEY_SHIFT,
    [SDLK_LCTRL & 0xff] = MU_KEY_CTRL,
    [SDLK_RCTRL & 0xff] = MU_KEY_CTRL,
    [SDLK_LALT & 0xff] = MU_KEY_ALT,
    [SDLK_RALT & 0xff] = MU_KEY_ALT,
    [SDLK_RETURN & 0xff] = MU_KEY_RETURN,
    [SDLK_BACKSPACE & 0xff] = MU_KEY_BACKSPACE,
};

int main(void)
{
    Program this = {0};

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("Couldn't initialize SDL2: %s\n", SDL_GetError());
        return -1;
    }

    this.window = SDL_CreateWindow("Hello Window", 0, 0, 800, 600, SDL_WINDOW_OPENGL);

    if (this.window == NULL)
    {
        printf("Couldn't create window: %s\n", SDL_GetError());
        return -1;
    }

    this.context = SDL_GL_CreateContext(this.window);
    if (this.context == NULL)
    {
        printf("Couldn't create gl context: %s\n", SDL_GetError());
        return -1;
    }

    glewInit();

    SDL_Event event;
    bool shouldCloseWindow = false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    struct NVGcontext *vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

    int x = 400;

    /* init gl */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    /* init texture */
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, ATLAS_WIDTH, ATLAS_HEIGHT, 0,
                 GL_ALPHA, GL_UNSIGNED_BYTE, atlas_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    assert(glGetError() == 0);

    mu_Context *ctx = malloc(sizeof(mu_Context));
    mu_init(ctx);
    ctx->text_width = text_width;
    ctx->text_height = text_height;

    while (!shouldCloseWindow)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                shouldCloseWindow = true;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    shouldCloseWindow = true;
                    break;
                case SDLK_LEFT:
                    x--;
                    break;
                case SDLK_RIGHT:
                    x++;
                    break;
                }
                break;
            case SDL_MOUSEMOTION:
                mu_input_mousemove(ctx, event.motion.x, event.motion.y);
                break;
            case SDL_MOUSEWHEEL:
                mu_input_scroll(ctx, 0, event.wheel.y * -30);
                break;
            case SDL_TEXTINPUT:
                mu_input_text(ctx, event.text.text);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                int b = button_map[event.button.button & 0xff];
                if (b && event.type == SDL_MOUSEBUTTONDOWN)
                {
                    mu_input_mousedown(ctx, event.button.x, event.button.y, b);
                }
                if (b && event.type == SDL_MOUSEBUTTONUP)
                {
                    mu_input_mouseup(ctx, event.button.x, event.button.y, b);
                }
                break;
            }
            }
        }
        glEnable(GL_STENCIL_TEST);
        glClearColor(1.f, 0, 0, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // nvgBeginFrame(vg, 800, 600, 1);
        // nvgBeginPath(vg);
        // nvgRect(vg, x, 100, 120, 30);
        // nvgFillColor(vg, nvgRGBA(255, 192, 0, 255));
        // nvgFill(vg);
        // nvgEndFrame(vg);

        mu_begin(ctx);
        if (mu_begin_window(ctx, "My Window", mu_rect(10, 10, 140, 86)))
        {
            mu_layout_row(ctx, 2, (int[]){60, -1}, 0);

            mu_label(ctx, "First:");
            if (mu_button(ctx, "Button1"))
            {
                printf("Button1 pressed\n");
            }

            mu_label(ctx, "Second:");
            if (mu_button(ctx, "Button2"))
            {
                mu_open_popup(ctx, "My Popup");
            }

            if (mu_begin_popup(ctx, "My Popup"))
            {
                mu_label(ctx, "Hello world!");
                mu_end_popup(ctx);
            }

            mu_end_window(ctx);
        }
        mu_end(ctx);

        mu_Command *cmd = NULL;
        while (mu_next_command(ctx, &cmd))
        {
            switch (cmd->type)
            {
            case MU_COMMAND_TEXT:
                r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color);
                break;
            case MU_COMMAND_RECT:
                r_draw_rect(cmd->rect.rect, cmd->rect.color);
                break;
            case MU_COMMAND_ICON:
                r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color);
                break;
            case MU_COMMAND_CLIP:
                r_set_clip_rect(cmd->clip.rect);
                break;
            }
        }

        r_present();
        SDL_GL_SwapWindow(this.window);
    }

    SDL_DestroyWindow(this.window);
    SDL_DestroyRenderer(this.renderer);

    return 0;
}