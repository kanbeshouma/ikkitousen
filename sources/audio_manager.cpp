#include <tchar.h>
#include "audio_manager.h"
#include "misc.h"

AudioManager::AudioManager()
{
	initialize_XAudio2();
	initialize_bgm();
	initialize_se();
}

AudioManager::~AudioManager()
{
	for (int i = 0; i < MAX_BGM_COUNT; ++i) {
		if (bgm_source_voices[i] != nullptr)
		{
			bgm_source_voices[i]->DestroyVoice();
			bgm_source_voices[i] = nullptr;
		}
		if (bgm_pdata_buffer[i] != nullptr) { delete bgm_pdata_buffer[i]; }
	}
	for (int i = 0; i < MAX_SE_COUNT; ++i) {
		if (se_source_voices[i] != nullptr)
		{
			se_source_voices[i]->DestroyVoice();
			se_source_voices[i] = nullptr;
		}
		if (se_pdata_buffer[i] != nullptr) { delete se_pdata_buffer[i]; }
	}
	if (pXAudio2 != nullptr)
	{
		pXAudio2->Release();
		pXAudio2 = nullptr;
	}
	CoUninitialize();
}

void AudioManager::initialize_XAudio2()
{
	HRESULT hr{ S_OK };
	// How to: Initialize XAudio2 https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--initialize-xaudio2
	// Make sure you have initialized COM.
	hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	// if (FAILED(hr)) return false;
	// Use the XAudio2Create function to create an instance of the XAudio2 engine.
	hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	// Use the CreateMasteringVoice method to create a mastering voice.
	IXAudio2MasteringVoice* p_master_voice;
	hr = pXAudio2->CreateMasteringVoice(&p_master_voice);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

//-------------------------------------------------------
//      BGM
//-------------------------------------------------------
void AudioManager::initialize_bgm()
{
	register_bgm(L"resources/Audios/BGM/title.wav", BGM_INDEX::TITLE, 0.1f);
	register_bgm(L"resources/Audios/BGM/tutorial.wav", BGM_INDEX::TUTORIAL, 0.1f);
	register_bgm(L"resources/Audios/BGM/gaming.wav", BGM_INDEX::GAME, 0.1f);
	register_bgm(L"resources/Audios/BGM/boss_battleship.wav", BGM_INDEX::BOSS_BATTLESHIP, 0.1f);
	register_bgm(L"resources/Audios/BGM/boss_human.wav", BGM_INDEX::BOSS_HUMANOID, 0.1f);
	register_bgm(L"resources/Audios/BGM/boss_dragon.wav", BGM_INDEX::BOSS_DRAGON, 0.1f);
	register_bgm(L"resources/Audios/BGM/clear.wav", BGM_INDEX::CLEAR, 0.1f);


}

void AudioManager::register_bgm(const LPCWSTR filename,
	BGM_INDEX index, float volume, bool is_loop)
{
	HRESULT hr{ S_OK };
	// Populating XAudio2 structures with the contents of RIFF chunks https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--load-audio-data-files-in-xaudio2
    // Declare WAVEFORMATEXTENSIBLE and XAUDIO2_BUFFER structures.
	WAVEFORMATEXTENSIBLE wfx = { 0 };
	XAUDIO2_BUFFER buffer = { 0 };
	// Open the audio file with CreateFile.
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL);
	_ASSERT_EXPR(INVALID_HANDLE_VALUE != hFile, L"Cannot find audio file");
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	// Locate the 'RIFF' chunk in the audio file, and check the file type.
	DWORD dwChunkSize;
	DWORD dwChunkPosition;
	FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
	DWORD filetype;
	ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
	_ASSERT_EXPR(filetype == fourccWAVE, L"Check the file type, should be fourccWAVE or 'XWMA'");
	// Locate the 'fmt ' chunk, and copy its contents into a WAVEFORMATEXTENSIBLE structure.
	FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
	ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
	// Locate the 'data' chunk, and read its contents into a buffer.
	FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
	bgm_pdata_buffer[static_cast<int>(index)] = new BYTE[dwChunkSize];
	ReadChunkData(hFile, bgm_pdata_buffer[static_cast<int>(index)], dwChunkSize, dwChunkPosition);
	// Populate an XAUDIO2_BUFFER structure.
	buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
	buffer.pAudioData = bgm_pdata_buffer[static_cast<int>(index)];  //buffer containing audio data
	buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer
	if (is_loop) {
		// ループの設定 https://docs.microsoft.com/ja-jp/windows/win32/api/xaudio2/ns-xaudio2-xaudio2_buffer?redirectedfrom=MSDN
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}
	// How to: Play a Sound with XAudio2 https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--play-a-sound-with-xaudio2
	// Create a source voice by calling the IXAudio2::CreateSourceVoice method on an instance of the XAudio2 engine. The format of the voice is specified by the values set in a WAVEFORMATEX structure.
	hr = pXAudio2->CreateSourceVoice(&bgm_source_voices[static_cast<int>(index)], (WAVEFORMATEX*)&wfx);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	// 12 Submit an XAUDIO2_BUFFER to the source voice using the function SubmitSourceBuffer.
	hr = bgm_source_voices[static_cast<int>(index)]->SubmitSourceBuffer(&buffer);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	hr = bgm_source_voices[static_cast<int>(index)]->SetVolume(volume);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void AudioManager::play_bgm(BGM_INDEX index)
{
	HRESULT hr{ S_OK };
	assert(bgm_source_voices[static_cast<int>(index)] != nullptr
		&& "BGMが設定されていません、再生するBGMのインデックスが間違えている可能性があります");
	// Use the Start function to start the source voice.
	hr = bgm_source_voices[static_cast<int>(index)]->Start(0);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void AudioManager::stop_bgm(BGM_INDEX index)
{
	HRESULT hr{ S_OK };
	// Use the Start function to start the source voice.
	hr = bgm_source_voices[static_cast<int>(index)]->Stop(0);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void AudioManager::stop_all_bgm()
{
	for(int i = 0;i < static_cast<int>(BGM_INDEX::BGM_COUNT);i++)
	{
		stop_bgm(static_cast<BGM_INDEX>(i));
	}
}

void AudioManager::set_volume_bgm(BGM_INDEX index, float volume)
{
	HRESULT hr{ S_OK };
	assert(bgm_source_voices[static_cast<int>(index)] != nullptr && "BGMが設定されていません");
	hr = bgm_source_voices[static_cast<int>(index)]->SetVolume(volume);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void AudioManager::set_all_volume_bgm(float volume)
{
	for (int i = 0; i < static_cast<int>(BGM_INDEX::BGM_COUNT); i++)
	{
		set_volume_bgm(static_cast<BGM_INDEX>(i), volume);
	}
}

//-------------------------------------------------------
//      SE
//-------------------------------------------------------
void AudioManager::initialize_se()
{
	enum LOOP_COUNT
	{
		LOOP_INFINITY = -1,
		LOOP_NONE,

		LOOP_MAX = 254,
	};

	register_se(L"resources/Audios/SE/select.wav", SE_INDEX::SELECT, 0.1f);
	register_se(L"resources/Audios/SE/decision.wav", SE_INDEX::DECISION, 0.1f);
	register_se(L"resources/Audios/SE/draw_sword.wav", SE_INDEX::DRAW_SWORD, 0.1f);
	register_se(L"resources/Audios/SE/return_sword.wav", SE_INDEX::RETURN_SWORD, 0.1f);
	register_se(L"resources/Audios/SE/swing_sword.wav", SE_INDEX::SWING_SWORD1, 0.1f);
	register_se(L"resources/Audios/SE/swing_sword.wav", SE_INDEX::SWING_SWORD2, 0.1f);
	register_se(L"resources/Audios/SE/attack_sword.wav", SE_INDEX::ATTACK_SWORD, 0.1f);
	register_se(L"resources/Audios/SE/enemy_emergence.wav", SE_INDEX::ENEMY_EMERGENCE, 0.1f);
	register_se(L"resources/Audios/SE/shot_bow.wav", SE_INDEX::SHOT_BOW, 0.1f);
	register_se(L"resources/Audios/SE/cannon.wav", SE_INDEX::CANNON, 0.1f);
	register_se(L"resources/Audios/SE/heart_beat.wav", SE_INDEX::HEART_BEAT, 0.1f);
	register_se(L"resources/Audios/SE/open_fire.wav", SE_INDEX::OPEN_FIRE, 0.1f, LOOP_INFINITY);
	register_se(L"resources/Audios/SE/transform1.wav", SE_INDEX::TRANSFORM1, 0.1f);
	register_se(L"resources/Audios/SE/transform2.wav", SE_INDEX::TRANSFORM2, 0.1f);
	register_se(L"resources/Audios/SE/transform3.wav", SE_INDEX::TRANSFORM3, 0.1f);
	register_se(L"resources/Audios/SE/transform4.wav", SE_INDEX::TRANSFORM4, 0.1f);
	register_se(L"resources/Audios/SE/transform5.wav", SE_INDEX::TRANSFORM5, 0.1f);
	register_se(L"resources/Audios/SE/transform6.wav", SE_INDEX::TRANSFORM6, 0.1f);
	register_se(L"resources/Audios/SE/boss_beam.wav", SE_INDEX::BOSS_BEAM, 0.1f, LOOP_INFINITY);
	register_se(L"resources/Audios/SE/dragon_roar.wav", SE_INDEX::DRAGON_ROAR, 0.1f);
	register_se(L"resources/Audios/SE/enemy_explosion.wav", SE_INDEX::ENEMY_EXPLOSION, 0.1f);
	register_se(L"resources/Audios/SE/stan.wav", SE_INDEX::STAN, 0.1f);
	register_se(L"resources/Audios/SE/wraparound_avoidance.wav", SE_INDEX::WRAPAROUND_AVOIDANCE, 0.1f);
	register_se(L"resources/Audios/SE/avoidance.wav", SE_INDEX::AVOIDANCE, 0.1f);
	register_se(L"resources/Audios/SE/dragon_boss_death.wav", SE_INDEX::DRAGON_BOSS_DEATH, 0.1f, LOOP_INFINITY);
	register_se(L"resources/Audios/SE/enemy_Pre-attack_sound.wav", SE_INDEX::ENEMY_PRE_ATTACK_SOUND, 0.1f);
	register_se(L"resources/Audios/SE/enemy_sword_attack.wav", SE_INDEX::ENEMY_SWORD_ATTACK, 0.1f);
	register_se(L"resources/Audios/SE/enemy_shield_attack.wav", SE_INDEX::ENEMY_SHIELD_ATTACK, 0.1f);
	register_se(L"resources/Audios/SE/boss_charge.wav", SE_INDEX::BOSS_CHARGE, 0.1f, LOOP_INFINITY);
	register_se(L"resources/Audios/SE/player_awaking.wav", SE_INDEX::PLAYER_AWAKING , 0.1f);
	register_se(L"resources/Audios/SE/player_damaged.wav", SE_INDEX::PLAYER_DAMAGED, 0.1f);
	register_se(L"resources/Audios/SE/fire_boll.wav", SE_INDEX::FIRE_BOLL, 0.1f);
	register_se(L"resources/Audios/SE/fire_boll_run.wav", SE_INDEX::FIRE_BOLL_RUN, 0.1f, LOOP_INFINITY);
	register_se(L"resources/Audios/SE/rock_on.wav", SE_INDEX::ROCK_ON, 0.1f);
	register_se(L"resources/Audios/SE/player_rush.wav", SE_INDEX::PLAYER_RUSH, 0.1f);
	register_se(L"resources/Audios/SE/spear_rush.wav", SE_INDEX::SPEAR_RUSH, 0.1f);
	register_se(L"resources/Audios/SE/boss_rush.wav", SE_INDEX::BOSS_RUSH, 0.1f, LOOP_INFINITY);
	register_se(L"resources/Audios/SE/title_cut.wav", SE_INDEX::TITLE_CUT , 0.1f);
	register_se(L"resources/Audios/SE/title_cutting.wav", SE_INDEX::TITLE_CUTTING, 0.1f);
	register_se(L"resources/Audios/SE/draw_pen.wav", SE_INDEX::DRAW_PEN, 0.1f);
	register_se(L"resources/Audios/SE/boss_explosion.wav", SE_INDEX::BOSS_EXPLOSION, 0.1f);
	register_se(L"resources/Audios/SE/gauge_max.wav", SE_INDEX::GAUGE_MAX, 0.1f);
	register_se(L"resources/Audios/SE/boss_ready.wav", SE_INDEX::BOSS_SMALL_ROAR, 0.1f);
	register_se(L"resources/Audios/SE/docking_1.wav", SE_INDEX::DOCKING_1, 0.1f);
	register_se(L"resources/Audios/SE/docking_2.wav", SE_INDEX::DOCKING_2, 0.1f);
	register_se(L"resources/Audios/SE/foot_transform.wav", SE_INDEX::FOOT_TRANSFORM, 0.1f);
	register_se(L"resources/Audios/SE/reverberation.wav", SE_INDEX::REVERBERATION, 0.1f);
	register_se(L"resources/Audios/SE/roar_1.wav", SE_INDEX::ROAR_1, 0.1f);
	register_se(L"resources/Audios/SE/roar_2.wav", SE_INDEX::ROAR_2, 0.1f);
	register_se(L"resources/Audios/SE/saber.wav", SE_INDEX::SABER, 0.1f);
	register_se(L"resources/Audios/SE/splinters.wav", SE_INDEX::SPLINTERS, 0.1f);
	register_se(L"resources/Audios/SE/tearing.wav", SE_INDEX::TEARING, 0.1f);
	register_se(L"resources/Audios/SE/throw.wav", SE_INDEX::THROW, 0.1f);
	register_se(L"resources/Audios/SE/disc.wav", SE_INDEX::DISC, 0.1f);
	register_se(L"resources/Audios/SE/grab.wav", SE_INDEX::GRAB, 0.1f);
	register_se(L"resources/Audios/SE/shoulder_armor.wav", SE_INDEX::SHOULDER_ARMOR, 0.1f);
	register_se(L"resources/Audios/SE/transform_fast.wav", SE_INDEX::TRANSFORM_FAST, 0.1f);
	register_se(L"resources/Audios/SE/boss_ready_fast.wav", SE_INDEX::BOSS_READY_FAST, 0.1f);
	register_se(L"resources/Audios/SE/behaind_avoidanvce_recharge.wav", SE_INDEX::BEHAIND_RECHARGE, 0.1f);
	register_se(L"resources/Audios/SE/ship_roar.wav", SE_INDEX::ROAR_3, 0.1f);

}

//-------------------------------------------------------------
// int loop_count       -1:無限ループ  0:ループなし(デフォルト)
// 最小値:-1 最大値:254  その他:任意回数ループ
//-------------------------------------------------------------
void AudioManager::register_se(const LPCWSTR filename,
	SE_INDEX index, float volume, int loop_count)
{
	HRESULT hr{ S_OK };
	// Populating XAudio2 structures with the contents of RIFF chunks https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--load-audio-data-files-in-xaudio2
	// Declare WAVEFORMATEXTENSIBLE and XAUDIO2_BUFFER structures.
	WAVEFORMATEXTENSIBLE wfx = { 0 };
	// Open the audio file with CreateFile.
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL);
	_ASSERT_EXPR(INVALID_HANDLE_VALUE != hFile, L"Cannot find audio file");
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	// Locate the 'RIFF' chunk in the audio file, and check the file type.
	DWORD dwChunkSize;
	DWORD dwChunkPosition;
	FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
	DWORD filetype;
	ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
	_ASSERT_EXPR(filetype == fourccWAVE, L"Check the file type, should be fourccWAVE or 'XWMA'");
	// Locate the 'fmt ' chunk, and copy its contents into a WAVEFORMATEXTENSIBLE structure.
	FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
	ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
	// Locate the 'data' chunk, and read its contents into a buffer.
	FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
	se_pdata_buffer[static_cast<int>(index)] = new BYTE[dwChunkSize];
	ReadChunkData(hFile, se_pdata_buffer[static_cast<int>(index)], dwChunkSize, dwChunkPosition);
	// Populate an XAUDIO2_BUFFER structure.
	buffer_se[static_cast<int>(index)].AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
	buffer_se[static_cast<int>(index)].pAudioData = se_pdata_buffer[static_cast<int>(index)];  //buffer containing audio data
	buffer_se[static_cast<int>(index)].Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer
	assert(loop_count >= -1 && "loop_countが有り得ない数字になっています。loop_countは-1以上254未満の数字にしてください");
	loop_count = (std::min)(XAUDIO2_MAX_LOOP_COUNT, loop_count);
	if (loop_count == -1) {
		// ループの設定 https://docs.microsoft.com/ja-jp/windows/win32/api/xaudio2/ns-xaudio2-xaudio2_buffer?redirectedfrom=MSDN
		// 無限ループ
		buffer_se[static_cast<int>(index)].LoopCount = XAUDIO2_LOOP_INFINITE;
	}
	else if (loop_count == 0) { /*ループなし*/ buffer_se[static_cast<int>(index)].LoopBegin = 0; buffer_se[static_cast<int>(index)].LoopLength = 0; }
	else { /*任意回数ループ*/ 	buffer_se[static_cast<int>(index)].LoopCount = loop_count; }
	// How to: Play a Sound with XAudio2 https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--play-a-sound-with-xaudio2
	// Create a source voice by calling the IXAudio2::CreateSourceVoice method on an instance of the XAudio2 engine. The format of the voice is specified by the values set in a WAVEFORMATEX structure.
	hr = pXAudio2->CreateSourceVoice(&se_source_voices[static_cast<int>(index)], (WAVEFORMATEX*)&wfx);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	// Submit an XAUDIO2_BUFFER to the source voice using the function SubmitSourceBuffer.
	hr = se_source_voices[static_cast<int>(index)]->SubmitSourceBuffer(&buffer_se[static_cast<int>(index)]); // バッファに転送
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	hr = se_source_voices[static_cast<int>(index)]->SetVolume(volume);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void AudioManager::play_se(SE_INDEX index)
{
	HRESULT hr{ S_OK };
	assert(se_source_voices[static_cast<int>(index)] != nullptr
		&& "SEが設定されていません、再生するSEのインデックスが間違えている可能性があります");
	// Use the Start function to start the source voice.
	hr = se_source_voices[static_cast<int>(index)]->Stop(0); // 一時停止
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	hr = se_source_voices[static_cast<int>(index)]->FlushSourceBuffers(); // 曲を最初に戻す
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	hr = se_source_voices[static_cast<int>(index)]->SubmitSourceBuffer(&buffer_se[static_cast<int>(index)]); // バッファに転送
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	hr = se_source_voices[static_cast<int>(index)]->Start(0); // 再生
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void AudioManager::set_volume_se(SE_INDEX index, float volume)
{
	HRESULT hr{ S_OK };
	assert(se_source_voices[static_cast<int>(index)] != nullptr	&& "SEが設定されていません");
	hr = se_source_voices[static_cast<int>(index)]->SetVolume(volume);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void AudioManager::set_all_volume_se(float volume)
{
	for (int i = 0; i < static_cast<int>(SE_INDEX::SE_COUNT); i++)
	{
		if(i == static_cast<int>(SE_INDEX::OPEN_FIRE))
		{
			set_volume_se(SE_INDEX::OPEN_FIRE, volume * 0.03f);
		}
		else
		{
			set_volume_se(static_cast<SE_INDEX>(i), volume);
		}
	}
}

void AudioManager::stop_se(SE_INDEX index)
{
	HRESULT hr{ S_OK };
	// Use the Start function to start the source voice.
	hr = se_source_voices[static_cast<int>(index)]->Stop(0);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void AudioManager::stop_all_se()
{
	for (int i = 0; i < static_cast<int>(SE_INDEX::SE_COUNT); i++)
	{
		stop_se(static_cast<SE_INDEX>(i));
	}
}
