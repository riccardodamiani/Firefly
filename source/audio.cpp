#include "stdafx.h"
#include "audio.h"
#include "gameEngine.h"
#include "gameObject.h"

#include <SDL_mixer.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <memory>
#include <math.h>

namespace fs = std::filesystem;

Audio::Audio() {
	
}

Audio::~Audio() {
	Mix_CloseAudio();
	Mix_Quit();
}

void Audio::Init() {
	this->_playRandomMusic = false;

	_mute = false;

	_channelMaster = new EntityName[_channelsToAllocate];
	memset(_channelMaster, 0, sizeof(EntityName)*_channelsToAllocate);

	Mix_Init(MIX_INIT_MP3);

	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1)		//22050 / 44100
		printf("\n Mix_OpenAudio Failed");

	Mix_AllocateChannels(_channelsToAllocate);
	Mix_VolumeMusic(_defaultMusicVolume);
	_musicVolume = _defaultMusicVolume;

	_audioGroupChRange = new std::pair <uint16_t, uint16_t>[_audioGroupChannels.size()];
	_audioGroupVolume = new std::atomic<int>[_audioGroupChannels.size()];
	//set groups channels
	int current_ch = 0;
	for (int i = 0; i < _audioGroupChannels.size(); i++) {
		_audioGroupChRange[i] = (std::pair<uint16_t, uint16_t>(current_ch, current_ch + _audioGroupChannels[i] - 1));
		Mix_GroupChannels(current_ch, current_ch + _audioGroupChannels[i] - 1, i);
		current_ch += _audioGroupChannels[i];
		_audioGroupVolume[i] = _defaultTrackVolume;
	}

	for (int i = 0; i < _channelsToAllocate; i++) {
		Mix_Volume(i, _defaultTrackVolume);
	}

	this->loadSoundFileInFolder("Sound");		//load sound files
}

void Audio::ConfigEngine(std::vector <unsigned short>& groupChannels, uint8_t defaultMusicVol, uint8_t defaultTrackVol) {
	_channelsToAllocate = 0;

	_defaultMusicVolume = defaultMusicVol;
	_defaultTrackVolume = defaultTrackVol;
	_audioGroupChannels = groupChannels;
	for (auto it = _audioGroupChannels.begin(); it != _audioGroupChannels.end(); it++) {
		_channelsToAllocate += *it;
	}
}

unsigned long Audio::GetTaskQueueLen() {
	return _requests.size();
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
			if (directory == "Sound\\Music") {		//is loading a soundtrack file
				Mix_Music* music = Mix_LoadMUS(filename.c_str());
				if (music != NULL) {
					EntityName ename = DecodeName(name.c_str());
					this->_musicTracks[ename] = music;
				}
				else {
					std::cout << "Cannot load soundtrack track " << filename.c_str() << std::endl;
				}
			}
			else if (directory == "Sound\\Tracks") {		//is loading a audio file
				Mix_Chunk* effect = Mix_LoadWAV(filename.c_str());
				if (effect != NULL) {
					EntityName ename = DecodeName(name.c_str());
					MixChunkData data;
					data.sound = effect;
					data.length = getChunkTimeMilliseconds(effect);
					_audioChunks[ename] = data;
					std::cout << "Registered " << name.c_str() << " as " << ename << std::endl;
				}
				else {
					std::cout << "Cannot load sound effect " << filename.c_str() << std::endl;
				}
			}
		}
	}
	catch (std::filesystem::filesystem_error const& ex) {
		
	}
	
}

