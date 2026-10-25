#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>  //character type提供字符分类函数

//定义错误码
enum {
    PARSE_OK = 0,
    PARSE_NO_NUMBER,
    PARSE_FLOAT_RANGE,
    PARSE_TRAILING_TEXT,
    PARSE_SENSOR_RANGE
};

/// @brief 解析温度值
/// @param text 传入字符串
/// @param out 输出解析出的温度值
/// @return 错误码
static int parse_temperature(const char *text, float *out)
{
    char *end = NULL;

    errno = 0;
    float temp = strtof(text, &end);

    if (end == text) {
        printf("no number\n");
        return PARSE_NO_NUMBER;
    }

    if (errno == ERANGE) {
        printf("float range error\n");
        return PARSE_FLOAT_RANGE;
    }

    char *tail = end;

    while (isspace((unsigned char)*tail)) {
        tail++;
    }

    if (*tail != '\0') {
        printf("invalid trailing text\n");
        return PARSE_TRAILING_TEXT;
    }

    if (temp < -50.0f || temp > 150.0f) {
        printf("sensor range error\n");
        return PARSE_SENSOR_RANGE;
    }

    *out = temp;
    printf("value=%.1f\n",temp);//转换浮点数结果
    printf("converted %td bytes\n",end-text);//转换了的字节数
    printf("rest=\"%s\"\n",tail);//跳过空白余下的字符
    return PARSE_OK;
}

int main(void)
{

    const char* tests[]={
        "25.0\n",
        "25.0xyz",
        "abc",
        "0.0",
        "200.0",
        "999999999999999999999999999999999999999999999"
    };
    for(size_t i=0;i<sizeof(tests)/sizeof(tests[0]);i++)
    {
        const char* text=tests[i];
        errno=0;//显式对errno清零
        float temp;

        printf("checking text=\"%s\"\n",text);//原字符串文本
        int ret=parse_temperature(text,&temp);

        #if 0 
        /*
        end - text      // strtof 实际转换了多少字符
        *end            // 它最初停在哪个字符
        *tail           // 跳过允许的空白后还剩什么
       */
        //转换完成：判断错误类型
        if(text==end)
        {
            printf("no number\n");
            continue;
        }

        if(errno==ERANGE)
        {
            printf("float range error\n");
            continue;
        }

        //跳过空格
        while(isspace((unsigned char)*tail))
        {
            tail++;
        }

        if (*tail!='\0')
        {
            /* code */
            printf("trailing text\n");//尾随字符
            continue;
        }

        
        
        if(temp>150.0f||temp<-50.0f)
        {
            //超出传感器范围
            printf("sensor range error\n");
            continue;
        }
        #endif

        printf("ret code=%d\n",ret);
    }
    return 0;

}

