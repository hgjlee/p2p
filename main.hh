#ifndef P2PAPP_MAIN_HH
#define P2PAPP_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>
#include <QTimer>

class ChatDialog : public QDialog
{
	Q_OBJECT

public:
	ChatDialog();
	NetSocket *socket;
	int seqnum;
    QTimer * timer;
    QTimer * anti_timer;
    void sendMessages(QByteArray); 
    QMap<QString, QMap<quint32, QvariantMap> message_list; 
    QMap<QString, quint32> wants;
    int peer;
    int peer_port; 

public slots:
	void gotReturnPressed();
	void timerHandler();
	void antiHandler();
	void readMessages(); 

private:
	QTextEdit *textview;
	QLineEdit *textline;
	QByteArray serialize_message(QString);
    QByteArray serialize_status();
};

class NetSocket : public QUdpSocket
{
	Q_OBJECT

public:
	NetSocket();

	// Bind this socket to a P2Papp-specific default port.
	bool bind();

private:
	int myPortMin, myPortMax, myPort;
};

#endif // P2PAPP_MAIN_HH
