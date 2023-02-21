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

	//<読み込んだデータのサイズ>//
	int data_count{};
	while (1) {
		/* まず文字情報を読み込む */
		std::memcpy(&chr, encode_data + data_count, sizeof(chr));
		data_count += sizeof(chr);

		/* 符号化データの終端を表す '\0'であった場合
		   ヘッダー読み込みを終了 */
		if (chr == '\0') break;

		/* CODE構造体を追加 */

		add = new Code;
		if (add == NULL) {
			printf("malloc error\n");
			return;
		}
		add->next = NULL;

		/* 文字情報に読み込んだ文字をセット */
		add->chr = chr;

		/* 符号化文字のビット数を読み込む */
		std::memcpy(&add->bit, encode_data + data_count, sizeof((add->bit)));
		data_count += sizeof(add->bit);


		/* この文字のエンコード結果を読み込む */
		std::memcpy(&add->value, encode_data + data_count, sizeof((add->value)));
		data_count += sizeof(add->value);


		headersize += sizeof(HeaderData);
		printf("chr = %c, bit = %d, value = %x\n", add->chr, add->bit, add->value);

		if (head == NULL) {
			/* CODE構造体が一つもない場合はリストの先頭とする */
			head = add;
			tail = head;
		}
		else {
			/* CODE構造体がすでにある場合はリストの最後尾に追加 */
			tail->next = add;
			tail = add;
		}
	}

	//<ヘッダーの '\0'にある元データの文字数を取得>//
	if (chr == '\0') {
		//fread(&textlength, sizeof(textlength), 1, fi);
		std::memcpy(&textlength, &encode_data + data_count, sizeof(textlength));
		data_count += sizeof(textlength);

	}
	printf("headrsize = %lu\n", headersize + 1 + sizeof(textlength));

	//<ビット列とビット数を0で初期化>//
	bits = 0;
	bitNum = 0;

	//<書き込んだバイト数>//
	int decode_count{};
	while (textlength > 0)
	{
		//<ファイルから１バイト分読み込み>//
		//chr = fgetc(fi);
		std::memcpy(&chr, &encode_data + data_count, sizeof(chr));
		data_count++;


		//<chrの最上位ビット7からビット0までループ>//
		for (i = 7; i >= 0; i--) {

			//<ビットiだけ立てたビット列を作成>//
			mask = 1 << i;

			//<論理積とシフト演算によりchrのビットiの値を取得>//
			oneBit = (chr & mask) >> i;

			//<ビット列を1ビット左シフトして右側に1ビット空きを作る>//
			bits = bits << 1;

			//<ビット列の右端ビットにビットiの値を詰め合わせる>//
			bits += oneBit;

			//<上記でビット数が増えたのでbitNumを増加>//
			bitNum++;

			//<bitsをvalueとし、bitNumをbitとするCODE構造体を探索>//
			code = SearchCode(head, bits, bitNum);

			//<そのようなCODE構造体がある場合は、そのCODE構造体のchrをファイルへ書き出し>//
			if (code != NULL)
			{
				std::memcpy(&decode_data + decode_count, &code->chr, sizeof(code->chr));
				decode_count += sizeof(code->chr);


				//<書き出したので符号ビット列bitsを0に、ビット数bitNumを0にセット>//
				bits = 0;
				bitNum = 0;

				//<１文字書き込んだので残り文字数を1減らす>//
				textlength--;

				//<残り文字数が0になったら終了>//
				if (textlength <= 0) {
					break;
				}
			}
		}
	}

	//<CODE構造体を削除>//
	FreeCode(head);

}
