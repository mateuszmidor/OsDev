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
 * VIDEO PART
 */
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

    void draw_rect(u16 x, u16 y, u16 width, u16 height, EgaColor c) {
        for (int i = 0; i < width; i++)
            for (int j = 0; j < height; j++)
                set_pixel_at(x + i, y + j, c);
    }

    void draw_circle(u16 x, u16 y, s16 radius, EgaColor c) {
        for (int i = -radius; i <= radius; i++)
            for (int j = -radius; j <= radius; j++)
                if (i*i + j*j < radius * radius)
                    set_pixel_at(x + i, y + j, c);
    }

    void flush_to_screen() {
        syscalls::vga_flush_video_buffer((const unsigned char*)vga_buffer.get());
    }

    u16 width, height;

private:
    std::unique_ptr<EgaColor[]> vga_buffer;
};

/**
 * MATH PART
 */
double abs(double d) {
    return (d >= 0.0) ? d : -d;
}

double min(double a, double b) {
    return (a < b) ? a : b;
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
    double operator*(const Vector2D& other) { return x * other.x + y * other.y; }  // DOT product
    double operator^(const Vector2D& other) { return x * other.y - y * other.x; }  // CROSS product
    double  x, y;
};

/**
 * COLLISION PART
 */
class Ray2D {
public:
    Ray2D(const Vector2D& pos, const Vector2D& dir, double length) : pos(pos), dir(dir), length(length) {}

    double dist_to_line(const Vector2D& point1, const Vector2D& point2) const {
        constexpr double INFINITE   {1000000.0};
        Vector2D v1 = pos - point1;
        Vector2D v2 = point2 - point1;
        Vector2D v3 = {-dir.y * length, dir.x * length};


        double dot = v2 * v3;
        if (abs(dot) < 0.000001)    // ray and line parallel
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

    bool check_collision(const Ray2D& ray, Vector2D& new_dir) const {
        Vector2D np = ray.pos + ray.dir * ray.length;    // next position

        // quick check if next position is within bounding volume
        if (!point_collides_volume(np))
            return false;

        double up_down_edge_collision_distance = min(ray.dist_to_line({x, y}, {x+w, y}), ray.dist_to_line({x, y+h}, {x+w, y+h}));
        double left_right_edge_collision_distance = min(ray.dist_to_line({x, y}, {x, y+h}), ray.dist_to_line({x+w, y}, {x+w, y+h}));
        if (up_down_edge_collision_distance < left_right_edge_collision_distance)
            new_dir = { ray.dir.x, -ray.dir.y };    // mirror Y
        else
            new_dir = { -ray.dir.x, ray.dir.y };    // mirror X

        return true;
    }

    double x, y, w, h;

private:
    bool point_collides_volume(const Vector2D& p) const {
        bool is_inside = (p.x >= x && p.x <= (x+w) && p.y >= y && p.y <= (y+h));
        return is_inside ^ is_inverted_volume;
    }

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

    void draw(Vga& vga) {
        vga.draw_rect(x, y, Paddle::PADDLE_WIDTH, Paddle::PADDLE_HEIGHT, EgaColor::Green);
    }

    constexpr static u16 PADDLE_WIDTH = 100;
    constexpr static u16 PADDLE_HEIGHT = 5;
    u16 x;
    u16 y;
};

struct Ball {
    void draw(Vga& vga) {
        vga.draw_circle(pos.x, pos.y, radius, EgaColor::Yellow);
    }

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

class Board {
    static constexpr EgaColor NO_BRICK  {EgaColor::Black};
    static constexpr u8 NUM_ROWS        {10};
    static constexpr u8 NUM_COLS        {16};
    const u16 brick_width;
    const u16 brick_height;
    const AABB board_bb;
    using BrickRow = std::array<EgaColor, NUM_COLS>;
    using BoardConstructor = std::function<EgaColor(u16 x, u16 y)>;   // take brick position x & y and return brick color

public:
    Board(u16 width, u16 height) : brick_width(width / NUM_COLS), brick_height(height / 2 / NUM_ROWS), board_bb(0, 0, width, height, true) {}

    void init(const BoardConstructor& constructor) {
        for (u16 y = 0; y < bricks.size(); y++)
            for (u16 x = 0; x < bricks[y].size(); x++)
                bricks[y][x] = constructor(x, y);
    }

    void draw(Vga& vga) {
        // draw background
        vga.draw_rect(0, 0, board_bb.w, board_bb.h, EgaColor::Black);

        // draw bricks
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
    std::array<BrickRow, NUM_ROWS> bricks;

    void draw_brick(u16 bx, u16 by, u16 brick_width, u16 brick_height, EgaColor color, Vga& vga) {
        vga.draw_rect(bx, by, brick_width, brick_height, color);
    }
};

class Game {
public:
    Game(Vga& vga) : killer_bottom_edge(0, vga.height, vga.width, 50.0), vga(vga), board(vga.width, vga.height) {
        paddle.reset(std::make_shared<Paddle>());
        syscalls::task_lightweight_run((unsigned long long)mouse_input_thread, (u64)this, "arkanoid_mouse_input");
    }

    void run() {
        paddle.get()->x = vga.width / 2 - Paddle::PADDLE_WIDTH / 2;
        paddle.get()->y = vga.height - 20;
        ball.pos = { vga.width / 2.0, vga.height - 25.0 };
       // auto chessboard_board_constructor = [](u16 x, u16 y)  { return EgaColor(((x + y) % 2) * EgaColor::LightGreen); };
        auto columns_board_constructor = [](u16 x, u16 y)  { return EgaColor(((x % 2) == 0) * EgaColor::LightCyan); };

        board.init(columns_board_constructor);
        terminated = false;
        game_loop();
    }


private:
    const AABB killer_bottom_edge;
    bool terminated = false;
    Monitor<Paddle> paddle;
    Vga& vga;
    Ball ball;
    Board board;
    Timer timer;

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

    void game_loop() {
        while (!terminated) {
            simulate_game(timer.get_delta_seconds());
            board.draw(vga);
            paddle.get()->draw(vga);
            ball.draw(vga);
            vga.flush_to_screen();
        }
    }
};

/**
 * @brief   Entry point
 * @return  0 on success
 */
int main(int argc, char* argv[]) {
    Vga vga;
    Game game(vga);
    game.run();
    return 0;
}
