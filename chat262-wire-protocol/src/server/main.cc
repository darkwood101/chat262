#include "common.h"
#include "server.h"

int main(int argc, char** argv) {
    server s;
    if (s.run(argc, argv) == status::ok) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}