#include <stdio.h>
#include <stdlib.h>

static int* middle;
static int* cache;

static void create_inferior_pointer(void)
{
    int* raw=malloc(10*sizeof(*raw));

    if(raw==NULL)
    {
        return;
    }

    raw[0]=20;

    /* 只留下指向内存块中间的指针 */
    middle = raw + 1;

}

int main(void)
{
    cache=malloc(10*sizeof(*cache));

    if(cache==NULL)
    {
        return 1;
    }

    cache[0]=10;
    create_inferior_pointer();

    printf("cache[0]=%d\n",cache[0]);

    /*正确的处理方式
    static int *raw_base;
    static int *middle;

    raw_base = malloc(10 * sizeof(*raw_base));
    middle = raw_base + 1;

    free(raw_base);
    raw_base = NULL;
    middle = NULL;
    */

    return 0;
}