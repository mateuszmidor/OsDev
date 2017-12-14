/**
 *   @file: arkanoid.cpp
 *
 *   @date: Dec 11, 2017
 * @author: Mateusz Midor
 */

#include <memory>
#include <array>
#include <utility>
#include <functional>
#include "_start.h"
#include "syscalls.h"
#include "Cout.h"
#include "Mouse.h"
#include "Vga.h"
#include "Monitor.h"
#include "StringUtils.h"


using namespace ustd;
using namespace middlespace;

/**
 * UTILS PART
 */
template <class T>
class Optional {
public:
    Optional() : invalid(true) {}
    Optional(const T& value) : value(value), invalid(false) {}
    Optional(T&& value) : value(std::move(value)), invalid(false) {}
    operator bool() const { return !invalid; }
    bool operator!() const { return invalid; }
    T value {};

private:
    bool invalid;
};

/**
 * VIDEO PART
 */
extern unsigned char g_8x8_font[];
class Vga {
public:
    Vga() {
        syscalls::vga_enter_graphics_mode();
        syscalls::vga_get_width_height(&width, &height);
        vga_buffer.resize(width * height);
    }

    ~Vga() {
        syscalls::vga_exit_graphics_mode();
    }

    void set_pixel_at(s16 x, s16 y, EgaColor64 c) {
        if (x < 0 || y < 0 || x >= width || y >= height)
            return;

        vga_buffer[x + y * width] = c;
    }

    void draw_rect(u16 x, u16 y, u16 width, u16 height, EgaColor64 c) {
        for (u16 i = 0; i < width; i++)
            for (u16 j = 0; j < height; j++)
                set_pixel_at(x + i, y + j, c);
    }

    void draw_rect(u16 x, u16 y, u16 width, u16 height, EgaColor64 frame, EgaColor64 body) {
        for (u16 i = 0; i < width; i++)
            for (u16 j = 0; j < height; j++)
                if (i == 0 || j == 0 || i == width-1 || j == height - 1)
                    set_pixel_at(x + i, y + j, frame);
                else
                    set_pixel_at(x + i, y + j, body);
    }

    void draw_circle(u16 x, u16 y, s16 radius, EgaColor64 c) {
        for (s16 i = -radius; i <= radius; i++)
            for (s16 j = -radius; j <= radius; j++)
                if (i*i + j*j < radius * radius)
                    set_pixel_at(x + i, y + j, c);
    }

    void draw_text(u16 px, u16 py, const char* s, EgaColor64 color) {
        while (char c = *s++) {
            draw_char_8x8(px, py, c, color);
            px += 8;
        }
    }

    void flush_to_screen() {
        syscalls::vga_flush_video_buffer((const unsigned char*)vga_buffer.data());
    }

    u16 width, height;

private:
    vector<EgaColor64> vga_buffer;

    void draw_char_8x8(u16 px, u16 py, char c, EgaColor64 color) {
        for (u8 y = 0; y < 8; y++) {
            char char_row_data = g_8x8_font[(u16)c * 8 + y];
            for (u8 x = 0; x < 8; x++)
                if (char_row_data & (1 << (7-x)))
                    set_pixel_at(px + x, py + y, color);
        }
    }
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
    friend class AABB;
    Ray2D(const Vector2D& pos, const Vector2D& normalized_dir, double length) : origin(pos), normalized_dir(normalized_dir), length(length) {}

    /**
     * @return  Value in range [0.0 - 1.0] (beginning of ray - end of ray) - if crossing happens
     *          INFINITE - if ray parallel to line or crossing happens behind the ray
     */
    double dist_to_line(const Vector2D& point1, const Vector2D& point2) const {
        constexpr double INFINITE   {1000000000.0};
        Vector2D v1 = origin - point1;
        Vector2D v2 = point2 - point1;
        Vector2D v3 = {-normalized_dir.y * length, normalized_dir.x * length};


        double dot = v2 * v3;
        if (abs(dot) < 0.000001)    // ray and line parallel
            return INFINITE;

        double t1 = (v2 ^ v1) / dot;
        double t2 = (v1 * v3) / dot;

        if (t1 >= 0.0 && t1 <= 1.0 && t2 >= 0.0 && t2 <= 1.0)
            return t1;

        return INFINITE;
    }

private:
    Vector2D    origin;
    Vector2D    normalized_dir;
    double      length;
};

