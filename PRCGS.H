#if (!defined(__PRCGS_H__))
#define __PRCGS_H__

#include <stdint.h>
#include "PRCGSDEF.H"

/* 64kb超えると一括読み込みが難しくなるので内部でファイルから1バイトずつ読み取る */

// PRCGSデータをオープンしてヘッダを読み取る
// 0:正常に読み取れた 1:ヘッダがおかしい
extern int StartReadPRCGS(const char* filename);
extern PRCGSHeader Header; // 読みとったヘッダ情報
extern const unsigned int GetWidth(void); // 現在のヘッダ情報に基づく幅
extern const unsigned int GetHeight(void); // 現在のヘッダ情報に基づく高さ
/* 指定されたバッファに8bit/pixelで展開する.バッファの確保と解放は呼出し側で面倒見ること
  0:正常終了 1:異常終了
 */
extern int Extract(unsigned char* r,unsigned char* g,unsigned char* b);
extern void FinishReadPRCGS(void);
#endif
