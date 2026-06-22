#include <stdio.h>
int main() {
    char hex[] = "7564721b6f7c7d6e1707006711761e76";
    unsigned int b;
    int r = sscanf_s(hex + 0, "%2x", &b);
    printf("r=%d b=0x%x\n", r, b, b);
    return 0;
}
