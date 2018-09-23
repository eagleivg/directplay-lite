@ECHO OFF

setlocal EnableDelayedExpansion

REM .obj files to be compiled from .cc source files
SET CC_OBJS=^
 googletest/src/gtest-all.obj^
 googletest/src/gtest_main.obj

REM .obj files to be compiled from .cpp source files
SET CPP_OBJS=^
 hookdll/hookdll.obj^
 src/AsyncHandleAllocator.obj^
 src/COMAPIException.obj^
 src/DirectPlay8Address.obj^
 src/DirectPlay8Peer.obj^
 src/EventObject.obj^
 src/HandleHandlingPool.obj^
 src/HostEnumerator.obj^
 src/network.obj^
 src/packet.obj^
 src/SendQueue.obj^
 tests/DirectPlay8Address.obj^
 tests/DirectPlay8Peer.obj^
 tests/HandleHandlingPool.obj^
 tests/PacketDeserialiser.obj^
 tests/PacketSerialiser.obj

REM .obj files to be compiled from .c source files
SET C_OBJS=^
 minhook/src/buffer.obj^
 minhook/src/hde/hde32.obj^
 minhook/src/hde/hde64.obj^
 minhook/src/hook.obj^
 minhook/src/trampoline.obj

REM .obj files which will be linked into all-tests.exe
SET TEST_OBJS=^
 googletest/src/gtest-all.obj^
 googletest/src/gtest_main.obj^
 src/AsyncHandleAllocator.obj^
 src/COMAPIException.obj^
 src/DirectPlay8Address.obj^
 src/DirectPlay8Peer.obj^
 src/EventObject.obj^
 src/HandleHandlingPool.obj^
 src/HostEnumerator.obj^
 src/network.obj^
 src/packet.obj^
 src/SendQueue.obj^
 tests/DirectPlay8Address.obj^
 tests/DirectPlay8Peer.obj^
 tests/HandleHandlingPool.obj^
 tests/PacketDeserialiser.obj^
 tests/PacketSerialiser.obj

SET TEST_LIBS=ws2_32.lib dxguid.lib ole32.lib

SET HOOK_DLLS=ddraw

SET HOOK_OBJS=^
 hookdll/hookdll.obj^
 minhook/src/buffer.obj^
 minhook/src/hde/hde32.obj^
 minhook/src/hde/hde64.obj^
 minhook/src/hook.obj^
 minhook/src/trampoline.obj^
 src/AsyncHandleAllocator.obj^
 src/COMAPIException.obj^
 src/DirectPlay8Address.obj^
 src/DirectPlay8Peer.obj^
 src/EventObject.obj^
 src/HandleHandlingPool.obj^
 src/HostEnumerator.obj^
 src/network.obj^
 src/packet.obj^
 src/SendQueue.obj

SET HOOK_LIBS=ws2_32.lib dxguid.lib ole32.lib

SET CFLAGS=^
 /Zi^
 /EHsc^
 /DNOMINMAX^
 /I"C:\Program Files (x86)\Microsoft DirectX 9.0 SDK (October 2004)\Include"^
 /I"minhook/include"^
 /I"googletest/include"^
 /I"googletest"

FOR %%o IN (%CC_OBJS%) DO (
	set obj=%%o
	echo ==
	echo == cl /c %CFLAGS% /Fo%%o !obj:~0,-4!.cc
	echo ==
	        cl /c %CFLAGS% /Fo%%o !obj:~0,-4!.cc || exit /b
	echo:
)

FOR %%o IN (%CPP_OBJS%) DO (
	set obj=%%o
	echo ==
	echo == cl /c %CFLAGS% /Fo%%o !obj:~0,-4!.cpp
	echo ==
	        cl /c %CFLAGS% /Fo%%o !obj:~0,-4!.cpp || exit /b
	echo:
)

FOR %%o IN (%C_OBJS%) DO (
	set obj=%%o
	echo ==
	echo == cl /c %CFLAGS% /Fo%%o !obj:~0,-4!.c
	echo ==
	        cl /c %CFLAGS% /Fo%%o !obj:~0,-4!.c || exit /b
	echo:
)

echo ==
echo == link /debug /out:tests/all-tests.exe %TEST_OBJS% %TEST_LIBS%
echo ==
        link /debug /out:tests/all-tests.exe %TEST_OBJS% %TEST_LIBS% || exit /b
echo:

FOR %%o IN (%HOOK_DLLS%) DO (
	echo ==
	echo == ml /c /Cx /coff /Fo hookdll/%%o.obj hookdll/%%o.asm
	echo ==
	        ml /c /Cx /coff /Fo hookdll/%%o.obj hookdll/%%o.asm || exit /b
	echo:
	
	echo ==
	echo == link /dll /out:hookdll/%%o.dll /def:hookdll/%%o.def hookdll/%%o.obj %HOOK_OBJS% %HOOK_LIBS%
	echo ==
	        link /dll /out:hookdll/%%o.dll /def:hookdll/%%o.def hookdll/%%o.obj %HOOK_OBJS% %HOOK_LIBS% || exit /b
	echo:
)