class AABB {
public:
    AABB(u16 x, u16 y, u16 width, u16 height) : x(x), y(y), w(width), h(height) {}

    Optional<Vector2D> check_collision(const Ray2D& ray) const {
        double up_down_edge_collision_distance = min(ray.dist_to_line({x, y}, {x+w, y}), ray.dist_to_line({x, y+h}, {x+w, y+h}));
        double left_right_edge_collision_distance = min(ray.dist_to_line({x, y}, {x, y+h}), ray.dist_to_line({x+w, y+h}, {x+w, y}));

        if ((up_down_edge_collision_distance > 1.0) && (left_right_edge_collision_distance > 1.0))
            return {};

        if (up_down_edge_collision_distance < left_right_edge_collision_distance)
            return { {ray.normalized_dir.x, -ray.normalized_dir.y} };    // mirror Y
        else
            return { {-ray.normalized_dir.x, ray.normalized_dir.y} };    // mirror X
    }

    double x, y, w, h;
};

/**
 *  GAME LOGIC PART
 */
struct Paddle {
    Optional<Vector2D> check_collision(const Ray2D& ray) {
        // middle part of the paddle bounces ball keeping its horizontal momentum
        AABB middle(x + TIP_SIZE, y, Paddle::PADDLE_WIDTH - TIP_SIZE * 2, 0.0);
        if (const auto collision_result = middle.check_collision(ray))
            return collision_result;

        // left paddle tip always bounces ball to the left
        AABB left_tip(x, y, TIP_SIZE, PADDLE_HEIGHT);
        if (auto collision_result = left_tip.check_collision(ray)) {
            if (collision_result.value.x > 0.0)
                collision_result.value.x *= -1.0;
            return collision_result;
        }
        // right paddle tip always bounces ball to the right
        AABB right_tip(x + PADDLE_WIDTH - TIP_SIZE, y, TIP_SIZE, PADDLE_HEIGHT);
        if (auto collision_result = right_tip.check_collision(ray)) {
            if (collision_result.value.x < 0.0)
                collision_result.value.x *= -1.0;
            return collision_result;
        }

        return {};
    }

    void draw(Vga& vga) {
        // left paddle tip
        vga.draw_rect(x, y, TIP_SIZE, Paddle::PADDLE_HEIGHT, TIP_COLOR);

        // middle part of the paddle
        vga.draw_rect(x + TIP_SIZE, y, Paddle::PADDLE_WIDTH - TIP_SIZE * 2, Paddle::PADDLE_HEIGHT, PADDLE_COLOR);

        // right paddle tip
        vga.draw_rect(x + Paddle::PADDLE_WIDTH - TIP_SIZE, y, TIP_SIZE, Paddle::PADDLE_HEIGHT, TIP_COLOR);

        vga.set_pixel_at(x, y, EgaColor64::Black);
        vga.set_pixel_at(x, y + Paddle::PADDLE_HEIGHT - 1, EgaColor64::Black);
        vga.set_pixel_at(x + Paddle::PADDLE_WIDTH - 1, y, EgaColor64::Black);
        vga.set_pixel_at(x + Paddle::PADDLE_WIDTH - 1, y + Paddle::PADDLE_HEIGHT - 1, EgaColor64::Black);
    }

