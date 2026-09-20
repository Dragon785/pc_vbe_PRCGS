# pc_vbe_PRCGS
PC互換機(VBE対応)用のPRCGSローダー
(プロテクトモードで動作)

# 動作確認環境
DosBox-X (DOS-V Mode)
FreeDOS/V
内部で一度RGBバッファを生成しているため、画像サイズによってはある程度のメモリが必要になると思います。
PRCGSは一般的に320＊200なので、問題になることは少ないと思いますが･･

# 開発環境
Watcom C/C++ v2.0

# ビルド方法
wmakeでlookvbe.exeができます。
