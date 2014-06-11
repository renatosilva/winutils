outdir := .
all:
	windres resource/noteshider.rc -O coff -o noteshider.res
	gcc noteshider.res noteshider.c -std=c99 -Wl,--subsystem,windows -o "${outdir}/noteshider.exe"
	rm noteshider.res
