/*
 * Screen Write
 * Copyright (c) 2014 Renato Silva
 * See the readme file for more information
 */

import java.awt.Color;
import java.awt.Font;
import java.awt.GridLayout;
import java.io.BufferedReader;
import java.io.InputStreamReader;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;

public class ScreenWrite extends JFrame {

    private static final long serialVersionUID = 1L;
    private final JLabel messageLabel = new JLabel();

    private ScreenWrite() {
        super();
        setUndecorated(true);
        setBackground(new Color(0, 0, 0, 0));
        setExtendedState(JFrame.MAXIMIZED_BOTH);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLayout(new GridLayout(0, 1));

        messageLabel.setForeground(Color.WHITE);
        messageLabel.setFont(new Font("Segoe UI", Font.BOLD + Font.ITALIC, 36));
        messageLabel.setHorizontalAlignment(SwingConstants.CENTER);
        messageLabel.setVerticalAlignment(SwingConstants.BOTTOM);
        add(messageLabel);

        for (int i = 0; i < 3; i++) {
            JPanel panel = new JPanel();
            panel.setOpaque(false);
            add(panel);
        }
        setVisible(true);
    }

    private void updateScreen(final String text) {
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                messageLabel.setText(text);
            }
        });
    }

    public static void main(String[] arguments) throws Exception {
        UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        new AutoHelp(AutoHelp.IS_GUI).parse(arguments, 0);
        ScreenWrite application = new ScreenWrite();
        BufferedReader inputReader = new BufferedReader(new InputStreamReader(System.in));
        if (arguments.length > 0)
            application.updateScreen(arguments[0]);
        String line = null;
        while ((line = inputReader.readLine()) != null)
            application.updateScreen(line);
        application.dispose();

    }
}