    static constexpr u16 PADDLE_WIDTH           = 100;
    static constexpr u16 PADDLE_HEIGHT          = 5;
    static constexpr u32 TIP_SIZE               = 10;
    static constexpr EgaColor64 PADDLE_COLOR    = EgaColor64::Gray;
    static constexpr EgaColor64 TIP_COLOR       = EgaColor64::DarkGreen;
    u16 x;
    u16 y;
};

struct Ball {
    void draw(Vga& vga) {
        vga.draw_circle(pos.x, pos.y, radius, EgaColor64::Gray);
    }

    Vector2D        pos {0.0, 0.0};
    Vector2D        dir {-0.7071067811865475, -0.7071067811865475};  // normalized value of (-1, -1)
    const double    speed {75.0};   // pixels/sec
    const double    radius {4.0};
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
    static constexpr EgaColor64 NO_BRICK    {(EgaColor64)0};
    static constexpr u8 NUM_ROWS            {10};
    static constexpr u8 NUM_COLS            {16};
    static constexpr u32 BRICKS_OFFSET      {20};
    const u16 brick_width;
    const u16 brick_height;
    const AABB board_bb;
    using BrickRow = std::array<EgaColor64, NUM_COLS>;
    using BoardConstructor = std::function<EgaColor64(u16 x, u16 y)>;   // take brick position x & y and return brick color

public:
    Board(u16 width, u16 height) : brick_width(width / NUM_COLS), brick_height(height / 2 / NUM_ROWS), board_bb(0, 0, width, height) {}

    void init(const BoardConstructor& constructor) {
        num_bricks = 0;
        for (u16 y = 0; y < bricks.size(); y++)
            for (u16 x = 0; x < bricks[y].size(); x++)
                if ((bricks[y][x] = constructor(x, y)) != NO_BRICK)
                    num_bricks++;
    }

    void draw(Vga& vga) {
        // draw background
        vga.draw_rect(0, 0, board_bb.w, board_bb.h, EgaColor64::Black);

        // draw bricks
        for (u16 y = 0; y < NUM_ROWS; y++)
            for (u16 x = 0; x < NUM_COLS; x++)
                if (bricks[y][x] != NO_BRICK)
                    draw_brick(x * brick_width, BRICKS_OFFSET + y * brick_height, brick_width, brick_height, bricks[y][x], vga);
    }

    // Returns <new direction after collision, brick type>
    Optional<std::pair<Vector2D, u8>> check_collision(const Ray2D& ray) {
        constexpr auto NO_BRICK = Board::NO_BRICK;

        // collision against board walls
        if (const auto collision_result = board_bb.check_collision(ray)) {
            return { {collision_result.value, (u8)NO_BRICK} };
        }

        // collision against bricks on the board
        for (s16 y = NUM_ROWS-1; y >= 0; y--)
            for (u16 x = 0; x < NUM_COLS; x++)  {
                EgaColor64 color = bricks[y][x];
                if (color == NO_BRICK)
                    continue;

                AABB brick_bb(x * brick_width, BRICKS_OFFSET + y * brick_height, brick_width, brick_height);
                if (const auto collision_result = brick_bb.check_collision(ray)) {
                    bricks[y][x] = NO_BRICK;
                    num_bricks--;
                    return { {collision_result.value, (u8)color} };
                }
            }
        return {};
    }

    bool is_board_clear() const {
        return num_bricks == 0;
    }

private:
    std::array<BrickRow, NUM_ROWS> bricks;
    u32 num_bricks {0};

    void draw_brick(u16 bx, u16 by, u16 brick_width, u16 brick_height, EgaColor64 color, Vga& vga) {
        vga.draw_rect(bx, by, brick_width, brick_height, EgaColor64::DarkYellow, color);
    }
};

class HUD {
public:
    void draw(Vga& vga, u16 score, u16 frame_to_frame_time_ms) {
        vga.draw_text(2, 5, StringUtils::format("Score: %", score).c_str(), EgaColor64::NormalRed);
        vga.draw_text(vga.width - 60, 5, StringUtils::format("dt: %", frame_to_frame_time_ms).c_str(), EgaColor64::NormalRed);
    }
};

