#include "mycodeeditor.h"
#include "myhighlighter.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>
#include <QTextStream>
//#include <QDebug>


MyCodeEditor::MyCodeEditor(QWidget *parent,QFont font) : QPlainTextEdit(parent)
{
    lineNumberWidget = new LineNumberWidget(this);

    //绑定
    initConnection();

    //高亮
    initHighlighter();

    //设置字体
    setAllFont(font);

    //行高亮
    highlightCurrentLine();

    //设置边距
    updataNumberWidgetWidth();

    setLineWrapMode(QPlainTextEdit::NoWrap);
}

MyCodeEditor::~MyCodeEditor()
{
    delete lineNumberWidget;
}

void MyCodeEditor::initConnection()
{
    //cursor 光标
    connect(this,SIGNAL(cursorPositionChanged()),this,SLOT(highlightCurrentLine()));

    //textChanged 事件
    connect(this,SIGNAL(textChanged()),this,SLOT(updataSaveState()));

    //blockCount 计数
    connect(this,SIGNAL(blockCountChanged(int)),this,SLOT(updataNumberWidgetWidth()));

    //updateRequest 最新请求
    connect(this,SIGNAL(updateRequest(QRect,int)),this,SLOT(updateRequest(QRect,int)));
}

void MyCodeEditor::setAllFont(QFont font)
{
    this->setFont(font);
    mHighlighter->setFont(font);
    updataNumberWidgetWidth();
}

bool MyCodeEditor::checkSaved()
{
    return isSaved;
}

void MyCodeEditor::initHighlighter()
{
    mHighlighter = new MyHighlighter(document());
}

int MyCodeEditor::getLineNumberWidgetWidth()
{
    //获取合适的宽度
    return 8+QString::number(blockCount()+1).length()*fontMetrics().horizontalAdvance(QChar('0'));
}

void MyCodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QColor(0,100,100,20));
    selection.format.setProperty(QTextFormat::FullWidthSelection,true);
    selection.cursor = textCursor();

    extraSelections.append(selection);
    setExtraSelections(extraSelections);
}

void MyCodeEditor::updateRequest(QRect rect, int dy)
{
    if(dy)
        lineNumberWidget->scroll(0,dy);
    else
        lineNumberWidget->update(0,rect.y(),getLineNumberWidgetWidth(),rect.height());
}

void MyCodeEditor::updataNumberWidgetWidth()
{
    //设置边距
    setViewportMargins(getLineNumberWidgetWidth(),0,0,0);
}

void MyCodeEditor::updataSaveState()
{
    //更新保存状态
    isSaved = false;
}

void MyCodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    lineNumberWidget->setGeometry(0,0,getLineNumberWidgetWidth(),contentsRect().height());
}

void MyCodeEditor::lineNumberWidgetPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberWidget);
    //绘制行号区域
    painter.fillRect(event->rect(),QColor(100,100,100,20));

    //拿到block
    QTextBlock block = firstVisibleBlock();
    //拿到行号
    int blockNumber = block.blockNumber();

    //拿到当前的block的top
    int cursoTop = blockBoundingGeometry(textCursor().block()).translated(contentOffset()).top();

    //拿到block的top
    int top = blockBoundingGeometry(block).translated(contentOffset()).top();

    //拿到Block的bottom
    int bottom = top + blockBoundingRect(block).height();
    while(block.isValid()&&top<=event->rect().bottom()){

        //设置画笔颜色
        painter.setPen(cursoTop==top?Qt::black:Qt::gray);
        //绘制文字
        painter.drawText(0,top,getLineNumberWidgetWidth()-3,bottom-top,Qt::AlignRight,QString::number(blockNumber+1));

        //拿到下一个block
        block = block.next();

        top = bottom;
        bottom = top + blockBoundingRect(block).height();
        blockNumber++;
    }
}

void MyCodeEditor::lineNumberWidgetPressEvent(QMouseEvent *event)
{
    setTextCursor(QTextCursor(document()->findBlockByLineNumber(event->y()/fontMetrics().height()+verticalScrollBar()->value())));
}

void MyCodeEditor::lineNumberWidgetWheelEvent(QWheelEvent *event)
{
    if(event->orientation()==Qt::Horizontal){
        horizontalScrollBar()->setValue(horizontalScrollBar()->value()-event->delta());
    }else{
        verticalScrollBar()->setValue(horizontalScrollBar()->value()-event->delta());
    }
    event->accept();
}

bool MyCodeEditor::saveFile()
{
    QString fileName;
    if(mFileName.isEmpty()){
        fileName = QFileDialog::getSaveFileName(this,"保存文件");
        mFileName = fileName;
    }else{
        fileName = mFileName;
    }
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly|QFile::Text)){
        QMessageBox::warning(this,"警告","无法保存文件:"+file.errorString());
        return false;
    }
    QTextStream out(&file);

    out<<toPlainText();
    file.close();

    isSaved = true;
    return true;
}

bool MyCodeEditor::saveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,"另存文件");
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly|QFile::Text)){
        QMessageBox::warning(this,"警告","无法保存文件:"+file.errorString());
        return false;
    }
    mFileName = fileName;
    QTextStream out(&file);
    QString text = toPlainText();
    out<<text;
    file.close();

    isSaved = true;
    return true;
}

void MyCodeEditor::setFileName(QString fileName)
{
    mFileName = fileName;
}

QString MyCodeEditor::getFileName()
{
    return mFileName;
}



