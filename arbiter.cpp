#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <string>
#include <sys/resource.h>
#include <vector>
#include <unistd.h>

constexpr int MIN_AREA = 2;
constexpr int MAX_AREA = 100;

constexpr long LIMIT_THREADS = 16;
constexpr long LIMIT_MEMORY = 64 * 1024 * 1024 * 1024L;
constexpr long LIMIT_MOVE_TIME = 10000;

using Player = uint8_t;
constexpr Player NONE = 0;
constexpr Player BLACK = 1;
constexpr Player WHITE = 2;
constexpr Player BOTH = BLACK | WHITE;

struct Move {

    int row;
    int col;

};

struct Board {

    int nrows;
    int ncols;
    std::vector<std::vector<Player>> moved;

    Board(int nrows, int ncols) : nrows(nrows), ncols(ncols) {
        // Check that board dimensions are valid
        if (nrows <= 0 || ncols <= 0 || nrows * ncols < MIN_AREA || nrows * ncols > MAX_AREA) {
            fprintf(stderr, "Error: nrows and ncols must be positive and their product must be between %d and %d.\n", MIN_AREA, MAX_AREA);
            exit(1);
        }
        moved.resize(nrows + 2, std::vector<Player>(ncols + 2, NONE));
    }

    bool move(Player player, Move move) {
        int row = move.row, col = move.col;
        if (row < 1 || row > nrows || col < 1 || col > ncols) {
            fprintf(stderr, "move(): (%d, %d) is out of bounds.\n", row, col);
            return false;
        }
        if (moved[row][col] != NONE) {
            fprintf(stderr, "move(): (%d, %d) is already occupied.\n", row, col);
            return false;
        }
        if (moved[row - 1][col] == player || moved[row + 1][col] == player || moved[row][col - 1] == player || moved[row][col + 1] == player) {
            fprintf(stderr, "move(): (%d, %d) is adjacent to another stone of the same color.\n", row, col);
            return false;
        }
        moved[row][col] = player;
        return true;
    }

    void show() const {
        for (int row = 1; row <= nrows; row++) {
            for (int col = 1; col <= ncols; col++) {
                printf("%c", moved[row][col] == NONE ? '.' : moved[row][col] == BLACK ? 'X' : 'O');
            }
            printf("\n");
        }
    }

};

struct Program {

    std::string executable;
    pid_t pid;
    FILE * input;
    FILE * output;
    struct pollfd pollfd;

