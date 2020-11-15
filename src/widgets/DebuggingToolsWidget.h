//
// Created by vasil on 15/11/2020.
//

#ifndef AVIONICS_PC_TOOLS_GUI_DEBUGGINGTOOLSWIDGET_H
#define AVIONICS_PC_TOOLS_GUI_DEBUGGINGTOOLSWIDGET_H

#include <QGroupBox>

class QLineEdit;
class DebuggingToolsWidget : public QGroupBox
{
    Q_OBJECT
public:
    DebuggingToolsWidget(QWidget *parent = nullptr);
    void resetRemoteTarget();
signals:
    void openOCDExecutableFileChanged(const QString & path);
    void openOCDBoardConfigFileChanged(const QString & path);
    void armGDBChanged(const QString & path);

private:
    void detectOpenOCDExecutableFile();
    void detectOpenOCDBoardConfigFile();
    void detectArmGdbExecutableFile();

    QLineEdit * mArmGDBTextEdit;
    QLineEdit * mOpenOCDExecTextEdit;
    QLineEdit * mOpenOCDBoardConfigTextEdit;
};


#endif //UNTITLED1_DEBUGGINGTOOLSWIDGET_H
