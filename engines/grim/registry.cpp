/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/grim/registry.h"

#include "audio/mixer.h"

#include "common/config-manager.h"

namespace Grim {

Registry *g_registry = nullptr;

// SpewOnError
// GrimLastSet
// JoystickEnabled
// MovementMode
// SpeechMode
// GammaCorrection, Gamma
// GrimDeveloper
// SfxVolume
// MusicVolume
// VoiceVolume
// DirectorsCommentary
// DirectorsCommentaryVolume
// TextSpeed
// MouseSpeed
// VoiceEffects
// LastSavedGame
// good_times
// ResolutionScaling

void Registry::Value::setString(const Common::String &str) {
	_val._str = str;
	_type = Registry::String;
}

void Registry::Value::setInt(int num) {
	_val._num = num;
	_type = Registry::Integer;
}

void Registry::Value::setBool(bool val) {
	_val._bool = val;
	_type = Registry::Boolean;
}

const Common::String &Registry::Value::getString() const {
	assert(_type == Registry::String);
	return _val._str;
}

int Registry::Value::getInt() const {
	if (_type == Registry::Integer)
		return _val._num;
	if (_type == Registry::Boolean)
		return _val._bool;

	return atoi(_val._str.c_str());
}

bool Registry::Value::getBool() const {
	if (_type == Registry::Boolean)
		return _val._bool;

	if (_type == Registry::Integer)
		return _val._num;

	if (_val._str.equalsIgnoreCase("true"))
		return true;

	return false;
}

Registry::ValueType Registry::Value::getType() const {
	return _type;
}

Registry::Registry() :
		_dirty(true) {
	// Default settings for GRIM
	ConfMan.registerDefault("subtitles", true);
	ConfMan.registerDefault("talkspeed", 179);
	ConfMan.registerDefault("game_devel_mode", false);
	ConfMan.registerDefault("widescreen", false);
	ConfMan.registerDefault("screen_scaling", false);
	ConfMan.registerDefault("advanced_lighting", true);
	ConfMan.registerDefault("commentary_enabled", false);

	// Read settings
	_spewOnError.setString(ConfMan.get("spew_on_error"));
	_dataPath.setString(ConfMan.get("path"));
	_savePath.setString(ConfMan.get("savepath"));
	_develMode.setBool(ConfMan.getBool("game_devel_mode"));
	_lastSet.setString(ConfMan.get("last_set"));
	_musicVolume.setInt(convertVolumeFromMixer(ConfMan.getInt("music_volume")));
	_sfxVolume.setInt(convertVolumeFromMixer(ConfMan.getInt("sfx_volume")));
	_voiceVolume.setInt(convertVolumeFromMixer(ConfMan.getInt("speech_volume")));
	_commentaryMode.setBool(ConfMan.getBool("commentary_enabled"));
	_commentaryVolume.setInt(convertVolumeFromMixer(ConfMan.getInt("commentary_volume")));
	_lastSavedGame.setString(ConfMan.get("last_saved_game"));
	_gamma.setInt(ConfMan.getInt("gamma"));
	_textSpeed.setInt(convertTalkSpeedFromGUI(ConfMan.getInt("talkspeed")));
	_mouseSpeed.setInt(convertTalkSpeedFromGUI(ConfMan.getInt("mousespeed")));
	_speechMode.setInt(convertSpeechModeFromGUI(ConfMan.getBool("subtitles"), ConfMan.getBool("speech_mute")));
	_screenMode.setBool(convertScreenModeFromGUI(ConfMan.getBool("widescreen")));
	_screenScaling.setBool(convertScreenScalingFromGUI(ConfMan.getBool("screen_scaling")));
	_advLighting.setBool(convertAdvLightingFromGUI(ConfMan.getBool("advanced_lighting")));
	_language.setInt(convertLanguageFromGUI(ConfMan.get("language")));

	// These can't be set as bool because the scripts do a check against "TRUE" and "FALSE".
	// Right, they do string comparisons. doh!
	_voiceEffects.setString(ConfMan.get("voice_effects"));
	_movement.setString(ConfMan.get("movement"));
	_joystick.setString(ConfMan.get("joystick"));
	_transcript.setString(ConfMan.get("transcript"));
}

Registry::Value &Registry::value(const Common::String &key) {
	if (scumm_stricmp("good_times", key.c_str()) == 0 || scumm_stricmp("GrimDeveloper", key.c_str()) == 0) {
		return _develMode;
	} else if (scumm_stricmp("GrimDataDir", key.c_str()) == 0) {
		return _dataPath;
	} else if (scumm_stricmp("savepath", key.c_str()) == 0) {
		return _savePath;
	} else if (scumm_stricmp("GrimLastSet", key.c_str()) == 0) {
		return _lastSet;
	} else if (scumm_stricmp("MusicVolume", key.c_str()) == 0) {
		return _musicVolume;
	} else if (scumm_stricmp("SfxVolume", key.c_str()) == 0) {
		return _sfxVolume;
	} else if (scumm_stricmp("VoiceVolume", key.c_str()) == 0) {
		return _voiceVolume;
	} else if (scumm_stricmp("DirectorsCommentary", key.c_str()) == 0) {
		return _commentaryMode;
	} else if (scumm_stricmp("DirectorsCommentaryVolume", key.c_str()) == 0) {
		return _commentaryVolume;
	} else if (scumm_stricmp("LastSavedGame", key.c_str()) == 0) {
		return _lastSavedGame;
	} else if (scumm_stricmp("Gamma", key.c_str()) == 0 || scumm_stricmp("GammaCorrection", key.c_str()) == 0) {
		return _gamma;
	} else if (scumm_stricmp("VoiceEffects", key.c_str()) == 0) {
		return _voiceEffects;
	} else if (scumm_stricmp("TextSpeed", key.c_str()) == 0) {
		return _textSpeed;
	} else if (scumm_stricmp("MouseSpeed", key.c_str()) == 0) {
		return _mouseSpeed;
	} else if (scumm_stricmp("TextMode", key.c_str()) == 0 || scumm_stricmp("SpeechMode", key.c_str()) == 0) {
		return _speechMode;
	} else if (scumm_stricmp("Widescreen", key.c_str()) == 0) {
		return _screenMode;
	} else if (scumm_stricmp("ResolutionScaling", key.c_str()) == 0) {
		return _screenScaling;
	} else if (scumm_stricmp("AdvancedLighting", key.c_str()) == 0) {
		return _advLighting;
	} else if (scumm_stricmp("MovementMode", key.c_str()) == 0) {
		return _movement;
	} else if (scumm_stricmp("JoystickEnabled", key.c_str()) == 0) {
		return _joystick;
	} else if (scumm_stricmp("SpewOnError", key.c_str()) == 0) {
		return _spewOnError;
	} else if (scumm_stricmp("Transcript", key.c_str()) == 0) {
		return _transcript;
	} else if (scumm_stricmp("Language", key.c_str()) == 0) {
		return _language;
	}

	assert(0);
}

const Registry::Value &Registry::value(const Common::String &key) const {
	if (scumm_stricmp("good_times", key.c_str()) == 0 || scumm_stricmp("GrimDeveloper", key.c_str()) == 0) {
		return _develMode;
	} else if (scumm_stricmp("GrimDataDir", key.c_str()) == 0) {
		return _dataPath;
	} else if (scumm_stricmp("savepath", key.c_str()) == 0) {
		return _savePath;
	} else if (scumm_stricmp("GrimLastSet", key.c_str()) == 0) {
		return _lastSet;
	} else if (scumm_stricmp("MusicVolume", key.c_str()) == 0) {
		return _musicVolume;
	} else if (scumm_stricmp("SfxVolume", key.c_str()) == 0) {
		return _sfxVolume;
	} else if (scumm_stricmp("VoiceVolume", key.c_str()) == 0) {
		return _voiceVolume;
	} else if (scumm_stricmp("DirectorsCommentary", key.c_str()) == 0) {
		return _commentaryMode;
	} else if (scumm_stricmp("DirectorsCommentaryVolume", key.c_str()) == 0) {
		return _commentaryVolume;
	} else if (scumm_stricmp("LastSavedGame", key.c_str()) == 0) {
		return _lastSavedGame;
	} else if (scumm_stricmp("Gamma", key.c_str()) == 0 || scumm_stricmp("GammaCorrection", key.c_str()) == 0) {
		return _gamma;
	} else if (scumm_stricmp("VoiceEffects", key.c_str()) == 0) {
		return _voiceEffects;
	} else if (scumm_stricmp("TextSpeed", key.c_str()) == 0) {
		return _textSpeed;
	} else if (scumm_stricmp("MouseSpeed", key.c_str()) == 0) {
		return _mouseSpeed;
	} else if (scumm_stricmp("TextMode", key.c_str()) == 0 || scumm_stricmp("SpeechMode", key.c_str()) == 0) {
		return _speechMode;
	} else if (scumm_stricmp("Widescreen", key.c_str()) == 0) {
		return _screenMode;
	} else if (scumm_stricmp("ResolutionScaling", key.c_str()) == 0) {
		return _screenScaling;
	} else if (scumm_stricmp("AdvancedLighting", key.c_str()) == 0) {
		return _advLighting;
	} else if (scumm_stricmp("MovementMode", key.c_str()) == 0) {
		return _movement;
	} else if (scumm_stricmp("JoystickEnabled", key.c_str()) == 0) {
		return _joystick;
	} else if (scumm_stricmp("SpewOnError", key.c_str()) == 0) {
		return _spewOnError;
	} else if (scumm_stricmp("Transcript", key.c_str()) == 0) {
		return _transcript;
	} else if (scumm_stricmp("Language", key.c_str()) == 0) {
		return _language;
	}

	assert(0);
}

const Common::String &Registry::getString(const Common::String &key) const {
	return value(key).getString();
}

int Registry::getInt(const Common::String &key) const {
	return value(key).getInt();
}

bool Registry::getBool(const Common::String &key) const {
	return value(key).getBool();
}

void Registry::setString(const Common::String &key, const Common::String &val) {
	_dirty = true;
	value(key).setString(val);
}

void Registry::setInt(const Common::String &key, int val) {
	_dirty = true;
	value(key).setInt(val);
}

void Registry::setBool(const Common::String &key, bool val) {
	_dirty = true;
	value(key).setBool(val);
}

Registry::ValueType Registry::getValueType(const Common::String &key) const {
	return value(key).getType();
}

void Registry::save() {
	if (!_dirty)
		return;

	ConfMan.set("spew_on_error", _spewOnError.getString());
	ConfMan.set("path", _dataPath.getString());
	ConfMan.set("savepath", _savePath.getString());
	ConfMan.set("last_set", _lastSet.getString());
	ConfMan.setBool("game_devel_mode", _develMode.getBool());
	ConfMan.setInt("music_volume", convertVolumeToMixer(_musicVolume.getInt()));
	ConfMan.setInt("sfx_volume", convertVolumeToMixer(_sfxVolume.getInt()));
	ConfMan.setInt("speech_volume", convertVolumeToMixer(_voiceVolume.getInt()));
	ConfMan.setInt("commentary_enabled", _commentaryMode.getBool());
	ConfMan.setInt("commentary_volume", convertVolumeToMixer(_commentaryVolume.getInt()));
	ConfMan.set("last_saved_game", _lastSavedGame.getString());
	ConfMan.setInt("gamma", _gamma.getInt());
	ConfMan.setInt("talkspeed", convertTalkSpeedToGUI(_textSpeed.getInt()));
	ConfMan.setInt("mousespeed", convertTalkSpeedToGUI(_mouseSpeed.getInt()));
	ConfMan.setBool("subtitles", convertSubtitlesToGUI(_speechMode.getInt()));
	ConfMan.setBool("widescreen", convertScreenModeToGUI(_screenMode.getInt()));
	ConfMan.setBool("screen_scaling", convertScreenScalingToGUI(_screenScaling.getInt()));
	ConfMan.setBool("advanced_lighting", convertAdvLightingToGUI(_advLighting.getInt()));
	ConfMan.setBool("speech_mute", convertSpeechMuteToGUI(_speechMode.getInt()));
	ConfMan.set("language", convertLanguageToGUI(_language.getInt()));

	ConfMan.set("movement", _movement.getString());
	ConfMan.set("joystick", _joystick.getString());
	ConfMan.set("voice_effects", _voiceEffects.getString());
	ConfMan.set("transcript", _transcript.getString());

	_dirty = false;
}

uint Registry::convertVolumeToMixer(uint grimVolume) {
	return CLIP<uint>(grimVolume * 2, 0, Audio::Mixer::kMaxMixerVolume);
}

uint Registry::convertVolumeFromMixer(uint volume) {
	return CLIP<uint>(volume / 2, 0, 127);
}

uint Registry::convertTalkSpeedToGUI(uint talkspeed) {
	return CLIP<uint>(talkspeed * 255 / 10, 0, 255);
}

uint Registry::convertTalkSpeedFromGUI(uint talkspeed) {
	return CLIP<uint>(talkspeed * 10 / 255, 1, 10);
}

bool Registry::convertSubtitlesToGUI(uint speechmode) {
	return speechmode == 1 || speechmode == 3;
}

bool Registry::convertSpeechMuteToGUI(uint speechmode) {
	return speechmode == 1;
}

uint Registry::convertSpeechModeFromGUI(bool subtitles, bool speechMute) {
	if (!subtitles && !speechMute) // Speech only
		return 2;
	else if (subtitles && !speechMute) // Speech and subtitles
		return 3;
	else if (subtitles && speechMute) // Subtitles only
		return 1;
	else
		warning("Wrong configuration: Both subtitles and speech are off. Assuming subtitles only");
	return 1;
}

bool Registry::convertScreenModeToGUI(uint widescreen_mode) {
	return (widescreen_mode == 2);
}

uint Registry::convertScreenModeFromGUI(bool widescreen_mode) {
	if (widescreen_mode)
		return 2;
	return 1;
}

bool Registry::convertScreenScalingToGUI(uint scaling) {
	return (scaling == 1);
}

uint Registry::convertScreenScalingFromGUI(bool scaling) {
	if (scaling)
		return 1;
	return 2;
}

bool Registry::convertAdvLightingToGUI(uint adv_lighting) {
	return (adv_lighting == 1);
}

uint Registry::convertAdvLightingFromGUI(bool adv_lighting) {
	if (adv_lighting)
		return 1;
	return 2;
}

uint Registry::convertLanguageFromGUI(const Common::String &language) {
	if(language == "en") {
		return 1;
	}
	warning("unhandled language %s, assuming english.", language.c_str());
	return 1;
}

Common::String Registry::convertLanguageToGUI(uint language) {
	if (language == 1) {
		return "en";
	}
	warning("unhandled language code %d, assuming english.", language);
	return "en";
}

} // end of namespace Grim
