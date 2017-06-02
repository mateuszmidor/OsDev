/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */


class Printer {
private:
    const unsigned int NUM_COLS = 80u;
    unsigned short* VGA = (unsigned short*)0xb8000;
    unsigned int cursor_pos = 0;

public:
    void println(const char null_terminated_str[] = "") {
        unsigned int str_pos = 0;
        while (null_terminated_str[str_pos]) {
            VGA[cursor_pos++] = 0x2f << 8 | null_terminated_str[str_pos];
            str_pos++;
        }
        cursor_pos += (NUM_COLS - str_pos); // next line
    }
};

/**
 * main
 * Kernel entry point
 */
int main() {
    Printer p;
    p.println();
    p.println("Hello in main() of main.cpp!");
}

