#include "stdafx.h"
#include "heartbeatreceiver.h"
#include <thread>
#include "datapusher.h"

volatile unsigned long long heartbeatreceiver::_calc;
static heartbeatreceiver *_receiver;

heartbeatreceiver::heartbeatreceiver()
{
	msignal = 0;
	_calc = 2;
	misdead = true;
	//Start();
}

void heartbeatreceiver::Increment()
{
	::_InterlockedIncrement( &_calc);
	_calc = 2;
}

void heartbeatreceiver::Start()
{
	misdead = false;
	_calc = 2;
	new std::thread(std::bind(&heartbeatreceiver::Decrement,this));
}

void heartbeatreceiver::Decrement()
{
	while (1)
	{
		std::this_thread::sleep_for(chrono::milliseconds(2500));
		::_InterlockedDecrement(&_calc);
		if (_calc == 0)
		{
			break;
		}

	}

	const char *data = "{\"type\":\"tabevent\",\"name\":\"onclosed\",\"info\":{},\"tab\":{}}";
	datapusher::getInstance()->push(data);
	misdead = true;
}
 
bool  heartbeatreceiver::isDead()
{
	return misdead;
}

bool heartbeatreceiver::Reset()
{
	if (misdead)
		return false;
	this->msignal = 1;

	return true;
}

heartbeatreceiver * heartbeatreceiver::getInstance()
{
	if (NULL == _receiver)
		_receiver = new heartbeatreceiver();

	return _receiver;
}

