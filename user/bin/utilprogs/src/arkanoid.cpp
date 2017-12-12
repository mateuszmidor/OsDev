/**
 *   @file: arkanoid.cpp
 *
 *   @date: Dec 11, 2017
 * @author: Mateusz Midor
 */

#include <memory>
#include <array>
#include <functional>
#include "_start.h"
#include "syscalls.h"
#include "Cout.h"
#include "Mouse.h"
#include "Vga.h"
#include "Monitor.h"



using namespace ustd;
using namespace middlespace;

struct Vector2D {
    double x, y;
};
struct AABB {
    AABB(u16 x, u16 y, u16 width, u16 height) : x(x), y(y), w(width), h(height) {}
    bool check_collision(const Vector2D& pos, const Vector2D& dir, Vector2D& new_dir) {
        return false;
    }

    u16 x, y, w, h;
};

struct Paddle {
    constexpr static u16 PADDLE_WIDTH = 100;
    constexpr static u16 PADDLE_HEIGHT = 5;
    u16 x;
    u16 y;
};

struct Ball {
    double  x;
    double  y;
    double  dx      = -50.0;    // pixels/sec
    double  dy      = -50.0;    // pixels/sec
    int     radius  = 4;
};

class Timer {
public:
    Timer() {
        syscalls::clock_gettime(CLOCK_MONOTONIC, &last_time);
    }

    double get_delta_seconds() {
        constexpr double NSEC = 1000*1000*1000;
        timespec ts;
        syscalls::clock_gettime(CLOCK_MONOTONIC, &ts);
        double dt = (ts.tv_sec - last_time.tv_sec) + (ts.tv_nsec - last_time.tv_nsec) / NSEC;
        last_time = ts;
        return dt;
    }

private:
    timespec    last_time;
};

class Vga {
public:
    Vga() {
        syscalls::vga_enter_graphics_mode();
        syscalls::vga_get_width_height(&width, &height);
        vga_buffer.reset(new EgaColor[width * height]);
    }

    ~Vga() {
        syscalls::vga_exit_graphics_mode();
    }

    void set_pixel_at(s16 x, s16 y, EgaColor c) {
        if (x < 0 || y < 0 || x >= width || y >= height)
            return;

        vga_buffer[x + y * width] = c;
    }

    void flush_to_screen() {
        syscalls::vga_flush_video_buffer((const unsigned char*)vga_buffer.get());
    }

    u16 width, height;

private:
    std::unique_ptr<EgaColor[]> vga_buffer;
};

class Board {
    static constexpr u8 NUM_ROWS    {10};
    static constexpr u8 NUM_COLS    {16};
    static constexpr u16 BRICK_WIDTH = 320 / NUM_COLS;
    static constexpr u16 BRICK_HEIGHT = 200 / 2/ NUM_ROWS;

public:
    using BrickRow = std::array<u8, NUM_COLS>;
    using BoardConstructor = std::function<u8(u16 x, u16 y)>;
    void init(const BoardConstructor& constructor) {
        for (u16 y = 0; y < bricks.size(); y++)
            for (u16 x = 0; x < bricks[y].size(); x++)
                bricks[y][x] = constructor(x, y);
    }

    void draw(Vga& vga) {


        for (u16 y = 0; y < NUM_ROWS; y++)
            for (u16 x = 0; x < NUM_COLS; x++)  {
                auto color = bricks.at(y).at(x);
                if (color == 0)
                    continue;

                draw_brick(x * BRICK_WIDTH, y * BRICK_HEIGHT, BRICK_WIDTH, BRICK_HEIGHT, (EgaColor)color, vga);
            }
    }

    void check_collision(Ball& b) {
        for (s16 y = NUM_ROWS-1; y >= 0; y--)
            for (u16 x = 0; x < NUM_COLS; x++)  {
                auto color = bricks.at(y).at(x);
                if (color == 0)
                    continue;

                if (b.x >= x * BRICK_WIDTH && b.x <= (x+1) * BRICK_WIDTH && b.y >= y * BRICK_HEIGHT && b.y <= (y+1) * BRICK_HEIGHT) {
                    bricks.at(y).at(x) = 0;
                    b.dy *= -1.0;
                    return;
                }
            }
    }

private:
    std::array<BrickRow, NUM_ROWS> bricks;

