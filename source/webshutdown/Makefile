outdir := .

defines = -DMG_DISABLE_HTTP_DIGEST_AUTH \
          -DMG_DISABLE_CGI \
          -DMG_DISABLE_DAV \
          -DMG_DISABLE_HTTP_WEBSOCKET \
          -DMG_DISABLE_DIRECTORY_LISTING \
          -DMG_DISABLE_FILESYSTEM

all: static

resource:
	windres webshutdown.rc -O coff -o webshutdown.res

static: resource
	gcc $(defines) webshutdown.res webshutdown.c -o "${outdir}/webshutdown.exe" -Wl,-Bstatic -lmongoose -lws2_32
	rm webshutdown.res

shared: resource
	gcc $(defines) webshutdown.res webshutdown.c -o "${outdir}/webshutdown-shared.exe" -lmongoose
	rm webshutdown.res
