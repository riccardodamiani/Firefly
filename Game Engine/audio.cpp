#include "stdafx.h"
#include "audio.h"
#include "gameEngine.h"
#include "gameObject.h"

#include <SDL_mixer.h>
#include <filesystem>
#include <fstream>
#include <mutex>

namespace fs = std::filesystem;

Audio::Audio(int effectChannels, int dialogsChannels, int uiChannels) {
	this->_playRandomMusic = false;
	this->_playSoundEffects = true;
	this->_playSoundtrack = true;
	this->_playDialog = true;
	this->_playUISounds = true;

	this->_mute = false;
	this->_musicVolume = 100;
	this->_effectsVolume = 100;
	this->_dialogVolume = 100;
	this->_uiSoundVolume = 100;

	this->_channelsToAllocate = effectChannels + dialogsChannels + uiChannels;
	this->_dialogChannels = dialogsChannels;
	this->_uiSoundChannels = uiChannels;
	this->_effectsChannel = effectChannels;

	Mix_Init(MIX_INIT_MP3);

	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1)		//22050 / 44100
		printf("\n Mix_OpenAudio Failed");

	Mix_AllocateChannels(this->_channelsToAllocate);
	Mix_VolumeMusic(this->_musicVolume);

	//set dialogs channels
	Mix_GroupChannels(0, this->_dialogChannels-1, 0);
	for (int i = 0; i < this->_dialogChannels; i++) {
		Mix_Volume(i, this->_dialogVolume);
	}

	//set ui sounds channels
	Mix_GroupChannels(this->_dialogChannels, this->_dialogChannels + this->_uiSoundChannels - 1, 2);
	for (int i = this->_dialogChannels; i < this->_dialogChannels + this->_uiSoundChannels; i++) {
		Mix_Volume(i, this->_uiSoundVolume);
	}

	//set sound effect channels
	Mix_GroupChannels(this->_dialogChannels + this->_uiSoundChannels, this->_channelsToAllocate - 1, 1);
	for (int i = this->_dialogChannels + this->_uiSoundChannels; i < this->_channelsToAllocate; i++) {
		Mix_Volume(i, this->_effectsVolume);
	}

	this->loadSoundFileInFolder("Sound");		//load sound files
}

void Audio::loadSoundFileInFolder(std::string directory) {

	try {
		for (auto& dirEntry : fs::recursive_directory_iterator(directory)) {
			//std::cout << dirEntry << std::endl;
			std::string filename = dirEntry.path().string();
			this->loadSoundFileInFolder(filename);		//try to load files that are inside a folder
											//remove what's before and after the filename
			std::string name;
			size_t index = filename.find_last_of(".");
			if (index != std::string::npos) {
				name = filename.substr(0, index);
			}
			else {
				name.assign(filename);
			}
			index = name.find_last_of('\\') + 1;
			if (index != std::string::npos) {
				name = name.substr(index, name.size());
			}
			//load the sound file
			if (directory == "Sound\\Soundtrack") {		//is loading a soundtrack file
				Mix_Music* music = Mix_LoadMUS(filename.c_str());
				if (music != NULL) {
					EntityName ename = DecodeName(name.c_str());
					this->_soundtracks[ename] = music;
				}
				else {
					std::cout << "Cannot load soundtrack track " << filename.c_str() << std::endl;
				}
			}
			else if (directory == "Sound\\SoundEffects") {		//is loading a sound effect file
				Mix_Chunk* effect = Mix_LoadWAV(filename.c_str());
				if (effect != NULL) {
					EntityName ename = DecodeName(name.c_str());
					MixChunkData data;
					data.sound = effect;
					data.length = getChunkTimeMilliseconds(effect);
					this->_soundEffects[ename] = data;
				}
				else {
					std::cout << "Cannot load sound effect " << filename.c_str() << std::endl;
				}
			}
			else if (directory == "Sound\\Dialogs") {
				Mix_Chunk* dialog = Mix_LoadWAV(filename.c_str());
				if (dialog != NULL) {
					EntityName ename = DecodeName(name.c_str());
					MixChunkData data;
					data.sound = dialog;
					data.length = getChunkTimeMilliseconds(dialog);
					this->_dialogs[ename] = data;
				}
				else {
					std::cout << "Cannot load dialog " << filename.c_str() << std::endl;
				}
			}
			else if (directory == "Sound\\UIEffects") {
				Mix_Chunk* effect = Mix_LoadWAV(filename.c_str());
				if (effect != NULL) {
					EntityName ename = DecodeName(name.c_str());
					MixChunkData data;
					data.sound = effect;
					data.length = getChunkTimeMilliseconds(effect);
					this->_uiSoundEffects[ename] = data;
				}
				else {
					std::cout << "Cannot load ui effect " << filename.c_str() << std::endl;
				}
			}
		}
	}
	catch (std::filesystem::filesystem_error const& ex) {
		
	}
	
}

