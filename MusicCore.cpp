#include <SDL/SDL.h>
#include "MusicCore.h"
#include <memory>

static const int AUDIO_DESIRED_FREQUENCY = 44100;
static const Uint16 AUDIO_DESIRED_FORMAT = AUDIO_S16SYS;
static const Uint8 AUDIO_DESIRED_CHANNELS = 2;
static const Uint16 AUDIO_DESIRED_SAMPLES = 4096;

std::vector<char> MusicCore::BGMdata::ConversionBuffer;

MusicCore::MusicCore() : ActiveFile(false), ObtainedAudioSpec(0){
	std::auto_ptr<SDL_AudioSpec> Desired(new SDL_AudioSpec);
	ObtainedAudioSpec = new SDL_AudioSpec;

	/* 22050Hz - FM Radio quality */
	Desired->freq		=	AUDIO_DESIRED_FREQUENCY;
	Desired->format		=	AUDIO_DESIRED_FORMAT;
	Desired->channels	=	AUDIO_DESIRED_CHANNELS;
	Desired->samples	=	AUDIO_DESIRED_SAMPLES;

	/* Our callback function */
	Desired->callback=MusicCore::FillAudioBuffer;
	Desired->userdata=this;
	if(SDL_OpenAudio(Desired.get(), ObtainedAudioSpec) < 0){
		delete ObtainedAudioSpec;
		ObtainedAudioSpec = 0;
		return;
	}
	VorbisSigned = (ObtainedAudioSpec->format == AUDIO_S8)
			|| (ObtainedAudioSpec->format == AUDIO_S16LSB)
			|| (ObtainedAudioSpec->format == AUDIO_S8);
	if((ObtainedAudioSpec->format == AUDIO_S8) || (ObtainedAudioSpec->format == AUDIO_U8))
		VorbisSize = 1;
	else
		VorbisSize = 2;
	if((ObtainedAudioSpec->format == AUDIO_U16MSB) || (ObtainedAudioSpec->format == AUDIO_S16MSB))
		VorbisEndian = 1;
	else
		VorbisEndian = 0;
}

MusicCore::~MusicCore(){
	ClearBgm();
	if(ObtainedAudioSpec){
		delete ObtainedAudioSpec;
		SDL_CloseAudio();
	}
}

void MusicCore::Pause(){	SDL_PauseAudio(1); }
void MusicCore::Play(){		SDL_PauseAudio(0); }
void MusicCore::Lock(){		SDL_LockAudio(); }
void MusicCore::Unlock(){	SDL_UnlockAudio(); }
bool MusicCore::SetBgm(const char* Filename){
	ClearBgm();
	return PushBgm(Filename);
}

bool MusicCore::PushBgm(const char* Filename){
	SDL_LockAudio();
	if(!MusicQueue.empty()){ // Save the current position
		MusicQueue.top().PCM_Offset = ov_pcm_tell(&Source);
		CloseFile();  // Clear the file
	}
	MusicQueue.push(BGMdata());
	ActiveFile = MusicQueue.top().OpenFile(Filename,ObtainedAudioSpec,&Source);
	if(!ActiveFile)
		MusicQueue.pop();
	SDL_UnlockAudio();
	return ActiveFile;
}

bool MusicCore::PopBgm(){
	if(MusicQueue.empty())
		return false;
	SDL_LockAudio();
	CloseFile();
	BGMdata& ToPop = MusicQueue.top();
	if(ToPop.Converter)
		delete ToPop.Converter;
	MusicQueue.pop();
	if(!MusicQueue.empty())
		ActiveFile = MusicQueue.top().ResumeFile(&Source);
	SDL_UnlockAudio();
	return true;
}

void MusicCore::ClearBgm(){
	Pause();
	while(SDL_GetAudioStatus() == SDL_AUDIO_PLAYING);
	SDL_LockAudio();
	CloseFile();
	while(!MusicQueue.empty()){
		BGMdata& ToPop = MusicQueue.top();
		if(ToPop.Converter)
			delete ToPop.Converter;
		MusicQueue.pop();
	}
	SDL_UnlockAudio();
}

void MusicCore::FillAudioBuffer(void* Core, uint8_t *stream, int len){
	if(((MusicCore*) Core)->MusicQueue.empty()) // Dummy check
		return;
	BGMdata &Data = ((MusicCore*) Core)->MusicQueue.top(); // Get our top playing song
	int PositionInStream, LeftToRead; // Allocate variables
	char* iStream;
	if(Data.Converter){ // If we're converting, calculate how much to read
		LeftToRead = len / Data.Converter->len_ratio;
		iStream = (char*) Data.Converter->buf;
	}
	else{ // Read directly into output buffer
		LeftToRead = len;
		iStream = (char*) stream;
	}

	while(LeftToRead){
		int BytesRead = ov_read(&(((MusicCore*) Core)->Source), iStream, LeftToRead, ((MusicCore*) Core)->VorbisEndian, ((MusicCore*) Core)->VorbisSize, ((MusicCore*) Core)->VorbisSigned, &PositionInStream);
		if(BytesRead > 0){
			LeftToRead -= BytesRead;
			iStream += BytesRead;
		}
		else if(BytesRead == 0)
			ov_pcm_seek(&(((MusicCore*) Core)->Source),0);
		else{
			memset(stream,0,len);
			return;
		}
	}
	if(Data.Converter){
		if(SDL_ConvertAudio(Data.Converter) != -1)
			memcpy(stream,Data.Converter->buf,len);
		else
			memset(stream,0,len);
	}
	return;
}

bool MusicCore::BGMdata::OpenFile(const char* Filename, const SDL_AudioSpec *ObtainedAudioSpec, OggVorbis_File *Source){
	if(Source == 0)
		return false;
	BGMdata::Filename = Filename;
	if(ov_fopen(Filename,Source) != 0)
		return false;
	vorbis_info *SourceInfo = ov_info(Source,-1);
	if((SourceInfo->rate != ObtainedAudioSpec->freq) || (SourceInfo->channels != ObtainedAudioSpec->channels)){
		Converter = new SDL_AudioCVT;
		if(SDL_BuildAudioCVT(Converter, ObtainedAudioSpec->format, SourceInfo->channels, SourceInfo->rate, ObtainedAudioSpec->format, ObtainedAudioSpec->channels, ObtainedAudioSpec->freq) == 0){
			ov_clear(Source);
			delete Converter;
			return false;
		}
		if(ConversionBuffer.size() < Converter->len_mult * AUDIO_DESIRED_SAMPLES)
			ConversionBuffer.resize(Converter->len_mult * AUDIO_DESIRED_SAMPLES);
		Converter->buf = (Uint8*) &(ConversionBuffer.at(0));
	}
	return true;
}

bool MusicCore::BGMdata::ResumeFile(OggVorbis_File *Source){
	if(Source == 0)
		return false;
	if(ov_fopen(Filename.c_str(),Source) != 0)
		return false;
	ov_pcm_seek(Source,PCM_Offset);
}
