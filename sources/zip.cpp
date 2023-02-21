#include"zip.h"

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
		std::memcpy(&textlength, &encode_data + data_count, sizeof(textlength));
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
		std::memcpy(&chr, &encode_data + data_count, sizeof(chr));
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
				std::memcpy(&decode_data + decode_count, &code->chr, sizeof(code->chr));
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
