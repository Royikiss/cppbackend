#include <stdio.h>
#include <stdlib.h>

// 介绍基础IO操作
#define __NAMESPACE_BaseIO_BEGIN__ namespace BaseIO {
#define __NAMESPACE_BaseIO_END__ } // namespace BaseIO

// 介绍文件IO操作
#define __NAMESPACE_FileIO_BEGIN__ namespace FileIO {
#define __NAMESPACE_FileIO_END__ } // namespace FileIO



__NAMESPACE_BaseIO_BEGIN__
int main() {
	int n;
	float f;
	char str[100];	
	scanf("%d%f%s", &n, &f, str);
	printf("%d\n%f\n%s\n", n, f, str);
	return 0;
}
__NAMESPACE_BaseIO_END__

__NAMESPACE_FileIO_BEGIN__
int main() {
    // 声明文件指针
    FILE *file;

    // 要写入文件的内容
    const char *text = "Hello, world!";
    size_t text_len = 14;  // 包含终止符 '\0'

    // 打开文件用于写入 ("w" 模式会清空已有文件内容)
    file = fopen("example.txt", "w");
    if (file == NULL) {
        perror("文件打开失败");
        return EXIT_FAILURE;
    }

    // 使用 fwrite 将文本写入文件
    // fwrite(要写入的内容, 每个元素的大小, 元素个数, 文件指针)
    size_t written = fwrite(text, sizeof(char), text_len, file);
    if (written != text_len) {
        perror("写入文件失败");
        fclose(file);
        return EXIT_FAILURE;
    }

    // 关闭文件
    if (fclose(file) != 0) {
        perror("文件关闭失败");
        return EXIT_FAILURE;
    }

    // 重新打开文件用于读取 ("r" 模式)
    file = fopen("example.txt", "r");
    if (file == NULL) {
        perror("文件重新打开失败");
        return EXIT_FAILURE;
    }

    // 创建一个缓冲区来存储读取到的数据
    char buffer[20];

    // 使用 fread 从文件中读取数据
    // fread(存储读取数据的缓冲区, 每个元素的大小, 读取的元素个数, 文件指针)
    size_t read = fread(buffer, sizeof(char), text_len, file);
    if (read != text_len) {
        if (feof(file)) {
            printf("已到达文件末尾。\n");
        } else {
            perror("读取文件失败");
            fclose(file);
            return EXIT_FAILURE;
        }
    }

    // 打印读取到的内容
    printf("读取的内容: %s\n", buffer);

    // 关闭文件
    if (fclose(file) != 0) {
        perror("文件关闭失败");
        return EXIT_FAILURE;
    }

    return 0;
}

__NAMESPACE_FileIO_END__

int main() {
	BaseIO::main();
	FileIO::main();
	return 0;
}