Audio::~Audio() {
	Mix_CloseAudio();
	Mix_Quit();
}

void Audio::PollRequests() {
	std::lock_guard <std::mutex> guard(request_mutex);

	playing_music = Mix_PlayingMusic();

	while (_requests.size() > 0) {
		AudioRequestData data = _requests[0];
		_requests.erase(_requests.begin());

		switch (data.type) {
		case AudioRequest::PLAY_SOUND_EFFECT:
			PlaySoundEffect_Internal(data.name, data.left, data.right);
			break;
		case AudioRequest::PLAY_UI_EFFECT:
			PlayUIEffect_Internal(data.name, data.left, data.right);
			break;

		case AudioRequest::PLAY_DIALOG:
			PlayDialog_Internal(data.name, data.left, data.right);
			break;

		case AudioRequest::PLAY_SOUNDTRACK:
			PlayMusic_Internal(data.name);
			break;
			
		case AudioRequest::SET_MUSIC_VOLUME:
			SetMusicVolume_Internal(data.volume);
			break;

		case AudioRequest::SET_DIALOG_VOLUME:
			SetDialogsVolume_Internal(data.volume);
			break;
		case AudioRequest::SET_SOUND_EFFECT_VOLUME:
			SetSoundEffectVolume_Internal(data.volume);
			break;

		case AudioRequest::SET_UI_EFFECT_VOLUME:
			SetUIEffectsVolume_Internal(data.volume);
			break;

		case AudioRequest::PAUSE_DIALOGS:
			PauseDialogs_Internal();
			break;

		case AudioRequest::PAUSE_UI_EFFECTS:
			PauseUIEffects_Internal();
			break;

		case AudioRequest::PAUSE_MUSIC:
			PauseMusic_Internal();
			break;

		case AudioRequest::PAUSE_SOUND_EFFECTS:
			PauseSoundEffects_Internal();
			break;

		case AudioRequest::RESUME_DIALOGS:
			ResumeDialogs_Internal();
			break;

		case AudioRequest::RESUME_SOUND_EFFECTS:
			ResumeSoundEffects_Internal();
			break;

		case AudioRequest::RESUME_UI_EFFECTS:
			ResumeUIEffects_Internal();
			break;

		case AudioRequest::RESUME_MUSIC:
			ResumeMusic_Internal();
			break;
		
		case AudioRequest::STOP_MUSIC:
			StopMusic_Internal();
			break;
		}
	}
}

double Audio::GetDialogLen(EntityName dialog) {
	std::lock_guard <std::mutex> guard(update_mutex);

	if (_dialogs.find(dialog) != _dialogs.end()) {
		return _dialogs[dialog].length;
	}
	return 0;
}

double Audio::GetSoundEffectLen(EntityName effect) {
	std::lock_guard <std::mutex> guard(update_mutex);

	if (this->_soundEffects.find(effect) != _soundEffects.end()) {
		return _dialogs[effect].length;
	}
	return 0;
}

double Audio::GetUIEffectLen(EntityName effect) {
	std::lock_guard <std::mutex> guard(update_mutex);

	if (this->_uiSoundEffects.find(effect) != _uiSoundEffects.end()) {
		return _dialogs[effect].length;
	}
	return 0;
}

bool Audio::IsPlayingMusic() {
	return playing_music;
}

