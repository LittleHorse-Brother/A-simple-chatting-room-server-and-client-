#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
public slots:
    void onconnected();
    void ondisconnected();
    void onerror(QAbstractSocket::SocketError socketError);
    void onreadyRead();

private slots:
    void on_btnsend_clicked();

    void on_btnimage_clicked();

private:
    Ui::Widget *ui;
    QTcpSocket tcpsocket;
    int imageindex;
    qint32 sizePacklast;

};
#endif // WIDGET_H
