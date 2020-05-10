# The structure of ELF file
- ELF file is start from 0x7f, 0x45(E), 0x4c(L), 0x46(F)  
- A section is an unit of a linker
- Sections are described in section header table
- A segment is an unit of a loader
- Segments are described in program header table
- You can see adresses of these headers using `readelf -h`

- The result of `readelf -h main.out`
```
ELF ヘッダ:
  マジック:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  クラス:                            ELF64
  データ:                            2 の補数、リトルエンディアン
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI バージョン:                    0
  型:                                EXEC (実行可能ファイル)
  マシン:                            Advanced Micro Devices X86-64
  バージョン:                        0x1
  エントリポイントアドレス:               0x40106b
  プログラムヘッダ始点:          64 (バイト)
  セクションヘッダ始点:          8920 (バイト)
  フラグ:                            0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         4
  Size of section headers:           64 (bytes)
  Number of section headers:         7
  Section header string table index: 6
```

- Names of sections are stored in `.shstrtab` section. For example, `readelf -x 6 main.out`
```
セクション '.shstrtab' の 十六進数ダンプ:
  0x00000000 002e7379 6d746162 002e7374 72746162 ..symtab..strtab
  0x00000010 002e7368 73747274 6162002e 74657874 ..shstrtab..text
  0x00000020 002e6568 5f667261 6d65002e 636f6d6d ..eh_frame..comm
  0x00000030 656e7400                            ent.

```

- You can check the contents of the program header using `readelf -l main.out`
```
Elf ファイルタイプは EXEC (実行可能ファイル) です
エントリポイント 0x40106b
There are 4 program headers, starting at offset 64

プログラムヘッダ:
  タイプ        オフセット          仮想Addr           物理Addr
                 ファイルサイズ        メモリサイズ         フラグ 整列
  LOAD           0x0000000000000000 0x0000000000400000 0x0000000000400000
                 0x0000000000000120 0x0000000000000120  R      0x1000
  LOAD           0x0000000000001000 0x0000000000401000 0x0000000000401000
                 0x0000000000000118 0x0000000000000118  R E    0x1000
  LOAD           0x0000000000002000 0x0000000000402000 0x0000000000402000
                 0x00000000000000b8 0x00000000000000b8  R      0x1000
  GNU_STACK      0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x0000000000000000 0x0000000000000000  RWE    0x10

 セグメントマッピングへのセクション:
  セグメントセクション...
   00     
   01     .text 
   02     .eh_frame 
   03     
```

