/**
 *   @file: plot.cpp
 *
 *   @date: Jan 3, 2018
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "Cin.h"
#include "Cout.h"
#include "VgaDevice.h"
#include "StringUtils.h"
#include "ReversePolishNotation.h"

using namespace ustd;

struct MinMax {
    double min;
    double max;
    double span() { return max - min; }
};

double scale_to_0_1(double val, MinMax from_range) {
    if (from_range.min == from_range.max)
        return 0.5;
    else
        return (val - from_range.min) / from_range.span();
}

double scale_from_0_1(double val, MinMax to_range) {
    return to_range.min + val * to_range.span();
}

Optional<vector<double>> get_samples(rpn::Calculator& calc, MinMax x_range, size_t num_samples) {
    vector<double> samples(num_samples);
    for (size_t i = 0; i < num_samples; i++) {
        double x_in_range_0_1 = scale_to_0_1(i, {0.0, num_samples-1.0});
        double x = scale_from_0_1(x_in_range_0_1, x_range);

        calc.define("x", x);
        if (auto result = calc.calc())
            samples[i] = result.value;
        else
            return {result.error_msg};
    }

    return {std::move(samples)};
}

Optional<vector<double>> evaluate_formula(const string& formula, MinMax x_range, size_t num_samples) {
    if (x_range.min >= x_range.max)
        return {"Invalid x range"};

    if (num_samples < 2)
        return {"Need at least 2 samples to plot"};

    rpn::Calculator calc;
    if (auto parse_result = calc.parse(formula))
        return get_samples(calc, x_range, num_samples);
    else
        return {parse_result.error_msg};
}

MinMax get_min_max(const vector<double>& v) {
    auto it = std::minmax_element(v.begin(), v.end());
    return {*it.first, *it.second};
}

void draw_samples(VgaDevice& vga, const vector<double>& samples) {
    MinMax minmax = get_min_max(samples);

    // assumption: samples.count == vga.width
    for (size_t x = 0; x < vga.width; x++) {
        double val_in_range_0_1 = scale_to_0_1(samples[x], minmax);
        u16 y = (1.0 - val_in_range_0_1) * vga.height;  // Y-mirror and then scale to window size
        vga.set_pixel_at(x, y, middlespace::EgaColor64::NormalYellow);
    }
}

void draw_labels(VgaDevice& vga, const MinMax& domain_range, const vector<double>& samples) {
    constexpr auto FONT_HEIGHT = 8; // vga draw_text uses 8 px height font
    constexpr auto FONT_WIDTH = 8;  // vga draw_text uses 8 px per char width font
    u16 bottom = vga.height - FONT_HEIGHT;


    MinMax y_range = get_min_max(samples);
    string y_min = StringUtils::from_double(y_range.min, 5);
    vga.draw_text(0, bottom, y_min.c_str(), middlespace::EgaColor64::White);
    string y_max = StringUtils::from_double(y_range.max, 5);
    vga.draw_text(0, 0, y_max.c_str(), middlespace::EgaColor64::White);

    string x_min = StringUtils::from_double(domain_range.min, 2);
    string x_max = StringUtils::from_double(domain_range.max, 2);
    string range = StringUtils::format("x = [%..%]", x_min, x_max);
    u16 range_x = vga.width - range.length() * FONT_WIDTH;
    vga.draw_text(range_x, 0, range.c_str(), middlespace::EgaColor64::White);
}

string build_formula(int argc, char* argv[]) {
    string formula;

    for (int i = 1; i < argc; i++)
        formula += argv[i];

    return std::move(formula);
}

/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout::print("plot: please speficy formula like sin(x)\n");
        return 1;
    }

    VgaDevice vga;
    string formula = build_formula(argc, argv);
    MinMax domain_range {-3.14, 3.14};

    if (auto result = evaluate_formula(formula, domain_range, vga.width)) {
        draw_samples(vga, result.value);
        draw_labels(vga, domain_range, result.value);
        vga.flush_to_screen();
        cin::readln();
        cout::format("plot done.\n");
        return 0;
    }
    else {
        cout::format("plot error: %\n", result.error_msg);
        return 1;
    }
}
