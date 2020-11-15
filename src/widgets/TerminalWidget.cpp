//
// Created by vasil on 02/11/2020.
//
#include "TerminalWidget.h"

#include <QtWidgets/QPushButton>
#include <QVBoxLayout>
#include <QSerialPort>
#include <iostream>
#include <QTextEdit>
#include <QSizePolicy>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QtCore/QTimer>
#include <QtCore/QtCore>
#include <QApplication>
#include <QClipboard>

TerminalWidget::TerminalWidget ( QWidget * parent):
QWidget(parent)
{
    auto root = new QVBoxLayout;

    mUser                   = "root";
    mCommandDelimiter       = ":~$ ";
    mCommandHistoryIndex    = 0;
    mConsoleText            = new MyTextEdit;

    mCustomSelection.format = QTextCharFormat();
    mCustomSelection.format.setBackground(Qt::blue);

    mConsoleText->setStyleSheet("QTextEdit { background: black; font-family: consolas; color: yellow; font-size: 14px }");
    mConsoleText->setContentsMargins(2, 0, 2, 2);
    mConsoleText->setText(mUser + mCommandDelimiter );
    mConsoleText->moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
    mConsoleText->setAcceptRichText(true);

    mCommandHistory.append("");
    mCommandHistoryIndex++;

    root->addWidget(mConsoleText);
    setLayout(root);


    connect(this, &TerminalWidget::onEnterPressed, [=](QKeyEvent * event)
    {
        auto text = mConsoleText->toPlainText();
        auto pos = text.lastIndexOf(mCommandDelimiter) + mCommandDelimiter.size();
        auto lastCommand = text.mid(pos);

        if(!lastCommand.isEmpty())
            mCommandHistory.last() = lastCommand;

        if(isKnownCommand(mCommandHistory.last()))
        {
            handleKnownCommands(mCommandHistory.last());
        }
        else
        {
            emit onRequest(mCommandHistory.last());
//            emit onBeginResponse("Command " + mCommandHistory.last() + " is not a recognized internal command.");
//            emit onEndResponse();
        }
    });

    connect(this, static_cast<void (TerminalWidget::*)(QByteArray)>(&TerminalWidget::onBeginResponse), this, [=](QByteArray data)
    {
        mConsoleText->setText(mConsoleText->toPlainText());
        mConsoleText->append(QString::fromStdString(data.toStdString()));
        mConsoleText->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    }, Qt::QueuedConnection);

    connect(this, static_cast<void (TerminalWidget::*)(const QString&)>(&TerminalWidget::onBeginResponse), this, [=](const QString & data)
    {
        mConsoleText->setText(mConsoleText->toPlainText());
        mConsoleText->append(data);
        mConsoleText->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    }, Qt::QueuedConnection);

    connect(this, &TerminalWidget::onEndResponse, this, [=](bool insertNewLine)
    {
        mConsoleText->setText(mConsoleText->toPlainText() + (insertNewLine ? "\n" : "") + mUser + mCommandDelimiter);
        mConsoleText->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);

        mCommandHistory.append("");
        mCommandHistoryIndex = mCommandHistory.size() - 1;

        mConsoleText->setReadOnly(false);
    }, Qt::QueuedConnection);

    connect(this, &TerminalWidget::onCommandInput, [=](QKeyEvent * event)
    {
        QTimer::singleShot(0, [=]
        {
            auto text = mConsoleText->toPlainText();
            auto pos = text.lastIndexOf(mCommandDelimiter) + mCommandDelimiter.size();

            mCommandHistory.last() = text.mid(pos);
        });
    });

    connect(mConsoleText, &MyTextEdit::onTextSelectionChanged, this, [=](int pos)
    {
        mCustomSelectionIndices.second = pos;
        mCustomSelection.cursor.setPosition(pos, QTextCursor::KeepAnchor);
        mConsoleText->setExtraSelections({mCustomSelection});
    });

    connect(mConsoleText, &MyTextEdit::onTextSelectionBegin, this, [=](int pos)
    {
        mCustomSelectionIndices.first = pos;
        mCustomSelection.cursor = QTextCursor(mConsoleText->document());
        mCustomSelection.cursor.setPosition(pos, QTextCursor::MoveAnchor);
    });

    connect(mConsoleText, &MyTextEdit::onTextSelectionEnd, this, [=](int pos)
    {
        mCustomSelectionIndices.second = pos;
        mCustomSelection.cursor.setPosition(pos, QTextCursor::KeepAnchor);
        mConsoleText->setExtraSelections({mCustomSelection});
    });

    installEventFilter(this);
    mConsoleText->installEventFilter(this);
    mConsoleText->setMouseTracking(false);
    qApp->installEventFilter(this);
}

