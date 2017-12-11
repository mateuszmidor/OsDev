/**
 *   @file: arkanoid.cpp
 *
 *   @date: Dec 11, 2017
 * @author: Mateusz Midor
 */

#include <memory>
#include "_start.h"
#include "syscalls.h"
#include "Cout.h"
#include "Mouse.h"
#include "Vga.h"
#include "Monitor.h"


using namespace ustd;
using namespace middlespace;

constexpr u16 PADDLE_WIDTH = 100;
constexpr u16 PADDLE_HEIGHT = 5;

struct Paddle {
    u16 x;
    u16 y;
};

struct Ball {
    u16 x;
    u16 y;
    s16 dx = 1;
    s16 dy = -1;
    u16 radius = 4;
};

Ball ball;
Monitor<Paddle> paddle;
u16 vga_width, vga_height;
EgaColor* vga_buffer;

s16 clamp(s16 v, s16 min, s16 max) {
    if (v < min) v = min;
    if (v > max) v = max;
    return v;
}

void vga_set_pixel_at(s16 x, s16 y, EgaColor c) {
    if (x < 0 || y < 0 || x >= vga_width || y >= vga_height)
        return;

    vga_buffer[x + y * vga_width] = c;
}

void game_over() {
    syscalls::msleep(2000);
    cout::format("GAME OVER\n");
    syscalls::vga_exit_graphics_mode();
    syscalls::exit_group(0);
}

void simulate_ball() {
    ball.x += ball.dx;
    ball.y += ball.dy;

    if (ball.x <= 0 || ball.x >= vga_width)
        ball.dx *= -1;

    if (ball.y <= 0)
        ball.dy *= -1;

    auto p = paddle.get();
    if (ball.y >= p->y && ball.y < p->y + PADDLE_HEIGHT) {
        if (ball.x >= p->x && ball.x <= p->x + PADDLE_WIDTH)
            ball.dy *= -1;
    }

    if (ball.y >= vga_height)
        game_over();
}

void draw_background() {
    for (int x = 0; x < vga_width; x++)
        for (int y = 0; y < vga_height; y++) {
            vga_set_pixel_at(x, y, EgaColor::Black);
        }

}

void draw_paddle() {
    auto px = paddle.get()->x;
    auto py = paddle.get()->y;

    for (int y = 0; y < PADDLE_HEIGHT; y++)
        for (int x = 0; x < PADDLE_WIDTH; x++)
            vga_set_pixel_at(x + px, y + py, EgaColor::Green);

}

void draw_ball() {
    for (int x = -ball.radius; x <= ball.radius; x++)
        for (int y = -ball.radius; y <= ball.radius; y++)
            if (x*x + y*y < ball.radius * ball.radius)
                vga_set_pixel_at(ball.x + x, ball.y + y, EgaColor::Yellow);
}

void mouse_input_thread() {
    int fd = syscalls::open("/dev/mouse", 2);
    if (fd < 0) {
        cout::print("arkanoid: cant open /dev/mouse\n");
        syscalls::exit(1);
    }

    MouseState ms;
    while (syscalls::read(fd, &ms, sizeof(MouseState)) == sizeof(MouseState)) {
        auto gms = paddle.get();
        gms->x = clamp(gms->x + ms.dx * 2, 0, vga_width - 1 - PADDLE_WIDTH);
    }

    cout::print("arkanoid: mouse state read error.\n");
    syscalls::close(fd);
}

void game_loop() {
    while (true) {
        simulate_ball();
        draw_background();
        draw_paddle();
        draw_ball();
        syscalls::vga_flush_video_buffer((const unsigned char*)vga_buffer);
    }
}

/**
 * @brief   Entry point
 * @return  0 on success
 */
int main(int argc, char* argv[]) {
    syscalls::vga_enter_graphics_mode();
    syscalls::vga_get_width_height(&vga_width, &vga_height);
    vga_buffer = new EgaColor[vga_width * vga_height];
    paddle.reset(std::make_shared<Paddle>());
    paddle.get()->x = vga_width / 2 - PADDLE_WIDTH / 2;
    paddle.get()->y = vga_height - 20;
    ball.x = vga_width / 2;
    ball.y = vga_height - 25;

    // run mouse input thread
    syscalls::task_lightweight_run((unsigned long long)mouse_input_thread, 0, "arkanoid_mouse_input");
    game_loop();

    delete[] vga_buffer;
    syscalls::vga_exit_graphics_mode();

    cout::print("Arkanoid done.\n");

    return 0;
}
