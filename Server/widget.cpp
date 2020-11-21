#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QHostAddress>
#include <QFileDialog>
#include <QDataStream>
#include <QString>
#include <QDateTime>



Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(&server,SIGNAL(newConnection()),this,SLOT(onnewConnection()));
    bool ok=server.listen(QHostAddress::AnyIPv4,8888); //设置监听IP与端口
    qDebug()<<"listen:"<<ok;
    imageindex=0;
    sizePacklast=0;
    ui->splitter->setStretchFactor(0,2);
    ui->splitter->setStretchFactor(1,1);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::onnewConnection()
{
    QTcpSocket* socket=server.nextPendingConnection(); //使用socket与客户端进行通信
    clients.append(socket);
    connect(socket,SIGNAL(connected()),this,SLOT(onconnected()));
    connect(socket,SIGNAL(disconnected()),this,SLOT(ondisconnected()));
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onerror(QAbstractSocket::SocketError)));
    connect(socket,SIGNAL(readyRead()),this,SLOT(onreadyRead()));

}

void Widget::onconnected() //连接成功
{
    qDebug()<<"connected";
}

void Widget::ondisconnected() //连接失败
{
    QObject *obj=this->sender(); //获取信号发送者
    QTcpSocket *socket=qobject_cast<QTcpSocket*>(obj);//转化为TCPserver
    qDebug()<<"disconnected";
    clients.removeAll(socket);
    socket->deleteLater();
}

void Widget::onerror(QAbstractSocket::SocketError socketError) //出错
{
    qDebug()<<"error:"<<socketError;
}

void Widget::onreadyRead()
{
    QObject *obj=this->sender(); //获取信号发送者
    QTcpSocket *socket=qobject_cast<QTcpSocket*>(obj);//转化为TCPsocket
//    QByteArray data=socket->readAll();

  qint64 sizeNow=0;
    do
    {
    sizeNow=socket->bytesAvailable();

    QDataStream stream(socket);
    if(sizePacklast==0){
    if(sizeNow<sizeof(quint32))return;


    stream>>sizePacklast;}

    //包不完整则等待包完整
    if(sizeNow<sizePacklast-4)return;

        qDebug()<<"full pack";
        QByteArray dataFull;
        stream>>dataFull;
         sizePacklast=0;
        //判断剩下的字节数是否会有沾包现象
         sizeNow=socket->bytesAvailable();


       QString prefix=dataFull.mid(0,4);
       QString dateTime=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
       if(prefix=="TXT:"){
           ui->textMsg->append(dateTime);
         ui->textMsg->append("<p>"+dataFull.mid(4)+"</p><br>");
       }
       else if(prefix=="IMG:"){
           ui->textMsg->append(dateTime);
           QString htmlTag=QString("<img src=\"%1\"></img><br>");
           QString index=QString::number(imageindex);
           htmlTag=htmlTag.arg(index+".png");

           QFile file(index+".png");
           file.open(QIODevice::WriteOnly);
           file.write(dataFull.mid(4));
           file.close();
           imageindex++;
           ui->textMsg->append(htmlTag);
       }
    }while(sizeNow>0);



}


void Widget::on_btnsend_clicked()
{
    QString msgInput="TXT:"+ui->textEdit->toPlainText();
    ui->textMsg->append("me:"+ui->textEdit->toPlainText());
    QByteArray dataSend;//封装的数据包
    QDataStream stream(&dataSend,QIODevice::WriteOnly);
    stream<<(quint32)0<<msgInput.toUtf8();
    stream.device()->seek(0);
    stream<<dataSend.size();
    for(QList<QTcpSocket*>::iterator itr=clients.begin();itr!=clients.end();itr++){
        QTcpSocket *client=*itr;
        client->write(dataSend);
    }
    ui->textEdit->clear();


}

void Widget::on_btnimage_clicked()
{
    QString image=QFileDialog::getOpenFileName(this,"title",".","image files(*.png *.jpg *.bmp)");
    if(image.isEmpty())return;
    QFile file(image);
    file.open(QIODevice::ReadOnly);
    QByteArray data="IMG:"+file.readAll();
    file.close();
    //封装包头,前面加数据长度,解决tcp沾包半包
    QByteArray dataSend;//封装的数据包
    QDataStream stream(&dataSend,QIODevice::WriteOnly);
    stream<<(quint32)0<<data;
    stream.device()->seek(0);
    stream<<dataSend.size();

    for(QList<QTcpSocket*>::iterator itr=clients.begin();itr!=clients.end();itr++){
        QTcpSocket *client=*itr;
        client->write(dataSend);
    }

}