bool TerminalWidget::isKnownCommand(const QString &command)
{
    if(command.isEmpty())
        return true;

    if(command == "test")
        return true;

    if(command == "clear")
        return true;

    if (command == "exit")
        return true;

    if (command == "reset")
        return true;

    return false;
}

void TerminalWidget::handleKnownCommands(const QString &command)
{
    if(command.isEmpty())
    {
        goToNextLine();
        return;
    }

    if(command == "test")
    {
        emit onEndResponse();
        return;
    }

    if(command == "clear")
    {
        mConsoleText->setText("");
        emit onEndResponse(false);
        return;
    }

    if (command == "exit")
    {
        QApplication::exit(0);
        emit onEndResponse();
        return;
    }

    if (command == "reset")
    {
        emit resetRemoteTargetCommand();
        return;
    }
}

void TerminalWidget::goToNextLine()
{
    mConsoleText->setText(mConsoleText->toPlainText() + '\n' + mUser + mCommandDelimiter);
    mConsoleText->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    mCommandHistoryIndex = mCommandHistory.size() - 1;
    mConsoleText->setReadOnly(false);
}

bool TerminalWidget::eventFilter(QObject *receiver, QEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
        // leave untouched to be received up by MyTextEdit::mouseMoveEvent
        event->ignore();
    }

    if (event->type() == QEvent::KeyPress)
    {
        // we are interested only in the LineEdit's events
        if (receiver == mConsoleText)
        {
            return handleKeyPressEvent(event);
        }
    }
    else if(event->type() == QEvent::KeyRelease)
    {
        // we would need remove the key if it is present in the set
        mPressedKeys -= ((QKeyEvent*)event)->key();
    }

    return QObject::eventFilter(receiver, event);
}


bool TerminalWidget::handleKeyPressEvent(QEvent *event)
{
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    mPressedKeys += keyEvent->key();

    if( mPressedKeys.contains(Qt::Key_Control) && mPressedKeys.contains(Qt::Key_Shift) &&  mPressedKeys.contains(Qt::Key_C))
    {
        // Ctrl + Shift + C combination is pressed
        copyCurrentSelectionToTheClipBoard();
        return true;
    }

    if (mPressedKeys.contains(Qt::Key_Return))
    {
        // one of both ENTER keys was pressed;
        emit onEnterPressed(keyEvent);
        return true;
    }

    if(mPressedKeys.contains(keyEvent->key() == Qt::Key_Enter))
    {
        // no side Enter is allowed
        return false;
    }

    if (mPressedKeys.contains(Qt::Key_Backspace))
    {
        return handleBackspaceKeyEvent(event);
    }

    if (mPressedKeys.contains(Qt::Key_Up))
    {
        handleUpKeyEvent();
        // do not allow this key to be processed by QLineEdit internal logic
        return true;
    }

    if (mPressedKeys.contains(Qt::Key_Down))
    {
        handleDownKeyEvent();
        // do not allow this key to be processed by QLineEdit internal logic
        return true;
    }

    if (mPressedKeys.contains(Qt::Key_Left))
    {
        return handleLeftKeyEvent(event);
    }

    if (mPressedKeys.contains(Qt::Key_Right))
    {
        return handleRightKeyEvent(event);
    }

    if (keyEvent->key() >= Qt::Key_Space && keyEvent->key() <= Qt::Key_AsciiTilde)
    {
        // propagate the valid symbols ONLY
        emit onCommandInput(keyEvent);
        return QObject::eventFilter(mConsoleText, event);
    }

    return false;
}