    Program(std::string executable, Player player, int nrows, int ncols, int cpu) : executable(executable) {
        // Check that the program is executable
        if (access(executable.c_str(), X_OK) == -1) {
            fprintf(stderr, "Error: %s does not exist or is not executable.\n", executable.c_str());
            exit(1);
        }
        // Create anonymous pipes for communication
        int in_pipe[2], out_pipe[2];
        if (pipe(in_pipe) == -1 || pipe(out_pipe) == -1) {
            fprintf(stderr, "Error: failed to create pipes.\n");
            exit(1);
        }
        // Fork the process
        pid = fork();
        if (pid == -1) {
            fprintf(stderr, "Error: failed to fork process.\n");
            exit(1);
        }
        if (pid == 0) { // Child process
            // Limit current process to 16 threads, 64 GB of ram, and a specified CPU core
            rlimit thread_limit = {LIMIT_THREADS, LIMIT_THREADS};
            if (setrlimit(RLIMIT_NPROC, &thread_limit) == -1) {
                fprintf(stderr, "Error: failed to set thread limit.\n");
                exit(1);
            }
            rlimit memory_limit = {LIMIT_MEMORY, LIMIT_MEMORY};
            if (setrlimit(RLIMIT_AS, &memory_limit) == -1) {
                fprintf(stderr, "Error: failed to set memory limit.\n");
                exit(1);
            }
            //if (cpu >= 0) {
                //cpu_set_t cpu_set;
                //CPU_ZERO(&cpu_set);
               // CPU_SET(cpu, &cpu_set);
                // if (sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set) == -1) {
                //     fprintf(stderr, "Error: failed to set cpu affinity.\n");
                //     exit(1);
                // }
            //}
            // Redirect stdin and stdout to pipes, closed unused pipe ends
            if (dup2(in_pipe[0], STDIN_FILENO) == -1 || dup2(out_pipe[1], STDOUT_FILENO) == -1) {
                fprintf(stderr, "Error: failed to redirect stdin and stdout.\n");
                exit(1);
            }
            if (close(in_pipe[0]) == -1 || close(out_pipe[1]) == -1 || close(in_pipe[1]) == -1 || close(out_pipe[0]) == -1) {
                fprintf(stderr, "Error: failed to close pipe ends.\n");
                exit(1);
            }
            // Execute the program
            if (execl(executable.c_str(),
                executable.c_str(),
                player == BLACK ? "b" : "w",
                std::to_string(nrows).c_str(),
                std::to_string(ncols).c_str(),
                NULL
            ) == -1) {
                fprintf(stderr, "Error: failed to execute %s.\n", executable.c_str());
                exit(1);
            }
            // Should never reach here
        } else { // Parent process
            // Close unused pipe ends
            if (close(in_pipe[0]) == -1 || close(out_pipe[1]) == -1) {
                fprintf(stderr, "Error: failed to close pipe ends.\n");
                exit(1);
            }
            // Prepare for polling
            if (fcntl(out_pipe[0], F_SETFL, O_NONBLOCK) == -1) {
                fprintf(stderr, "Error: failed to set pipe to non-blocking.\n");
                exit(1);
            }
            pollfd.fd = out_pipe[0];
            pollfd.events = POLLIN;
            // Store the pipe ends
            input = fdopen(in_pipe[1], "w");
            output = fdopen(out_pipe[0], "r");
            if (input == NULL || output == NULL) {
                fprintf(stderr, "Error: failed to open pipe ends.\n");
                exit(1);
            }
        }
    }

    ~Program() {
        fclose(input);
        fclose(output);
        kill(pid, SIGTERM);
    }

    bool write(Move move) {
        if (fprintf(input, "%d %d\n", move.row, move.col) < 0) {
            fprintf(stderr, "write(): failed to write move.\n");
            return false;
        }
        fflush(input);
        return true;
    }

    bool read(Move & move) {
        if (poll(&pollfd, 1, LIMIT_MOVE_TIME) <= 0) {
            fprintf(stderr, "read(): timed out waiting for move.\n");
            return false;
        }
        if (fscanf(output, "%d %d", &move.row, &move.col) != 2) {
            fprintf(stderr, "read(): failed to read move.\n");
            return false;
        }
        return true;
    }

};

int main(int argc, char const * const argv[]) {

    if (argc != 7) {
        std::cerr << "Usage: " << argv[0] << " <nrows> <ncols> <black-exe> <black-cpu> <white-exe> <white-cpu>\n";
        return 1;
    }

    auto board = Board{std::stoi(argv[1]), std::stoi(argv[2])};
    Program programs[2] = {
        Program{argv[3], BLACK, board.nrows, board.ncols, std::stoi(argv[4])},
        Program{argv[5], WHITE, board.nrows, board.ncols, std::stoi(argv[6])}
    };

    printf("board: %d x %d\n", board.nrows, board.ncols);
    printf("black: %s\n", programs[BLACK-1].executable.c_str());
    printf("white: %s\n", programs[WHITE-1].executable.c_str());
    printf("---\n");
    Player current = BLACK;
    while (true) {
        Move move;
        if (!programs[current-1].read(move)) {
            break;
        }
        printf("%s (%d, %d)\n", current == BLACK ? "black" : "white", move.row, move.col);
        if (!board.move(current, move)) {
            break;
        }
        board.show();
        current = BOTH ^ current;
        if (!programs[current-1].write(move)) {
            break;
        }
    }
    printf("---\n");
    printf("%s wins\n", current == BLACK ? "white" : "black");

    return 0;
}
