outdir := .
all:
	javac source/*.java build/*.java
	mv source/*.class build/*.class .
	java HelpBuilder .
	jar cvfe screenwrite.jar ScreenWrite ScreenWrite*.class AutoHelp.class help.*
	java WindowsBuilder "$(ECLIPSE_HOME)" build/screenwrite.ico build/winrun4j.ini screenwrite.jar "${outdir}/screenwrite.exe"
	rm -rf *.class *.jar help.*
