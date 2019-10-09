package com.higgs.draw;

import javax.imageio.ImageIO;
import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;
import java.io.IOException;

import static com.higgs.draw.Utils.EMPTY_STRING;
import static java.lang.Thread.sleep;

public class Draw {
    public static void main(final String[] args) {
        final Draw draw = new Draw();
    }

    private static final double iBezier = 0.001;

    private static final int _vScale = 600;

    private static final File _inputDir = new File(System.getProperty("user.dir") + "/vector/");

    private JFrame _frame;
    private JPanel _vectorSelectPanel;

    private JButton _loadComm;

    private File _activeFile;

    private ArduinoComLink _arduinoComLink;

    public Draw() {
        System.out.println(_inputDir.getAbsolutePath());
        _arduinoComLink = new ArduinoComLink("COM5", 9600, "uno");
        initUI();
    }

    private void initUI() {
        _frame = new JFrame("Vector Selector");

        _frame.setSize(new Dimension(1366, 768));

        final JPanel panel = new JPanel(new BorderLayout());

        _vectorSelectPanel = createVectorSelectorPanel();

        final JPanel buttonPanel = createButtonPanel();

        panel.add(_vectorSelectPanel, BorderLayout.CENTER);
        panel.add(buttonPanel, BorderLayout.SOUTH);

        _frame.pack();

        _frame.setEnabled(true);
        _frame.setVisible(true);
    }

    private JPanel createVectorSelectorPanel() {
        final JPanel panel = new JPanel(new BorderLayout(5, 10));

        final JPanel selectPanel = new JPanel(new GridLayout());

        final File[] inputFiles = _inputDir.listFiles();
        if(inputFiles != null && inputFiles.length > 0) {
            for(final File file : inputFiles) {
                if(file != null && file.getAbsolutePath().endsWith(".svg")) {
                    final JButton button = new JButton();
                    try {
                        final Image img = ImageIO.read(file);
                        button.setIcon(new ImageIcon(img));
                    } catch (final IOException ex) {
                        System.out.println("error reading vector image file");
                    }
                    button.addMouseListener(new MouseAdapter() {
                        public void mouseClicked(final MouseEvent e) {
                            super.mouseClicked(e);
                            _activeFile = file;
                        }
                    });
                    selectPanel.add(button);
                }
            }
        }

        final JScrollPane selectorScrollPane = new JScrollPane(selectPanel);

        panel.add(selectorScrollPane, BorderLayout.CENTER);

        return panel;
    }

