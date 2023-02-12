#pragma once

#include<iostream>
#include<zlib.h>
#include <sstream>
#include <fstream>


#include"DebugConsole.h"

//<zlib�̎g�����̃����N>//
//https://www.s-yata.jp/docs/zlib/

constexpr auto BUFFER_SIZE = 0x4000;
// �W�J����
inline std::string ZlibDevelopment(std::string& data)
{
	auto size = static_cast<unsigned int>(data.size());
	auto out_buf = new unsigned char[BUFFER_SIZE]();
	std::stringstream out_stream;
	z_stream stream{ 0 };

	//<������>//
	auto ret = inflateInit(&stream);

	//<���̓f�[�^�ݒ�>//
	stream.avail_in = size;
	stream.next_in = reinterpret_cast<unsigned char*>(const_cast<char*>(data.data()));

	do
	{
		//<�o�̓f�[�^�ݒ�>//
		stream.next_out = out_buf;
		stream.avail_out = BUFFER_SIZE;

		//<�W�J>//
		//������:z_stream�I�u�W�F�N�g
		//������:��{�I�ɂ�Z_NO_FLUSH��ݒ�
		ret = inflate(&stream, Z_NO_FLUSH);

		switch (ret)
		{
		case Z_NEED_DICT:
			DebugConsole::Instance().WriteDebugConsole("deflateSetDictionary() �Ŏw�肵���������K�v", TextColor::Red);
			return;
			break;
		case Z_BUF_ERROR:
			DebugConsole::Instance().WriteDebugConsole("���̓f�[�^�E�o�̓o�b�t�@�̕s��", TextColor::Red);
			return;
			break;
		case Z_STREAM_ERROR:
			DebugConsole::Instance().WriteDebugConsole("next_in, next_out �� NULL������Ԃ��j�󂳂�܂���", TextColor::Red);
			return;
			break;
		case Z_DATA_ERROR:
			DebugConsole::Instance().WriteDebugConsole("zlib�œW�J�o���Ȃ��f�[�^�����͂���܂���", TextColor::Red);
			return;
			break;
		case Z_MEM_ERROR:
			DebugConsole::Instance().WriteDebugConsole("�������̊m�ۂɎ��s���܂���", TextColor::Red);
			return;
			break;
		default:
			break;
		}


		auto out_size = BUFFER_SIZE - stream.avail_out;
		out_stream.write(reinterpret_cast<char*>(out_buf), out_size);
	} while (stream.avail_out == 0);

	inflateEnd(&stream);

	return out_stream.str();
}

/// <summary>
/// ���k����
/// </summary>
/// <param name="data">���k���錳�f�[�^</param>
/// <param name="level">���k���x��</param>
/// <param name="level_0">���k����</param>
/// <param name="level_1">���k���x���ō�</param>
/// <param name="level_6">�f�t�H���g</param>
/// <param name="level_9">���k���ō�</param>
/// <returns></returns>
inline std::string ZlibCompression(std::string& data, const int level)
{
	auto size = static_cast<unsigned int>(data.size());
	auto out_buf = new unsigned char[BUFFER_SIZE]();
	std::stringstream out_stream;
	z_stream stream{ 0 };

	//<z_stream�̏�����>//
	//������ : z_stream
	//������ : ���k���x��
	auto ret = deflateInit(&stream, level);
	if (ret != Z_OK)
	{
		DebugConsole::Instance().WriteDebugConsole("���k�̏��������s",TextColor::Red);
		return;
	}

	//<���̓f�[�^�ݒ�>//
	stream.avail_in = size;
	stream.next_in = reinterpret_cast<unsigned char*>(const_cast<char*>(data.data()));
	do
	{
		//<�o�̓f�[�^�ݒ�>//
		stream.next_out = out_buf;
		stream.avail_out = BUFFER_SIZE;

		//<���k>//
		//������:z_stream�I�u�W�F�N�g
		//������ Z_NO_FLUSH,Z_FINISH
		//Z_NO_FLUSH:�S�Ẵf�[�^�����k���Ă��Ȃ���
		//Z_FINISH:�S�Ẵf�[�^�����k������
		ret = deflate(&stream, Z_FINISH);

		if (ret == Z_BUF_ERROR)
		{
			DebugConsole::Instance().WriteDebugConsole("���̓f�[�^���o�̓o�b�t�@���s�����Ă��܂�", TextColor::Red);
			return;
		}
		else if (ret == Z_STREAM_ERROR)
		{
			DebugConsole::Instance().WriteDebugConsole("�f�[�^�����Ă��邩�A�o�̓f�[�^��NULL�ł�", TextColor::Red);
			return;

		}

		auto out_size = BUFFER_SIZE - stream.avail_out;
		out_stream.write(reinterpret_cast<char*>(out_buf), out_size);
	} while (stream.avail_out == 0);

	deflateEnd(&stream);
	return out_stream.str();
}


