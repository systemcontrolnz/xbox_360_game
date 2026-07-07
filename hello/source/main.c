#include <stdio.h>
#include <console/console.h>
#include <xenos/xenos.h>

int main(void) {
    xenos_init(VIDEO_MODE_AUTO);
    console_init();

    printf("Hello, Xbox 360!\n");
    printf("libxenon toolchain pipeline working.\n");

    while (1) {
    }

    return 0;
}
