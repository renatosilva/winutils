/*
 * Auto Help 2014.4.25
 * Copyright (c) 2014 Renato Silva
 * GNU GPLv2 licensed
 */

import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.util.Scanner;

import javax.swing.JOptionPane;

/**
 * A simple helper for handling the <code>--help</code> option of the
 * application, using the following approach. First, {@link #build(String)
 * the build method} should be called <i>at build time</i> in order to process
 * the {@value #README_FILE}} file, placed in the current working directory.
 * This file should contain the help text at the top, separated from the other
 * sections (such as build instructions or change logs) by a double line break.
 *
 * The idea is that the help text and the readme file usually share common
 * project information (version, copyright, license, description, usage,
 * options), and this tool allows it to be specified in one single place.
 *
 * Then, at run time, the object can be instantiated for then parsing the
 * command line arguments and possibly displaying the help text.
 * Console applications get the help text displayed in standard output, while
 * graphical applications get it displayed in a GUI dialog.
 */
public class AutoHelp {

    public static final boolean IS_GUI = true;
    public static final boolean IS_CONSOLE = false;
    private static final String README_FILE = "readme.txt";
    protected boolean graphicalApplication;
    protected String help;

    /**
     * Creates this helper for further parsing and displaying the help text.
     * @param applicationType non-null type of the the underlying application.
     * @param help a custom help text. No resource loading is performed and no
     *        build time step is required.
     */
    public AutoHelp(boolean graphicalApplication, String help) {
        this.graphicalApplication = graphicalApplication;
        this.help = help;
    }

    /**
     * Creates this helper for further parsing and displaying the help text.
     * @param applicationType non-null type of the the underlying application.
     */
    public AutoHelp(boolean graphicalApplication) {
        this(graphicalApplication, null);
        String resource = resourceName(graphicalApplication);
        this.help = load(this.getClass().getResourceAsStream(resource), "\\Z");
    }

    private static String resourceName(boolean graphicalApplication) {
        String format = graphicalApplication? "html" : "txt";
        return String.format("help.%s", format);
    }

    private static String toHTML(String textHelp) {
        return String.format("<html><i>%s</i></html>", textHelp
            .replaceAll("&", "&amp;")
            .replaceAll("<", "&lt;")
            .replaceAll(">", "&gt;")
            .replaceAll("\\r\\n", "<br>")
            .replaceAll("[\\r\\n]", "<br>"));
    }

    private static String load(InputStream source, String delimiter) {
        Scanner scanner = new Scanner(source);
        String help = scanner.useDelimiter(delimiter).next();
        scanner.close();
        return help;
    }

    private static void write(String targetDir, String fileName, String content) throws IOException {
        FileWriter writer = new FileWriter(String.format("%s/%s", targetDir, fileName));
        writer.write(content);
        writer.close();
    }

    private void showHelpAndExit(int exitStatus) {
        if (graphicalApplication)
            JOptionPane.showMessageDialog(null, help);
        else
            System.out.println(help);
        System.exit(exitStatus);
    }

    protected void showGraphicalDialog() {
        JOptionPane.showMessageDialog(null, help);
    }

    /**
     * This is meant to be called <i>at build time</i>.
     * @param binaryDir the same directory to where this class gets compiled.
     *        This is where the help text will get looked up from in run time.
     *        If the application is packaged as JAR, it must include the help
     *        files generated in this directory by this method.
     * @throws IOException
     */
    public static void build(String binaryDir) throws IOException {
        String textHelp = load(new FileInputStream(README_FILE), "((\\r\\n){3}|\\r{3}|\\n{3}|\\Z)");
        write(binaryDir, resourceName(IS_CONSOLE), textHelp);
        write(binaryDir, resourceName(IS_GUI), toHTML(textHelp));

    }

    /**
     * Seeks for the <code>--help</code> or <code>-h</code> option in the
     * provided arguments, and when found, displays the help text according to the
     * application type (graphical dialog or console output), then exits the
     * application. Returns normally otherwise.
     *
     * @param arguments the arguments provided to the underlying application.
     * @param minimumArguments this allows for the application to show help text
     *        also when not enough arguments have been given. A zero value will
     *        allow any count (help will get displayed only if specified).
     */
    public void parse(String[] arguments, int minimumArguments) {
        for (String argument : arguments) {
            if (!argument.matches("(--help|-h)"))
                continue;
            showHelpAndExit(0);
        }
        if (arguments.length < minimumArguments)
            showHelpAndExit(1);
    }

    public static void main(String[] arguments) throws IOException {
        String help = String.format("Usage: %s <binary_dir>", AutoHelp.class.getName());
        new AutoHelp(AutoHelp.IS_CONSOLE, help).parse(arguments, 1);
        build(arguments[0]);
    }

}
