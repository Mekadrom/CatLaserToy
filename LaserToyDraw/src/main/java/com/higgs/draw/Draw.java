package com.higgs.draw;

import javax.swing.*;
import java.awt.*;
import java.io.File;

public class Draw {
    public static void main(final String[] args) {
        final Draw draw = new Draw();
    }

    private static final File _inputDir = new File(System.getProperty("user.dir") + "/vector/");
    private JFrame _frame;
    private JPanel _vectorSelectPanel;

    private File _activeFile;

    public Draw() {
        System.out.println(_inputDir.getAbsolutePath());
        initUI();
    }

    private void initUI() {
        _frame = new JFrame("Vector Selector");

        _frame.setSize(new Dimension(1366, 768));

        _vectorSelectPanel = createVectorSelectorPanel();
        final JPanel buttonPanel = createButtonPanel();

        _frame.pack();

        _frame.setEnabled(true);
        _frame.setVisible(true);
    }

    private JPanel createVectorSelectorPanel() {
        final JPanel panel = new JPanel(new BorderLayout());

        final JPanel selectPanel = new JPanel(new GridLayout());

        final JScrollPane selectorScrollPane = new JScrollPane(selectPanel);

        final File[] inputFiles = _inputDir.listFiles();
        if(inputFiles != null && inputFiles.length > 0) {
            for(final File file : inputFiles) {
                if(file != null && file.getAbsolutePath().endsWith(".svg")) {
                    final JButton button = new JButton();
                }
            }
        }

        panel.add(selectorScrollPane, BorderLayout.CENTER);

        return panel;
    }

    private JPanel createButtonPanel() {
        return null;
    }
}
