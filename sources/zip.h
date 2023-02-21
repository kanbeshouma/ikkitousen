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
		Node* SearchNode(Node* node, char chr)
		{

			while (node != NULL) {
				//-----同じ文字が見つかったらその節を返す-----//
				if (node->chr == chr) {
					return node;
				}
				//-----なかったら次の節に切り替える-----//
				node = node->next;
			}
			//-----ここに来るのは見つからなかった時になるのでnullが返る-----//
			return node;
		}

		/// <summary>
		/// 節を追加する
		/// </summary>
		/// <param name="chr">新しく生成するNode構造体に保存する文字</param>
		/// <returns></returns>
		Node* NewNode(char chr)
		{
			Node* node = new Node;

			node->freq = 1;
			node->chr = chr;

			return node;
		}

		/// <summary>
		/// 文から文字を読み取ってそれぞれの節を作成する
		/// </summary>
		/// <param name="data">データ</param>
		/// <returns></returns>
		Node* MakeNodes(std::string data)
		{
			//<リストの先頭>//
			Node* head{ nullptr };
			//<リストの最後尾>//
			Node* tail{ nullptr };
			Node* add{ nullptr };
			Node* search{ nullptr };

			for (auto c : data)
			{
				search = SearchNode(head, c);

				//-----節がなかったら新しく節を追加-----//
				if (search == nullptr)
				{
					add = NewNode(c);
					if (add == nullptr)
					{
						assert("error MakeNodes");
						return nullptr;
					}

					//-----最初の節の場合はheadとtailにその節を指させる-----//
					if (head == nullptr)
					{
						//<先頭に節を設定>//
						head = add;
						//<最後尾に節を設定>//
						tail = add;
					}
					else
					{
						//<今ある最後尾の次のポインタに今追加されたNodeを設定>//
						tail->next = add;
						//<追加されたNodeを最後尾に設定>//
						tail = add;
					}
				}
				else
				{
					//-----その文字に対応する節があるなら出現回数をカウントする-----//
					search->freq++;
				}

			}
			return head;
		}


		/// <summary>
		/// 親のいない節の中から一番出現回数が少ない節と二番目に少ない節を探索する関数
		/// 一番少ない節はleft,二番目に少ないのはrightに指させる
		/// </summary>
		/// <param name="left">左側に保存するNode</param>
		/// <param name="right">右側に保存するNode</param>
		/// <param name="head">Node構造体リストの先頭アドレス</param>
		void GetChild(Node** left, Node** right, Node* head)
		{
			//-----出現回数の最小値を格納する変数-----//
			unsigned int first = 999999;

			//-----出現回数の二番目に少ない値をを格納する変数-----//
			unsigned int second = 999999;

			*left = nullptr;
			*right = nullptr;

			//-----リストの先頭から探索-----//
			while (head != nullptr)
			{
				if (head->parent == nullptr)
				{
					//-----出現回数が一番少ないかどうかを判断-----//
					if (first > head->freq && second > head->freq)
					{
						//-----出現回数を保存-----//
						second = first;
						first = head->freq;
						//<二番目に小さいものを右に登録>//
						*right = *left;
						//<一番小さい物を左に登録>//
						*left = head;
					}
					else if (second > head->freq)
					{
						//<二番目に小さいものを右に登録>//
						second = head->freq;
						*right = head;
					}
				}
				//-----次の節に進む-----//
				head = head->next;
			}

		}

		/// <summary>
		/// ハフマン木を作成する
		/// </summary>
		/// <param name="head">Node構造体のリストの先頭</param>
		void MakeHuffmanTree(Node* head)
		{
			Node* left = nullptr;
			Node* right = nullptr;
			Node* tail = nullptr;
			Node* add = nullptr;

			if (head == nullptr)
			{
				assert("Error MakeHuffmanTree head = nullptr");
				return;
			}

			//-----リストの終端までtailを移動-----//
			tail = head;
			while (tail->next != nullptr)
			{
				tail = tail->next;
			}

			//-----出現回数の少ない二つの節を取得-----//
			GetChild(&left, &right, head);

			//-----親のいない節が2つ未満になるまでループ-----//
			while (left != nullptr && right != nullptr)
			{
				//-----新しいノードを追加-----//
				//<ここでヌル文字を設定しているのは葉以外は文字に対応しなから>//
				add = NewNode('\0');
				if (add == nullptr)
				{
					assert("Error MakeHuffmanTree add = nullptr");
					return;
				}

				//-----追加した節はleftとrightの親となるようにパラメータ設定-----//
				//<左右の親を設定>//
				left->parent = add;
				right->parent = add;
				//<左右の子を設定>//
				add->left = left;
				add->right = right;
				//<freqを設定>//
				add->freq = left->freq + right->freq;

				//-----追加した節をリストの終端に登録-----//
				tail->next = add;
				tail = add;

				//-----出現回数の少ない二つの節を取得-----//
				GetChild(&left, &right, head);

			}

		};

		/// <summary>
		/// 文字にハフマン符号を割り当てその情報をCode構造体に格納する
		/// </summary>
		/// <param name="node">Node構造体のリストの先頭</param>
		/// <returns></returns>
		Code* MakeHuffmanCode(Node* node)
		{
			Node* child = nullptr;

			Node* parent = nullptr;

			Code* code = nullptr;

			Code* top = nullptr;

			Code* add = nullptr;

			//-----符号(ビット列)-----//
			uint16_t value{};

			//-----符号のビット数-----//
			int bit{};

			//-----葉全てに符号を割り当てたら終了-----//
			//-----葉以外の節の文字は'\0'-----//
			while (node != nullptr && node->chr != '\0')
			{
				//-----初期化-----//
				value = 0;
				bit = 0;

				//-----構造体を追加-----//
				add = new Code;

				//-----追加した構造体の文字を設定-----//
				add->chr = node->chr;

				//-----childは今注目している節へのポインタ-----//
				child = node;

				//-----parentは今注目している節の親へのポインタ-----//
				parent = node->parent;

				//-----葉から根まで親を遡る-----//
				while (parent != nullptr)
				{
					//-----親からみて今注目している節が左の子-----//
					if (parent->left == child)
					{
						//-----ビット列の一番左へビット0を追加-----//
						value = value + (0 << bit);
					}
					//-----親からみて今注目している節が右の子-----//
					else if (parent->right == child)
					{
						//-----ビット列の一番左へビット1を追加-----//
						value = value + (1 << bit);
					}
					//-----ビット列のビットを1増やす-----//
					bit++;

					//-----今注目している節の親を新しく注目節に更新-----//
					child = parent;

					//-----親も更新-----//
					parent = parent->parent;

				}

				//-----符号を求め終わったので追加したCodeにそのビット数を設定-----//
				add->value = value;
				add->bit = bit;

				//-----文字が一種類の場合の例外処理-----//
				if (add->bit == 0) add->bit = 1;

				//-----Code構造体がまだ1つもない場合-----//
				if (code == nullptr)
				{
					code = add;
					top = code;
					code->next = nullptr;
				}
				//-----Code構造体がある場合は最後尾に追加-----//
				else
				{
					code->next = add;
					code = code->next;
					code->next = nullptr;
				}
				//-----次の節に対して符号を算出-----//
				node = node->next;
			}
			return top;
		}

		/// <summary>
		/// nodeが先頭を指す構造体リストを全削除する
		/// </summary>
		/// <param name="node">Node構造体リストの先頭</param>
		void FreeNode(Node* node)
		{
			Node* next;
			while (node != nullptr)
			{
				next = node->next;
				delete node;
				node = next;
			}
		}


		/// <summary>
		/// 引数の文字をメンバに持つCode構造体を探索する
		/// </summary>
		/// <param name="head">Code構造体リストの先頭</param>
		/// <param name="chr">検索する文字</param>
		/// <returns></returns>
		Code* SearchCode(Code* head, char chr)
		{
			while (head != nullptr)
			{
				if (head->chr == chr) return head;

				head = head->next;
			}
			return nullptr;
		}


		/// <summary>
		/// 符号化を行ってファイルに書き出しする
		/// </summary>
		/// <param name="data">元データ</param>
		/// <param name="encode_data">圧縮データ</param>
		/// <param name="head">Code構造体リストの先頭</param>
		void EncodeData(std::string data,char* encode_data, Code* head)
		{
			Code* code{ nullptr };
			unsigned char byte{};
			char empty_bits{};

			unsigned int text_length{};
			uint16_t mask{};
			char one_bit{};
			unsigned int header_size{};
			char chr{};
			int i{};
			char terminal = '\0';

			//<最終的な圧縮データサイズ>//
			int count{ 0 };

			text_length = 0;
			//-----入力文字数をカウント-----//
			for (auto t : data) text_length++;

			//-----ヘッダーを書き込む-----//
			header_size = 0;
			code = head;

			//<ヘッダー情報を書き込んだ回数>//
			int header_count{ 0 };

			//-----Code構造体のリストの先頭から終端まで文字の符号化情報をヘッダーとして書き出す-----//
			while (code != nullptr)
			{
				//<ヘッダーデータ>//
				HeaderData data;
				//<文字を書き込む>//
				data.chr = code->chr;

				//<符号化文字のビット数を書き込む>//
				data.bit = code->bit;

				//<この文字のエンコード結果を書き込む>//
				data.value = code->value;

				//<ヘッダーサイズを増やす>//
				header_size += sizeof(HeaderData);
				count = sizeof(HeaderData);

				//-----次の符号化情報書き出しに移る-----//
				code = code->next;

				std::memcpy(encode_data + (sizeof(HeaderData) * header_count), (char*)&data,sizeof(HeaderData));

				//<ヘッダー情報を書き込んだ回数を増やす>//
				header_count++;
			}


			//-----ヘッダーの終端として\0と文字数を書き出す-----//
			std::memcpy(encode_data + (sizeof(HeaderData) * header_count), (char*)&terminal, sizeof(terminal));
			//fwrite(&terminal, sizeof(terminal), 1, fo);
			//count++;

			//fwrite(&text_length, sizeof(text_length), 1, fo);
			std::memcpy(encode_data + (sizeof(HeaderData) * header_count) + sizeof(terminal), (char*)&text_length, sizeof(text_length));

			count += 4;
			header_size += 1 + sizeof(text_length);

			//<ヘッダーサイズ>//
			int hedder_max_size = (sizeof(HeaderData) * header_count) + sizeof(terminal) + sizeof(text_length);

			//-----バイトデータを0で初期化-----//
			byte = 0;

			//-----バイトの空きビットを8に設定-----//
			empty_bits = 8;

			int byte_count{};
			for (auto t : data)
			{
				//-----文字からCode構造体を取得-----//
				code = SearchCode(head, t);

				//-----符号の最上位ビットからバイトデータへ詰め合わせ-----//
				for (i = code->bit - 1; i >= 0; i--)
				{
					//-----iビット目が1でそれ以外が0のビット列を作成-----//
					mask = 1 << i;

					//-----iビット目以外を0にしたのちiビット分右にシフト-----//
					one_bit = (code->value & mask) >> i;

					//-----バイトデータの空きビット数 - 1分左シフトして足し合わせ-----//
					byte += one_bit << (empty_bits - 1);

					//-----空いているビット数を1減らす-----//
					empty_bits--;

					//-----空いているビットが0になったらファイルへ書き出し-----//
					if (empty_bits == 0)
					{
						std::memcpy(encode_data + hedder_max_size + (sizeof(byte) * byte_count), (char*)&byte, sizeof(byte));

						//-----ファイルに書き出したので空きビットとbyteを0で初期化-----//
						empty_bits = 8;
						byte = 0;
						count++;
						byte_count++;
					}
				}
			}

			//-----1バイト分つまらなかった分を最後にファイルへ書き出し-----//
			if (empty_bits < 8)
			{
				std::memcpy(encode_data + hedder_max_size + (sizeof(byte) * byte_count), (char*)&byte, sizeof(byte));
				count++;
			}
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
		/// ハフマン符号化を使用した圧縮
		/// </summary>
		/// <param name="data">圧縮前のデータ</param>
		/// <param name="encode_data">圧縮データ</param>
		void Encode(char* data, char* encode_data)
		{
			Node* nodes{ nullptr };
			Code* codes{ nullptr };

			//作成したNodeのリストの先頭のポインタが帰ってくる
			nodes = MakeNodes(data);

			MakeHuffmanTree(nodes);

			codes = MakeHuffmanCode(nodes);

			FreeNode(nodes);

			EncodeData(data, encode_data, codes);

			FreeCode(codes);
		}

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