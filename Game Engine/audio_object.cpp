#include "stdafx.h"
#include "audio_object.h"
#include "audio.h"

unsigned long AudioObj::_tracksCounter = 0;

AudioObj::AudioObj(std::string trackName, bool spatial_sound, double maxDistance, vector2 source_position) :
	_name(DecodeName(trackName.c_str())),
	_spatial_sound(spatial_sound),
	_maxDistance(maxDistance),
	_src_pos(source_position)
{
	_len = 0;
	_tracksCounter++;
	_trackId = _tracksCounter;
	_status = AudioStatus::AUDIO_STATUS_IDLE;
}

//return a unique id for the current track
unsigned long AudioObj::getTrackId() {
	return _trackId;
}

//get length in seconds of the current track
bool AudioObj::GetLen() {
	return _len;
}

//pause the track
void AudioObj::Pause() {
	_AudioEngine->PauseAudioObject(shared_from_this());
}

//stops the track
void AudioObj::Stop() {
	_AudioEngine->StopAudioObject(shared_from_this());
}

//resume a paused track
void AudioObj::Resume() {
	_AudioEngine->ResumeAudioObject(shared_from_this());
}

//return the current status of the track
AudioStatus AudioObj::GetStatus() {
	if (_status == AudioStatus::AUDIO_STATUS_PLAYING) {	//could be already finished
		if (!_AudioEngine->AudioPlaying(_channel, _trackId)) {	//already finished
			_status = AudioStatus::AUDIO_STATUS_FINISHED;
			return _status;
		}
	}
	return _status;
}

//returns -1 if the channel was not assigned, a valid channel otherwise.
//you should not trust the channel to know if the audio is being played or not
int AudioObj::getAudioChannel() {
	return _channel;
}

//internal use only
void AudioObj::__setStatus(AudioStatus status) {
	_status = status;
}

//internal use only
void AudioObj::__setChannel(int channel) {
	_status = AUDIO_STATUS_PLAYING;
	_channel = channel;
}




DialogAudioObj::DialogAudioObj(std::string trackName, bool spatial_sound, double maxDistance, vector2 source_position) :
	AudioObj(trackName, spatial_sound, maxDistance, source_position)
{
	_len = _AudioEngine->GetDialogLen(_name);
}

bool DialogAudioObj::Play() {
	if (GetStatus() != AUDIO_STATUS_IDLE) {
		return false;
	}
	_AudioEngine->PlayDialog(_name, shared_from_this(), _spatial_sound, _maxDistance, _src_pos);
}




EffectAudioObj::EffectAudioObj(std::string trackName, bool spatial_sound, double maxDistance, vector2 source_position) :
	AudioObj(trackName, spatial_sound, maxDistance, source_position)
{
	_len = _AudioEngine->GetSoundEffectLen(_name);
}

bool EffectAudioObj::Play() {
	if (GetStatus() != AUDIO_STATUS_IDLE) {
		return false;
	}
	_AudioEngine->PlaySoundEffect(_name, shared_from_this(), _spatial_sound, _maxDistance, _src_pos);
}




UIAudioObj::UIAudioObj(std::string trackName, bool spatial_sound, double maxDistance, vector2 source_position) :
	AudioObj(trackName, spatial_sound, maxDistance, source_position)
{
	_len = _AudioEngine->GetUIEffectLen(_name);
}

bool UIAudioObj::Play() {
	if (GetStatus() != AUDIO_STATUS_IDLE) {
		return false;
	}
	_AudioEngine->PlayUIEffect(_name, shared_from_this(), _spatial_sound, _maxDistance, _src_pos);
}
