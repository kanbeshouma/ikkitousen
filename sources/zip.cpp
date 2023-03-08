#include"zip.h"
#include"DebugConsole.h"

void Hahuman::HahumanDecode::Decode(char* encode_data, char* decode_data)
{
	Code* head = NULL;
	Code* tail = NULL;
	Code* add = NULL;
	Code* code = NULL;
	unsigned int headersize{};
	unsigned int textlength{};
	int bits{};
	char bitNum{};
	int oneBit{};
	char chr{};
	int mask{};
	int i{};

	//<�ǂݍ��񂾃f�[�^�̃T�C�Y>//
	int data_count{};
	while (1) {
		/* �܂���������ǂݍ��� */
		std::memcpy(&chr, encode_data + data_count, sizeof(chr));
		data_count += sizeof(chr);

		/* �������f�[�^�̏I�[��\�� '\0'�ł������ꍇ
		   �w�b�_�[�ǂݍ��݂��I�� */
		if (chr == '\0') break;

		/* CODE�\���̂�ǉ� */

		add = new Code;
		if (add == NULL) {
			printf("malloc error\n");
			return;
		}
		add->next = NULL;

		/* �������ɓǂݍ��񂾕������Z�b�g */
		add->chr = chr;

		/* �����������̃r�b�g����ǂݍ��� */
		std::memcpy(&add->bit, encode_data + data_count, sizeof((add->bit)));
		data_count += sizeof(add->bit);


		/* ���̕����̃G���R�[�h���ʂ�ǂݍ��� */
		std::memcpy(&add->value, encode_data + data_count, sizeof((add->value)));
		data_count += sizeof(add->value);


		headersize += sizeof(HeaderData);
		printf("chr = %c, bit = %d, value = %x\n", add->chr, add->bit, add->value);

		if (head == NULL) {
			/* CODE�\���̂�����Ȃ��ꍇ�̓��X�g�̐擪�Ƃ��� */
			head = add;
			tail = head;
		}
		else {
			/* CODE�\���̂����łɂ���ꍇ�̓��X�g�̍Ō���ɒǉ� */
			tail->next = add;
			tail = add;
		}
	}

	//<�w�b�_�[�� '\0'�ɂ��錳�f�[�^�̕��������擾>//
	if (chr == '\0') {
		//fread(&textlength, sizeof(textlength), 1, fi);
		std::memcpy(&textlength, encode_data + data_count, sizeof(textlength));
		data_count += sizeof(textlength);

	}
	printf("headrsize = %lu\n", headersize + 1 + sizeof(textlength));

	//<�r�b�g��ƃr�b�g����0�ŏ�����>//
	bits = 0;
	bitNum = 0;

	//<�������񂾃o�C�g��>//
	int decode_count{};
	while (textlength > 0)
	{
		//<�t�@�C������P�o�C�g���ǂݍ���>//
		//chr = fgetc(fi);
		std::memcpy(&chr, encode_data + data_count, sizeof(chr));
		std::cout << chr << std::endl;
		data_count++;


		//<chr�̍ŏ�ʃr�b�g7����r�b�g0�܂Ń��[�v>//
		for (i = 7; i >= 0; i--) {

			//<�r�b�gi�������Ă��r�b�g����쐬>//
			mask = 1 << i;

			//<�_���ςƃV�t�g���Z�ɂ��chr�̃r�b�gi�̒l���擾>//
			oneBit = (chr & mask) >> i;

			//<�r�b�g���1�r�b�g���V�t�g���ĉE����1�r�b�g�󂫂����>//
			bits = bits << 1;

			//<�r�b�g��̉E�[�r�b�g�Ƀr�b�gi�̒l���l�ߍ��킹��>//
			bits += oneBit;

			//<��L�Ńr�b�g�����������̂�bitNum�𑝉�>//
			bitNum++;

			//<bits��value�Ƃ��AbitNum��bit�Ƃ���CODE�\���̂�T��>//
			code = SearchCode(head, bits, bitNum);

			//<���̂悤��CODE�\���̂�����ꍇ�́A����CODE�\���̂�chr���t�@�C���֏����o��>//
			if (code != NULL)
			{
				if (code->chr == '0')code->chr = 0;
				std::memcpy(decode_data + decode_count, &code->chr, sizeof(code->chr));
				decode_count += sizeof(code->chr);


				//<�����o�����̂ŕ����r�b�g��bits��0�ɁA�r�b�g��bitNum��0�ɃZ�b�g>//
				bits = 0;
				bitNum = 0;

				//<�P�����������񂾂̂Ŏc�蕶������1���炷>//
				textlength--;

				//<�c�蕶������0�ɂȂ�����I��>//
				if (textlength <= 0) {
					break;
				}
			}
		}
	}

	//<CODE�\���̂��폜>//
	FreeCode(head);

}

Hahuman::Node* Hahuman::HahumanEncode::SearchNode(Node* node, char chr)
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

