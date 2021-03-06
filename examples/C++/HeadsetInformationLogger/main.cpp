/****************************************************************************
**
** Copyright 2015 by Emotiv. All rights reserved
** Example - Headset Information Logger
** This example allows getting headset infor: contactquality, wireless strength
** battery level.  
** This example work on single connection
****************************************************************************/

#include <iostream>
#include <fstream>
#if _WIN32
#include <conio.h>
#endif

#include "Iedk.h"
#include "IedkErrorCode.h"
#include "IEmoStateDLL.h"

using namespace std;

#ifdef __linux__
static int _kbhit(void)
{
    struct timeval tv;
    fd_set read_fd;

    tv.tv_sec=0;
    tv.tv_usec=0;

    FD_ZERO(&read_fd);
    FD_SET(0,&read_fd);

    if(select(1, &read_fd,NULL, NULL, &tv) == -1)
    return 0;

    if(FD_ISSET(0,&read_fd))
        return 1;

    return 0;
}
#endif

int main(int argc, char** argv)
{
	EmoEngineEventHandle eEvent = IEE_EmoEngineEventCreate();
	EmoStateHandle eState = IEE_EmoStateCreate();
	unsigned int userID = 0;
	bool readytocollect = false;
	bool onStateChanged = false;
	int state = 0;
	IEE_EEG_ContactQuality_t* contactQuality = new IEE_EEG_ContactQuality_t;

	float systemUpTime = 0;
	int batteryLevel, maxBatteryLevel = 0;
	IEE_SignalStrength_t wirelessStrength;

    if (IEE_EngineConnect() != EDK_OK) {
            //throw std::runtime_error("Emotiv Driver start up failed.");
            std::cout << "Emotiv Driver start up failed.";
            return -1;
    }

	std::ofstream ofs;
	ofs.open("test.csv");
	ofs << "Time, Wireless Strength, Battery Level, AF3, T7, Pz, T8, AF4" << std::endl;

	while (!_kbhit()) {

		state = IEE_EngineGetNextEvent(eEvent);
		if (state == EDK_OK) {

			IEE_Event_t eventType = IEE_EmoEngineEventGetType(eEvent);
			IEE_EmoEngineEventGetUserId(eEvent, &userID);

			switch (eventType)
			{
				case IEE_UserAdded:
					std::cout << "User added" << std::endl;
					readytocollect = true;
					break;
				case IEE_UserRemoved:
					std::cout << "User removed" << std::endl;
					readytocollect = false; //single connection
					break;
				case IEE_EmoStateUpdated:
					onStateChanged = true;
					IEE_EmoEngineEventGetEmoState(eEvent, eState);
					break;
				default:
					break;
			}
		}

		if (readytocollect && onStateChanged)
		{
			onStateChanged = false;
			systemUpTime = IS_GetTimeFromStart(eState);
			wirelessStrength = IS_GetWirelessSignalStatus(eState);

			if (wirelessStrength != NO_SIG)
			{
				std::cout << "Time: " << systemUpTime << std::endl;
				IS_GetBatteryChargeLevel(eState, &batteryLevel, &maxBatteryLevel);

				ofs << systemUpTime << ",";
				ofs << wirelessStrength << ",";
				ofs << batteryLevel << ",";
				ofs << IS_GetContactQuality(eState, IEE_CHAN_AF3) << ", ";
				ofs << IS_GetContactQuality(eState, IEE_CHAN_T7) << ", ";
				ofs << IS_GetContactQuality(eState, IEE_CHAN_Pz) << ", ";
				ofs << IS_GetContactQuality(eState, IEE_CHAN_T8) << ", ";
				ofs << IS_GetContactQuality(eState, IEE_CHAN_AF4) << ", ";

				ofs << std::endl;
			}
		}
	}

	ofs.close();
	IEE_EngineDisconnect();
	IEE_EmoStateFree(eState);
	IEE_EmoEngineEventFree(eEvent);
}