class Game {
public:
    Game(Vga& vga) : killer_bottom_edge(0, vga.height, vga.width, 50.0), vga(vga), board(vga.width, vga.height) {
        paddle.reset(std::make_shared<Paddle>());
        mouse_task_id = syscalls::task_lightweight_run((unsigned long long)mouse_input_thread, (u64)this, "arkanoid_mouse_input");
    }
    ~Game() {
        syscalls::task_wait(mouse_task_id);
    }

    void run() {
//        static constexpr const auto topline_board_constructor = [](u16 x, u16 y) { return EgaColor64((y == 0) * EgaColor64::LightBlue); };
//        static constexpr const auto columns_board_constructor = [](u16 x, u16 y) { return EgaColor64(((x % 2) == 0) * EgaColor64::LightCyan); };
        static constexpr const auto chessboard_board_constructor = [](u16 x, u16 y) { return EgaColor64(((x + y) % 2) * (u8)EgaColor64::DarkGreen); };

        paddle.get()->x = vga.width / 2 - Paddle::PADDLE_WIDTH / 2;
        paddle.get()->y = vga.height - 20;
        ball.pos = { vga.width / 2.0, vga.height - 25.0 };
        board.init(chessboard_board_constructor);
        terminated = false;
        game_loop();
    }

private:
    const AABB killer_bottom_edge;
    bool terminated = false;
    s64 mouse_task_id;
    Monitor<Paddle> paddle;
    Vga& vga;
    HUD hud;
    Ball ball;
    Board board;
    Timer timer;
    u32 score = 0;
    double average_dt {0.02}; // 20 Milliseconds to begin with

    static void mouse_input_thread(u64 arg) {
        Game* game = (Game*)arg;
        int fd = syscalls::open("/dev/mouse", 2);
        if (fd < 0) {
            cout::print("arkanoid: cant open /dev/mouse\n");
            game->terminated = true;
            syscalls::exit(1);
        }

        MouseState ms;
        while (syscalls::read(fd, &ms, sizeof(MouseState)) == sizeof(MouseState) && !game->terminated) {
            auto gms = game->paddle.get();
            gms->x = clamp(gms->x + ms.dx * 2, 0, game->vga.width - Paddle::PADDLE_WIDTH);
        }

        syscalls::close(fd);
        syscalls::exit(0);
    }

    void simulate_game(double delta_time) {
        Ray2D ray(ball.pos, ball.dir, ball.speed * delta_time + ball.radius);

        if (const auto collision_result = paddle.get()->check_collision(ray))
            ball.dir = collision_result.value;

        if (const auto collision_result = board.check_collision(ray)) {
            ball.dir = collision_result.value.first;
            score += collision_result.value.second * 10;
        }

        // player won
        if (board.is_board_clear()) {
            syscalls::msleep(2000);
            cout::format("VICTORY\n");
            terminated = true;
        }

        // player lost
        if (killer_bottom_edge.check_collision(ray)) {
            syscalls::msleep(2000);
            cout::format("GAME OVER\n");
            terminated = true;
        }

        ball.pos += ball.dir * ball.speed * delta_time;
    }

