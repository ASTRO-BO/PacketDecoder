/***************************************************************************
                          RTAencoder.cpp  -  description
                             -------------------
    copyright            : (C) 2013 Andrea Bulgarelli
                               2013 Andrea Zoli
                               2015 Valentina Fioretti (author)
    email                : bulgarelli@iasfbo.inaf.it
                           zoli@iasfbo.inaf.it
                           fioretti@iasfbo.inaf.it
 ***************************************************************************/
/***************************************************************************
- Description:
Encoding into raw binary packets from a dummy CTA trigger event.
- Last modified:
05/05/2015 (V. Fioretti)
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

#define RBLOCK_PIXEL 0
#define RBLOCK_SAMPLE 0
#define RBLOCK_PIXELID 1

#define MATH_PI 3.14159265359

#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <math.h>

#include <time.h>
#include <vector>

#include <packet/PacketLibDefinition.h>
#include <packet/PacketExceptionIO.h>
#include <packet/OutputPacketStream.h>
#include <packet/ByteStream.h>
#include <packet/Output.h>
#include <packet/OutputFile.h>



using namespace std;


/// Writing the Packet
int main(int argc, char *argv[])
{
    try
    {
        clock_t t;
        
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

        // ------------> preparing the packetlib output
        //Objects that manages a stream of byte
        PacketLib::OutputPacketStream* ops = 0;
        
        //create output packet stream - configFileName is the name of the xml file
        ops = new PacketLib::OutputPacketStream("./conf/rta_fadc_v3.xml");
        
        // Create and open an output device: file
        PacketLib::Output* out = (PacketLib::Output*) new PacketLib::OutputFile(ops->isBigEndian());
        vector<string> param;
        char name_fadc_out[100];   // array to hold the packet name.
        
        param.push_back(argv[1]);
        out->openDevice(param);
        
        // connect the output packet stream with the output
        ops->setOutput(out);
        
        //get a packet to encode the data of a camera
        PacketLib::Packet* p = ops->getPacketType("triggered_telescope1");
        
        /// Looping in the triggered events
        srand(0);
        long counts = 0;
        unsigned short SSC_index;
        vector<int16_t> SSC_array;		
        PacketLib::word ssc = 0;
        
        unsigned long totbytes = 0;

        // Dummy parameters
        int16_t NTel = 10;
        int16_t NTel_Trig = 5;
        int16_t numberOfEvent = 100;
        int16_t RunNumber = 0;
        vector<int16_t> vecTrigTelID;
        vector<int16_t> vecTelID;
        vecTelID.resize(NTel);
        for(int alltelind = 0; alltelind<NTel; alltelind++) {
            vecTelID[alltelind] = 10*(alltelind+1);
        }
        int16_t NSamples = 20;
        int16_t NSamples_Trig;
        int16_t NPixels = 10;
        int16_t NPixels_Trig;

        for(int evtindex = 0; evtindex<numberOfEvent; evtindex++) {
			cout << "Event ID:" << evtindex << endl;
            
            SSC_array.resize(NTel);
            vecTrigTelID.resize(NTel_Trig);
            
            cout << "N Triggered telescope " << NTel_Trig << endl;

            //for each triggere telescope, generate a telemetry packet
            for(int telindex = 0; telindex<NTel_Trig; telindex++) {
				

                vecTrigTelID[telindex] = telindex + 10;
                
                p->getPacketHeader()->setFieldValue("APID", vecTrigTelID[telindex]); //the data generator (for now, the telescope)
                
                for (int j = 0; j < vecTelID.size(); j++){
                    if ( vecTrigTelID[telindex] == vecTelID[j]){
                        SSC_index = j;
                        break;
                    }
                }
                
                ssc = SSC_array[SSC_index];
                
                
                // number of pixels
                NPixels_Trig = NPixels;
                
                // number of samples
                NSamples_Trig = NSamples;
                
                //p->getPacketHeader()->setFieldValue_16ui("", ); //the metadata
                p->getPacketHeader()->setFieldValue("Packet Subtype", NSamples_Trig); //the metadata
                p->getPacketHeader()->setFieldValue("Source Sequence Counter", ssc);	//a unique counter of packets
                cout << "ssc " << ssc << endl;
                
                //store some informations on the Data Field Headers
                p->getPacketDataFieldHeader()->setFieldValue_32f("LTtime", telindex*1000);
                p->getPacketDataFieldHeader()->setFieldValue_16ui("ArrayID", 1);
                p->getPacketDataFieldHeader()->setFieldValue_16ui("runNumber", RunNumber);
                p->getPacketDataFieldHeader()->setFieldValue_32ui("eventNumber", telindex);
                p->getPacketDataFieldHeader()->setFieldValue_16ui("TelescopeID", vecTrigTelID[telindex]);
                p->getPacketDataFieldHeader()->setFieldValue("numberOfTriggeredTelescopes", NTel_Trig);

                cout << "Triggered Telescope ID " << vecTrigTelID[telindex] << endl;
                
                counts++;
                p->getPacketDataFieldHeader()->setFieldValue_16ui("telescopeCounter", counts);
                
                
                //store some informations on the Source Data Field
                p->getPacketSourceDataField()->setFieldValue_16ui("Number of pixels", NPixels_Trig);
                p->getPacketSourceDataField()->setFieldValue_16ui("Number of samples", NSamples_Trig);
                p->getPacketSourceDataField()->setFieldValue_16ui("Number of pixels ID zero-suppressed", NPixels_Trig);
                
                //set the number blocks
                p->getPacketSourceDataField()->setNumberOfBlocks(NPixels_Trig, RBLOCK_PIXEL);
                p->getPacketSourceDataField()->setNumberOfBlocks(NPixels_Trig, RBLOCK_PIXELID);
                
                //TODO-> Adding the zerosuppressed pixels
                
                
                //store the information of the pixels
                //set each single field
                PacketLib::word jpixel;
                PacketLib::word jsample;
                
                cout << "NPixels " << NPixels_Trig << endl;
                cout << "NSamples " << NSamples_Trig << endl;
                
                for(jpixel=0; jpixel<NPixels_Trig; jpixel++){
                    p->getPacketSourceDataField()->getBlock(jpixel, RBLOCK_PIXEL)->setNumberOfBlocks(NSamples_Trig, RBLOCK_SAMPLE);
                    for(jsample=0; jsample<NSamples_Trig; jsample++) {
                        int16_t sample_value = (int)(rand() % 255);
                        p->getPacketSourceDataField()->getBlock(jpixel, RBLOCK_PIXEL)->getBlock(jsample, RBLOCK_SAMPLE)->setFieldValue(0, sample_value);
                            cout << "FADC: " <<  sample_value << endl;
                        
                    }
                    PacketLib::word PixelID = jpixel;
                    p->getPacketSourceDataField()->getBlock(jpixel, RBLOCK_PIXELID)->setFieldValue(0, PixelID);
                }
                
                
                SSC_array[SSC_index] = SSC_array[SSC_index] + 1;
                
                
                //encode the packet
                p->encode();
                
                //compress the data
                // compression level (0 = do not compress)" << endl;
                
                int16_t compress = 0;
                if(compress) p->compressData(LZ4, compress);
                
                //write the encoded packet to output
                ops->writePacket(p);
                
                //get the size of the packet (only for measurement of performances)
                PacketLib::dword packetSize = p->size();
                totbytes += packetSize;

            }

        }
        
        t = clock() - t;
        //printf ("It took me %d clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);
        cout << "END " << counts << endl;
        return 0;

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