Hahuman::Node* Hahuman::HahumanEncode::NewNode(char chr)
{
	Node* node = new Node;

	node->freq = 1;
	node->chr = chr;

	return node;
}

Hahuman::Node* Hahuman::HahumanEncode::MakeNodes(char* data,int d_size)
{
	//<���X�g�̐擪>//
	Node* head{ nullptr };
	//<���X�g�̍Ō��>//
	Node* tail{ nullptr };
	Node* add{ nullptr };
	Node* search{ nullptr };

	for (int s = 0; s < d_size; s++)
	{
		char c = *data;

		search = SearchNode(head, c);

		//-----�߂��Ȃ�������V�����߂�ǉ�-----//
		if (search == nullptr)
		{

			//������0(null)�̎��͕�����0�����Ă���
			//�����ŕ��������Ă��闝�R�̓w�b�_�[���̏I����e�m�[�h��
			//0(null)���g�p���邩�炻��Ƃ͕ʂő��݂��Ă��Ȃ��Ƃ����Ȃ�����
			if (c == '\0') c = '0';
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
		data++;
	}
	return head;
}

void Hahuman::HahumanEncode::GetChild(Node** left, Node** right, Node* head)
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

void Hahuman::HahumanEncode::MakeHuffmanTree(Node* head)
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

}
Hahuman::Code* Hahuman::HahumanEncode::MakeHuffmanCode(Node* node)
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

void Hahuman::HahumanEncode::FreeNode(Node* node)
{
	Node* next;
	while (node != nullptr)
	{
		next = node->next;
		delete node;
		node = next;
	}
}

Hahuman::Code* Hahuman::HahumanEncode::SearchCode(Code* head, char chr)
{
	while (head != nullptr)
	{
		if (chr == '\0') chr = '0';

		if (head->chr == chr) return head;

		head = head->next;
	}
	return nullptr;
}

void Hahuman::HahumanEncode::EncodeData(char* data,char* encode_data, int d_size,Code* head)
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

	text_length = d_size;
	//-----���͕��������J�E���g-----//
	//for (auto t : data) text_length++;

	//-----�w�b�_�[����������-----//
	header_size = 0;
	code = head;

	//<�w�b�_�[�����������񂾉�>//
	int header_count{ 0 };

	//-----Code�\���̂̃��X�g�̐擪����I�[�܂ŕ����̕����������w�b�_�[�Ƃ��ď����o��-----//
	while (code != nullptr)
	{
		//<�w�b�_�[�f�[�^>//
		HeaderData h_data;
		//<��������������>//
		h_data.chr = code->chr;

		//<�����������̃r�b�g������������>//
		h_data.bit = code->bit;

		//<���̕����̃G���R�[�h���ʂ���������>//
		h_data.value = code->value;


		printf("chr = %c, bit = %d, value = %x\n", h_data.chr, h_data.bit, h_data.value);

		//<�w�b�_�[�T�C�Y�𑝂₷>//
		header_size += sizeof(HeaderData);
		count = sizeof(HeaderData);

		//-----���̕�������񏑂��o���Ɉڂ�-----//
		code = code->next;

		std::memcpy(encode_data + (sizeof(HeaderData) * header_count), (char*)&h_data, sizeof(HeaderData));

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
	for (int s = 0; s < d_size; s++)
	{
		char t = *data;
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

				std::cout << byte << std::endl;

				//-----�t�@�C���ɏ����o�����̂ŋ󂫃r�b�g��byte��0�ŏ�����-----//
				empty_bits = 8;
				byte = 0;
				count++;
				byte_count++;
			}
		}
		data++;
	}

	//-----1�o�C�g���܂�Ȃ����������Ō�Ƀt�@�C���֏����o��-----//
	if (empty_bits < 8)
	{
		std::memcpy(encode_data + hedder_max_size + (sizeof(byte) * byte_count), (char*)&byte, sizeof(byte));
		count++;
	}
}

void Hahuman::HahumanEncode::FreeCode(Code* node)
{
	Code* next;
	while (node != nullptr)
	{
		next = node->next;
		delete node;
		node = next;
	}
}

void Hahuman::HahumanEncode::Encode(char* data, int d_size, char* encode_data)
{
	Node* nodes{ nullptr };
	Code* codes{ nullptr };

	//�쐬����Node�̃��X�g�̐擪�̃|�C���^���A���Ă���
	nodes = MakeNodes(data, d_size);

	if (nodes == nullptr)
	{
		DebugConsole::Instance().WriteDebugConsole("�n�t�}���؂̐߃f�[�^�쐬���s",TextColor::Red);
		return;
	}

	MakeHuffmanTree(nodes);

	codes = MakeHuffmanCode(nodes);

	if (codes == nullptr)
	{
		DebugConsole::Instance().WriteDebugConsole("�n�t�}���������f�[�^�쐬���s", TextColor::Red);
		return;
	}

	FreeNode(nodes);

	EncodeData(data, encode_data, d_size,codes);

	FreeCode(codes);
}
