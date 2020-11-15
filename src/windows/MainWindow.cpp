//
// Created by vasil on 02/11/2020.
//

#include "MainWindow.h"
#include <QVBoxLayout>
#include <QtGui/QtGui>
#include <QDockWidget>
#include <iostream>

#include "widgets/TerminalWidget.h"
#include "widgets/SerialPortSelectorWidget.h"
#include "widgets/DebuggingToolsWidget.h"

MainWindow::MainWindow()
{
    setWindowTitle("UMSATS Avionics Flight-Computer GUI Controller");

    auto terminal = new TerminalWidget();
    auto confWidget = new QWidget;
    auto confRoot = new QVBoxLayout;
    confWidget->setLayout(confRoot);
    auto serialPortConf = new SerialPortSelectorWidget();
    auto debuggingTools = new DebuggingToolsWidget();
    confRoot->addWidget(serialPortConf);
    confRoot->addWidget(debuggingTools);

    auto terminalDock = new QDockWidget(this);
    auto confDock = new QDockWidget(this);

    terminalDock->setWidget(terminal);
    terminalDock->setFloating(false);

    confDock->setWidget(confWidget);
    confDock->setFloating(false);

    addDockWidget(Qt::TopDockWidgetArea, confDock);
    addDockWidget(Qt::BottomDockWidgetArea, terminalDock);

    connect(serialPortConf, &SerialPortSelectorWidget::onDataReceived, [=](QByteArray data)
    {
        emit terminal->onBeginResponse(data);
        emit terminal->onEndResponse();
    });

    connect(terminal, &TerminalWidget::onRequest, [=](const QString & data)
    {
        serialPortConf->writeData(QByteArray::fromStdString(data.toStdString()));
    });

    connect(terminal, &TerminalWidget::resetRemoteTargetCommand, [=]
    {
        if(serialPortConf->isSerialPortOpen())
        {
            debuggingTools->resetRemoteTarget();
        }
        else
        {
            serialPortConf->emitSerialPortError();
        }
    });
}
