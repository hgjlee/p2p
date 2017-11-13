
#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>

#include "main.hh"

ChatDialog::ChatDialog()
{
	setWindowTitle("P2Papp");

	// Read-only text box where we display messages from everyone.
	// This widget expands both horizontally and vertically.
	textview = new QTextEdit(this);
	textview->setReadOnly(true);

	// Small text-entry box the user can enter messages.
	textline = new QLineEdit(this);

	// Lay out the widgets to appear in the main window.
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(textview);
	layout->addWidget(textline);
	setLayout(layout);

	socket = new NetSocket();
	socket->bind();

	seqnum = 0; 
	message_status = new QMap<QString, quint32>;

	//Setting timer
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timerHandler()));

	//Setting antientropy timer
	anti_timer = new QTimer(this);
	connect(anti_timer, SIGNAL(timeout()), this, SLOT(antiHandler()));
	anti_timer->start(10000);
	

	//Register a callback on the textline's returnPressed signal
	connect(textline, SIGNAL(returnPressed()), this, SLOT(gotReturnPressed()));

	//Register a callback on the textline's readyRead signal 
	connect(socket, SIGNAL(readyRead()), this, SLOT(readMessage()));
}

QByteArray ChatDialog::serialize_message(QString message){
	QVariantMap message_qmap;

	//Constructing a variantmap
	message_qmap["ChatText"] = message; 
	message_qmap["Origin"] = socket->myPort; 
	message_qmap["SeqNo"] = seqnum;

	//Append new messages to a list
	QList<QString> tmp;
	tmp.insert(seqnum, message); 
    message_list.insert(QString::number(socket->myPort), tmp);

	seqnum++; 

	//We serialize the message into QByteArray
	QByteArray byte_arr;
	QDataStream stream(&byte_arr,QIODevice::WriteOnly);
	stream << message_qmap;

	return byte_arr;
}

QByteArray ChatDialog::serialize_status(){
	QMap<QString, QMap<QString, quint32> > status; 
	status.insert("Want", wants);

	QByteArray status_data;
    QDataStream stream(&status_data,QIODevice::ReadWrite);
    stream << status;

    return status_data; 
}

void ChatDialog::pickAndSend(QByteArray message){

	//Choosing to which peer to send the messages
	if (socket->myPort == socket->myPortMin){ 
		peer = socket->myPort + 1;
	} else if (socket->myPort == socket->myPortMax){
		peer = socket->myPort - 1; 
	} else { 
		srand(time(NULL));
		int random = (rand() % 2); 
		if(random == 1){
			peer = socket->myPort + 1;
		} else{
			peer = socket->myPort -1; 
		}
	}

	socket->writeDatagram(message, QHostAddress("127.0.0.1"), peer);
}

void ChatDialog::sendStatus(QByteArray status){
	socket->writeDatagram(status.data(), QHostAddress("127.0.0.1"), peer_port);
}

void ChatDialog::rumor(QVariantMap data){
	QByteArray rumors;
    QDataStream stream(&rumors,QIODevice::ReadWrite);
    stream << data;

    pickAndSend(rumors);
}

void ChatDialog::readPendingDatagrams()
{ 
    //reference: https://doc.qt.io/qt-5.6/qudpsocket.html
    while (socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        // receive and read message
        socket->readDatagram(datagram.data(), datagram.size(), &sender,  &senderPort);
        //save the sender port number in a global variable
        peer_port = senderPort;
        processTheDatagram(datagram);
    }
}


void ChatDialog::processTheDatagram(QByteArray bytesIn)
{
    
    QVariantMap message;
    QMap<QString, QMap<QString, quint32> > status;
    QDataStream readmessage(&bytesIn, QIODevice::ReadOnly);

    readmessage >> message;
    readmessage >> status; 

    if(readmessage.status() != QDataStream::Ok) {
        qDebug() << "ERROR: Failed to deserialize datagram into QVariantMap";
        return;
    }

    if (message.contains("ChatText")){
        qDebug() << "INFO: Received a chat message";
        readMessage(message);
    }
    else if(message.contains("Want")){
        qDebug() << "INFO: Received a status message";
        readStatus(status);
    }
    else{
        qDebug() << "ERROR: Message is neither ChatText or Want";
        return; 
    }
}

void ChatDialog::readMessage(QVariantMap wants){

}

void ChatDialog::readStatus(QMap<QString, QMap<QString, quint32> > wants){

}

void ChatDialog::gotReturnPressed()
{
	// Initially, just echo the string locally.
	// Insert some networking code here...
	qDebug() << "Return Pressed. Sending: " << textline->text();
	textview->append(QString::number(socket->myPort) + ": " + textline->text());

	//Fetch the message then serialize it. 
	QString str_input = textline->text();
	QByteArray serialized_str = serialize_message(str_input);

	//Update the neighbors' want list
	pickAndSend(serialized_str);
	// Clear the textline to get ready for the next input message.
	textline->clear();
}

NetSocket::NetSocket()
{
	// Pick a range of four UDP ports to try to allocate by default,
	// computed based on my Unix user ID.
	// This makes it trivial for up to four P2Papp instances per user
	// to find each other on the same host,
	// barring UDP port conflicts with other applications
	// (which are quite possible).
	// We use the range from 32768 to 49151 for this purpose.
	myPortMin = 32768 + (getuid() % 4096)*4;
	myPortMax = myPortMin + 3;
}

bool NetSocket::bind()
{
	// Try to bind to each of the range myPortMin..myPortMax in turn.
	for (int p = myPortMin; p <= myPortMax; p++) {
		if (QUdpSocket::bind(p)) {
			qDebug() << "bound to UDP port " << p;
			return true;
		}
	}

	qDebug() << "Oops, no ports in my default range " << myPortMin
		<< "-" << myPortMax << " available";
	return false;
}

int main(int argc, char **argv)
{
	// Initialize Qt toolkit
	QApplication app(argc,argv);

	// Create an initial chat dialog window
	ChatDialog dialog;
	dialog.show();

	// Create a UDP network socket
	NetSocket sock;
	if (!sock.bind())
		exit(1);

	// Enter the Qt main loop; everything else is event driven
	return app.exec();
}