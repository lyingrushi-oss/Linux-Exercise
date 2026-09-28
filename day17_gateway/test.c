#if 0
#include <math.h>
#include <stdio.h>

int main(void)
{
    double x=sin(1.0);
    printf("sin(1.0)=%.2f\n",x);

    return x>0;
}
#endif

#include <math.h>
#include <stdio.h>

int main(void)
{
    double x;

    printf("input x: ");

    if (scanf("%lf", &x) != 1)
    {
        return 1;
    }

    printf("sin(%.2f)=%.2f\n", x, sin(x));
    return 0;
}