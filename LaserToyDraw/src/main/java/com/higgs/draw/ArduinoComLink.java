package com.higgs.draw;

import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;

public class ArduinoComLink {
    private String _port;
    private int _baudrate;
    private String _board;

    public ArduinoComLink(final String port, final int baudrate, final String board) {
        setPort(port);
        setBaudrate(baudrate);
        setBoard(board);
    }

    public void sendMessage(final String mes) throws Exception {
        final CommPortIdentifier portIdentifier = CommPortIdentifier.getPortIdentifier(_port.toUpperCase());
        if(portIdentifier != null) {
            final SerialPort serialPort = (SerialPort)portIdentifier.open(_board, 0);
            if(serialPort != null) {
                serialPort.setSerialPortParams(_baudrate, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);
                serialPort.getOutputStream().write(mes.getBytes());
            }
        }
    }

    public void setPort(final String port) {
        _port = port;
    }

    public String getPort() {
        return _port;
    }

    public void setBaudrate(final int baudrate) {
        _baudrate = baudrate;
    }

    public int getBaudrate() {
        return _baudrate;
    }

    public void setBoard(final String board) {
        _board = getBoardName(board);
    }

    public String getBoard() {
        return _board;
    }

    private String getBoardName(final String board) {
        if("mega".equalsIgnoreCase(board)) {
            return "Arduino/Genuino Mega or Mega 2560";
        } else if("uno".equalsIgnoreCase(board)) {
            return "Arduino/Genuino Uno";
        } else {
            return board;
        }
    }
}
