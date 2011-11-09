//
// root of the demo - provides audio playback
//
#pragma once

#include <ops/Op.h>


#include <ops/Context.h>

#include <portaudio/portaudio.h>
#include <stblib/stb_vorbis.h>










//
// root of the demo - provides audio playback
//
BASE_DECL_SMARTPTR(DemoOp);
class DemoOp : public base::ops::Op
{
public:

	struct StreamData
	{
		short *data;
		int len;
		int current;
	};


	DemoOp( std::string streamPath = "" ) : base::ops::Op(), m_streamPath(streamPath), m_stream(0)
	{

	}

	virtual void execute()
	{
		// TODO: maybe use streamtime over watchclock?
		base::ops::Context::setTime( 1.0f );
		// execute inputs
		for( OpList::iterator it = m_opList.begin(); it != m_opList.end(); ++it)
			(*it)->execute();
	}

	void startAudio()
	{
		// try to load stream ============
		if(!m_streamPath.empty())
		{
			int channels;
			StreamData *streamData = new StreamData();
			streamData->current = 0;
			std::cout << "DemoOp::startAudio - loading stream " << m_streamPath << std::endl;
			//streamData->len = stb_vorbis_decode_filename("C:\\projects\\sandbox\\temp\\code\\sketch039.ogg", &channels, &streamData->data);
			//streamData->len = stb_vorbis_decode_filename("/usr/people/david-k/dev/testprojects/sandbox/temp/sketch039.ogg", &channels, &streamData->data);
			streamData->len = stb_vorbis_decode_filename( const_cast<char *>(m_streamPath.c_str()), &channels, &streamData->data);

			if(!streamData->len)
			{
				std::cerr << "DemoOp::startAudio - error loading ogg file\n";
			}else
			{
				std::cout << "DemoOp::startAudio - number of channels " << channels << std::endl;
				std::cout << "DemoOp::startAudio - number of samples " << streamData->len << std::endl;
				m_stream = streamData;
			}

			// setup audio ===============
			PaStreamParameters outputParameters;

			PaError err;

			err = Pa_Initialize();

			outputParameters.device = Pa_GetDefaultOutputDevice(); // default output device
			if (outputParameters.device == paNoDevice)
			{
				std::cerr << "DemoOp::startAudio - error: No default output device.\n";
			}
			outputParameters.channelCount = 2;       // stereo output
			outputParameters.sampleFormat = paInt16;
			outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
			outputParameters.hostApiSpecificStreamInfo = NULL;

			err = Pa_OpenStream(
			&m_stream,
			NULL, // no input
			&outputParameters,
			44100,
			64,
			paClipOff,      // we won't output out of range samples so don't bother clipping them
			streamDataCallback,
			streamData );

			err = Pa_SetStreamFinishedCallback( m_stream, &streamFinishedCallback );


			// play..
			err = Pa_StartStream( m_stream );
		}else
			std::cerr << "DemoOp::startAudio - empty streampath";


	}

	void stopAudio()
	{
		if( m_stream )
		{
			PaError err;

			// stop
			err = Pa_StopStream( m_stream );

			err = Pa_CloseStream( m_stream );

			Pa_Terminate();
			std::cout << "DemoOp::stopAudio - finished\n";
		}
	}

	static DemoOpPtr create( std::string streamPath = "" )
	{
		return DemoOpPtr( new DemoOp(streamPath) );
	}

private:


	// Audio =============================

	// This routine will be called by the PortAudio engine when audio is needed.
	// It may called at interrupt level on some machines so don't do anything
	// that could mess up the system like calling malloc() or free().
	//
	static int streamDataCallback( const void *inputBuffer, void *outputBuffer,
								unsigned long framesPerBuffer,
								const PaStreamCallbackTimeInfo* timeInfo,
								PaStreamCallbackFlags statusFlags,
								void *userData )
	{
		StreamData *streamData = (StreamData *)userData;
		short *out = (short*)outputBuffer;
		unsigned long i;

		(void) timeInfo; // Prevent unused variable warnings.
		(void) statusFlags;
		(void) inputBuffer;

		for( i=0; i<framesPerBuffer; i++ )
		{
			*out++ = streamData->data[streamData->current];  // left
			*out++ = streamData->data[streamData->current+1];  // right
			streamData->current += 2;
		}

		return paContinue;
	}

	//
	// This routine is called by portaudio when playback is done.
	//
	static void streamFinishedCallback( void* userData )
	{
		StreamData *streamData = (StreamData *) userData;
		delete streamData;

	}


	PaStream               *m_stream;
	std::string         m_streamPath;

};