unsigned long int Audio::getChunkTimeMilliseconds(Mix_Chunk *chunk){
	Uint32 points = 0;
	Uint32 frames = 0;
	int freq = 0;
	Uint16 fmt = 0;
	int chans = 0;
	// Chunks are converted to audio device format…
	if (!Mix_QuerySpec(&freq, &fmt, &chans))
		return 0; // never called Mix_OpenAudio() ? !

	// bytes / samplesize == sample points 
	points = (chunk->alen / ((fmt & 0xFF) / 8));

	// sample points / channels == sample frames
	frames = (points / chans);

	// (sample frames * 1000) / frequency == play length in ms */
	return ((frames * 1000) / freq);
}


void Audio::PlaySoundEffect(EntityName sound, bool spatial_sound, double maxDistance, vector2 source_position) {
	std::lock_guard <std::mutex> guard(request_mutex);

	if (!this->_playSoundEffects)
		return;
	double left, right;
	left = right = 255;
	if (spatial_sound) {
		GameObject* camera = _GameEngine->FindGameObject(DecodeName("MainCamera"));
		if (camera) {
			vector2 cameraPos = camera->transform.position;
			vector2 cameraScale = camera->transform.scale;
			double angle = ((source_position.x - cameraPos.x) / (cameraScale.x / 2)) * 255.0;
			double distance = sqrt((source_position.x - cameraPos.x) * (source_position.x - cameraPos.x) + (source_position.y - cameraPos.y) * (source_position.y - cameraPos.y));
			distance = (distance / maxDistance) * 255;
			if (angle < 0) right += angle;
			else left -= angle;
			right -= distance;
			left -= distance;
			if (right < 0) right = 0;
			if (left < 0) left = 0;
		}
	}

	AudioRequestData data;
	data.name = sound;
	data.left = left;
	data.right = right;
	data.type = AudioRequest::PLAY_SOUND_EFFECT;
	_requests.push_back(data);
}


void Audio::PlayDialog(EntityName dialog, bool spatial_sound, double maxDistance, vector2 source_position) {
	std::lock_guard <std::mutex> guard(request_mutex);

	if (!this->_playDialog)
		return;
	double left, right;
	left = right = 255;
	if (spatial_sound) {
		GameObject* camera = _GameEngine->FindGameObject(DecodeName("MainCamera"));
		if (camera) {
			vector2 cameraPos = camera->transform.position;
			vector2 cameraScale = camera->transform.scale;
			double angle = ((source_position.x - cameraPos.x) / (cameraScale.x / 2)) * 255.0;
			double distance = sqrt((source_position.x - cameraPos.x) * (source_position.x - cameraPos.x) + (source_position.y - cameraPos.y) * (source_position.y - cameraPos.y));
			distance = (distance / maxDistance) * 255;
			if (angle < 0) right += angle;
			else left -= angle;
			right -= distance;
			left -= distance;
			if (right < 0) right = 0;
			if (left < 0) left = 0;
		}
	}

	AudioRequestData data;
	data.name = dialog;
	data.left = left;
	data.right = right;
	data.type = AudioRequest::PLAY_DIALOG;
	_requests.push_back(data);
}

void Audio::PlayUIEffect(EntityName sound, bool spatial_sound, double maxDistance, vector2 source_position) {
	std::lock_guard <std::mutex> guard(request_mutex);

	if (!this->_playUISounds)
		return;
	double left, right;
	left = right = 255;
	if (spatial_sound) {
		GameObject* camera = _GameEngine->FindGameObject(DecodeName("MainCamera"));
		if (camera) {
			vector2 cameraPos = camera->transform.position;
			vector2 cameraScale = camera->transform.scale;
			double angle = ((source_position.x - cameraPos.x) / (cameraScale.x/2)) * 255.0;
			double distance = sqrt((source_position.x - cameraPos.x) * (source_position.x - cameraPos.x) + (source_position.y - cameraPos.y) * (source_position.y - cameraPos.y));
			distance = (distance/maxDistance)*255;
			if (angle < 0) right += angle;
			else left -= angle;
			right -= distance;
			left -= distance;
			if (right < 0) right = 0;
			if (left < 0) left = 0;
		}
	}

	AudioRequestData data;
	data.name = sound;
	data.left = left;
	data.right = right;
	data.type = AudioRequest::PLAY_UI_EFFECT;
	_requests.push_back(data);
}

void Audio::PlayMusic(EntityName music) {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData data;
	data.name = music;
	data.type = AudioRequest::PLAY_SOUNDTRACK;
	_requests.push_back(data);

}


