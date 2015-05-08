/***************************************************************************
                          RTAdecoder.cpp  -  description
                             -------------------
    copyright            : (C) 2015 Valentina Fioretti
                               2013 Andrea Bulgarelli
                               2013 Andrea Zoli
 
    email                : fioretti@iasfbo.inaf.it
                           bulgarelli@iasfbo.inaf.it
                           zoli@iasfbo.inaf.it
 
 ***************************************************************************/
/***************************************************************************
- Description:
Decoding the raw binary packets.
- Last modified:
7/05/2015 (V. Fioretti)
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

//#include <packet/PacketLibDefinition.h>
//#include <packet/PacketExceptionIO.h>
//#include <packet/InputPacketStream.h>
//#include <packet/ByteStream.h>
//#include <packet/InputFile.h>

#include <packet/Packet.h>
#include <packet/InputPacketStream.h>
#include <packet/InputFile.h>

using namespace std;

/// Reading the Packet
int main(int argc, char *argv[])
{
    try
    {

        clock_t t = clock();
        string ctarta;
        const char* home = getenv("CTARTA");
        if(argc > 1) {
            /// The Packet containing the FADC value of each triggered telescope
            if (!home)
            {
                std::cerr << "CTARTA environment variable is not defined." << std::endl;
                return 0;
            }
            
            ctarta = home;
        } else {
            if(argc == 1){
                cerr << "Please, provide the name of the .raw file" << endl;
                return 0;
            }
        }

        //string configFileName = ctarta + "./conf/rta_fadc_v3.xml";
        char* filename = argv[1];
      
        unsigned long totbytes = 0;
        long nops = 0;

        //Objects that manages a stream of byte
        PacketLib::InputPacketStream* ips = 0;

        //create output packet stream - configFileName is the name of the xml file
        ips = new PacketLib::InputPacketStream("./conf/rta_fadc_v3.xml");

        // Create and open an output device: file
        PacketLib::Input* in = (PacketLib::Input*) new PacketLib::InputFile(ips->isBigEndian());
        vector<string> param;
        param.push_back(filename);
        in->openDevice(param); /// open input
        
        // connect the input packet stream with the input device
        ips->setInput(in);
        
        //decode for routing
        PacketLib::Packet* p = ips->getPacketType("triggered_telescope1");
        PacketLib::byte ctaCamId = p->getPacketID();
        
        while(p = ips->readPacket()) {//if not end of file
            nops++;
            cout << p << endl;
            //cout << ctaCamId << endl;
            if(p->getPacketID() == PACKETNOTRECOGNIZED)
                cout << "Packet not recognized" << endl;
            if(p->getPacketID() == ctaCamId) {

                cout << "--" << endl;

                //packet recognized, do something, e.g.
                
                //get the size of the packet
                PacketLib::dword packetSize = p->size();
                totbytes += packetSize;
            
                //access the packet header information
                cout << "APID: " << p->getPacketHeader()->getFieldValue("APID") << endl;

                //store some informations on the Data Field Headers
                p->getPacketDataFieldHeader()->getFieldValue_32f("LTtime");
                p->getPacketDataFieldHeader()->getFieldValue_16ui("ArrayID");
                p->getPacketDataFieldHeader()->getFieldValue_16ui("runNumber");
                p->getPacketDataFieldHeader()->getFieldValue_32ui("eventNumber");
                p->getPacketDataFieldHeader()->getFieldValue_16ui("TelescopeID");
                p->getPacketDataFieldHeader()->getFieldValue("numberOfTriggeredTelescopes");
                
                //get information from the packet: number of pixels and samples, trigger time, event number, packet length
                int npixels =   p->getPacketSourceDataField()->getFieldValue("Number of pixels");
                int nsamples =  p->getPacketSourceDataField()->getFieldValue("Number of samples");
                PacketLib::dword evetnum = p->getPacketSourceDataField()->getFieldValue_32ui("eventNumber");
                cout << "N pixels " << npixels << " N samples " << nsamples << " " << " Evt Num. " << evetnum << endl;
                cout << p->getPacketHeader()->getPacketLength() << endl;
                
                //version 1 - fast
                //get the array of camera data - if packet is compressed, decompress them in a transparent way
                
                PacketLib::ByteStreamPtr cameraDataStream = p->getData();
                cameraDataStream->swapWordForIntel(); //take into account the endianity
                PacketLib::word* cameraData = (PacketLib::word*)cameraDataStream->stream;
                
                //process the camera data
                for(PacketLib::word pixel=0; pixel<npixels; pixel++) {
                    for(PacketLib::word sample=0; sample<nsamples; sample++) {
                        cout << cameraData[pixel*nsamples + sample] << " ";
                    }
                    cout << endl;
                }
                
                //version 2 - slow
                /*
                 if(p->isCompressed()) p->decompress();
                 for(word pixel=0; pixel<npixels; pixel++) {
                 for(word sample=0; sample<nsamples; sample++) {
                 cout << p->getPacketSourceDataField()->getBlock(pixel)->getFieldValue(sample) << " ";
                 }
                 cout << endl;
                 }
                 */
            }
        }
    }
    catch(PacketLib::PacketExceptionIO* e)
    {
        cout << e->geterror() << endl;;
    }
    catch(PacketLib::PacketException* e)
    {
        cout << e->geterror() << endl;
    }

	return 1;
}