    void game_loop() {
        while (!terminated) {
            constexpr double SMOOTHNESS {0.99};
            average_dt = average_dt * SMOOTHNESS + timer.get_delta_seconds() * (1.0 - SMOOTHNESS);
            simulate_game(average_dt);
            board.draw(vga);
            paddle.get()->draw(vga);
            ball.draw(vga);
            hud.draw(vga, score, average_dt * 1000);
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

/*****************************************************************************
8X8 AND 8X16 FONTS
*****************************************************************************/
unsigned char g_8x8_font[2048] {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7E, 0x81, 0xA5, 0x81, 0xBD, 0x99, 0x81, 0x7E,
    0x7E, 0xFF, 0xDB, 0xFF, 0xC3, 0xE7, 0xFF, 0x7E,
    0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00,
    0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10, 0x00,
    0x38, 0x7C, 0x38, 0xFE, 0xFE, 0x92, 0x10, 0x7C,
    0x00, 0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x7C,
    0x00, 0x00, 0x18, 0x3C, 0x3C, 0x18, 0x00, 0x00,
    0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF,
    0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00,
    0xFF, 0xC3, 0x99, 0xBD, 0xBD, 0x99, 0xC3, 0xFF,
    0x0F, 0x07, 0x0F, 0x7D, 0xCC, 0xCC, 0xCC, 0x78,
    0x3C, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x7E, 0x18,
    0x3F, 0x33, 0x3F, 0x30, 0x30, 0x70, 0xF0, 0xE0,
    0x7F, 0x63, 0x7F, 0x63, 0x63, 0x67, 0xE6, 0xC0,
    0x99, 0x5A, 0x3C, 0xE7, 0xE7, 0x3C, 0x5A, 0x99,
    0x80, 0xE0, 0xF8, 0xFE, 0xF8, 0xE0, 0x80, 0x00,
    0x02, 0x0E, 0x3E, 0xFE, 0x3E, 0x0E, 0x02, 0x00,
    0x18, 0x3C, 0x7E, 0x18, 0x18, 0x7E, 0x3C, 0x18,
    0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00,
    0x7F, 0xDB, 0xDB, 0x7B, 0x1B, 0x1B, 0x1B, 0x00,
    0x3E, 0x63, 0x38, 0x6C, 0x6C, 0x38, 0x86, 0xFC,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E, 0x7E, 0x00,
    0x18, 0x3C, 0x7E, 0x18, 0x7E, 0x3C, 0x18, 0xFF,
    0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x00,
    0x18, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x00,
    0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00,
    0x00, 0x30, 0x60, 0xFE, 0x60, 0x30, 0x00, 0x00,
    0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xFE, 0x00, 0x00,
    0x00, 0x24, 0x66, 0xFF, 0x66, 0x24, 0x00, 0x00,
    0x00, 0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x00, 0x00,
    0x00, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00,
    0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00,
    0x18, 0x7E, 0xC0, 0x7C, 0x06, 0xFC, 0x18, 0x00,
    0x00, 0xC6, 0xCC, 0x18, 0x30, 0x66, 0xC6, 0x00,
    0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00,
    0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00,
    0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00,
    0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00,
    0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30,
    0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00,
    0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00,
    0x7C, 0xCE, 0xDE, 0xF6, 0xE6, 0xC6, 0x7C, 0x00,
    0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xFC, 0x00,
    0x78, 0xCC, 0x0C, 0x38, 0x60, 0xCC, 0xFC, 0x00,
    0x78, 0xCC, 0x0C, 0x38, 0x0C, 0xCC, 0x78, 0x00,
    0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x1E, 0x00,
    0xFC, 0xC0, 0xF8, 0x0C, 0x0C, 0xCC, 0x78, 0x00,
    0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00,
    0xFC, 0xCC, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00,
    0x78, 0xCC, 0xCC, 0x78, 0xCC, 0xCC, 0x78, 0x00,
    0x78, 0xCC, 0xCC, 0x7C, 0x0C, 0x18, 0x70, 0x00,
    0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00,
    0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30,
    0x18, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x00,
    0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00,
    0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00,
    0x3C, 0x66, 0x0C, 0x18, 0x18, 0x00, 0x18, 0x00,
    0x7C, 0xC6, 0xDE, 0xDE, 0xDC, 0xC0, 0x7C, 0x00,
    0x30, 0x78, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00,
    0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00,
    0x3C, 0x66, 0xC0, 0xC0, 0xC0, 0x66, 0x3C, 0x00,
    0xF8, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00,
    0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00,
    0xFE, 0x62, 0x68, 0x78, 0x68, 0x60, 0xF0, 0x00,
    0x3C, 0x66, 0xC0, 0xC0, 0xCE, 0x66, 0x3A, 0x00,
    0xCC, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0xCC, 0x00,
    0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
    0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00,
    0xE6, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0xE6, 0x00,
    0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00,
    0xC6, 0xEE, 0xFE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00,
    0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00,
    0x38, 0x6C, 0xC6, 0xC6, 0xC6, 0x6C, 0x38, 0x00,
    0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00,
    0x7C, 0xC6, 0xC6, 0xC6, 0xD6, 0x7C, 0x0E, 0x00,
    0xFC, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0xE6, 0x00,
    0x7C, 0xC6, 0xE0, 0x78, 0x0E, 0xC6, 0x7C, 0x00,
    0xFC, 0xB4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
    0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xFC, 0x00,
    0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00,
    0xC6, 0xC6, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00,
    0xC6, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0xC6, 0x00,
    0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x30, 0x78, 0x00,
    0xFE, 0xC6, 0x8C, 0x18, 0x32, 0x66, 0xFE, 0x00,
    0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00,
    0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00,
    0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00,
    0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00,
    0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66, 0xDC, 0x00,
    0x00, 0x00, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00,
    0x1C, 0x0C, 0x0C, 0x7C, 0xCC, 0xCC, 0x76, 0x00,
    0x00, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00,
    0x38, 0x6C, 0x64, 0xF0, 0x60, 0x60, 0xF0, 0x00,
    0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8,
    0xE0, 0x60, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00,
    0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00,
    0x0C, 0x00, 0x1C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78,
    0xE0, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0xE6, 0x00,
    0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
    0x00, 0x00, 0xCC, 0xFE, 0xFE, 0xD6, 0xD6, 0x00,
    0x00, 0x00, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0x00,
    0x00, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0x78, 0x00,
    0x00, 0x00, 0xDC, 0x66, 0x66, 0x7C, 0x60, 0xF0,
    0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0x1E,
    0x00, 0x00, 0xDC, 0x76, 0x62, 0x60, 0xF0, 0x00,
    0x00, 0x00, 0x7C, 0xC0, 0x70, 0x1C, 0xF8, 0x00,
    0x10, 0x30, 0xFC, 0x30, 0x30, 0x34, 0x18, 0x00,
    0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00,
    0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00,
    0x00, 0x00, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00,
    0x00, 0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00,
    0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8,
    0x00, 0x00, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00,
    0x1C, 0x30, 0x30, 0xE0, 0x30, 0x30, 0x1C, 0x00,
    0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00,
    0xE0, 0x30, 0x30, 0x1C, 0x30, 0x30, 0xE0, 0x00,
    0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0x00,
    0x7C, 0xC6, 0xC0, 0xC6, 0x7C, 0x0C, 0x06, 0x7C,
    0x00, 0xCC, 0x00, 0xCC, 0xCC, 0xCC, 0x76, 0x00,
    0x1C, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00,
    0x7E, 0x81, 0x3C, 0x06, 0x3E, 0x66, 0x3B, 0x00,
    0xCC, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00,
    0xE0, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00,
    0x30, 0x30, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00,
    0x00, 0x00, 0x7C, 0xC6, 0xC0, 0x78, 0x0C, 0x38,
    0x7E, 0x81, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00,
    0xCC, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00,
    0xE0, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00,
    0xCC, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00,
    0x7C, 0x82, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00,
    0xE0, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00,
    0xC6, 0x10, 0x7C, 0xC6, 0xFE, 0xC6, 0xC6, 0x00,
    0x30, 0x30, 0x00, 0x78, 0xCC, 0xFC, 0xCC, 0x00,
    0x1C, 0x00, 0xFC, 0x60, 0x78, 0x60, 0xFC, 0x00,
    0x00, 0x00, 0x7F, 0x0C, 0x7F, 0xCC, 0x7F, 0x00,
    0x3E, 0x6C, 0xCC, 0xFE, 0xCC, 0xCC, 0xCE, 0x00,
    0x78, 0x84, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00,
    0x00, 0xCC, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00,
    0x00, 0xE0, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00,
    0x78, 0x84, 0x00, 0xCC, 0xCC, 0xCC, 0x76, 0x00,
    0x00, 0xE0, 0x00, 0xCC, 0xCC, 0xCC, 0x76, 0x00,
    0x00, 0xCC, 0x00, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8,
    0xC3, 0x18, 0x3C, 0x66, 0x66, 0x3C, 0x18, 0x00,
    0xCC, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x00,
    0x18, 0x18, 0x7E, 0xC0, 0xC0, 0x7E, 0x18, 0x18,
    0x38, 0x6C, 0x64, 0xF0, 0x60, 0xE6, 0xFC, 0x00,
    0xCC, 0xCC, 0x78, 0x30, 0xFC, 0x30, 0xFC, 0x30,
    0xF8, 0xCC, 0xCC, 0xFA, 0xC6, 0xCF, 0xC6, 0xC3,
    0x0E, 0x1B, 0x18, 0x3C, 0x18, 0x18, 0xD8, 0x70,
    0x1C, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00,
    0x38, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00,
    0x00, 0x1C, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00,
    0x00, 0x1C, 0x00, 0xCC, 0xCC, 0xCC, 0x76, 0x00,
    0x00, 0xF8, 0x00, 0xB8, 0xCC, 0xCC, 0xCC, 0x00,
    0xFC, 0x00, 0xCC, 0xEC, 0xFC, 0xDC, 0xCC, 0x00,
    0x3C, 0x6C, 0x6C, 0x3E, 0x00, 0x7E, 0x00, 0x00,
    0x38, 0x6C, 0x6C, 0x38, 0x00, 0x7C, 0x00, 0x00,
    0x18, 0x00, 0x18, 0x18, 0x30, 0x66, 0x3C, 0x00,
    0x00, 0x00, 0x00, 0xFC, 0xC0, 0xC0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xFC, 0x0C, 0x0C, 0x00, 0x00,
    0xC6, 0xCC, 0xD8, 0x36, 0x6B, 0xC2, 0x84, 0x0F,
    0xC3, 0xC6, 0xCC, 0xDB, 0x37, 0x6D, 0xCF, 0x03,
    0x18, 0x00, 0x18, 0x18, 0x3C, 0x3C, 0x18, 0x00,
    0x00, 0x33, 0x66, 0xCC, 0x66, 0x33, 0x00, 0x00,
    0x00, 0xCC, 0x66, 0x33, 0x66, 0xCC, 0x00, 0x00,
    0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0xDB, 0xF6, 0xDB, 0x6F, 0xDB, 0x7E, 0xD7, 0xED,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0xF8, 0x18, 0x18, 0x18,
    0x18, 0x18, 0xF8, 0x18, 0xF8, 0x18, 0x18, 0x18,
    0x36, 0x36, 0x36, 0x36, 0xF6, 0x36, 0x36, 0x36,
    0x00, 0x00, 0x00, 0x00, 0xFE, 0x36, 0x36, 0x36,
    0x00, 0x00, 0xF8, 0x18, 0xF8, 0x18, 0x18, 0x18,
    0x36, 0x36, 0xF6, 0x06, 0xF6, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x00, 0x00, 0xFE, 0x06, 0xF6, 0x36, 0x36, 0x36,
    0x36, 0x36, 0xF6, 0x06, 0xFE, 0x00, 0x00, 0x00,
    0x36, 0x36, 0x36, 0x36, 0xFE, 0x00, 0x00, 0x00,
    0x18, 0x18, 0xF8, 0x18, 0xF8, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xF8, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x1F, 0x00, 0x00, 0x00,
    0x18, 0x18, 0x18, 0x18, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x1F, 0x18, 0x18, 0x18,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
    0x18, 0x18, 0x18, 0x18, 0xFF, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x1F, 0x18, 0x1F, 0x18, 0x18, 0x18,
    0x36, 0x36, 0x36, 0x36, 0x37, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x37, 0x30, 0x3F, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x3F, 0x30, 0x37, 0x36, 0x36, 0x36,
    0x36, 0x36, 0xF7, 0x00, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0x00, 0xF7, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x37, 0x30, 0x37, 0x36, 0x36, 0x36,
    0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00,
    0x36, 0x36, 0xF7, 0x00, 0xF7, 0x36, 0x36, 0x36,
    0x18, 0x18, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00,
    0x36, 0x36, 0x36, 0x36, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0x00, 0xFF, 0x18, 0x18, 0x18,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x3F, 0x00, 0x00, 0x00,
    0x18, 0x18, 0x1F, 0x18, 0x1F, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x1F, 0x18, 0x1F, 0x18, 0x18, 0x18,
    0x00, 0x00, 0x00, 0x00, 0x3F, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0xFF, 0x36, 0x36, 0x36,
    0x18, 0x18, 0xFF, 0x18, 0xFF, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0xF8, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1F, 0x18, 0x18, 0x18,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x76, 0xDC, 0xC8, 0xDC, 0x76, 0x00,
    0x00, 0x78, 0xCC, 0xF8, 0xCC, 0xF8, 0xC0, 0xC0,
    0x00, 0xFC, 0xCC, 0xC0, 0xC0, 0xC0, 0xC0, 0x00,
    0x00, 0x00, 0xFE, 0x6C, 0x6C, 0x6C, 0x6C, 0x00,
    0xFC, 0xCC, 0x60, 0x30, 0x60, 0xCC, 0xFC, 0x00,
    0x00, 0x00, 0x7E, 0xD8, 0xD8, 0xD8, 0x70, 0x00,
    0x00, 0x66, 0x66, 0x66, 0x66, 0x7C, 0x60, 0xC0,
    0x00, 0x76, 0xDC, 0x18, 0x18, 0x18, 0x18, 0x00,
    0xFC, 0x30, 0x78, 0xCC, 0xCC, 0x78, 0x30, 0xFC,
    0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0x6C, 0x38, 0x00,
    0x38, 0x6C, 0xC6, 0xC6, 0x6C, 0x6C, 0xEE, 0x00,
    0x1C, 0x30, 0x18, 0x7C, 0xCC, 0xCC, 0x78, 0x00,
    0x00, 0x00, 0x7E, 0xDB, 0xDB, 0x7E, 0x00, 0x00,
    0x06, 0x0C, 0x7E, 0xDB, 0xDB, 0x7E, 0x60, 0xC0,
    0x38, 0x60, 0xC0, 0xF8, 0xC0, 0x60, 0x38, 0x00,
    0x78, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x00,
    0x00, 0x7E, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00,
    0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x7E, 0x00,
    0x60, 0x30, 0x18, 0x30, 0x60, 0x00, 0xFC, 0x00,
    0x18, 0x30, 0x60, 0x30, 0x18, 0x00, 0xFC, 0x00,
    0x0E, 0x1B, 0x1B, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x18, 0xD8, 0xD8, 0x70,
    0x18, 0x18, 0x00, 0x7E, 0x00, 0x18, 0x18, 0x00,
    0x00, 0x76, 0xDC, 0x00, 0x76, 0xDC, 0x00, 0x00,
    0x38, 0x6C, 0x6C, 0x38, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
    0x0F, 0x0C, 0x0C, 0x0C, 0xEC, 0x6C, 0x3C, 0x1C,
    0x58, 0x6C, 0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00,
    0x70, 0x98, 0x30, 0x60, 0xF8, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