void Audio::SetMusicVolume(int volume) {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData data;
	data.volume = volume;
	data.type = AudioRequest::SET_MUSIC_VOLUME;
	_requests.push_back(data);
}

void Audio::SetSoundEffectVolume(int volume) {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData data;
	data.volume = volume;
	data.type = AudioRequest::SET_SOUND_EFFECT_VOLUME;
	_requests.push_back(data);
}

void Audio::SetDialogsVolume(int volume) {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData data;
	data.volume = volume;
	data.type = AudioRequest::SET_DIALOG_VOLUME;
	_requests.push_back(data);
}

void Audio::SetUIEffectsVolume(int volume) {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData data;
	data.volume = volume;
	data.type = AudioRequest::SET_UI_EFFECT_VOLUME;
	_requests.push_back(data);
}

int Audio::GetMusicVolume() {
	return _musicVolume;
}

int Audio::GetSoundEffectVolume() {
	return _effectsVolume;
	
}

int Audio::GetDialogsVolume() {
	return _dialogVolume;
}

int Audio::GetUIEffectsVolume() {
	return _uiSoundVolume;
}

void Audio::PauseAll() {

	PauseSoundEffects();
	PauseDialogs();
	PauseUIEffects();
	PauseMusic();
}

void Audio::ResumeAll() {
	ResumeSoundEffects();
	ResumeDialogs();
	ResumeUIEffects();
	ResumeMusic();
}
/*
void Audio::pauseSoundChannel(int channel) {
	std::lock_guard <std::mutex> guard(update_mutex);
	Mix_Pause(channel);
}

void Audio::resumeSoundChannel(int channel) {
	std::lock_guard <std::mutex> guard(update_mutex);
	Mix_Resume(channel);
}
*/
void Audio::PauseSoundEffects() {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData data;
	data.type = AudioRequest::PAUSE_SOUND_EFFECTS;
	_requests.push_back(data);
}

void Audio::PauseDialogs() {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData data;
	data.type = AudioRequest::PAUSE_DIALOGS;
	_requests.push_back(data);
}

void Audio::PauseUIEffects() {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData data;
	data.type = AudioRequest::PAUSE_UI_EFFECTS;
	_requests.push_back(data);
}

void Audio::ResumeSoundEffects() {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData data;
	data.type = AudioRequest::RESUME_SOUND_EFFECTS;
	_requests.push_back(data);
}

void Audio::ResumeDialogs() {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData data;
	data.type = AudioRequest::RESUME_DIALOGS;
	_requests.push_back(data);
}

void Audio::ResumeUIEffects() {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData data;
	data.type = AudioRequest::RESUME_UI_EFFECTS;
	_requests.push_back(data);
}

void Audio::PauseMusic() {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData data;
	data.type = AudioRequest::PAUSE_MUSIC;
	_requests.push_back(data);
}

void Audio::ResumeMusic() {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData data;
	data.type = AudioRequest::RESUME_MUSIC;
	_requests.push_back(data);
}

void Audio::StopMusic() {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData data;
	data.type = AudioRequest::STOP_MUSIC;
	_requests.push_back(data);
}

void Audio::StopAll() {

	this->PauseAll();
	this->StopMusic();
	
}

void Audio::PlaySoundEffect_Internal(EntityName sound, int left, int right) {
	std::lock_guard <std::mutex> guard(update_mutex);

	if (!this->_playSoundEffects)
		return;

	int availCh = Mix_GroupAvailable(1);

	if (availCh != -1 && this->_soundEffects.find(sound) != _soundEffects.end()) {
		Mix_SetPanning(availCh, left, right);
		Mix_PlayChannel(availCh, this->_soundEffects[sound].sound, 0);
	}
	return;
}

void Audio::PlayDialog_Internal(EntityName dialog, int left, int right) {
	std::lock_guard <std::mutex> guard(update_mutex);

	if (!this->_playDialog)
		return;

	int availCh = Mix_GroupAvailable(0);

	if (availCh != -1 && this->_dialogs.find(dialog) != _dialogs.end()) {
		Mix_SetPanning(availCh, left, right);
		Mix_PlayChannel(availCh, this->_dialogs[dialog].sound, 0);
	}
	return;
}


