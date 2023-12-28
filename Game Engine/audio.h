#ifndef AUDIO_H
#define AUDIO_H

#include <map>
#include <SDL_mixer.h>
#include <mutex>
#include <atomic>
#include "gameEngine.h"
#include <memory>
#include "audio_object.h"

class Audio {
	enum class AudioRequest {
		PLAY_SOUND_EFFECT,
		PLAY_UI_EFFECT,
		PLAY_DIALOG,
		PLAY_SOUNDTRACK,
		SET_MUSIC_VOLUME,
		SET_DIALOG_VOLUME,
		SET_SOUND_EFFECT_VOLUME,
		SET_UI_EFFECT_VOLUME,
		PAUSE_DIALOGS,
		PAUSE_UI_EFFECTS,
		PAUSE_MUSIC,
		PAUSE_SOUND_EFFECTS,
		RESUME_DIALOGS,
		RESUME_SOUND_EFFECTS,
		RESUME_UI_EFFECTS,
		PAUSE_AUDIO_OBJECT,
		RESUME_AUDIO_OBJECT,
		STOP_AUDIO_OBJECT,
		RESUME_MUSIC,
		STOP_MUSIC,
	};

	typedef struct mixChunkData {
		Mix_Chunk* sound;
		double length;
	}MixChunkData;

	typedef struct audioRequestData {
		AudioRequest type;
		EntityName name;
		int left;
		int right;
		int angle;
		int distance;
		int volume;
		std::shared_ptr <AudioObj> audio_obj;
	}AudioRequestData;
public:
	//Audio();
	Audio(int effectChannels, int dialogsChannels, int uiChannels);
	~Audio();
	
	unsigned long GetTaskQueueLen();

	void PollRequests();

	double GetDialogLen(EntityName dialog);
	double GetSoundEffectLen(EntityName effect);
	double GetUIEffectLen(EntityName effect);

	void PlaySoundEffect(EntityName sound, std::shared_ptr<AudioObj> = nullptr, bool spatial_sound = false, double maxDistance = 0, vector2 source_position = {0, 0});
	void PlayDialog(EntityName dialog, std::shared_ptr<AudioObj> = nullptr, bool spatial_sound = false, double maxDistance = 0, vector2 source_position = { 0, 0 });
	void PlayUIEffect(EntityName sound, std::shared_ptr<AudioObj> = nullptr, bool spatial_sound = false, double maxDistance = 0, vector2 source_position = { 0, 0 });
	bool AudioPlaying(int channel, unsigned long trackId);

	void PlayMusic(EntityName music);
	bool IsPlayingMusic();

	void SetMusicVolume(int volume);
	void SetSoundEffectVolume(int volume);
	void SetDialogsVolume(int volume);
	void SetUIEffectsVolume(int volume);

	int GetMusicVolume();
	int GetSoundEffectVolume();
	int GetDialogsVolume();
	int GetUIEffectsVolume();

	void PauseAll();
	void ResumeAll();
	/*void pauseSoundChannel(int channel);
	void resumeSoundChannel(int channel);*/

	void PauseSoundEffects();
	void ResumeSoundEffects();
	void PauseDialogs();
	void ResumeDialogs();
	void PauseUIEffects();
	void ResumeUIEffects();
	void PauseAudioObject(std::shared_ptr <AudioObj> audio_obj);
	void ResumeAudioObject(std::shared_ptr <AudioObj> audio_obj);
	void StopAudioObject(std::shared_ptr <AudioObj> audio_obj);

	void PauseMusic();
	void ResumeMusic();
	void StopMusic();
	void StopAll();
private:
	unsigned long int getChunkTimeMilliseconds(Mix_Chunk* chunk);
	void loadSoundFileInFolder(std::string directory);

	void PlaySoundEffect_Internal(EntityName sound, int left, int right, std::shared_ptr <AudioObj> audio_obj);
	void PlayDialog_Internal(EntityName dialog, int left, int right, std::shared_ptr <AudioObj> audio_obj);
	void PlayUIEffect_Internal(EntityName sound, int left, int right, std::shared_ptr <AudioObj> audio_obj);

	void PlayMusic_Internal(EntityName music);

	void SetMusicVolume_Internal(int volume);
	void SetSoundEffectVolume_Internal(int volume);
	void SetDialogsVolume_Internal(int volume);
	void SetUIEffectsVolume_Internal(int volume);

	void PauseSoundEffects_Internal();
	void ResumeSoundEffects_Internal();
	void PauseDialogs_Internal();
	void ResumeDialogs_Internal();
	void PauseUIEffects_Internal();
	void ResumeUIEffects_Internal();
	void PauseAudioObject_Internal(std::shared_ptr <AudioObj> audio_obj);
	void ResumeAudioObject_Internal(std::shared_ptr <AudioObj> audio_obj);
	void StopAudioObject_Internal(std::shared_ptr <AudioObj> audio_obj);

	void PauseMusic_Internal();
	void ResumeMusic_Internal();
	void StopMusic_Internal();

	std::map <EntityName, MixChunkData> _soundEffects;
	std::map <EntityName, MixChunkData> _dialogs;
	std::map <EntityName, MixChunkData> _uiSoundEffects;
	std::map <EntityName, Mix_Music*> _soundtracks;
	
	bool _mute;
	bool _playSoundtrack;
	bool _playSoundEffects;
	bool _playDialog;
	bool _playUISounds;
	bool _playRandomMusic;

	std::atomic <int> _musicVolume;
	std::atomic <int> _effectsVolume;
	std::atomic <int> _dialogVolume;
	std::atomic <int> _uiSoundVolume;

	int _channelsToAllocate;
	int _dialogChannels;
	int _uiSoundChannels;
	int _effectsChannel;

	bool playing_music;

	std::vector <AudioRequestData> _requests;
	std::shared_ptr <AudioObj>* _audioObjTracks;

	std::mutex update_mutex;
	std::mutex request_mutex;
};

#endif
