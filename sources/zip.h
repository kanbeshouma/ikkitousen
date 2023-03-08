#pragma once

#include<iostream>
//#include<zlib.h>
#include <sstream>
#include <fstream>
#include<assert.h>

#include"DebugConsole.h"

//<zlibの使い方のリンク>//
//https://www.s-yata.jp/docs/zlib/

constexpr auto BUFFER_SIZE = 0x4000;
// 展開する
#if 0
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
			return "error";
			break;
		case Z_BUF_ERROR:
			DebugConsole::Instance().WriteDebugConsole("入力データ・出力バッファの不足", TextColor::Red);
			return "error";
			break;
		case Z_STREAM_ERROR:
			DebugConsole::Instance().WriteDebugConsole("next_in, next_out が NULL内部状態が破壊されました", TextColor::Red);
			return "error";
			break;
		case Z_DATA_ERROR:
			DebugConsole::Instance().WriteDebugConsole("zlibで展開出来ないデータが入力されました", TextColor::Red);
			return "error";
			break;
		case Z_MEM_ERROR:
			DebugConsole::Instance().WriteDebugConsole("メモリの確保に失敗しました", TextColor::Red);
			return "error";
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
/// <param name="level_0">圧縮無効 Z_BEST_SPEED</param>
/// <param name="level_1">圧縮速度を最高 Z_BEST_SPEED</param>
/// <param name="level_6">デフォルト Z_DEFAULT_COMPRESSION</param>
/// <param name="level_9">圧縮率最高 Z_BEST_COMPRESSION</param>
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
		DebugConsole::Instance().WriteDebugConsole("圧縮の初期化失敗", TextColor::Red);
		return "error";
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
			return "error";
		}
		else if (ret == Z_STREAM_ERROR)
		{
			DebugConsole::Instance().WriteDebugConsole("データが壊れているか、出力データがNULLです", TextColor::Red);
			return "error";

		}

		auto out_size = BUFFER_SIZE - stream.avail_out;
		out_stream.write(reinterpret_cast<char*>(out_buf), out_size);
	} while (stream.avail_out == 0);


	deflateEnd(&stream);
	return out_stream.str();
}

#endif // 0

//<ハフマン符号化>//
namespace Hahuman
{

	//<ハフマン木の節データ>//
	struct Node
	{
		//<親へのポインタ>//
		Node* parent{ nullptr };
		//<左の子へのポインタ>//
		Node* left{ nullptr };
		//<右の子へのポインタ>//
		Node* right{ nullptr };
		//<次の節へのポインタ>//
		Node* next{ nullptr };
		//<出現回数>//
		unsigned int freq;
		//<文字>//
		char chr;
	};


	//<ハフマン符号化データ>//
	struct  Code
	{
		//<文字>//
		char chr;
		//<符号のビット数>//
		char bit;
		//<符号(ビット列)>//
		uint16_t value;
		//<次の符号情報へのポインタ>//
		Code* next{ nullptr };
	};


	//<ヘッダーデータ>//
	struct HeaderData
	{
		//<文字>//
		char chr;
		//<符号のビット数>//
		char bit;
		//<符号(ビット列)>//
		uint16_t value;
	};

	//<ハフマン符号化でのデータ圧縮>//
	struct HahumanEncode
	{
	private:
		/// <summary>
		/// 指定したchrを持つ節を探す
		/// </summary>
		/// <param name="node">文字が保存してあるNode構造体</param>
		/// <param name="chr">対象の文字</param>
		/// <returns></returns>
		Node* SearchNode(Node* node, char chr);

		/// <summary>
		/// 節を追加する
		/// </summary>
		/// <param name="chr">新しく生成するNode構造体に保存する文字</param>
		/// <returns></returns>
		Node* NewNode(char chr);

		/// <summary>
		/// 文から文字を読み取ってそれぞれの節を作成する
		/// </summary>
		/// <param name="data">データ</param>
		/// <param name="d_size">データのサイズ</param>
		/// <returns></returns>
		Node* MakeNodes(char* data, int d_size);


		/// <summary>
		/// 親のいない節の中から一番出現回数が少ない節と二番目に少ない節を探索する関数
		/// 一番少ない節はleft,二番目に少ないのはrightに指させる
		/// </summary>
		/// <param name="left">左側に保存するNode</param>
		/// <param name="right">右側に保存するNode</param>
		/// <param name="head">Node構造体リストの先頭アドレス</param>
		void GetChild(Node** left, Node** right, Node* head);

		/// <summary>
		/// ハフマン木を作成する
		/// </summary>
		/// <param name="head">Node構造体のリストの先頭</param>
		void MakeHuffmanTree(Node* head);

		/// <summary>
		/// 文字にハフマン符号を割り当てその情報をCode構造体に格納する
		/// </summary>
		/// <param name="node">Node構造体のリストの先頭</param>
		/// <returns></returns>
		Code* MakeHuffmanCode(Node* node);

		/// <summary>
		/// nodeが先頭を指す構造体リストを全削除する
		/// </summary>
		/// <param name="node">Node構造体リストの先頭</param>
		void FreeNode(Node* node);

		/// <summary>
		/// 引数の文字をメンバに持つCode構造体を探索する
		/// </summary>
		/// <param name="head">Code構造体リストの先頭</param>
		/// <param name="chr">検索する文字</param>
		/// <returns></returns>
		Code* SearchCode(Code* head, char chr);


		/// <summary>
		/// 符号化を行ってファイルに書き出しする
		/// </summary>
		/// <param name="data">元データ</param>
		/// <param name="encode_data">圧縮データ</param>
		/// <param name="d_size">元データサイズ</param>
		/// <param name="head">Code構造体リストの先頭</param>
		void EncodeData(char* data, char* encode_data,int d_size, Code* head);

		/// <summary>
		/// codeが先頭を指す構造体リストを全削除する
		/// </summary>
		/// <param name="node">解放するCode構造体リストの先頭</param>
		void FreeCode(Code* node);

	public:

		/// <summary>
		/// ハフマン符号化を使用した圧縮
		/// </summary>
		/// <param name="data">圧縮前のデータ</param>
		/// <param name="data">圧縮前のデータサイズ</param>
		/// <param name="encode_data">圧縮データ</param>
		void Encode(char* data,int d_size, char* encode_data);

	};

	//<ハフマン符号化でのデータ解凍>//
	struct HahumanDecode
	{
	private:
		/// <summary>
		/// 入力されたvalueとbit数に対応するCODE構造体を探索する関数
		/// </summary>
		/// <param name="head">Code構造体リストの先頭</param>
		/// <param name="value">比べるvalue</param>
		/// <param name="bit">比べるbit</param>
		/// <returns></returns>
		Code* SearchCode(Code* head, uint16_t value, char bit)
		{
			while (head != NULL)
			{
				if (head->value == value && head->bit == bit)
				{
					return head;
				}
				head = head->next;
			}
			return NULL;
		}

		/// <summary>
		/// codeが先頭を指す構造体リストを全削除する
		/// </summary>
		/// <param name="node">解放するCode構造体リストの先頭</param>
		void FreeCode(Code* node)
		{
			Code* next;
			while (node != nullptr)
			{
				next = node->next;
				delete node;
				node = next;
			}
		}

	public:
		/// <summary>
		/// データ解凍
		/// </summary>
		/// <param name="encode_data">圧縮データ</param>
		/// <param name="decode_data">解凍データ</param>
		void Decode(char* encode_data, char* decode_data);
	};


}