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
#include "Timer.h"
#include "Mouse.h"
#include "Monitor.h"
#include "VgaDevice.h"
#include "StringUtils.h"
#include "Optional.h"

using namespace cstd;
using namespace cstd::ustd;
using namespace middlespace;

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
            return { Vector2D{ray.normalized_dir.x, -ray.normalized_dir.y} };    // mirror Y
        else
            return { Vector2D{-ray.normalized_dir.x, ray.normalized_dir.y} };    // mirror X
    }

    double x, y, w, h;
};

/**
 *  GAME LOGIC PART
 */
using BrickType = EgaColor64;

struct Paddle {
    void reset_position(u16 x, u16 y) {
        this->x = x;
        this->y = y;
    }

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

    void draw(VgaDevice& vga) {
        // left paddle tip
        vga.draw_rect(x, y, TIP_SIZE, Paddle::PADDLE_HEIGHT, TIP_COLOR);

        // middle part of the paddle
        vga.draw_rect(x + TIP_SIZE, y, Paddle::PADDLE_WIDTH - TIP_SIZE * 2, Paddle::PADDLE_HEIGHT, PADDLE_COLOR);

        // right paddle tip
        vga.draw_rect(x + Paddle::PADDLE_WIDTH - TIP_SIZE, y, TIP_SIZE, Paddle::PADDLE_HEIGHT, TIP_COLOR);

        // remove corners so the paddle looks more rounded
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
    void draw(VgaDevice& vga) {
        vga.draw_circle(pos.x, pos.y, radius, EgaColor64::Gray);
    }

    Vector2D        pos {0.0, 0.0};
    Vector2D        dir {-0.7071067811865475, -0.7071067811865475};  // normalized value of (-1, -1)
    const double    speed {75.0};   // pixels/sec
    const double    radius {4.0};
};

class Board {
    static constexpr BrickType NO_BRICK     {BrickType::Black};
    static constexpr u8 NUM_ROWS            {10};
    static constexpr u8 NUM_COLS            {16};
    static constexpr u32 BRICKS_Y_OFFSET    {20};
    const u16 brick_width;
    const u16 brick_height;
    const AABB board_bb;
    using BrickRow = std::array<BrickType, NUM_COLS>;
    using BoardConstructor = std::function<BrickType(u16 x, u16 y)>;   // take brick position x & y and return brick type

public:
    Board(u16 width, u16 height) : brick_width(width / NUM_COLS), brick_height(height / 2 / NUM_ROWS), board_bb(0, 0, width, height) {}

    void init(const BoardConstructor& constructor) {
        num_bricks = 0;
        for (u16 y = 0; y < bricks.size(); y++)
            for (u16 x = 0; x < bricks[y].size(); x++)
                if ((bricks[y][x] = constructor(x, y)) != NO_BRICK)
                    num_bricks++;
    }

    void draw(VgaDevice& vga) {
        // draw background
        vga.draw_rect(0, 0, board_bb.w, board_bb.h, EgaColor64::Black);

        // draw bricks
        for (u16 row = 0; row < NUM_ROWS; row++)
            for (u16 col = 0; col < NUM_COLS; col++)
                if (bricks[row][col] != NO_BRICK)
                    draw_brick(col, row, bricks[row][col], vga);
    }

    // Returns <new direction after collision, brick type>
    Optional<std::pair<Vector2D, BrickType>> check_collision(const Ray2D& ray) {
        constexpr auto NO_BRICK = Board::NO_BRICK;

        // collision against board walls
        if (const auto collision_result = board_bb.check_collision(ray)) {
            return { {collision_result.value, NO_BRICK} };
        }

        // collision against bricks on the board
        BrickType type ;
        for (s16 row = NUM_ROWS-1; row >= 0; row--)
            for (u16 col = 0; col < NUM_COLS; col++)
                if ((type = bricks[row][col]) != NO_BRICK)
                    if (const auto collision_result = collide_brick(col, row, ray))
                        return { {collision_result.value, type} };

        return {};
    }

