//
// Created by vasil on 13/11/2020.
//

#ifndef AVIONICS_PC_TOOLS_GUI_SERIALPORTSELECTORWIDGET_H
#define AVIONICS_PC_TOOLS_GUI_SERIALPORTSELECTORWIDGET_H

#include <QGroupBox>
#include <QRunnable>
#include <memory>
#include "serial/serial.h"


class Worker : public QObject, public QRunnable
{
    Q_OBJECT
public:
    Worker ( serial::Serial & serialBackend )
            : isRunning ( false ), mSerialBackend ( serialBackend ) { }

    void run();
public:
    using QObject::QObject;
    Q_SLOT void stop() { isRunning = false; }
    Q_SIGNAL void finished();
    Q_SIGNAL void onDataReady ( const QString & data );
private:
    bool isRunning;
    serial::Serial & mSerialBackend;
    std::string mBuffer;
};



class SerialPortSelectorWidget: public QGroupBox
{
    Q_OBJECT
public:
    SerialPortSelectorWidget(QWidget *parent = nullptr);
    ~SerialPortSelectorWidget() override;
    bool isSerialPortOpen() const;
    void emitSerialPortError();

signals:
    void onDataReceived(QByteArray data, bool isError = false);
public slots:
    void writeData(const QByteArray & data);
private:
    serial::Serial mSerialBackend;
    std::string mBuffer;
    Worker * mWorker;
//    bool isDataReady;
//    bool mIsBufferConsumerRunning = false;
};


#endif //UNTITLED1_SERIALPORTSELECTORWIDGET_H
