#include "stdafx.h"
#include "audio_source.h"
#include "audio.h"

#include <memory>

AudioSource::AudioSource(EntityName audioSrcName, std::string trackName, uint16_t audioGroup, double maxDistance, bool spatial_sound = true) :
	_trackName(DecodeName(trackName.c_str())),
	_spatial_sound(spatial_sound),
	_maxDistance(maxDistance),
	_audioGroup(audioGroup)
{
	_len = _AudioEngine->GetTrackLen(_trackName);
	_status = AudioStatus::AUDIO_STATUS_IDLE;
	_objectName = audioSrcName;
	_GameEngine->RegisterGameObject(this, audioSrcName);
	_playTimer = 0;
	_updateTimer = 0;
}

AudioSource::~AudioSource() {
	Destroy();
}

//get length in seconds of the current track
bool AudioSource::GetLen() {
	return _len;
}

bool AudioSource::Play() {
	if (GetStatus() != AUDIO_STATUS_IDLE && GetStatus() != AUDIO_STATUS_STOPPED &&
		GetStatus() != AUDIO_STATUS_FINISHED) {
		return false;
	}
	_AudioEngine->PlayTrack(_trackName, _audioGroup, _maxDistance, transform.position, _spatial_sound, _objectName);
}

//pause the track
void AudioSource::Pause() {
	_AudioEngine->PauseAudioSource(_objectName);
}

//stops the track. You can restart it from the beginning by calling Play()
void AudioSource::Stop() {
	_AudioEngine->StopAudioSource(_objectName);
}

//resume a paused track
void AudioSource::Resume() {
	_AudioEngine->ResumeAudioSource(_objectName);
}

//return the current status of the track
AudioStatus AudioSource::GetStatus() {
	/*if (_status == AudioStatus::AUDIO_STATUS_PLAYING) {	//could be already finished
		if (!_AudioEngine->AudioSourcePlaying(_channel, _objectName)) {	//already finished
			_status = AudioStatus::AUDIO_STATUS_FINISHED;
			return _status;
		}
	}*/
	return _status;
}

//returns -1 if the channel was not assigned, a valid channel otherwise.
//you should not trust the channel to know if the audio is being played or not
int AudioSource::getAudioChannel() {
	return _channel;
}

//internal use only
void AudioSource::__setStatus(AudioStatus status) {
	_status = status;
}

//internal use only
void AudioSource::__setChannel(int channel) {
	_status = AUDIO_STATUS_PLAYING;
	_channel = channel;
}

//internal use only: update the status and the of the audio source object
void AudioSource::update(double timeElapsed) {
	if (_status != AudioStatus::AUDIO_STATUS_PLAYING)
		return;
	_updateTimer += timeElapsed;
	_playTimer += timeElapsed;
	if (_updateTimer > 1.0 / 30) {
		if (_playTimer > _len) {	//track finished
			_status = AudioStatus::AUDIO_STATUS_FINISHED;
			_playTimer = 0;
		}
		if(_spatial_sound)
			_AudioEngine->updateAudioSource(getObjectName(), _channel, transform.position, _maxDistance, _audioGroup);
		_updateTimer = 0;
	}
}