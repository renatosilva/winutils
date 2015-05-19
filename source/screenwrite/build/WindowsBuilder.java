/*
 * Windows Builder 2014.4.25
 * Copyright (c) 2014 Renato Silva
 * GNU GPLv2 licensed
 */

import java.io.File;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * This class creates a Windows executable using the WinRun4J Eclipse plug-in.
 * It is meant to be executed as a project builder under Eclipse, but should
 * work from console as well.
 */
public class WindowsBuilder {

    private static final String WINRUN4J_PLUGIN_ID = "org.boris.winrun4j.eclipse";

    private static void copyFile(File source, File target) throws IOException {
        InputStream input = new FileInputStream(source);
        OutputStream output = new FileOutputStream(target);
        int nextByte;
        while ((nextByte = input.read()) != -1)
            output.write(nextByte);
        input.close();
        output.close();
    }

    private static String findEclipsePlugin(String eclipseHome, final String pluginId) {
        String eclipsePlugins = String.format("%s/plugins", eclipseHome);
        File[] pluginVersions = new File(eclipsePlugins).listFiles(new FileFilter() {
            @Override
            public boolean accept(File file) {
                return file.isDirectory() && file.getName().matches(String.format("%s[^\\\\/]*", pluginId));
            }
        });
        if (pluginVersions.length < 1)
            return null;
        return pluginVersions[pluginVersions.length - 1].getAbsolutePath();
    }

    private static void execute(String... command) throws IOException, InterruptedException {
        if (new ProcessBuilder(command).inheritIO().start().waitFor() != 0)
            throw new IOException(String.format("Build error at %s", command.toString()));
    }

    private static void build(String eclipseHome, String iconPath, String iniPath, String jarPath, String executablePath) throws IOException, InterruptedException {
        String winrun4jHome = findEclipsePlugin(eclipseHome, WINRUN4J_PLUGIN_ID);
        String rcedit = String.format("%s/launcher/rcedit.exe", winrun4jHome);
        String winrun4j = String.format("%s/launcher/winrun4j.exe", winrun4jHome);

        copyFile(new File(winrun4j), new File(executablePath));
        execute(rcedit, "/C", executablePath);
        execute(rcedit, "/N", executablePath, iniPath);
        execute(rcedit, "/I", executablePath, iconPath);
        execute(rcedit, "/J", executablePath, jarPath);
    }

    public static void main(String[] arguments) throws IOException, InterruptedException {
        String help = String.format("Usage: %s <eclipse_home> <icon> <winrun4j_ini> <jar> <executable>", WindowsBuilder.class.getName());
        new AutoHelp(AutoHelp.IS_CONSOLE, help).parse(arguments, 5);
        build(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
    }

}
