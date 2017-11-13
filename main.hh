#ifndef P2PAPP_MAIN_HH
#define P2PAPP_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>
#include <QTimer>
#include <QDataStream>
#include <QVBoxLayout>
#include <QtGlobal>
#include <QHostInfo>
#include <QtGlobal>
#include <QList>

class NetSocket : public QUdpSocket
{
	Q_OBJECT

	public:
		NetSocket();

		// Bind this socket to a P2Papp-specific default port.
		bool bind();
		int myPortMin, myPortMax, myPort;
};

class ChatDialog : public QDialog
{
	Q_OBJECT

	public:
		ChatDialog();
		NetSocket *socket;
		int seqnum;
	    QTimer * timer;
	    QTimer * anti_timer;
	    QMap<QString, QList<QString> > message_list; 
	    QMap<QString, quint32> wants;
	    int peer;
	    int peer_port; 

	public slots:
		void gotReturnPressed();
		// void timerHandler();
		// void antiHandler();
		// void readMessages(); 
		void sendStatus(QByteArray status);
		void rumor(QVariantMap data);
		void pickAndSend(QByteArray); 
		void processTheDatagram(QByteArray bytesIn);
		void readPendingDatagrams();
		void readMessage(QVariantMap wants);
		void readStatus(QMap<QString, QMap<QString, quint32> > wants);

	
	private:
		QTextEdit *textview;
		QLineEdit *textline;
		QByteArray serialize_message(QString);
	    QByteArray serialize_status();
	    QMap<QString, quint32>* message_status;
}; 

#endif // P2PAPP_MAIN_HH