    private JPanel createButtonPanel() {
        final JPanel panel = new JPanel(new BorderLayout());

        final JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.RIGHT));

        final JButton refresh = new JButton("Refresh");
        final JButton draw = new JButton("Draw");
        final JButton roam = new JButton("Roam");

        refresh.addMouseListener(new MouseAdapter() {
            public void mouseClicked(final MouseEvent e) {
                super.mouseClicked(e);
                _vectorSelectPanel = createVectorSelectorPanel();
            }
        });

        draw.addMouseListener(new MouseAdapter() {
            public void mouseClicked(final MouseEvent e) {
                super.mouseClicked(e);
                try {
                    _arduinoComLink.sendMessage("s delegate");
                } catch(final Exception ex) {
                    System.out.println("error telling arduino to delegate");
                    ex.printStackTrace();
                }
                drawSelected();
            }
        });

        roam.addMouseListener(new MouseAdapter() {
            public void mouseClicked(final MouseEvent e) {
                super.mouseClicked(e);
                roam();
            }
        });

        buttonPanel.add(refresh);
        buttonPanel.add(draw);
        buttonPanel.add(roam);

        final JPanel dataPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));

        final JTextField comField = new JTextField(5);
        final JTextField boardField = new JTextField(63);
        final JTextField baudrateField = new JTextField(10);

        _loadComm = new JButton("Load");
        _loadComm.addMouseListener(new MouseAdapter() {
            public void mouseClicked(final MouseEvent e) {
                super.mouseClicked(e);
                _arduinoComLink.setBoard(boardField.getText());
                boardField.setText(_arduinoComLink.getBoard());

                try {
                    _arduinoComLink.setBaudrate(Integer.parseInt(baudrateField.getText()));
                } catch(final NumberFormatException nfe) {
                    System.out.println("baudrate should be a positive integer");
                    nfe.printStackTrace();
                }
            }
        });

        dataPanel.add(new JLabel("Port:"));
        dataPanel.add(comField);
        dataPanel.add(new JLabel("Board:"));
        dataPanel.add(boardField);
        dataPanel.add(new JLabel("Baudrate:"));
        dataPanel.add(baudrateField);

        panel.add(dataPanel, BorderLayout.WEST);
        panel.add(buttonPanel, BorderLayout.EAST);

        return panel;
    }

    private void drawSelected() {
        drawFile(_activeFile);
    }

    private void drawFile(final File activeFile) {
        try {
            final String parsed = Utils.svgToStringArr(activeFile);
            if(!Utils.isEmpty(parsed)) {
                drawVector(parsed);
            }
        } catch(final IOException e) {
            System.out.println("error drawing vector");
            e.printStackTrace();
        }
    }

    private void drawVector(final String parsed) {
        String[] lines = parsed.split("\\r?\\n");
        lines = scale(lines, _vScale / largest(lines));
        drawVector(lines);
    }

    private void drawVector(final String[] commands) {
        for(final String command : commands) {
            final String[] arguments = command.split(" ");

            if(arguments.length > 0) {
                final String curCommand = arguments[0];

                if(curCommand != null) {
                    if("M".equalsIgnoreCase(curCommand)) {
                        sendMove((int)(Double.parseDouble(arguments[1])), (int)(Double.parseDouble(arguments[2])));
                    } else if("L".equalsIgnoreCase(curCommand)) {
                        final int delay = 25;

                        laserOn(true);
                        sleep(delay);
                        sendMove((int)(Double.parseDouble(arguments[1])), (int)(Double.parseDouble(arguments[2])));
                        sleep(delay);
                        laserOn(false);
                    } else if("C".equalsIgnoreCase(curCommand)) {
                        final Point p0 = new Point((int) Double.parseDouble(arguments[1]), (int) Double.parseDouble(arguments[2]));
                        final Point p1 = new Point((int) Double.parseDouble(arguments[3]), (int) Double.parseDouble(arguments[4]));
                        final Point p2 = new Point((int) Double.parseDouble(arguments[5]), (int) Double.parseDouble(arguments[6]));

                        laserOn(true);
                        for(double j = 0.0; j < 1.0; j += iBezier) {
                            sendMove((int)((p0.x - (2 * p1.x) + p2.x) * (Math.pow(j, 2)) + (2 * (p1.x - p0.x) * j) + (p0.x)),
                                     (int)((p0.y - (2 * p1.y) + p2.y) * (Math.pow(j, 2)) + (2 * (p1.y - p0.y) * j) + (p0.y)));
                        }
                        laserOn(false);
                    }
                    sleep(10);
                }
            }
        }
    }

    private void laserOn(final boolean onOrOff) {
        _loadComm.doClick();
        try {
            _arduinoComLink.sendMessage("l " + (onOrOff ? "on" : "off"));
        } catch(final Exception e) {
            System.out.println("error sending laser toggle message");
            e.printStackTrace();
        }
    }

    private void sendMove(final int x, final int z) {
        _loadComm.doClick();
        try {
            _arduinoComLink.sendMessage(String.format("m %d,%d", x, z));
        } catch(final Exception e) {
            System.out.println("error sending move message");
            e.printStackTrace();
        }
    }

    private void roam() {
        _loadComm.doClick();
        try {
            _arduinoComLink.sendMessage("s roam");
        } catch(final Exception e) {
            System.out.println("error sending switch message");
            e.printStackTrace();
        }
    }

    private double largest(final String[] lines) {
        double largest = 0;
        for(final String string : lines) {
            final String[] args = string.split(" ");
            if(!args[0].equalsIgnoreCase("z")) {
                for(int j = 1; j <  args.length; j += 2) {
                    final String s = args[j];
                    if(s != null) {
                        double xVal = Double.parseDouble(s);
                        if(xVal > largest) {
                            largest = xVal;
                        }
                    }
                }
            }
        }
        return largest;
    }

    private String[] scale(final String[] lines, double scale) {
        final String[] result = new String[lines.length];
        for(int i = 0; i < lines.length; i++) {
            final String[] args = lines[i].split(" ");
            final String command = args[0];
            if(!command.equalsIgnoreCase("z")) {
                for(int j = 1; j < args.length; j += 1)	{
                    final String s = args[j];
                    if(s != null) {
                        double val = Double.parseDouble(s);
                        val *= scale;
                        args[j] = Double.toString(val);
                    }
                }
            }
            for(final String arg : args) {
                if(arg != null) {
                    result[i] += arg + " ";
                    result[i] = result[i].replaceAll("null", EMPTY_STRING);
                }
            }
        }
        return result;
    }

    private void sleep(final long millis) {
        try { Thread.sleep(millis); } catch(InterruptedException ignored) {}
    }
}
