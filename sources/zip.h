#pragma once

#include<iostream>
//#include<zlib.h>
#include <sstream>
#include <fstream>
#include<assert.h>

#include"DebugConsole.h"

//<zlib�̎g�����̃����N>//
//https://www.s-yata.jp/docs/zlib/

constexpr auto BUFFER_SIZE = 0x4000;
// �W�J����
#if 0
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
			return "error";
			break;
		case Z_BUF_ERROR:
			DebugConsole::Instance().WriteDebugConsole("���̓f�[�^�E�o�̓o�b�t�@�̕s��", TextColor::Red);
			return "error";
			break;
		case Z_STREAM_ERROR:
			DebugConsole::Instance().WriteDebugConsole("next_in, next_out �� NULL������Ԃ��j�󂳂�܂���", TextColor::Red);
			return "error";
			break;
		case Z_DATA_ERROR:
			DebugConsole::Instance().WriteDebugConsole("zlib�œW�J�o���Ȃ��f�[�^�����͂���܂���", TextColor::Red);
			return "error";
			break;
		case Z_MEM_ERROR:
			DebugConsole::Instance().WriteDebugConsole("�������̊m�ۂɎ��s���܂���", TextColor::Red);
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
/// ���k����
/// </summary>
/// <param name="data">���k���錳�f�[�^</param>
/// <param name="level">���k���x��</param>
/// <param name="level_0">���k���� Z_BEST_SPEED</param>
/// <param name="level_1">���k���x���ō� Z_BEST_SPEED</param>
/// <param name="level_6">�f�t�H���g Z_DEFAULT_COMPRESSION</param>
/// <param name="level_9">���k���ō� Z_BEST_COMPRESSION</param>
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
		DebugConsole::Instance().WriteDebugConsole("���k�̏��������s", TextColor::Red);
		return "error";
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
			return "error";
		}
		else if (ret == Z_STREAM_ERROR)
		{
			DebugConsole::Instance().WriteDebugConsole("�f�[�^�����Ă��邩�A�o�̓f�[�^��NULL�ł�", TextColor::Red);
			return "error";

		}

		auto out_size = BUFFER_SIZE - stream.avail_out;
		out_stream.write(reinterpret_cast<char*>(out_buf), out_size);
	} while (stream.avail_out == 0);


	deflateEnd(&stream);
	return out_stream.str();
}

#endif // 0

//<�n�t�}��������>//
namespace Hahuman
{

	//<�n�t�}���؂̐߃f�[�^>//
	struct Node
	{
		//<�e�ւ̃|�C���^>//
		Node* parent{ nullptr };
		//<���̎q�ւ̃|�C���^>//
		Node* left{ nullptr };
		//<�E�̎q�ւ̃|�C���^>//
		Node* right{ nullptr };
		//<���̐߂ւ̃|�C���^>//
		Node* next{ nullptr };
		//<�o����>//
		unsigned int freq;
		//<����>//
		char chr;
	};


	//<�n�t�}���������f�[�^>//
	struct  Code
	{
		//<����>//
		char chr;
		//<�����̃r�b�g��>//
		char bit;
		//<����(�r�b�g��)>//
		uint16_t value;
		//<���̕������ւ̃|�C���^>//
		Code* next{ nullptr };
	};


	//<�w�b�_�[�f�[�^>//
	struct HeaderData
	{
		//<����>//
		char chr;
		//<�����̃r�b�g��>//
		char bit;
		//<����(�r�b�g��)>//
		uint16_t value;
	};

	//<�n�t�}���������ł̃f�[�^���k>//
	struct HahumanEncode
	{
	private:
		/// <summary>
		/// �w�肵��chr�����߂�T��
		/// </summary>
		/// <param name="node">�������ۑ����Ă���Node�\����</param>
		/// <param name="chr">�Ώۂ̕���</param>
		/// <returns></returns>
		Node* SearchNode(Node* node, char chr)
		{

			while (node != NULL) {
				//-----�������������������炻�̐߂�Ԃ�-----//
				if (node->chr == chr) {
					return node;
				}
				//-----�Ȃ������玟�̐߂ɐ؂�ւ���-----//
				node = node->next;
			}
			//-----�����ɗ���̂͌�����Ȃ��������ɂȂ�̂�null���Ԃ�-----//
			return node;
		}

		/// <summary>
		/// �߂�ǉ�����
		/// </summary>
		/// <param name="chr">�V������������Node�\���̂ɕۑ����镶��</param>
		/// <returns></returns>
		Node* NewNode(char chr)
		{
			Node* node = new Node;

			node->freq = 1;
			node->chr = chr;

			return node;
		}