    bool is_board_clear() const {
        return num_bricks == 0;
    }

private:
    std::array<BrickRow, NUM_ROWS> bricks;
    u32 num_bricks {0};

    void draw_brick(u16 brick_col, u16 brick_row, BrickType type, VgaDevice& vga) {
        u16 x = brick_col * brick_width;
        u16 y = brick_row * brick_height + BRICKS_Y_OFFSET;
        EgaColor64 color = (EgaColor64)type; // identity map type to color
        vga.draw_rect(x, y, brick_width, brick_height, EgaColor64::DarkYellow, color);
    }

    Optional<Vector2D> collide_brick(u16 brick_col, u16 brick_row, const Ray2D& ray) {
        u16 x = brick_col * brick_width;
        u16 y = brick_row * brick_height + BRICKS_Y_OFFSET;
        AABB brick_bb(x, y, brick_width, brick_height);
        if (const auto collision_result = brick_bb.check_collision(ray)) {
            bricks[brick_row][brick_col] = NO_BRICK;
            num_bricks--;
            return {collision_result.value};
        }
        return {};
    }
};

class HUD {
public:
    void draw(VgaDevice& vga, u16 score, u16 frame_to_frame_time_ms) {
        vga.draw_text(2, 5, StringUtils::format("Score: %", score).c_str(), EgaColor64::NormalRed);
        vga.draw_text(vga.width - 60, 5, StringUtils::format("dt: %", frame_to_frame_time_ms).c_str(), EgaColor64::NormalRed);
    }
};

class Game {
public:
    Game(VgaDevice& vga) : killer_bottom_edge(0, vga.height, vga.width, 50.0), vga(vga), board(vga.width, vga.height) {
        paddle.reset(std::make_shared<Paddle>());
        mouse_input_task_id = syscalls::task_lightweight_run((unsigned long long)mouse_input_task, (u64)this, "arkanoid_mouse_input");
    }

    ~Game() {
        syscalls::task_wait(mouse_input_task_id);
    }

    void run() {
//        static constexpr const auto topline_board_constructor = [](u16 x, u16 y) { return EgaColor64((y == 0) * (u8)EgaColor64::LightBlue); };
//        static constexpr const auto columns_board_constructor = [](u16 x, u16 y) { return EgaColor64(((x % 2) == 0) * (u8)EgaColor64::LightCyan); };
        static constexpr const auto chessboard_board_constructor = [](u16 x, u16 y) { return EgaColor64(((x + y) % 2) * (u8)EgaColor64::DarkGreen); };

        board.init(chessboard_board_constructor);
        ball.pos = { vga.width / 2.0, vga.height - 25.0 };
        paddle.get()->reset_position(vga.width / 2 - Paddle::PADDLE_WIDTH / 2, vga.height - 20);
        terminated = false;
        game_loop();
    }

private:
    const AABB killer_bottom_edge;
    bool terminated = false;
    s64 mouse_input_task_id;
    Monitor<Paddle> paddle;
    VgaDevice& vga;
    HUD hud;
    Ball ball;
    Board board;
    Timer timer;
    u32 score = 0;
    double average_dt {0.02}; // 20 Milliseconds to begin with

    static void mouse_input_task(u64 arg) {
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
            gms->x = clamp(gms->x + ms.dx, 0, game->vga.width - Paddle::PADDLE_WIDTH);
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
            score += (u32)collision_result.value.second * 10;
        }

        // player won
        if (board.is_board_clear()) {
            cout::format("VICTORY\n");
            terminated = true;
        }

        // player lost
        if (killer_bottom_edge.check_collision(ray)) {
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
        syscalls::msleep(2000);
    }
};

/**
 * @brief   Entry point
 * @return  0 on success
 */
int main(int argc, char* argv[]) {
    VgaDevice vga;
    Game game(vga);
    game.run();

    return 0;
}