void Audio::PlayUIEffect_Internal(EntityName sound, int left, int right) {
	std::lock_guard <std::mutex> guard(update_mutex);

	if (!this->_playUISounds)
		return;

	int availCh = Mix_GroupAvailable(2);

	if (availCh != -1 && this->_uiSoundEffects.find(sound) != _uiSoundEffects.end()) {
		//Mix_SetPosition(availCh, angle, distance);
		Mix_SetPanning(availCh, left, right);
		Mix_PlayChannel(availCh, this->_uiSoundEffects[sound].sound, 0);
		return;
	}
	return;
}

void Audio::PlayMusic_Internal(EntityName music) {
	std::lock_guard <std::mutex> guard(update_mutex);

	if (this->_soundtracks.find(music) != _soundtracks.end() && this->_playSoundtrack) {
		Mix_PlayMusic(this->_soundtracks[music], 0);
	}
}

void Audio::SetMusicVolume_Internal(int volume) {
	std::lock_guard <std::mutex> guard(update_mutex);
	this->_musicVolume = volume;
	Mix_VolumeMusic(this->_musicVolume);
}


void Audio::SetSoundEffectVolume_Internal(int volume) {
	std::lock_guard <std::mutex> guard(update_mutex);
	this->_effectsVolume = volume;
	for (int i = this->_dialogChannels + this->_uiSoundChannels; i < this->_channelsToAllocate; i++) {
		Mix_Volume(i, this->_effectsVolume);
	}
}

void Audio::SetDialogsVolume_Internal(int volume) {
	std::lock_guard <std::mutex> guard(update_mutex);
	this->_dialogVolume = volume;
	for (int i = 0; i < this->_dialogChannels; i++) {
		Mix_Volume(i, this->_dialogVolume);
	}
}

void Audio::SetUIEffectsVolume_Internal(int volume) {
	std::lock_guard <std::mutex> guard(update_mutex);
	this->_uiSoundVolume = volume;
	for (int i = this->_dialogChannels; i < this->_dialogChannels + this->_uiSoundChannels; i++) {
		Mix_Volume(i, this->_uiSoundVolume);
	}
}


void Audio::PauseSoundEffects_Internal() {
	std::lock_guard <std::mutex> guard(update_mutex);
	for (int i = this->_dialogChannels + this->_uiSoundChannels; i < this->_channelsToAllocate; i++) {
		Mix_Pause(i);
	}
}

void Audio::ResumeSoundEffects_Internal() {
	std::lock_guard <std::mutex> guard(update_mutex);
	for (int i = this->_dialogChannels; i < this->_channelsToAllocate; i++) {
		Mix_Resume(i);
	}
}

void Audio::PauseDialogs_Internal() {
	std::lock_guard <std::mutex> guard(update_mutex);
	for (int i = 0; i < this->_dialogChannels; i++) {
		Mix_Pause(i);
	}
}

void Audio::ResumeDialogs_Internal() {
	std::lock_guard <std::mutex> guard(update_mutex);
	for (int i = 0; i < this->_dialogChannels; i++) {
		Mix_Resume(i);
	}
}

void Audio::PauseUIEffects_Internal() {
	std::lock_guard <std::mutex> guard(update_mutex);
	for (int i = this->_dialogChannels; i < this->_dialogChannels + this->_uiSoundChannels; i++) {
		Mix_Pause(i);
	}
}

void Audio::ResumeUIEffects_Internal() {
	std::lock_guard <std::mutex> guard(update_mutex);
	for (int i = this->_dialogChannels; i < this->_dialogChannels + this->_uiSoundChannels; i++) {
		Mix_Resume(i);
	}
}


void Audio::PauseMusic_Internal() {
	std::lock_guard <std::mutex> guard(update_mutex);
	Mix_PausedMusic();
}

void Audio::ResumeMusic_Internal() {
	std::lock_guard <std::mutex> guard(update_mutex);
	Mix_ResumeMusic();
}

void Audio::StopMusic_Internal() {
	std::lock_guard <std::mutex> guard(update_mutex);
	Mix_RewindMusic();
	//Mix_PausedMusic();
	Mix_HaltMusic();
}