		/// <summary>
		/// �����當����ǂݎ���Ă��ꂼ��̐߂��쐬����
		/// </summary>
		/// <param name="data">�f�[�^</param>
		/// <returns></returns>
		Node* MakeNodes(std::string data)
		{
			//<���X�g�̐擪>//
			Node* head{ nullptr };
			//<���X�g�̍Ō��>//
			Node* tail{ nullptr };
			Node* add{ nullptr };
			Node* search{ nullptr };

			for (auto c : data)
			{
				search = SearchNode(head, c);

				//-----�߂��Ȃ�������V�����߂�ǉ�-----//
				if (search == nullptr)
				{
					add = NewNode(c);
					if (add == nullptr)
					{
						assert("error MakeNodes");
						return nullptr;
					}

					//-----�ŏ��̐߂̏ꍇ��head��tail�ɂ��̐߂��w������-----//
					if (head == nullptr)
					{
						//<�擪�ɐ߂�ݒ�>//
						head = add;
						//<�Ō���ɐ߂�ݒ�>//
						tail = add;
					}
					else
					{
						//<������Ō���̎��̃|�C���^�ɍ��ǉ����ꂽNode��ݒ�>//
						tail->next = add;
						//<�ǉ����ꂽNode���Ō���ɐݒ�>//
						tail = add;
					}
				}
				else
				{
					//-----���̕����ɑΉ�����߂�����Ȃ�o���񐔂��J�E���g����-----//
					search->freq++;
				}

			}
			return head;
		}


		/// <summary>
		/// �e�̂��Ȃ��߂̒������ԏo���񐔂����Ȃ��߂Ɠ�Ԗڂɏ��Ȃ��߂�T������֐�
		/// ��ԏ��Ȃ��߂�left,��Ԗڂɏ��Ȃ��̂�right�Ɏw������
		/// </summary>
		/// <param name="left">�����ɕۑ�����Node</param>
		/// <param name="right">�E���ɕۑ�����Node</param>
		/// <param name="head">Node�\���̃��X�g�̐擪�A�h���X</param>
		void GetChild(Node** left, Node** right, Node* head)
		{
			//-----�o���񐔂̍ŏ��l���i�[����ϐ�-----//
			unsigned int first = 999999;

			//-----�o���񐔂̓�Ԗڂɏ��Ȃ��l�����i�[����ϐ�-----//
			unsigned int second = 999999;

			*left = nullptr;
			*right = nullptr;

			//-----���X�g�̐擪����T��-----//
			while (head != nullptr)
			{
				if (head->parent == nullptr)
				{
					//-----�o���񐔂���ԏ��Ȃ����ǂ����𔻒f-----//
					if (first > head->freq && second > head->freq)
					{
						//-----�o���񐔂�ۑ�-----//
						second = first;
						first = head->freq;
						//<��Ԗڂɏ��������̂��E�ɓo�^>//
						*right = *left;
						//<��ԏ������������ɓo�^>//
						*left = head;
					}
					else if (second > head->freq)
					{
						//<��Ԗڂɏ��������̂��E�ɓo�^>//
						second = head->freq;
						*right = head;
					}
				}
				//-----���̐߂ɐi��-----//
				head = head->next;
			}

		}

		/// <summary>
		/// �n�t�}���؂��쐬����
		/// </summary>
		/// <param name="head">Node�\���̂̃��X�g�̐擪</param>
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

			//-----���X�g�̏I�[�܂�tail���ړ�-----//
			tail = head;
			while (tail->next != nullptr)
			{
				tail = tail->next;
			}

			//-----�o���񐔂̏��Ȃ���̐߂��擾-----//
			GetChild(&left, &right, head);

			//-----�e�̂��Ȃ��߂�2�����ɂȂ�܂Ń��[�v-----//
			while (left != nullptr && right != nullptr)
			{
				//-----�V�����m�[�h��ǉ�-----//
				//<�����Ńk��������ݒ肵�Ă���̂͗t�ȊO�͕����ɑΉ����Ȃ���>//
				add = NewNode('\0');
				if (add == nullptr)
				{
					assert("Error MakeHuffmanTree add = nullptr");
					return;
				}

				//-----�ǉ������߂�left��right�̐e�ƂȂ�悤�Ƀp�����[�^�ݒ�-----//
				//<���E�̐e��ݒ�>//
				left->parent = add;
				right->parent = add;
				//<���E�̎q��ݒ�>//
				add->left = left;
				add->right = right;
				//<freq��ݒ�>//
				add->freq = left->freq + right->freq;

				//-----�ǉ������߂����X�g�̏I�[�ɓo�^-----//
				tail->next = add;
				tail = add;

				//-----�o���񐔂̏��Ȃ���̐߂��擾-----//
				GetChild(&left, &right, head);

			}

		};

