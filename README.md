## 依赖

- pkgconf  (用于获取安装库的路径)
- CLI11 (用于解析命令行参数)
- tbb   (Intel 开发的并行库)

macos 系统使用 brew 进行库的安装：

```bash
brew install pkgconf
brew install cli11
brew install tbb
```

Linux 可以使用 apt 进行安装，Windows 也许需要进行一些环境变量配置。

## 编译

请将代码与数据的路径如下组织：

```
├── code
│   ├── CMakeLists.txt
│   ├── README.md
│   └── src
├── data
    ├── document_retrieval
    └── software_antivirus
```

使用 cmake 进行构建：

```bash
cd code
cmake -B build -S .
cmake --build build
```

在编译完成后程序的路径为 `code/build/src/match`。

## 使用

可选 3 个参数：
- `-d` 数据所在目录
- `-t` 线程数
- `-o` 输出目录

本地测试的运行结果：

```bash
$ ./code/build/src/match -d ./data -t 8 -o ./result
==========Task 1 Begin==========
Took: 689ms
===========Task 1 End===========

==========Task 2 Begin==========
Took: 171ms
===========Task 2 End===========
```