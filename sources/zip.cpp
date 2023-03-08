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
		std::memcpy(&textlength, encode_data + data_count, sizeof(textlength));
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
		std::memcpy(&chr, encode_data + data_count, sizeof(chr));
		std::cout << chr << std::endl;
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
				if (code->chr == '0')code->chr = 0;
				std::memcpy(decode_data + decode_count, &code->chr, sizeof(code->chr));
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

Hahuman::Node* Hahuman::HahumanEncode::SearchNode(Node* node, char chr)
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

Hahuman::Node* Hahuman::HahumanEncode::NewNode(char chr)
{
	Node* node = new Node;

	node->freq = 1;
	node->chr = chr;

	return node;
}

Hahuman::Node* Hahuman::HahumanEncode::MakeNodes(char* data,int d_size)
{
	//<リストの先頭>//
	Node* head{ nullptr };
	//<リストの最後尾>//
	Node* tail{ nullptr };
	Node* add{ nullptr };
	Node* search{ nullptr };

	for (int s = 0; s < d_size; s++)
	{
		char c = *data;

		search = SearchNode(head, c);

		//-----節がなかったら新しく節を追加-----//
		if (search == nullptr)
		{

			//数字が0(null)の時は文字の0を入れておく
			//ここで文字を入れている理由はヘッダー情報の終わりや親ノードで
			//0(null)を使用するからそれとは別で存在していないといけないから
			if (c == '\0') c = '0';
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
		data++;
	}
	return head;
}

void Hahuman::HahumanEncode::GetChild(Node** left, Node** right, Node* head)
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

}
Hahuman::Code* Hahuman::HahumanEncode::MakeHuffmanCode(Node* node)
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

	//<最終的な圧縮データサイズ>//
	int count{ 0 };

	text_length = d_size;
	//-----入力文字数をカウント-----//
	//for (auto t : data) text_length++;

	//-----ヘッダーを書き込む-----//
	header_size = 0;
	code = head;

	//<ヘッダー情報を書き込んだ回数>//
	int header_count{ 0 };

	//-----Code構造体のリストの先頭から終端まで文字の符号化情報をヘッダーとして書き出す-----//
	while (code != nullptr)
	{
		//<ヘッダーデータ>//
		HeaderData h_data;
		//<文字を書き込む>//
		h_data.chr = code->chr;

		//<符号化文字のビット数を書き込む>//
		h_data.bit = code->bit;

		//<この文字のエンコード結果を書き込む>//
		h_data.value = code->value;


		printf("chr = %c, bit = %d, value = %x\n", h_data.chr, h_data.bit, h_data.value);

		//<ヘッダーサイズを増やす>//
		header_size += sizeof(HeaderData);
		count = sizeof(HeaderData);

		//-----次の符号化情報書き出しに移る-----//
		code = code->next;

		std::memcpy(encode_data + (sizeof(HeaderData) * header_count), (char*)&h_data, sizeof(HeaderData));

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
	for (int s = 0; s < d_size; s++)
	{
		char t = *data;
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

				std::cout << byte << std::endl;

				//-----ファイルに書き出したので空きビットとbyteを0で初期化-----//
				empty_bits = 8;
				byte = 0;
				count++;
				byte_count++;
			}
		}
		data++;
	}

	//-----1バイト分つまらなかった分を最後にファイルへ書き出し-----//
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

	//作成したNodeのリストの先頭のポインタが帰ってくる
	nodes = MakeNodes(data, d_size);

	if (nodes == nullptr)
	{
		DebugConsole::Instance().WriteDebugConsole("ハフマン木の節データ作成失敗",TextColor::Red);
		return;
	}

	MakeHuffmanTree(nodes);

	codes = MakeHuffmanCode(nodes);

	if (codes == nullptr)
	{
		DebugConsole::Instance().WriteDebugConsole("ハフマン符号化データ作成失敗", TextColor::Red);
		return;
	}

	FreeNode(nodes);

	EncodeData(data, encode_data, d_size,codes);

	FreeCode(codes);
}