		/// <summary>
		/// �����Ƀn�t�}�����������蓖�Ă��̏���Code�\���̂Ɋi�[����
		/// </summary>
		/// <param name="node">Node�\���̂̃��X�g�̐擪</param>
		/// <returns></returns>
		Code* MakeHuffmanCode(Node* node)
		{
			Node* child = nullptr;

			Node* parent = nullptr;

			Code* code = nullptr;

			Code* top = nullptr;

			Code* add = nullptr;

			//-----����(�r�b�g��)-----//
			uint16_t value{};

			//-----�����̃r�b�g��-----//
			int bit{};

			//-----�t�S�Ăɕ��������蓖�Ă���I��-----//
			//-----�t�ȊO�̐߂̕�����'\0'-----//
			while (node != nullptr && node->chr != '\0')
			{
				//-----������-----//
				value = 0;
				bit = 0;

				//-----�\���̂�ǉ�-----//
				add = new Code;

				//-----�ǉ������\���̂̕�����ݒ�-----//
				add->chr = node->chr;

				//-----child�͍����ڂ��Ă���߂ւ̃|�C���^-----//
				child = node;

				//-----parent�͍����ڂ��Ă���߂̐e�ւ̃|�C���^-----//
				parent = node->parent;

				//-----�t���獪�܂Őe��k��-----//
				while (parent != nullptr)
				{
					//-----�e����݂č����ڂ��Ă���߂����̎q-----//
					if (parent->left == child)
					{
						//-----�r�b�g��̈�ԍ��փr�b�g0��ǉ�-----//
						value = value + (0 << bit);
					}
					//-----�e����݂č����ڂ��Ă���߂��E�̎q-----//
					else if (parent->right == child)
					{
						//-----�r�b�g��̈�ԍ��փr�b�g1��ǉ�-----//
						value = value + (1 << bit);
					}
					//-----�r�b�g��̃r�b�g��1���₷-----//
					bit++;

					//-----�����ڂ��Ă���߂̐e��V�������ڐ߂ɍX�V-----//
					child = parent;

					//-----�e���X�V-----//
					parent = parent->parent;

				}

				//-----���������ߏI������̂Œǉ�����Code�ɂ��̃r�b�g����ݒ�-----//
				add->value = value;
				add->bit = bit;

				//-----���������ނ̏ꍇ�̗�O����-----//
				if (add->bit == 0) add->bit = 1;

				//-----Code�\���̂��܂�1���Ȃ��ꍇ-----//
				if (code == nullptr)
				{
					code = add;
					top = code;
					code->next = nullptr;
				}
				//-----Code�\���̂�����ꍇ�͍Ō���ɒǉ�-----//
				else
				{
					code->next = add;
					code = code->next;
					code->next = nullptr;
				}
				//-----���̐߂ɑ΂��ĕ������Z�o-----//
				node = node->next;
			}
			return top;
		}

		/// <summary>
		/// node���擪���w���\���̃��X�g��S�폜����
		/// </summary>
		/// <param name="node">Node�\���̃��X�g�̐擪</param>
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
		/// �����̕����������o�Ɏ���Code�\���̂�T������
		/// </summary>
		/// <param name="head">Code�\���̃��X�g�̐擪</param>
		/// <param name="chr">�������镶��</param>
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
		/// ���������s���ăt�@�C���ɏ����o������
		/// </summary>
		/// <param name="data">���f�[�^</param>
		/// <param name="encode_data">���k�f�[�^</param>
		/// <param name="head">Code�\���̃��X�g�̐擪</param>
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

			//<�ŏI�I�Ȉ��k�f�[�^�T�C�Y>//
			int count{ 0 };

			text_length = 0;
			//-----���͕��������J�E���g-----//
			for (auto t : data) text_length++;

			//-----�w�b�_�[����������-----//
			header_size = 0;
			code = head;

			//<�w�b�_�[�����������񂾉�>//
			int header_count{ 0 };

			//-----Code�\���̂̃��X�g�̐擪����I�[�܂ŕ����̕����������w�b�_�[�Ƃ��ď����o��-----//
			while (code != nullptr)
			{
				//<�w�b�_�[�f�[�^>//
				HeaderData data;
				//<��������������>//
				data.chr = code->chr;

				//<�����������̃r�b�g������������>//
				data.bit = code->bit;

				//<���̕����̃G���R�[�h���ʂ���������>//
				data.value = code->value;

				//<�w�b�_�[�T�C�Y�𑝂₷>//
				header_size += sizeof(HeaderData);
				count = sizeof(HeaderData);

				//-----���̕�������񏑂��o���Ɉڂ�-----//
				code = code->next;

				std::memcpy(encode_data + (sizeof(HeaderData) * header_count), (char*)&data,sizeof(HeaderData));

				//<�w�b�_�[�����������񂾉񐔂𑝂₷>//
				header_count++;
			}


