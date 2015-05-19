outdir := .

all: static

resource:
	windres webshutdown.rc -O coff -o webshutdown.res

static: resource
	gcc webshutdown.res webshutdown.c -o "${outdir}/webshutdown.exe" -Wl,-Bstatic -lmongoose -lws2_32
	rm webshutdown.res

shared: resource
	gcc webshutdown.res webshutdown.c -o "${outdir}/webshutdown-shared.exe" -lmongoose
	rm webshutdown.res
