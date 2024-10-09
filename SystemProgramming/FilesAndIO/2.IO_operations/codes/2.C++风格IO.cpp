#include <iostream>
#include <fstream>
#include <string>

#define __NAMESPACE_BaseIO_BEGIN__ namespace BaseIO {
#define __NAMESPACE_BaseIO_END__ } // namespace BaseIO

#define __NAMESPACE_FileIO_BEGIN__ namespace FileIO {
#define __NAMESPACE_FileIO_END__ } // namespace FileIO


__NAMESPACE_BaseIO_BEGIN__

int main() {
	int n;
	float f;
	std::string str;
	std::cin >> n >> f >> str;
	std::cout << n << " " << f << " " << str << std::endl;
	return 0;
}

__NAMESPACE_BaseIO_END__

__NAMESPACE_FileIO_BEGIN__

int main() {
    // 要写入文件的内容
    std::string text = "Hello, world!";

    // 创建一个 ofstream 对象用于写入文件
    std::ofstream outFile("example.txt");
    
    // 检查文件是否成功打开
    if (!outFile.is_open()) {
        std::cerr << "文件打开失败" << std::endl;
        return EXIT_FAILURE;
    }

    // 写入内容到文件
    outFile << text;

    // 关闭文件
    outFile.close();

    // 创建一个 ifstream 对象用于读取文件
    std::ifstream inFile("example.txt");

    // 检查文件是否成功打开
    if (!inFile.is_open()) {
        std::cerr << "文件重新打开失败" << std::endl;
        return EXIT_FAILURE;
    }

    // 创建一个字符串用于存储读取到的数据
    std::string buffer;

    // 从文件中读取内容
    std::getline(inFile, buffer);

    // 检查读取是否成功
    if (inFile.fail() && !inFile.eof()) {
        std::cerr << "读取文件失败" << std::endl;
        inFile.close();
        return EXIT_FAILURE;
    }

    // 打印读取到的内容
    std::cout << "读取的内容: " << buffer << std::endl;

    // 关闭文件
    inFile.close();
    return 0;
}

__NAMESPACE_FileIO_END__

int main() {
	BaseIO::main();
	FileIO::main();
	return 0;
}