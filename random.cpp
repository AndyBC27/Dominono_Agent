#include <cstdio>
#include <string>
#include <unistd.h>

int main(int argc, char const * const argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <color> <nrows> <ncols>\n", argv[0]);
        return 1;
    }
    bool black = std::string(argv[1]) == "b";
    int nrows = std::stoi(argv[2]);
    int ncols = std::stoi(argv[3]);
    fprintf(stderr, "%s playing %s on a %d x %d board\n", argv[0], black ? "black" : "white", nrows, ncols);
    srand(getpid());
    while (true) {
        if (black) {
            int row = rand() % nrows + 1;
            int col = rand() % ncols + 1;
            printf("%d %d\n", row, col);
            fflush(stdout);
        } else {
            scanf("%*d %*d");
        }
        black = !black;
    }
    return 0;
}
