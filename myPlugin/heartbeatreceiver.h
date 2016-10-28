#include "stdafx.h"

class heartbeatreceiver{
public:
	heartbeatreceiver();
	void Increment();
	 
	bool isDead();
	void Start();
	bool Reset();
	static heartbeatreceiver * getInstance();
protected:
	void Decrement();
private:
	volatile bool misdead;
	volatile int  msignal;
	volatile static unsigned long long _calc;
};