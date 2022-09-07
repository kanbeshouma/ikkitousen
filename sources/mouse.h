#pragma once

#include <Windows.h>
#include <DirectXMath.h>

using MouseButton = unsigned int;

class Mouse
{
public:
    //----< Key >----//
    static const MouseButton BTN_LEFT_CLICK   = (1 << 0);
    static const MouseButton BTN_MIDDLE       = (1 << 1);
    static const MouseButton BTN_RIGHT_CLICK  = (1 << 2);
    static const MouseButton BTN_SPACE        = (1 << 3);
    static const MouseButton BTN_ESC          = (1 << 4);
    static const MouseButton BTN_I            = (1 << 5);
    static const MouseButton BTN_K            = (1 << 6);
    static const MouseButton BTN_M            = (1 << 7);
    static const MouseButton BTN_Z            = (1 << 8);
    static const MouseButton BTN_Q            = (1 << 9);
    static const MouseButton BTN_E            = (1 << 10);
    static const MouseButton BTN_F1           = (1 << 11);
    static const MouseButton BTN_F2           = (1 << 12);
    static const MouseButton BTN_F3           = (1 << 13);
    static const MouseButton BTN_F4           = (1 << 14);
    static const MouseButton BTN_F5           = (1 << 15);
    static const MouseButton BTN_F6           = (1 << 16);
    static const MouseButton BTN_F7           = (1 << 17);
    static const MouseButton BTN_F8           = (1 << 18);
    static const MouseButton BTN_F9           = (1 << 19);
    static const MouseButton BTN_F10          = (1 << 20);
    static const MouseButton BTN_F11          = (1 << 21);
    static const MouseButton BTN_F12          = (1 << 22);
    static const MouseButton BTN_W            = (1 << 23);
    static const MouseButton BTN_A            = (1 << 24);
    static const MouseButton BTN_S            = (1 << 25);
    static const MouseButton BTN_D            = (1 << 26);
    static const MouseButton BTN_UP           = (1 << 27);
    static const MouseButton BTN_LEFT         = (1 << 28);
    static const MouseButton BTN_DOWN         = (1 << 29);
    static const MouseButton BTN_RIGHT        = (1 << 30);
    //--複数キー--//
    static const MouseButton BTN_W_UP         = BTN_W | BTN_UP;
    static const MouseButton BTN_A_LEFT       = BTN_A | BTN_LEFT;
    static const MouseButton BTN_S_DOWN       = BTN_S | BTN_DOWN;
    static const MouseButton BTN_D_RIGHT      = BTN_D | BTN_RIGHT;
public:
    //--------< コンストラクタ/関数等 >--------//
    Mouse()
        : cursor_pos()
        , old_cursor_pos()
        , point()
        , state()
        , do_show(true)
        , n_notch()
        , z_delta()
        , is_wheel(false) {}
    ~Mouse() {}
public:
    //--------< 関数 >--------//
    // 更新
    /*呼び出しはFrameworkのみ*/
    void update(HWND hwnd);
    //カーソルを非表示にする
    void set_do_show(bool show) { do_show = show; }     //カーソルを非表示にする
    // ホイール情報をリセット
    void reset_wheel()
    {
        z_delta = 0;
        n_notch = 0;
        is_wheel = false;
    }
    // ホイールが動いたか(チルド)
    bool up_tilt() { return n_notch > 0; }
    bool down_tilt() { return n_notch < 0; }
    //--------<getter/setter>--------//
    // カーソル
    const DirectX::XMFLOAT2& get_cursor_pos() const { return cursor_pos; }
    const DirectX::XMFLOAT2& get_old_cursor_pos() const { return old_cursor_pos; }
    // ホイール
    void set_n_notch(int n_not) { n_notch = n_not; }
    const int get_z_delta() const { return z_delta; }
    void set_z_delta(int z_del) { z_delta = z_del; }
    void set_is_wheel(bool is_wh) { is_wheel = is_wh; }
    const bool get_is_wheel() { return is_wheel; }
    // ボタン入力状態の取得
    MouseButton common_button();
    MouseButton game_button();
    MouseButton debug_button();
    // ボタン押下状態の取得
    MouseButton common_button_down();
    MouseButton game_button_down();
    MouseButton debug_button_down();
    // ボタン押上状態の取得
    MouseButton common_button_up();
    MouseButton game_button_up();
    MouseButton debug_button_up();
    // 操作有効、無効
    void operation_disablement() { operable = false; }
    void operation_activation() { operable = true; }
private:
    //--------< 関数 >--------//
    void update_cursor(HWND hwnd);
    //--------< 変数 >--------//
    // カーソル
    DirectX::XMFLOAT2 cursor_pos;
    DirectX::XMFLOAT2 old_cursor_pos;
    POINT point;
    int state;
    bool do_show;
    // ホイール
    int n_notch;
    int z_delta;
    bool is_wheel;
    // キーボード
    MouseButton	button_state[2] = { 0 };
    MouseButton	button_down = 0;
    MouseButton	button_up = 0;

    bool operable = true;
};