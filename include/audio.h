#ifndef AUDIO_H
#define AUDIO_H

#include <map>
#include <mutex>
#include <atomic>
#include "gameEngine.h"
#include <memory>
#include <vector>

#include "audio_source.h"
#include "entity.h"

struct Mix_Chunk;
struct _Mix_Music;
typedef struct _Mix_Music Mix_Music;

class AudioEngine {
	enum class AudioRequest {
		PLAY_AUDIOTRACK,
		PLAY_MUSIC,
		SET_GROUP_VOLUME,
		SET_MUSIC_VOLUME,
		PAUSE_GROUP,
		PAUSE_MUSIC,
		RESUME_GROUP,
		RESUME_MUSIC,
		STOP_MUSIC,
		STOP_GROUP,
		PAUSE_AUDIO_SOURCE,
		RESUME_AUDIO_SOURCE,
		STOP_AUDIO_SOURCE,
		UPDATE_AUDIO_SOURCE
	};

	class AudioRequestData {
	public:
		virtual ~AudioRequestData() {};
		AudioRequest type;
	};

	class AudioPlayData : public AudioRequestData {
	public:
		EntityName audioSrcName;
		EntityName audioName;
		vector2 position;
		uint8_t audioGroup;
		int maxDistance;
		bool spatial_sound;
	};

	class MusicPlayData : public AudioRequestData {
	public:
		EntityName audioName;
	};

	class AudioVolumeData : public AudioRequestData {
	public:
		uint8_t group;
		uint8_t volume;
	};

	class AudioControlData : public AudioRequestData {
	public:
		uint8_t group;
	};

	class AudioSrcControlData : public AudioRequestData {
	public:
		EntityName audioSrcName;
	};

	class AudioSrcUpdateData : public AudioRequestData {
	public:
		EntityName audioSrcName;
		uint16_t channel;
		vector2 position;
		uint8_t group;
		double maxDistance;
	};


	typedef struct mixChunkData {
		Mix_Chunk* sound;
		double length;
	}MixChunkData;
public:
    static AudioEngine& getInstance() {
        static AudioEngine instance;
        return instance;
    }

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

	void Init(AudioOptions& options);
	unsigned long GetTaskQueueLen();

	void PollRequests();

	double GetTrackLen(EntityName trackName);

	void PlayTrack(EntityName trackName, uint16_t audioGroup, double maxDistance, 
		vector2 source_position, bool spatial_sound, EntityName audioSrcName);
	bool AudioSourcePlaying(int channel, EntityName audioSource);

	void PlayMusic(EntityName music);
	bool IsPlayingMusic();

	void SetMusicVolume(uint8_t volume);
	void SetGroupVolume(uint8_t volume, uint8_t audioGroup);

	int GetMusicVolume();
	int GetGroupVolume(uint8_t audioGroup);

	void PauseAll();
	void ResumeAll();
	void StopAll();

	void PauseGroup(uint8_t audioGroup);
	void ResumeGroup(uint8_t audioGroup);
	void StopGroup(uint8_t audioGroup);

	void PauseAudioSource(EntityName audioSrcName);
	void ResumeAudioSource(EntityName audioSrcName);
	void StopAudioSource(EntityName audioSrcName);
	void updateAudioSource(EntityName audioSrcName, uint16_t channel, vector2 position, double distance, uint8_t group);

	void PauseMusic();
	void ResumeMusic();
	void StopMusic();

	void ClearAudio();
private:
	AudioEngine();
	~AudioEngine();

	unsigned long int getChunkTimeMilliseconds(Mix_Chunk* chunk);
	bool calc_spatial_sound_panning(double maxDistance, vector2 position, uint8_t& left, uint8_t& right);
	void loadSoundFileInFolder(std::string directory);

	bool PlayTrack_Internal(AudioPlayData*);

	void PlayMusic_Internal(EntityName music);

	void SetMusicVolume_Internal(AudioVolumeData*);
	void SetGroupVolume_Internal(AudioVolumeData*);

	void PauseGroup_Internal(AudioControlData*);
	void ResumeGroup_Internal(AudioControlData* data);
	void StopGroup_Internal(AudioControlData* data);

	void PauseAudioSource_Internal(AudioSrcControlData*);
	void ResumeAudioSource_Internal(AudioSrcControlData*);
	void StopAudioSource_Internal(AudioSrcControlData*);
	void updateAudioSource_Internal(AudioSrcUpdateData*);

	void PauseMusic_Internal();
	void ResumeMusic_Internal();
	void StopMusic_Internal();

	std::map <EntityName, MixChunkData> _audioChunks;
	std::map <EntityName, Mix_Music*> _musicTracks;
	
	bool _mute;
	bool _playSoundtrack;
	bool _playRandomMusic;

	std::atomic <int> _musicVolume;
	std::atomic <int>* _audioGroupVolume;
	uint8_t _defaultMusicVolume;
	uint8_t _defaultTrackVolume;

	int _channelsToAllocate;
	std::vector <unsigned short> _audioGroupChannels;
	//first value is the starting channel of the group, second value is the ending channel of the group (included)
	std::pair <uint16_t, uint16_t>* _audioGroupChRange;

	bool playing_music;

	std::vector <AudioRequestData*> _requests;
	std::vector <AudioRequestData*> _rescheduledRequests;

	//keeps track of which audio source is in control of a audio channel
	EntityName *_channelMaster;

	std::mutex update_mutex;
	std::mutex request_mutex;
};

#endif
