sudo apt update

sudo apt-get install build-essential make dpkg-dev

wget https://github.com/ModernN64SDK/n64sdkmod/archive/refs/heads/master.zip

unzip master.zip

cd n64sdkmod-master/

make

cd debs

sudo apt-get install ./binutils-mips-n64_2.39-1_amd64.deb 

sudo apt-get install ./gcc-mips-n64_12.2.0-1_amd64.deb 

sudo apt-get install ./newlib-mips-n64_4.3.0.20230120-3-1_amd64.deb 

sudo apt-get install ./n64sdk-common.deb 

sudo apt-get install ./n64sdk.deb 

sudo apt-get install ./makemask.deb 

