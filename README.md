1、安装WasmEdge最新稳定版
curl -sSf https://raw.githubusercontent.com/WasmEdge/WasmEdge/master/utils/install.sh | bash
source ~/.bashrc
wasmedge --version

2、安装wasi-sdk
https://github.com/WebAssembly/wasi-sdk/releases
选择合适的版本
如果是Ubuntu可以直接下载deb二进制包，然后dpkg安装
sudo dpkg -i wasi-sdk-33.0-x86_64-linux.deb

3、安装工具包
sudo apt install clang-tools-21
