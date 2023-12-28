#ifndef AUDIO_OBJECT_H
#define AUDIO_OBJECT_H

#include <string>
#include <atomic>
#include <memory>

//Audio objects are used to keep ownership of a audio track (effect, ui effect or dialog sound). This is useful to be able to play, pause and stop a 
//track or to know the status of a audio track.
//NB. audio objects must ALWAYS be handled using SHARED POINTERS

enum AudioStatus {
	AUDIO_STATUS_IDLE,	//not yet played
	AUDIO_STATUS_PLAYING,	//is playing
	AUDIO_STATUS_PAUSE,	//paused but still owns the channel
	AUDIO_STATUS_STOPPED,	//stopped: doesn't own the channel anymore
	AUDIO_STATUS_FINISHED	//finished: doesn't own the channel anymore
};

class AudioObj : public std::enable_shared_from_this<AudioObj> {
public:
	AudioObj(std::string trackName, bool spatial_sound = false, double maxDistance = 0, vector2 source_position = { 0, 0 });
	virtual bool Play() = 0;
	bool GetLen();
	void Pause();
	void Stop();
	void Resume();
	AudioStatus GetStatus();
	int getAudioChannel();		//returns -1 if the channel was not assigned, a valid channel otherwise
	unsigned long getTrackId();

	//internal use only
	void __setStatus(AudioStatus status);
	void __setChannel(int channel);
protected:
	const EntityName _name;
	double _len;
	const bool _spatial_sound;
	const double _maxDistance;
	const vector2 _src_pos;
private:
	std::atomic <int> _channel = -1;	//channel where the track is being played or was played
	std::atomic <AudioStatus> _status;	//track status
	unsigned long _trackId;	//unique id for the current track
	static unsigned long _tracksCounter;	//keeps track of the total number of tracks objects created

};


class DialogAudioObj : public AudioObj {
public:
	DialogAudioObj(std::string trackName, bool spatial_sound = false, double maxDistance = 0, vector2 source_position = { 0, 0 });
	virtual bool Play();
private:
};


class EffectAudioObj : public AudioObj {
public:
	EffectAudioObj(std::string trackName, bool spatial_sound = false, double maxDistance = 0, vector2 source_position = { 0, 0 });
	virtual bool Play();
private:
};


class UIAudioObj : public AudioObj {
public:
	UIAudioObj(std::string trackName, bool spatial_sound = false, double maxDistance = 0, vector2 source_position = { 0, 0 });
	virtual bool Play();
private:
};

#endif