void TerminalWidget::handleDownKeyEvent()
{
    auto text = mConsoleText->toPlainText();
    auto pos = text.lastIndexOf (mCommandDelimiter) + mCommandDelimiter.size();

    if ( mCommandHistoryIndex + 1 < mCommandHistory.size() )
    {
        mCommandHistoryIndex++;
        mConsoleText->setText (text.mid (0, pos ) + mCommandHistory.at (mCommandHistoryIndex ) ) ;
    }

    mConsoleText->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
}

void TerminalWidget::handleUpKeyEvent()
{
    auto text = mConsoleText->toPlainText();
    auto pos = text.lastIndexOf (mCommandDelimiter) + mCommandDelimiter.size() ;

    if ( mCommandHistoryIndex - 1 >= 0 )
    {
        mCommandHistoryIndex--;
        mConsoleText->setText (text.mid (0, pos ) + mCommandHistory.at (mCommandHistoryIndex ) ) ;
    }

    mConsoleText->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
}


bool TerminalWidget::handleLeftKeyEvent(QEvent *event)
{
    auto text = mConsoleText->toPlainText();
    if(text.mid(mConsoleText->textCursor().position() - mCommandDelimiter.size(), mCommandDelimiter.size()) != mCommandDelimiter)
    {
        // allow stepping back until the start of mCommandDelimiter ONLY
        return QObject::eventFilter(mConsoleText, event);
    }
    else
    {
        return true;
    }
}


bool TerminalWidget::handleBackspaceKeyEvent(QEvent *event)
{
    auto text = mConsoleText->toPlainText();
    qDebug() << text;
    if(text.mid(text.size() - mCommandDelimiter.size()) != mCommandDelimiter)
    {
        // allow removing characters till the end of the current line ONLY
        return QObject::eventFilter(mConsoleText, event);
    }
    else
    {
        return true;
    }
}

bool TerminalWidget::handleRightKeyEvent(QEvent *event)
{
    auto text = mConsoleText->toPlainText();
    if(mConsoleText->textCursor().position() < text.size())
    {
        // allow stepping forward till the end of the current line ONLY
        return QObject::eventFilter(mConsoleText, event);
    }
    else
    {
        return true;
    }
}

void TerminalWidget::copyCurrentSelectionToTheClipBoard()
{
    auto startPos =  qMin(mCustomSelectionIndices.first, mCustomSelectionIndices.second);
    auto nPos = qMax(mCustomSelectionIndices.first, mCustomSelectionIndices.second) - startPos;
    auto text4Clipboard = mConsoleText->toPlainText().mid(startPos, nPos);
    QApplication::clipboard()->setText(text4Clipboard);
}


void MyTextEdit::mousePressEvent(QMouseEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        if(event->button() == Qt::LeftButton)
        {
            isSelectionEnabled = true;
            emit onTextSelectionBegin(cursorForPosition(mapFromGlobal(QCursor::pos())).position());
        }
    }

    if (event->type() == QEvent::MouseButtonRelease)
    {
        if(event->button() == Qt::LeftButton)
        {
            isSelectionEnabled = false;
            emit onTextSelectionEnd(cursorForPosition(mapFromGlobal(QCursor::pos())).position());
        }
    }

    if (event->type() == QEvent::MouseButtonDblClick)
    {
        return;
    }

    QWidget::mousePressEvent(event);
}


void MyTextEdit::mouseMoveEvent(QMouseEvent *event)
{
    if(isSelectionEnabled)
    {
        emit onTextSelectionChanged(cursorForPosition(mapFromGlobal(QCursor::pos())).position());
    }

    QTextEdit::mouseMoveEvent(event);
}

