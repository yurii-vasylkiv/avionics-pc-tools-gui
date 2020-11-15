//
// Created by vasil on 13/11/2020.
//

#ifndef AVIONICS_PC_TOOLS_GUI_SERIALPORTSELECTORWIDGET_H
#define AVIONICS_PC_TOOLS_GUI_SERIALPORTSELECTORWIDGET_H

#include <QGroupBox>
#include <QSerialPort>

class SerialPortSelectorWidget: public QGroupBox
{
    Q_OBJECT
public:
    SerialPortSelectorWidget(QWidget *parent = nullptr);
    ~SerialPortSelectorWidget() override;
    bool isSerialPortOpen() const;
    void emitSerialPortError();

signals:
    void onDataReceived(QByteArray data);
public slots:
    void writeData(const QByteArray & data);
private:
    QSerialPort mSerialBackend;
    std::string mBuffer;
    bool mIsBufferConsumerRunning = false;
};


#endif //UNTITLED1_SERIALPORTSELECTORWIDGET_H
