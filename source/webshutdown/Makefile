outdir := .

defines = -DMONGOOSE_NO_AUTH \
          -DMONGOOSE_NO_CGI \
          -DMONGOOSE_NO_DAV \
          -DMONGOOSE_NO_WEBSOCKET \
          -DMONGOOSE_NO_DIRECTORY_LISTING \
          -DMONGOOSE_NO_FILESYSTEM \
          -DMONGOOSE_NO_USER

all: static

resource:
	windres webshutdown.rc -O coff -o webshutdown.res

static: resource
	gcc $(defines) webshutdown.res webshutdown.c -o "${outdir}/webshutdown.exe" -Wl,-Bstatic -lmongoose -lws2_32
	rm webshutdown.res

shared: resource
	gcc $(defines) webshutdown.res webshutdown.c -o "${outdir}/webshutdown-shared.exe" -lmongoose
	rm webshutdown.res
