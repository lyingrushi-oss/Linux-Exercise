#include <stdio.h>
#include <unistd.h>

int main()
{
	printf("before exec\n");
	execl("/bin/ls","ls","-l",NULL);
	printf("after exec\n");


	return 0;
}