			//-----�w�b�_�[�̏I�[�Ƃ���\0�ƕ������������o��-----//
			std::memcpy(encode_data + (sizeof(HeaderData) * header_count), (char*)&terminal, sizeof(terminal));
			//fwrite(&terminal, sizeof(terminal), 1, fo);
			//count++;

			//fwrite(&text_length, sizeof(text_length), 1, fo);
			std::memcpy(encode_data + (sizeof(HeaderData) * header_count) + sizeof(terminal), (char*)&text_length, sizeof(text_length));

			count += 4;
			header_size += 1 + sizeof(text_length);

			//<�w�b�_�[�T�C�Y>//
			int hedder_max_size = (sizeof(HeaderData) * header_count) + sizeof(terminal) + sizeof(text_length);

			//-----�o�C�g�f�[�^��0�ŏ�����-----//
			byte = 0;

			//-----�o�C�g�̋󂫃r�b�g��8�ɐݒ�-----//
			empty_bits = 8;

			int byte_count{};
			for (auto t : data)
			{
				//-----��������Code�\���̂��擾-----//
				code = SearchCode(head, t);

				//-----�����̍ŏ�ʃr�b�g����o�C�g�f�[�^�֋l�ߍ��킹-----//
				for (i = code->bit - 1; i >= 0; i--)
				{
					//-----i�r�b�g�ڂ�1�ł���ȊO��0�̃r�b�g����쐬-----//
					mask = 1 << i;

					//-----i�r�b�g�ڈȊO��0�ɂ����̂�i�r�b�g���E�ɃV�t�g-----//
					one_bit = (code->value & mask) >> i;

					//-----�o�C�g�f�[�^�̋󂫃r�b�g�� - 1�����V�t�g���đ������킹-----//
					byte += one_bit << (empty_bits - 1);

					//-----�󂢂Ă���r�b�g����1���炷-----//
					empty_bits--;

					//-----�󂢂Ă���r�b�g��0�ɂȂ�����t�@�C���֏����o��-----//
					if (empty_bits == 0)
					{
						std::memcpy(encode_data + hedder_max_size + (sizeof(byte) * byte_count), (char*)&byte, sizeof(byte));

						//-----�t�@�C���ɏ����o�����̂ŋ󂫃r�b�g��byte��0�ŏ�����-----//
						empty_bits = 8;
						byte = 0;
						count++;
						byte_count++;
					}
				}
			}

			//-----1�o�C�g���܂�Ȃ����������Ō�Ƀt�@�C���֏����o��-----//
			if (empty_bits < 8)
			{
				std::memcpy(encode_data + hedder_max_size + (sizeof(byte) * byte_count), (char*)&byte, sizeof(byte));
				count++;
			}
		}

		/// <summary>
		/// code���擪���w���\���̃��X�g��S�폜����
		/// </summary>
		/// <param name="node">�������Code�\���̃��X�g�̐擪</param>
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
		/// �n�t�}�����������g�p�������k
		/// </summary>
		/// <param name="data">���k�O�̃f�[�^</param>
		/// <param name="encode_data">���k�f�[�^</param>
		void Encode(char* data, char* encode_data)
		{
			Node* nodes{ nullptr };
			Code* codes{ nullptr };

			//�쐬����Node�̃��X�g�̐擪�̃|�C���^���A���Ă���
			nodes = MakeNodes(data);

			MakeHuffmanTree(nodes);

			codes = MakeHuffmanCode(nodes);

			FreeNode(nodes);

			EncodeData(data, encode_data, codes);

			FreeCode(codes);
		}

	};

	//<�n�t�}���������ł̃f�[�^��>//
	struct HahumanDecode
	{
	private:
		/// <summary>
		/// ���͂��ꂽvalue��bit���ɑΉ�����CODE�\���̂�T������֐�
		/// </summary>
		/// <param name="head">Code�\���̃��X�g�̐擪</param>
		/// <param name="value">��ׂ�value</param>
		/// <param name="bit">��ׂ�bit</param>
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
		/// code���擪���w���\���̃��X�g��S�폜����
		/// </summary>
		/// <param name="node">�������Code�\���̃��X�g�̐擪</param>
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
		/// �f�[�^��
		/// </summary>
		/// <param name="encode_data">���k�f�[�^</param>
		/// <param name="decode_data">�𓀃f�[�^</param>
		void Decode(char* encode_data, char* decode_data);
	};


}