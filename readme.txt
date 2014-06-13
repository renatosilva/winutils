Screen Write 2014.4.25
Copyright (c) 2014 Renato Silva
GNU GPLv2 licensed

This program prints standard input to the screen. Output from other programs
can be redirected, and screen will keep displaying the current line. HTML is
supported by starting lines with the <html> tag. This can be used for
changing the default white text color.

Usage: screenwrite [initial text]


Development notes:
    * Automated JAR build requires Eclipse.
    * Automated Windows build requires Eclipse and the WinRun4J plug-in.
    * Manual build:
        1. Compile
        2. Run HelpBuilder
        3. Run Ant script
        4. Run WindowsBuilder
    * Windows icon by the Oxygen theme team.
