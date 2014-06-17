# SendTray makefile for MinGW
# Copyright (c) 2014 Renato Silva
# GNU GPLv2 licensed

outdir := distribution

all:
	mkdir -p "${outdir}/locale/pt_BR/LC_MESSAGES"
	msgfmt -c -o "${outdir}/locale/pt_BR/LC_MESSAGES/sendtray.mo" po/pt_BR.po

	gcc -c sendtrayhook.c
	gcc -shared -o "${outdir}/sendtray.dll" sendtrayhook.o -Wl,--out-implib,sendtray.a
	windres resource/sendtray.rc -O coff -o sendtray.res

	gcc -c sendtray.c -std=c99 -D_WIN32_IE=0x501 -DNIN_SELECT="(WM_USER+0)"
	gcc sendtray.o -o "${outdir}/sendtray.exe" -mwindows sendtray.res "${outdir}/sendtray.dll" /mingw/lib/libintl.a /mingw/lib/libiconv.a
	rm sendtray.res sendtray.a sendtray.o sendtrayhook.o
