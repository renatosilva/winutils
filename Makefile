outdir := .
all:
	windres togglehidden.rc -O coff -o togglehidden.res
	gcc togglehidden.res togglehidden.c -std=c99 -Wl,--subsystem,windows -o "${outdir}/togglehidden.exe"
	rm togglehidden.res