    void draw_brick(u16 bx, u16 by, u16 brick_width, u16 brick_height, EgaColor color, Vga& vga) {
        for (u16 x = 0; x < brick_width; x++)
            for (u16 y = 0; y < brick_height; y++)
                vga.set_pixel_at(bx + x, by + y, color);
    }
};

class Game {
public:
    Game() {
        paddle.reset(std::make_shared<Paddle>());
        syscalls::task_lightweight_run((unsigned long long)mouse_input_thread, (u64)this, "arkanoid_mouse_input");
    }

    ~Game() {
    }

    void run() {
        paddle.get()->x = vga.width / 2 - Paddle::PADDLE_WIDTH / 2;
        paddle.get()->y = vga.height - 20;
        ball.x = vga.width / 2;
        ball.y = vga.height - 25;
        auto board_constructor = [](u16 x, u16 y) { return ((x + y) % 2) * EgaColor::LightGreen; };
        board.init(board_constructor);
        terminated = false;
        game_loop();
    }


private:
    bool terminated = false;
    Vga vga;
    Monitor<Paddle> paddle;
    Timer timer;
    Ball ball;
    Board board;

    static void mouse_input_thread(u64 arg) {
        Game* game = (Game*)arg;
        int fd = syscalls::open("/dev/mouse", 2);
        if (fd < 0) {
            cout::print("arkanoid: cant open /dev/mouse\n");
            game->terminated = true;
            syscalls::exit(1);
        }

        MouseState ms;
        while (syscalls::read(fd, &ms, sizeof(MouseState)) == sizeof(MouseState)) {
            auto gms = game->paddle.get();
            gms->x = clamp(gms->x + ms.dx * 2, 0, game->vga.width - 1 - Paddle::PADDLE_WIDTH);
        }

        cout::print("arkanoid: mouse state read error.\n");
        syscalls::close(fd);
        game->terminated = true;
        syscalls::exit(1);
    }

    static s16 clamp(s16 v, s16 min, s16 max) {
        if (v < min) v = min;
        if (v > max) v = max;
        return v;
    }

    bool simulate_game(double delta_time) {
        ball.x += ball.dx * delta_time;
        ball.y += ball.dy * delta_time;

        board.check_collision(ball);

        if (ball.x <= 0 || ball.x >= vga.width) {
            ball.x = clamp(ball.x, 0, vga.width - 1);
            ball.dx *= -1;
        }

        if (ball.y <= 0) {
            ball.y = clamp(ball.y, 0, vga.height - 1);
            ball.dy *= -1.0;
        }

        auto p = paddle.get();
        if (ball.y >= p->y){ // && ball.y < p->y + Paddle::PADDLE_HEIGHT) {
            if (ball.x >= p->x && ball.x <= p->x + Paddle::PADDLE_WIDTH)
                ball.dy *= -1.0;
        }

        if (ball.y >= vga.height) {
            syscalls::msleep(2000);
            cout::format("GAME OVER\n");
            return false;
        }
        return true;
    }

    void draw_background() {
        for (int x = 0; x < vga.width; x++)
            for (int y = 0; y < vga.height; y++)
                vga.set_pixel_at(x, y, EgaColor::Black);
    }

    void draw_paddle() {
        auto px = paddle.get()->x;
        auto py = paddle.get()->y;

        for (int y = 0; y < Paddle::PADDLE_HEIGHT; y++)
            for (int x = 0; x < Paddle::PADDLE_WIDTH; x++)
                vga.set_pixel_at(x + px, y + py, EgaColor::Green);

    }

    void draw_ball() {
        for (int x = -ball.radius; x <= ball.radius; x++)
            for (int y = -ball.radius; y <= ball.radius; y++)
                if (x*x + y*y < ball.radius * ball.radius - 1)
                    vga.set_pixel_at(ball.x + x, ball.y + y, EgaColor::Yellow);
    }

    void game_loop() {
        while (!terminated && simulate_game(timer.get_delta_seconds())) {
            draw_background();
            board.draw(vga);
            draw_paddle();
            draw_ball();
            vga.flush_to_screen();
        }
    }
};



/**
 * @brief   Entry point
 * @return  0 on success
 */
int main(int argc, char* argv[]) {
    Game game;
    game.run();
    return 0;
}
