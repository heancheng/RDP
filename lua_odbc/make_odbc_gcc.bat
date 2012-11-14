gcc -c -I..\..\lua-5.2.0\src -I. common.c
gcc -c -I..\..\lua-5.2.0\src -I. main.c
gcc -c -I..\..\lua-5.2.0\src -I. connection.c
gcc -c -I..\..\lua-5.2.0\src -I. statement.c
dllwrap -oluaodbc.dll --def lua.def main.o common.o connection.o statement.o ..\..\lua-5.2.0\lua52.dll  -lodbc32
del *.o