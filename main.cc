
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

	//Setting timer
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timerHandler()));

	//Setting antientropy timer
	/*
	anti_timer = new QTimer(this);
	connect(anti_timer, SIGNAL(timeout()), this, SLOT(antiHandler()));
	anti_timer->start(10000);
	*/

	seqnum = 0; 
	message_status = new QMap<QString, quint32>;

	//Register a callback on the textline's returnPressed signal
	connect(textline, SIGNAL(returnPressed()), this, SLOT(gotReturnPressed()));

	//Register a callback on the textline's readyRead signal 
	connect(socket, SIGNAL(readyRead()), this, SLOT(readMessages()));
}

QByteArray ChatDialog::serialize_message(QString message){
	QVariantMap message_qmap;

	//Constructing rumor message
	message_qmap.insert("ChatText", message);
	message_qmap.insert("Origin", QString::number(socket->myPort);
	message_qmap.insert("SeqNo", seqnum);

	//Append new messages to a list
	if(!message_list.contains(QString::number(socket->myPort))){
		QList tmp;
		tmp.insert(seqnum, message); 
        message_list.insert(QString::number(socket->myPort), tmp);
	} else {
        message_list.insert(QString::number(socket->myPort), tmp);
	}

	seqnum += 1; 

	//We serialize the message into QByteArray
	QByteArray byte_arr;
	QDataStream stream(&byte_arr,QIODevice::ReadWrite);
	stream << message_qmap;

	return byte_arr;
}

QByteArray ChatDialog::serialize_status(){
	QMap<QString, QMap<QString, quint32> status; 
	status.insert("Want", wants);

	QByteArray status_data;
    QDataStream stream(&status_data,QIODevice::ReadWrite);
    stream << status;

    return status_data; 
}

void ChatDialog::gotReturnPressed()
{
	// Initially, just echo the string locally.
	// Insert some networking code here...
	qDebug() << "Return Pressed. Sending: " << textline->text();
	textview->append(QString::number(socket->myPort) + ": " + textline->text());

	//Fetch the message then serialize it. 
	Qstring str_input = textline->text();
	QByteArray serialized_str = serialize_message(str_input);

	//Update the neighbors' want list
	if()

	sendMessages(serialized_str);
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

