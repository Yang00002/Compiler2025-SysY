wget https://www.antlr.org/download/antlr4-cpp-runtime-4.13.2-source.zip
unzip antlr4-cpp-runtime-4.13.2-source.zip -d antlr4-cpp-runtime-4.13.2
sudo apt update
sudo apt install cmake uuid-dev pkg-config
cd antlr4-cpp-runtime-4.13.2/
mkdir build && mkdir run && cd build
cmake ..
make install DESTDIR=../run
cd ../run/usr/local/include
sudo cp -r antlr4-runtime/* /usr/local/include
cd ../lib
sudo cp * /usr/local/lib
sudo ldconfig
cd ../../../../../
rm -rf antlr4-cpp-runtime-4.13.2/
rm antlr4-cpp-runtime-4.13.2-source.zip