void Audio::PollRequests() {
	std::lock_guard <std::mutex> guard(request_mutex);

	playing_music = Mix_PlayingMusic();

	//add the rescheduled requests
	for (auto it = _rescheduledRequests.begin(); it != _rescheduledRequests.end(); it++) {
		_requests.push_back(*it);
	}
	_rescheduledRequests.clear();

	while (_requests.size() > 0) {
		AudioRequestData *data = _requests[0];
		_requests.erase(_requests.begin());

		switch (data->type) {
		case AudioRequest::PLAY_AUDIOTRACK:
			if (!PlayTrack_Internal(dynamic_cast<AudioPlayData*>(data))) {
				_rescheduledRequests.push_back(data);
				break;
			}
			delete data;
			break;
		case AudioRequest::SET_MUSIC_VOLUME:
			SetMusicVolume_Internal(dynamic_cast<AudioVolumeData*>(data));
			delete data;
			break;

		case AudioRequest::SET_GROUP_VOLUME:
			SetGroupVolume_Internal(dynamic_cast<AudioVolumeData*>(data));
			delete data;
			break;

		case AudioRequest::PAUSE_GROUP:
			PauseGroup_Internal(dynamic_cast<AudioControlData*>(data));
			delete data;
			break;

		case AudioRequest::PAUSE_MUSIC:
			PauseMusic_Internal();
			delete data;
			break;

		case AudioRequest::RESUME_GROUP:
			ResumeGroup_Internal(dynamic_cast<AudioControlData*>(data));
			delete data;
			break;

		case AudioRequest::RESUME_MUSIC:
			ResumeMusic_Internal();
			delete data;
			break;

		case AudioRequest::STOP_MUSIC:
			StopMusic_Internal();
			delete data;
			break;

		case AudioRequest::STOP_GROUP:
			StopGroup_Internal(dynamic_cast<AudioControlData*>(data));
			delete data;
			break;

		case AudioRequest::PAUSE_AUDIO_SOURCE:
			PauseAudioSource_Internal(dynamic_cast<AudioSrcControlData*>(data));
			delete data;
			break;

		case AudioRequest::STOP_AUDIO_SOURCE:
			StopAudioSource_Internal(dynamic_cast<AudioSrcControlData*>(data));
			delete data;
			break;

		case AudioRequest::RESUME_AUDIO_SOURCE:
			ResumeAudioSource_Internal(dynamic_cast<AudioSrcControlData*>(data));
			delete data;
			break;

		case AudioRequest::UPDATE_AUDIO_SOURCE:
			updateAudioSource_Internal(dynamic_cast<AudioSrcUpdateData*>(data));
			delete data;
			break;
		default:
			delete data;
			break;
		}	//switch
	}
}

