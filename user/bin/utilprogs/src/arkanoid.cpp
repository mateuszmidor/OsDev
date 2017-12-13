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

/**
 * MATH PART
 */
double abs(double d) {
    return d >= 0.0 ? d : -d;
}

double min(double a, double b) {
    return a < b ? a : b;
}

s16 clamp(s16 v, s16 min, s16 max) {
    if (v < min) v = min;
    if (v > max) v = max;
    return v;
}

struct Vector2D {
    Vector2D operator+(const Vector2D& other) const { return {x + other.x, y + other.y }; }
    Vector2D operator-(const Vector2D& other) const { return {x - other.x, y - other.y }; }
    Vector2D& operator+=(const Vector2D& other) { *this = *this + other; return *this; }
    Vector2D operator*(double scalar) const { return {x * scalar, y * scalar}; }
    double operator*(const Vector2D& other) { return x * other.x + y * other.y; }   // DOT product
    double operator^(const Vector2D& other) { return x * other.y - y * other.x; }  // CROSS product
    double  x, y;
};

/**
 * COLLISION PART
 */
struct Ray2D {
    double intersects_line(const Vector2D& point1, const Vector2D& point2) const {
        constexpr double INFINITE   {1000000.0};
        Vector2D v1 = pos - point1;
        Vector2D v2 = point2 - point1;
        Vector2D v3 = {-dir.y * length, dir.x * length};


        double dot = v2 * v3;
        if (abs(dot) < 0.000001)
            return INFINITE;

        double t1 = (v2 ^ v1) / dot;
        double t2 = (v1 * v3) / dot;

        if (t1 >= 0.0 && t2 >= 0.0 && t2 <= 1.0)
            return t1;

        return INFINITE;
    }

    Vector2D    pos;
    Vector2D    dir;
    double      length;
};

class AABB {
public:
    AABB(u16 x, u16 y, u16 width, u16 height, bool is_inverted = false) : x(x), y(y), w(width), h(height), is_inverted_volume(is_inverted) {}

    bool check_collision(const Ray2D& ray, Vector2D& new_dir) {
        Vector2D np = ray.pos + ray.dir * ray.length;    // nex position

        if (point_collides_volume(np)) {
            double up_down_edge_collision_distance = min(ray.intersects_line({x, y}, {x+w, y}), ray.intersects_line({x, y+h}, {x+w, y+h}));
            double left_right_edge_collision_distance = min(ray.intersects_line({x, y}, {x, y+h}), ray.intersects_line({x+w, y}, {x+w, y+h}));
            if (up_down_edge_collision_distance < left_right_edge_collision_distance)
                new_dir = { ray.dir.x, -ray.dir.y };    // mirror Y
            else
                new_dir = { -ray.dir.x, ray.dir.y };    // mirror X

            return true;
        }
        else
            return false;
    }

private:
    bool point_collides_volume(const Vector2D& p) {
        bool is_inside = (p.x >= x && p.x <= (x+w) && p.y >= y && p.y <= (y+h));
        return is_inside ^ is_inverted_volume;
    }

    double x, y, w, h;
    bool is_inverted_volume;    // false = cant enter the box, true = cant escape the box
};

/**
 *  GAME LOGIC PART
 */
struct Paddle {
    bool check_collision(const Ray2D& ray, Vector2D& new_dir) {
        AABB bb(x, y, PADDLE_WIDTH, PADDLE_HEIGHT);
        return bb.check_collision(ray, new_dir);
    }

    constexpr static u16 PADDLE_WIDTH = 100;
    constexpr static u16 PADDLE_HEIGHT = 5;
    u16 x;
    u16 y;
};

struct Ball {
    Vector2D    pos {0.0, 0.0};
    Vector2D    dir {-0.7071067811865475, -0.7071067811865475};  // normalized value of (-1, -1)
    double      speed {50.0};   // pixels/sec
    int         radius  = 4;
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
    static constexpr EgaColor NO_BRICK {EgaColor::Black};
    static constexpr u8 NUM_ROWS        {10};
    static constexpr u8 NUM_COLS        {16};
    using BrickRow = std::array<EgaColor, NUM_COLS>;
    using BoardConstructor = std::function<EgaColor(u16 x, u16 y)>;   // take brick position x & y and return brick color

public:
    void init(u16 width, u16 height, const BoardConstructor& constructor) {
        board_bb = {0, 0, width, height, true};
        brick_width = width / NUM_COLS;         // bricks take entire board width
        brick_height = height / 2 / NUM_ROWS;   // and half the board height
        for (u16 y = 0; y < bricks.size(); y++)
            for (u16 x = 0; x < bricks[y].size(); x++)
                bricks[y][x] = constructor(x, y);
    }

    void draw(Vga& vga) {
        for (u16 y = 0; y < NUM_ROWS; y++)
            for (u16 x = 0; x < NUM_COLS; x++)
                if (bricks[y][x] != NO_BRICK)
                    draw_brick(x * brick_width, y * brick_height, brick_width, brick_height, bricks[y][x], vga);
    }

    bool check_collision(const Ray2D& ray, Vector2D& new_dir) {
        // collision against board walls
        if (board_bb.check_collision(ray, new_dir)) {
            return true;
        }

        // collision against bricks on the board
        for (s16 y = NUM_ROWS-1; y >= 0; y--)
            for (u16 x = 0; x < NUM_COLS; x++)  {
                EgaColor color = bricks[y][x];
                if (color == EgaColor::Black)
                    continue;

                AABB brick_bb(x * brick_width, y * brick_height, brick_width, brick_height);
                if (brick_bb.check_collision(ray, new_dir)) {
                    bricks[y][x] = NO_BRICK;
                    return true;
                }
            }
        return false;
    }

private:
    u16 board_width;
    u16 board_height;
    u16 brick_width;
    u16 brick_height;
    std::array<BrickRow, NUM_ROWS> bricks;
    AABB board_bb   {0, 0, 320, 200, true};

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

    void run() {
        paddle.get()->x = vga.width / 2 - Paddle::PADDLE_WIDTH / 2;
        paddle.get()->y = vga.height - 20;
        ball.pos = { vga.width / 2.0, vga.height - 25.0 };
       // auto chessboard_board_constructor = [](u16 x, u16 y)  { return EgaColor(((x + y) % 2) * EgaColor::LightGreen); };
        auto columns_board_constructor = [](u16 x, u16 y)  { return EgaColor(((x % 2) == 0) * EgaColor::LightCyan); };

        board.init(vga.width, vga.height, columns_board_constructor);
        killer_bottom_edge = {0, vga.height, vga.width, 100};
        terminated = false;
        game_loop();
    }


private:
    bool terminated = false;
    Monitor<Paddle> paddle;
    Vga vga;
    Ball ball;
    Board board;
    Timer timer;
    AABB killer_bottom_edge {0, 0, 0, 0, false};

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

    void simulate_game(double delta_time) {
        Ray2D ray {ball.pos, ball.dir, ball.speed * delta_time + ball.radius};
        Vector2D new_dir {};

        if (paddle.get()->check_collision(ray, new_dir))
            ball.dir = new_dir;

        if (board.check_collision(ray, new_dir))
            ball.dir = new_dir;

        if (killer_bottom_edge.check_collision(ray, new_dir)) {
            syscalls::msleep(2000);
            cout::format("GAME OVER\n");
            terminated = true;
        }

        ball.pos += ball.dir * ball.speed * delta_time;
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
                    vga.set_pixel_at(ball.pos.x + x, ball.pos.y + y, EgaColor::Yellow);
    }

    void game_loop() {
        while (!terminated) {
            simulate_game(timer.get_delta_seconds());
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
