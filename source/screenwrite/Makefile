outdir := .
all:
	windres -I. resource/screenwrite.rc -O coff -o screenwrite.res
	gcc -g screenwrite.res screenwrite.c -std=c99 -Wl,--subsystem,windows -lcomctl32 -lgdi32 -o "${outdir}/screenwrite.exe"
	rm screenwrite.res
