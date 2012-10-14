#ifndef MUSIC_CORE_H
#define MUSIC_CORE_H

#include <vorbis/vorbisfile.h>
#include <stack>
#include <string>
#include <vector>

struct SDL_AudioCVT;
struct SDL_AudioSpec;

class MusicCore{
	struct BGMdata{
		static std::vector<char> ConversionBuffer;
		OggVorbis_File Source;
		std::string Filename;
		unsigned long long PCM_Offset;
		SDL_AudioCVT* Converter;
	public:
		BGMdata() : Converter(0){};
		bool OpenFile(const char* Filename, const SDL_AudioSpec *ObtainedAudioSpec);
	};
	SDL_AudioSpec *ObtainedAudioSpec;
	std::stack<BGMdata> MusicQueue;
	int VorbisEndian;
	int VorbisSize;
	int VorbisSigned;
private:
	MusicCore(MusicCore &NoCopiesAllowed);
	MusicCore(const MusicCore &NoCopiesAllowed);
	MusicCore& operator=(MusicCore &NoCopiesAllowed);
	MusicCore& operator=(const MusicCore &NoCopiesAllowed);
public:
	MusicCore();
	~MusicCore();
	void Pause();
	void Play();
	void Lock();
	void Unlock();
	bool SetBgm(const char* Filename);
	bool PushBgm(const char* Filename);
	bool PopBgm();
	void ClearBgm();
	static void FillAudioBuffer(void* Core, uint8_t *stream, int len);
};

#endif
