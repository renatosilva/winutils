# SendTray makefile for MinGW
# Copyright (c) 2014 Renato Silva
# GNU GPLv2 licensed

outdir := distribution

all:
	mkdir -p "${outdir}/locale/pt_BR/LC_MESSAGES"
	msgfmt -c -o "${outdir}/locale/pt_BR/LC_MESSAGES/sendtray.mo" po/pt_BR.po

	gcc -c sendtrayhook.c
	gcc -shared -o "${outdir}/sendtray.dll" sendtrayhook.o -Wl,--out-implib,sendtray.a
	windres -I. resource/sendtray.rc -O coff -o sendtray.res

	gcc -c sendtray.c -std=c99
	gcc sendtray.o -o "${outdir}/sendtray.exe" -mwindows sendtray.res "${outdir}/sendtray.dll" -lcomctl32 -lshlwapi -Wl,-Bstatic -lintl -liconv
	rm sendtray.res sendtray.a sendtray.o sendtrayhook.o
