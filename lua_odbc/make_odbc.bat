cl -c /MT /Zi -I..\lua\include -I"D:\Program Files\Microsoft SDKs\Windows\v7.0\Include" -I. common.c
cl -c /MT /Zi -I..\lua\include -I"D:\Program Files\Microsoft SDKs\Windows\v7.0\Include" -I. main.c
cl -c /MT /Zi -I..\lua\include -I"D:\Program Files\Microsoft SDKs\Windows\v7.0\Include" -I. connection.c
cl -c /MT /Zi -I..\lua\include -I"D:\Program Files\Microsoft SDKs\Windows\v7.0\Include" -I. statement.c
link /dll  /def:lua.def /out:luaodbc.dll  main.obj common.obj connection.obj statement.obj ..\lua\lib\lua5.1.lib
@del *.obj