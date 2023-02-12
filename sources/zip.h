#pragma once

#include<iostream>
#include<zlib.h>
#include <sstream>
#include <fstream>


#include"DebugConsole.h"

//<zlibの使い方のリンク>//
//https://www.s-yata.jp/docs/zlib/

constexpr auto BUFFER_SIZE = 0x4000;
// 展開する
inline std::string ZlibDevelopment(std::string& data)
{
	auto size = static_cast<unsigned int>(data.size());
	auto out_buf = new unsigned char[BUFFER_SIZE]();
	std::stringstream out_stream;
	z_stream stream{ 0 };

	//<初期化>//
	auto ret = inflateInit(&stream);

	//<入力データ設定>//
	stream.avail_in = size;
	stream.next_in = reinterpret_cast<unsigned char*>(const_cast<char*>(data.data()));

	do
	{
		//<出力データ設定>//
		stream.next_out = out_buf;
		stream.avail_out = BUFFER_SIZE;

		//<展開>//
		//第一引数:z_streamオブジェクト
		//第二引数:基本的にはZ_NO_FLUSHを設定
		ret = inflate(&stream, Z_NO_FLUSH);

		switch (ret)
		{
		case Z_NEED_DICT:
			DebugConsole::Instance().WriteDebugConsole("deflateSetDictionary() で指定した辞書が必要", TextColor::Red);
			return;
			break;
		case Z_BUF_ERROR:
			DebugConsole::Instance().WriteDebugConsole("入力データ・出力バッファの不足", TextColor::Red);
			return;
			break;
		case Z_STREAM_ERROR:
			DebugConsole::Instance().WriteDebugConsole("next_in, next_out が NULL内部状態が破壊されました", TextColor::Red);
			return;
			break;
		case Z_DATA_ERROR:
			DebugConsole::Instance().WriteDebugConsole("zlibで展開出来ないデータが入力されました", TextColor::Red);
			return;
			break;
		case Z_MEM_ERROR:
			DebugConsole::Instance().WriteDebugConsole("メモリの確保に失敗しました", TextColor::Red);
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
/// 圧縮する
/// </summary>
/// <param name="data">圧縮する元データ</param>
/// <param name="level">圧縮レベル</param>
/// <param name="level_0">圧縮無効</param>
/// <param name="level_1">圧縮速度を最高</param>
/// <param name="level_6">デフォルト</param>
/// <param name="level_9">圧縮率最高</param>
/// <returns></returns>
inline std::string ZlibCompression(std::string& data, const int level)
{
	auto size = static_cast<unsigned int>(data.size());
	auto out_buf = new unsigned char[BUFFER_SIZE]();
	std::stringstream out_stream;
	z_stream stream{ 0 };

	//<z_streamの初期化>//
	//第一引数 : z_stream
	//第二引数 : 圧縮レベル
	auto ret = deflateInit(&stream, level);
	if (ret != Z_OK)
	{
		DebugConsole::Instance().WriteDebugConsole("圧縮の初期化失敗",TextColor::Red);
		return;
	}

	//<入力データ設定>//
	stream.avail_in = size;
	stream.next_in = reinterpret_cast<unsigned char*>(const_cast<char*>(data.data()));
	do
	{
		//<出力データ設定>//
		stream.next_out = out_buf;
		stream.avail_out = BUFFER_SIZE;

		//<圧縮>//
		//第一引数:z_streamオブジェクト
		//第二引数 Z_NO_FLUSH,Z_FINISH
		//Z_NO_FLUSH:全てのデータを圧縮していない時
		//Z_FINISH:全てのデータを圧縮した時
		ret = deflate(&stream, Z_FINISH);

		if (ret == Z_BUF_ERROR)
		{
			DebugConsole::Instance().WriteDebugConsole("入力データか出力バッファが不足しています", TextColor::Red);
			return;
		}
		else if (ret == Z_STREAM_ERROR)
		{
			DebugConsole::Instance().WriteDebugConsole("データが壊れているか、出力データがNULLです", TextColor::Red);
			return;

		}

		auto out_size = BUFFER_SIZE - stream.avail_out;
		out_stream.write(reinterpret_cast<char*>(out_buf), out_size);
	} while (stream.avail_out == 0);

	deflateEnd(&stream);
	return out_stream.str();
}


