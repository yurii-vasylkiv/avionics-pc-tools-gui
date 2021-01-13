//
// Created by vasil on 13/11/2020.
//

#include "SerialPortSelectorWidget.h"
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>

#include <QTimer>
#include <QSet>
#include <QtCore/QThreadPool>
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <iostream>
#include <QGridLayout>
#include <QDebug>

SerialPortSelectorWidget::SerialPortSelectorWidget(QWidget *parent): QGroupBox(parent)
{
    setTitle("Serial Port:");

    auto root = new QHBoxLayout;
    auto left_root = new QGridLayout;
    auto right_root = new QGridLayout;

    auto portLbl = new QLabel("Port:");
    auto portCmb = new QComboBox();
    left_root->addWidget(portLbl, 0, 0);
    left_root->addWidget(portCmb, 0 ,1);

    auto descriptionLbl = new QLabel("Description:");
    auto descriptionVLbl = new QLabel();

    left_root->addWidget(descriptionLbl, 1, 0);
    left_root->addWidget(descriptionVLbl, 1, 1);

    auto manufacturerLbl = new QLabel("Manufacturer:");
    auto manufacturerVLbl = new QLabel();

    left_root->addWidget(manufacturerLbl, 2, 0);
    left_root->addWidget(manufacturerVLbl, 2, 1);

    auto serialNoLbl = new QLabel("Serial #:");
    auto serialNoVLbl = new QLabel();

    left_root->addWidget(serialNoLbl, 3, 0);
    left_root->addWidget(serialNoVLbl, 3 ,1);

    auto locationLbl = new QLabel("System Location:");
    auto locationVLbl = new QLabel();

    left_root->addWidget(locationLbl, 4, 0);
    left_root->addWidget(locationVLbl, 4 ,1);

    auto VIDLbl = new QLabel("VID:");
    auto VID_VLbl = new QLabel();

    left_root->addWidget(VIDLbl, 5 , 0);
    left_root->addWidget(VID_VLbl, 5, 1);

    auto PIDLbl = new QLabel("PID:");
    auto PID_VLbl = new QLabel();

    left_root->addWidget(PIDLbl, 6, 0);
    left_root->addWidget(PID_VLbl, 6, 1);

    left_root->setContentsMargins(10,10,10,10);


    auto baudRateLbl = new QLabel("Baud Rate:");
    auto baudRateCmb = new QComboBox();
    baudRateLbl->setMinimumWidth(200);
    baudRateCmb->setMinimumWidth(100);

    right_root->addWidget(baudRateLbl, 0 , 0);
    right_root->addWidget(baudRateCmb, 0, 1);

    auto bitsLbl = new QLabel("Bits:");
    auto bitsCmb = new QComboBox();
    bitsLbl->setMinimumWidth(200);
    bitsCmb->setMinimumWidth(100);

    right_root->addWidget(bitsLbl, 1, 0);
    right_root->addWidget(bitsCmb, 1, 1);

    auto parityLbl = new QLabel("Parity:");
    auto parityCmb = new QComboBox();
    parityLbl->setMinimumWidth(200);
    parityCmb->setMinimumWidth(100);

    right_root->addWidget(parityLbl, 2, 0);
    right_root->addWidget(parityCmb, 2, 1);

    auto stopBitsLbl = new QLabel("Stop Bits:");
    auto stopBitsCmb = new QComboBox();
    stopBitsLbl->setMinimumWidth(200);
    stopBitsCmb->setMinimumWidth(100);

    right_root->addWidget(stopBitsLbl, 3, 0);
    right_root->addWidget(stopBitsCmb, 3, 1);

    auto flowControlLbl = new QLabel("Flow Control:");
    auto flowControlCmb = new QComboBox();
    flowControlLbl->setMinimumWidth(200);
    flowControlCmb->setMinimumWidth(100);

    right_root->addWidget(flowControlLbl, 4, 0);
    right_root->addWidget(flowControlCmb, 4, 1);

    auto connectButton = new QPushButton("CONNECT");
    right_root->addWidget(connectButton, 5, 1);

    right_root->setContentsMargins(10,10,10,10);

    root->addLayout(left_root);
    root->addSpacing(10);
    root->addLayout(right_root);
    setLayout(root);

    adjustSize();

    connect(portCmb, &QComboBox::currentTextChanged, [=](const QString & port)
    {
        auto availablePorts = QSerialPortInfo::availablePorts();
        QStringList list;

        for (const auto & availablePort : availablePorts)
        {
            if (port == availablePort.portName())
            {
                descriptionVLbl->setText(availablePort.description());
                manufacturerVLbl->setText(availablePort.manufacturer());
                serialNoVLbl->setText(availablePort.serialNumber());
                locationVLbl->setText(availablePort.systemLocation());
                VID_VLbl->setText(availablePort.vendorIdentifier() ? QString::number(availablePort.vendorIdentifier(), 16) : "" );
                PID_VLbl->setText(availablePort.productIdentifier() ? QString::number(availablePort.productIdentifier(), 16) : "");
                break;
            }
        }
    });

    static auto enablePortConfigs = [=](bool enable)
    {
        baudRateCmb->setEnabled(enable);
        bitsCmb->setEnabled(enable);
        parityCmb->setEnabled(enable);
        stopBitsCmb->setEnabled(enable);
        flowControlCmb->setEnabled(enable);
        portCmb->setEnabled(enable);
    };

    static auto configurePorts = [=]
    {
        if(!mSerialBackend.isOpen())
        {
            mSerialBackend.setBaudRate(baudRateCmb->currentText().toInt(), QSerialPort::Direction::AllDirections);
            switch (bitsCmb->currentIndex())
            {
                case 0:
                    mSerialBackend.setDataBits(QSerialPort::DataBits::Data5);
                    break;
                case 1:
                    mSerialBackend.setDataBits(QSerialPort::DataBits::Data6);
                    break;
                case 2:
                    mSerialBackend.setDataBits(QSerialPort::DataBits::Data7);
                    break;
                case 3:
                    mSerialBackend.setDataBits(QSerialPort::DataBits::Data8);
                    break;
                default:
                    mSerialBackend.setDataBits(QSerialPort::DataBits::UnknownDataBits);
                    break;
            }

            switch (stopBitsCmb->currentIndex())
            {
                case 0:
                    mSerialBackend.setStopBits(QSerialPort::StopBits::OneStop);
                    break;
                case 1:
                    mSerialBackend.setStopBits(QSerialPort::StopBits::OneAndHalfStop);
                    break;
                case 2:
                    mSerialBackend.setStopBits(QSerialPort::StopBits::TwoStop);
                    break;
                default:
                    mSerialBackend.setStopBits(QSerialPort::StopBits::UnknownStopBits);
                    break;
            }

            switch (parityCmb->currentIndex())
            {
                case 0:
                    mSerialBackend.setParity(QSerialPort::NoParity);
                    break;
                case 1:
                    mSerialBackend.setParity(QSerialPort::EvenParity);
                    break;
                case 2:
                    mSerialBackend.setParity(QSerialPort::OddParity);
                    break;
                case 3:
                    mSerialBackend.setParity(QSerialPort::SpaceParity);
                    break;
                case 4:
                    mSerialBackend.setParity(QSerialPort::MarkParity);
                    break;
                default:
                    mSerialBackend.setParity(QSerialPort::Parity::UnknownParity);
                    break;
            }

            switch (flowControlCmb->currentIndex())
            {
                case 0:
                    mSerialBackend.setFlowControl(QSerialPort::FlowControl::NoFlowControl);
                    break;
                case 1:
                    mSerialBackend.setFlowControl(QSerialPort::FlowControl::HardwareControl);
                    break;
                case 2:
                    mSerialBackend.setFlowControl(QSerialPort::FlowControl::SoftwareControl);
                    break;
                default:
                    mSerialBackend.setFlowControl(QSerialPort::FlowControl::UnknownFlowControl);
                    break;
            }
        }
    };

    connect(connectButton, &QPushButton::released, [=]
    {
        if(!mSerialBackend.isOpen())
        {
            mSerialBackend.setPortName(portCmb->currentText());

            configurePorts();

            if (mSerialBackend.open(QIODevice::ReadWrite))
            {
                connectButton->setText("DISCONNECT");
                enablePortConfigs(false);
            }
        }
        else
        {
            mSerialBackend.close();
            if(!mSerialBackend.isOpen())
            {
                connectButton->setText("CONNECT");
                enablePortConfigs(true);
            }
        }
    });


    connect(&mSerialBackend, &QIODevice::readyRead, [=]
    {
        while(mSerialBackend.canReadLine())
        {
            auto line = mSerialBackend.readLine().toStdString();
            mBuffer += line;
        }
    });

    QThreadPool::globalInstance()->start([=]
    {
        mIsBufferConsumerRunning = true;
        while(mIsBufferConsumerRunning)
        {
            QThread::sleep(1);
            if(!mBuffer.empty())
            {
                emit onDataReceived(QByteArray::fromStdString(mBuffer));
                mBuffer.clear();
            }
        }
    });

    for (auto const & mother : QSerialPortInfo::availablePorts())
        portCmb->addItem(mother.portName());

    baudRateCmb->addItems({"75", "110", "134", "150", "300", "600", "1200", "1800", "2400", "4800", "7200", "9600", "14400", "19200", "38400", "57600", "115200", "128000"});
    baudRateCmb->setCurrentIndex(16);

    bitsCmb->addItems({"5", "6", "7", "8"});
    bitsCmb->setCurrentIndex(3);

    parityCmb->addItems({"None", "Even", "Odd", "Space", "Mark"});
    parityCmb->setCurrentIndex(0);

    stopBitsCmb->addItems({"1", "1.5", "2"});
    stopBitsCmb->setCurrentIndex(0);

    flowControlCmb->addItems({"None", "Hardware", "Software"});
    flowControlCmb->setCurrentIndex(0);





}

void SerialPortSelectorWidget::writeData(const QByteArray &data)
{
    if(isSerialPortOpen())
    {
        for (auto i = 0 ; i < data.size(); i++)
        {
            std::string sym(1, data[i]);
            mSerialBackend.write(sym.c_str());
            QThread::msleep(1);
        }
        mSerialBackend.write("\r");
        mSerialBackend.flush();
    }
    else
    {
        emitSerialPortError();
    }
}

SerialPortSelectorWidget::~SerialPortSelectorWidget()
{
    mIsBufferConsumerRunning = false;
}

bool SerialPortSelectorWidget::isSerialPortOpen() const
{
    return mSerialBackend.isOpen();
}

void SerialPortSelectorWidget::emitSerialPortError()
{
    emit onDataReceived(QByteArray::fromStdString("Error: No serial port is open! Abort."), true);
}