double Audio::GetTrackLen(EntityName trackName) {
	std::lock_guard <std::mutex> guard(update_mutex);

	if (_audioChunks.find(trackName) != _audioChunks.end()) {
		return _audioChunks[trackName].length / 1000.0;
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

bool Audio::calc_spatial_sound_panning(double maxDistance, vector2 position, uint8_t& left, uint8_t& right) {
	double Dleft, Dright;
	double power = 255;
	left = right = 255;
	//calculate panning for spatial sound
	GameObject* camera = _GameEngine->FindGameObject(DecodeName("MainCamera"));
	if (!camera) {
		return false;
	}
	vector2 cameraPos = camera->transform.position;
	double _distance = sqrt((position.x - cameraPos.x) * (position.x - cameraPos.x) +
		(position.y - cameraPos.y) * (position.y - cameraPos.y));
	//vector2 relPos = { position.x - cameraPos.x, position.y - cameraPos.y };
	//double _angle = atan2(relPos.y, relPos.x) * 180.0 / PI;
	power = 255.0 - 255.0 * (_distance / maxDistance);
	//_distance = (_distance / maxDistance) * 255.0;
	if (power < 0) power = 0;
	double panningVal = (position.x - cameraPos.x) / maxDistance;
	if (panningVal > 1) panningVal = 1;
	if (panningVal < -1) panningVal = -1;
	Dright = (power / 2.0) + panningVal * (power / 2.0);
	right = Dright;
	left = power - Dright;
	return true;
}

/*
Plays an audio track.

trackName: name of the audio track file
audioGroup: audio group to play the track in
maxDistance: maximum distance the sound can be hear from (leave blank if not using spacial sound)
source_position: source position of the sound (leave blank if not using spacial sound)
spatial_sound: set to true to enable spatial sound (leave blank if not using spatial sound)
audioSrcName: internal use only (leave blank)
*/
void Audio::PlayTrack(EntityName trackName,
	uint16_t audioGroup, double maxDistance = 0, vector2 source_position = {0, 0},
	bool spatial_sound = false, EntityName audioSrcName = 0) {
	
	if (trackName == 0 || audioGroup >= _audioGroupChannels.size())
		return;

	AudioPlayData *data = new AudioPlayData();
	data->type = AudioRequest::PLAY_AUDIOTRACK;
	data->audioName = trackName;
	data->audioSrcName = audioSrcName;
	data->maxDistance = maxDistance;
	data->position = source_position;
	data->spatial_sound = spatial_sound;
	data->audioGroup = audioGroup;

	std::lock_guard <std::mutex> guard(request_mutex);
	_requests.push_back(data);
}

bool Audio::PlayTrack_Internal(AudioPlayData* data) {
	if (!data)
		return true;
	AudioSource* as = nullptr;
	if (data->audioSrcName != 0) {
		as = dynamic_cast<AudioSource*>(_GameEngine->FindGameObject(data->audioSrcName));
		if (!as) {
			return false;	//not created yet: reschedule the request
		}
	}

	uint8_t left, right;
	calc_spatial_sound_panning(data->maxDistance, data->position, left, right);
	
	std::lock_guard <std::mutex> guard(update_mutex);
	int availCh = Mix_GroupAvailable(data->audioGroup);
	if (availCh != -1 && _audioChunks.find(data->audioName) != _audioChunks.end()) {
		//allocate a channel and start the sound
		Mix_SetPanning(availCh, left, right);
		Mix_PlayChannel(availCh, _audioChunks[data->audioName].sound, 0);
		_channelMaster[availCh] = data->audioSrcName;
		//update the audio source object channel and status
		if (data->audioSrcName != 0) {
			as->__setChannel(availCh);
		}
	}
	return true;
}

bool Audio::AudioSourcePlaying(int channel, EntityName audioSource) {
	if (channel < 0 || channel > _channelsToAllocate)
		return false;

	if (_channelMaster[channel] == 0)
		return false;
	
	if (_channelMaster[channel] == audioSource && Mix_Playing(channel)) {
		return true;
	}

	return false;
}

void Audio::PlayMusic(EntityName music) {
	std::lock_guard <std::mutex> guard(request_mutex);

	MusicPlayData *data = new MusicPlayData();
	data->audioName = music;
	data->type = AudioRequest::PLAY_MUSIC;
	_requests.push_back(data);

}


void Audio::SetMusicVolume(uint8_t volume) {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioVolumeData* data = new AudioVolumeData();
	data->volume = volume;
	data->type = AudioRequest::SET_MUSIC_VOLUME;
	_requests.push_back(data);
}

int Audio::GetMusicVolume() {
	return _musicVolume;
}

int Audio::GetGroupVolume(uint8_t audioGroup) {
	return _audioGroupVolume[audioGroup];
	
}

void Audio::PauseAll() {

	for (int i = 0; i < _audioGroupChannels.size(); i++) {
		PauseGroup(i);
	}
	PauseMusic();
}

void Audio::ResumeAll() {
	for (int i = 0; i < _audioGroupChannels.size(); i++) {
		ResumeGroup(i);
	}
	ResumeMusic();
}

void Audio::StopAll() {

	for (int i = 0; i < _audioGroupChannels.size(); i++) {
		StopGroup(i);
	}
	StopMusic();
}

void Audio::PauseGroup(uint8_t audioGroup) {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioControlData* data = new AudioControlData();
	data->type = AudioRequest::PAUSE_GROUP;
	data->group = audioGroup;
	_requests.push_back(data);
}

void Audio::PauseGroup_Internal(AudioControlData *data) {
	std::lock_guard <std::mutex> guard(update_mutex);
	for (int i = _audioGroupChRange[data->group].first; i <= _audioGroupChRange[data->group].second; i++) {
		Mix_Pause(i);
		if (_channelMaster[i] != 0) {
			AudioSource* as = dynamic_cast<AudioSource*>(_GameEngine->FindGameObject(_channelMaster[i]));
			if (as == nullptr)	//invalid channel
				continue;
			as->__setStatus(AudioStatus::AUDIO_STATUS_PAUSE);
		}
	}
}

void Audio::StopGroup(uint8_t audioGroup) {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioControlData* data = new AudioControlData();
	data->type = AudioRequest::STOP_GROUP;
	data->group = audioGroup;
	_requests.push_back(data);
}

void Audio::StopGroup_Internal(AudioControlData* data) {
	if (!data) return;
	std::lock_guard <std::mutex> guard(update_mutex);
	for (int i = _audioGroupChRange[data->group].first; i <= _audioGroupChRange[data->group].second; i++) {
		if (_channelMaster[i] != 0 && Mix_Playing(i)) {
			AudioSource* as = dynamic_cast<AudioSource*>(_GameEngine->FindGameObject(_channelMaster[i]));
			if (as == nullptr)	//invalid channel
				continue;
			as->__setStatus(AudioStatus::AUDIO_STATUS_STOPPED);
		}
		Mix_HaltChannel(i);
	}
}

void Audio::ResumeGroup(uint8_t audioGroup) {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioControlData* data = new AudioControlData();
	data->type = AudioRequest::RESUME_GROUP;
	data->group = audioGroup;
	_requests.push_back(data);
}

void Audio::ResumeGroup_Internal(AudioControlData* data) {
	std::lock_guard <std::mutex> guard(update_mutex);
	for (int i = _audioGroupChRange[data->group].first; i <= _audioGroupChRange[data->group].second; i++) {
		if (_channelMaster[i] != 0 && Mix_Paused(i)) {
			AudioSource* as = dynamic_cast<AudioSource*>(_GameEngine->FindGameObject(_channelMaster[i]));
			if (as == nullptr)	//invalid channel
				continue;
			as->__setStatus(AudioStatus::AUDIO_STATUS_PLAYING);
		}
		Mix_Resume(i);
	}
}

void Audio::PauseAudioSource(EntityName audioSrcName) {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioSrcControlData* data = new AudioSrcControlData();
	data->type = AudioRequest::PAUSE_AUDIO_SOURCE;
	data->audioSrcName = audioSrcName;
	_requests.push_back(data);
}

void Audio::PauseAudioSource_Internal(AudioSrcControlData* data) {
	std::lock_guard <std::mutex> guard(update_mutex);

	AudioSource* as = dynamic_cast<AudioSource*>(_GameEngine->FindGameObject(data->audioSrcName));

	if (as == nullptr)	//invalid channel
		return;

	uint8_t channel = as->getAudioChannel();

	if (channel >= _channelsToAllocate)
		return;

	if (_channelMaster[channel] != 0 && _channelMaster[channel] == data->audioSrcName && Mix_Playing(channel)) {
		Mix_Pause(channel);
		as->__setStatus(AudioStatus::AUDIO_STATUS_PAUSE);
	}
}

void Audio::ResumeAudioSource(EntityName audioSrcName) {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioSrcControlData* data = new AudioSrcControlData();
	data->type = AudioRequest::RESUME_AUDIO_SOURCE;
	data->audioSrcName = audioSrcName;
	_requests.push_back(data);
}

void Audio::ResumeAudioSource_Internal(AudioSrcControlData* data) {
	std::lock_guard <std::mutex> guard(update_mutex);

	AudioSource* as = dynamic_cast<AudioSource*>(_GameEngine->FindGameObject(data->audioSrcName));

	if (as == nullptr)	//invalid channel
		return;

	uint8_t channel = as->getAudioChannel();

	if (channel >= _channelsToAllocate)
		return;

	if (_channelMaster[channel] != 0 && _channelMaster[channel] == data->audioSrcName) {
		if (Mix_Playing(channel) && Mix_Paused(channel)) {	//channel still playing but paused
			Mix_Resume(channel);
			as->__setStatus(AudioStatus::AUDIO_STATUS_PLAYING);
		}
	}
}


void Audio::StopAudioSource(EntityName audioSrcName) {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioSrcControlData* data = new AudioSrcControlData();
	data->type = AudioRequest::STOP_AUDIO_SOURCE;
	data->audioSrcName = audioSrcName;
	_requests.push_back(data);
}

void Audio::StopAudioSource_Internal(AudioSrcControlData* data) {
	std::lock_guard <std::mutex> guard(update_mutex);

	AudioSource* as = dynamic_cast<AudioSource*>(_GameEngine->FindGameObject(data->audioSrcName));

	if (as == nullptr)	//invalid channel
		return;

	uint8_t channel = as->getAudioChannel();

	if (channel >= _channelsToAllocate)
		return;

	if (_channelMaster[channel] != 0 && _channelMaster[channel] == data->audioSrcName) {
		if (Mix_Playing(channel)) {	//channel still playing
			Mix_HaltChannel(channel);
			as->__setStatus(AudioStatus::AUDIO_STATUS_STOPPED);
			_channelMaster[channel] = 0;	//channel no longer owned by the audio object
		}
	}
}

void Audio::updateAudioSource(EntityName audioSrcName, uint16_t channel, vector2 position, double distance, uint8_t group) {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioSrcUpdateData* data = new AudioSrcUpdateData();
	data->type = AudioRequest::UPDATE_AUDIO_SOURCE;
	data->audioSrcName = audioSrcName;
	data->channel = channel;
	data->position = position;
	data->maxDistance = distance;
	data->group = group;
	_requests.push_back(data);
}

void Audio::updateAudioSource_Internal(AudioSrcUpdateData *data) {
	if (!data)
		return;

	uint8_t left, right;
	if (!calc_spatial_sound_panning(data->maxDistance, data->position, left, right))
		return;

	std::lock_guard <std::mutex> guard(update_mutex);

	if (_channelMaster[data->channel] == data->audioSrcName && data->audioSrcName != 0) {
		//allocate a channel and start the sound
		Mix_SetPanning(data->channel, left, right);
	}
}


void Audio::PauseMusic() {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData* data = new AudioRequestData();
	data->type = AudioRequest::PAUSE_MUSIC;
	_requests.push_back(data);
}

void Audio::ResumeMusic() {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData* data = new AudioRequestData();
	data->type = AudioRequest::RESUME_MUSIC;
	_requests.push_back(data);
}

void Audio::StopMusic() {
	std::lock_guard <std::mutex> guard(request_mutex);

	AudioRequestData* data = new AudioRequestData();
	data->type = AudioRequest::STOP_MUSIC;
	_requests.push_back(data);
}

void Audio::PlayMusic_Internal(EntityName music) {
	std::lock_guard <std::mutex> guard(update_mutex);

	if (this->_musicTracks.find(music) != _musicTracks.end() && this->_playSoundtrack) {
		Mix_PlayMusic(this->_musicTracks[music], 0);
	}
}

void Audio::SetMusicVolume_Internal(AudioVolumeData *data) {
	if (!data)
		return;
	std::lock_guard <std::mutex> guard(update_mutex);
	this->_musicVolume = data->volume;
	Mix_VolumeMusic(_musicVolume);
}


void Audio::SetGroupVolume(uint8_t volume, uint8_t audioGroup) {
	std::lock_guard <std::mutex> guard(request_mutex);

	if (volume < 0 || volume > 255 || audioGroup >= _audioGroupChannels.size()) {
		return;
	}

	AudioVolumeData* data = new AudioVolumeData();
	data->group = audioGroup;
	data->volume = volume;
	_requests.push_back(data);
}

void Audio::SetGroupVolume_Internal(AudioVolumeData* data) {
	if (!data)
		return;
	std::lock_guard <std::mutex> guard(update_mutex);
	_audioGroupVolume[data->group] = data->volume;
	for (int i = _audioGroupChRange[data->group].first; i <= _audioGroupChRange[data->group].second; i++) {
		Mix_Volume(i, data->volume);
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

//internal use only: stop all audio and clear channel masters
void Audio::ClearAudio() {
	{
		std::lock_guard <std::mutex> guard(request_mutex);
		memset(_channelMaster, 0, sizeof(EntityName) * _channelsToAllocate);
		_requests.clear();
		_rescheduledRequests.clear();
	}
	StopAll();
	SetMusicVolume(_defaultMusicVolume);
	for (int i = 0; i < _audioGroupChannels.size(); i++) {
		SetGroupVolume(_defaultTrackVolume, i);
	}
}