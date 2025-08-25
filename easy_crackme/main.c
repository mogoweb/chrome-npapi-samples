#include <stdio.h>
#include <string.h>

int main(void) {
    // 定义一个缓冲区来存储用户输入
    char userInput[100];

    // 打印提示信息
    printf("Please input password: ");

    // 从标准输入读取一行字符串，并存储到 userInput 中
    // 使用 fgets 是比 scanf 更安全的选择，可以防止缓冲区溢出
    if (fgets(userInput, sizeof(userInput), stdin) != NULL) {
        // fgets 会读取并包含换行符 '\n'，我们需要移除它以便比较
        // strcspn 会返回字符串中第一个不包含在指定字符集内的字符的索引
        userInput[strcspn(userInput, "\n")] = 0;
    }

    // 使用 strcmp 函数比较用户输入和硬编码的密码
    // strcmp 在两个字符串相等时返回 0
    if (strcmp(userInput, "I am the real password!") == 0) {
        // 如果密码正确
        printf("correct! welcome!\n");
    } else {
        // 如果密码错误
        printf("mistake!\n");
    }

    return 0;
}
