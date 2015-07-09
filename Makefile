MINGW_PREFIX = $(shell which gcc | awk -F/bin/ '{ printf $$1 }')
readme_style = body { font-family: "Segoe UI", Arial; color: \#555; margin: 20px auto; max-width: 800px; }

all: installer sources

programs:
	mkdir -p build/documents
	cp LICENSE build/documents
	cp -r $(MINGW_PREFIX)/share/licenses/libwinpthread build/documents
	mkd2html -header '<style type="text/css">$(readme_style)</style>' README.md
	mv README.html build/documents
	make -C source outdir=../../build

installer: programs
	gcc uninstall.c -Wl,--subsystem,windows -o build/uninstall.exe
	makensis winutils.nsi
	rm -rf build

sources:
	./makesource.sh
