#ifndef AUDIO_OBJECT_H
#define AUDIO_OBJECT_H

#include <string>
#include <atomic>
#include <memory>

#include "gameObject.h"

//Audio sources are used to keep ownership of a audio track (effect, ui effect or dialog sound). This is useful to be able to play, pause and stop a 
//track or to know the status of a audio track.
//NB. audio objects must ALWAYS be handled using SHARED POINTERS

enum AudioStatus {
	AUDIO_STATUS_IDLE,	//not yet played
	AUDIO_STATUS_PLAYING,	//is playing
	AUDIO_STATUS_PAUSE,	//paused but still owns the channel
	AUDIO_STATUS_STOPPED,	//stopped: doesn't own the channel anymore
	AUDIO_STATUS_FINISHED	//finished: doesn't own the channel anymore
};

class AudioSource : public GameObject {
public:
	AudioSource(EntityName name, std::string trackName, uint16_t audioGroup, double maxDistance, bool spatial_sound);
	~AudioSource();
	bool Play();
	bool GetLen();
	void Pause();
	void Stop();
	void Resume();
	AudioStatus GetStatus();
	int getAudioChannel();		//returns -1 if the channel was not assigned, a valid channel otherwise

	//internal use only
	void __setStatus(AudioStatus status);
	void __setChannel(int channel);
	void update(double timeElapsed);
protected:
	const EntityName _trackName;
	double _len;
	const bool _spatial_sound;
	const double _maxDistance;
	const uint16_t _audioGroup;
private:
	std::atomic <int> _channel = -1;	//channel where the track is being played or was played
	std::atomic <AudioStatus> _status;	//track status
	double _updateTimer, _playTimer;
};

#endif