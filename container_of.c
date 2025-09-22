#include <stdio.h>

struct data {
    char a;
    int b;
    float c;
};

void getContainerOf(void *b_ptr) {
    //b_ptr = (char*)b_ptr;
    // need to find offset
    long offset = (long)&(((struct data *)0)->b);
    // void pointer is treated as char* for arithmetic
    // so we can do pointer - offset directly
    struct data *d_ptr = (struct data *)(b_ptr - offset);
    printf("a: %c, b: %d, c: %.2f\n", d_ptr->a, d_ptr->b, d_ptr->c);
}

int main(){
    struct data d = {'x', 42, 3.14f};

    getContainerOf(&d.b);
    return 0;
}
