# src
Cognitive Neuroimaging Unit's 3-dimensional image processing tools C++/C/Fortran source code

Fetch this src and also include files from  https://github.com/CNUHub/ such as:

mkdir /Users/johndoe/cnuhub/
cd /Users/johndoe/cnuhub/
git clone https://github.com/CNUHub/include
git clone https://github.com/CNUHub/src

To compile under a seperate build directory:

mkdir /Users/johndoe/cnuhub/build;
cd /Users/johndoe/cnuhub/build;
cmake -DCMAKE_INSTALL_PREFIX=/Users/johndoe/cnuhub/ -source /Users/johndoe/cnuhub/src
make;
make install;

this should install binaries in:
/Users/johndoe/cnuhub/bin

and psyimg library in:
/Users/johndoe/cnuhub/lib

