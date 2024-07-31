#include <emu.h>

int main(int argc, char **argv) {
    emu myemu(argc,argv);
    return myemu.emu_run();
}
