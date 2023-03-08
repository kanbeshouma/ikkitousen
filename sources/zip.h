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
		Node* SearchNode(Node* node, char chr);

		/// <summary>
		/// �߂�ǉ�����
		/// </summary>
		/// <param name="chr">�V������������Node�\���̂ɕۑ����镶��</param>
		/// <returns></returns>
		Node* NewNode(char chr);

		/// <summary>
		/// �����當����ǂݎ���Ă��ꂼ��̐߂��쐬����
		/// </summary>
		/// <param name="data">�f�[�^</param>
		/// <param name="d_size">�f�[�^�̃T�C�Y</param>
		/// <returns></returns>
		Node* MakeNodes(char* data, int d_size);


		/// <summary>
		/// �e�̂��Ȃ��߂̒������ԏo���񐔂����Ȃ��߂Ɠ�Ԗڂɏ��Ȃ��߂�T������֐�
		/// ��ԏ��Ȃ��߂�left,��Ԗڂɏ��Ȃ��̂�right�Ɏw������
		/// </summary>
		/// <param name="left">�����ɕۑ�����Node</param>
		/// <param name="right">�E���ɕۑ�����Node</param>
		/// <param name="head">Node�\���̃��X�g�̐擪�A�h���X</param>
		void GetChild(Node** left, Node** right, Node* head);

		/// <summary>
		/// �n�t�}���؂��쐬����
		/// </summary>
		/// <param name="head">Node�\���̂̃��X�g�̐擪</param>
		void MakeHuffmanTree(Node* head);

		/// <summary>
		/// �����Ƀn�t�}�����������蓖�Ă��̏���Code�\���̂Ɋi�[����
		/// </summary>
		/// <param name="node">Node�\���̂̃��X�g�̐擪</param>
		/// <returns></returns>
		Code* MakeHuffmanCode(Node* node);

		/// <summary>
		/// node���擪���w���\���̃��X�g��S�폜����
		/// </summary>
		/// <param name="node">Node�\���̃��X�g�̐擪</param>
		void FreeNode(Node* node);

		/// <summary>
		/// �����̕����������o�Ɏ���Code�\���̂�T������
		/// </summary>
		/// <param name="head">Code�\���̃��X�g�̐擪</param>
		/// <param name="chr">�������镶��</param>
		/// <returns></returns>
		Code* SearchCode(Code* head, char chr);


		/// <summary>
		/// ���������s���ăt�@�C���ɏ����o������
		/// </summary>
		/// <param name="data">���f�[�^</param>
		/// <param name="encode_data">���k�f�[�^</param>
		/// <param name="d_size">���f�[�^�T�C�Y</param>
		/// <param name="head">Code�\���̃��X�g�̐擪</param>
		void EncodeData(char* data, char* encode_data,int d_size, Code* head);

		/// <summary>
		/// code���擪���w���\���̃��X�g��S�폜����
		/// </summary>
		/// <param name="node">�������Code�\���̃��X�g�̐擪</param>
		void FreeCode(Code* node);

	public:

		/// <summary>
		/// �n�t�}�����������g�p�������k
		/// </summary>
		/// <param name="data">���k�O�̃f�[�^</param>
		/// <param name="data">���k�O�̃f�[�^�T�C�Y</param>
		/// <param name="encode_data">���k�f�[�^</param>
		void Encode(char* data,int d_size, char* encode_data);

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