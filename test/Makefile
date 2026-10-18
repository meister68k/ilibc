# test/Makefile — ilibc 試験のビルド・実行経路(本リポジトリの正規のテストビルド経路)。
#
# 根拠: spec/ilibc_test_spec.md
#   §4 実行順序とゲート(段階2=ヘッダ試験はハードゲート、段階3〜6は失敗を記録して続行)
#   §5 試験基盤(§5.3 コンパイル時、§5.4 コンパイル失敗、§5.5 成果物・リンク、
#       §5.6 実行時ハーネス・oneshot、§5.7 ホスト差分・サニタイザ)、§7.2 カバレッジ、§11.3 ログ先頭の版情報
#   spec/ilibc_cctype_spec.md §8 (TC-CT-01〜11)
#   spec/ilibc_cstring_spec.md §9 (TC-ST-01〜17)、§3 (mem* のビルドフラグ)
#   spec/ilibc_cstdlib_spec.md §8 (TC-SL-01〜24)、spec/ilibc_header_spec.md §5 (cerrno)
#   spec/ilibc_cerrno_spec.md §9、spec/ilibc_header_spec.md §10 (TC-HD-09/10、errno)
#   spec/ilibc_header_spec.md §7 (スタブ: TC-HD-01/11/12/13、TC-GN-03 統合)
#   spec/ilibc_cassert_spec.md §8 (TC-HD-01/02/03/06/07/08/16/17、assert・既定フック)
#   spec/ilibc_cstdio_spec.md §11 (TC-IO-01・02・04〜07・15・17: <ilibc/format.hpp> 書式化)
#
# 使い方(リポジトリルートから):
#   make -C test                 # check = 段階0〜7(既定ターゲット)
#   make -C test headers         # 個別ターゲット: headers / constexpr / compilefail /
#   make -C test coverage        #   host / sanitize / runtime / formatio / oneshot / artifact /
#   make -C test help            #   sizes(任意)、coverage(任意)、clean
#
# 前提: 32ビットマルチリブの sysroot(既定 /tmp/opencode/sysroot)。
#   欠ける場合は check-toolchain が作成手順を表示して停止する。
#
# 範囲外(将来の追加): E2〜E4 のクロス実行、TC-TL 本体(§8.2)、カバレッジの
#   <cstring>追加分岐目標、変異試験(D-T4 第2段階)、コンパイル失敗試験の
#   ci/run_compilefail.py への分割(§5.4)。

SHELL := /bin/bash
.ONESHELL:
.SHELLFLAGS := -eu -o pipefail -c
.DEFAULT_GOAL := check

ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)
TST  := $(ROOT)/test
BUILD ?= $(TST)/build
SYSROOT ?= /tmp/opencode/sysroot

CC  = gcc
CXX = g++
CXXMAJOR ?= $(shell $(CXX) -dumpversion 2>/dev/null | cut -d. -f1)

# E1(x86 Linux、-m32 ILP32)の検証済みフラグ(TST §3.2/§3.3、概要仕様書 §2.3)
ISYS := -isystem /usr/include/x86_64-linux-gnu \
        -isystem $(SYSROOT)/usr/include/x86_64-linux-gnu \
        -isystem $(SYSROOT)/usr/include/x86_64-linux-gnu/c++/$(CXXMAJOR)/32
LD32 := -B $(SYSROOT)/usr/lib32/ -B $(SYSROOT)/usr/lib/gcc/x86_64-linux-gnu/$(CXXMAJOR)/32/
WARN := -pedantic -Wall -Wextra -Wconversion -Werror
FREE := -ffreestanding -fno-builtin -fno-exceptions -fno-rtti -fno-threadsafe-statics
IINC := -isystem $(ROOT)/include     # 試験コードからは -isystem(仕様どおりの検索順)
INC  := -I$(ROOT)/include            # ヘッダ自身の警告検査には -I(警告を検査対象にする)
# TC-CT-09 / TC-ST-15 の最適化レベル(TST §3.3、STR-001 §9)
OPTS ?= -O0 -Os -O2 -O3

# ILIBC-STR-001 の mem* 実装。ループが同名関数への呼び出しに置換されるのを
# 防ぐ(cstring仕様 §3: GCC は -fno-tree-loop-distribute-patterns)。
SRC_STR := $(ROOT)/src/string/memcpy.cpp $(ROOT)/src/string/memmove.cpp \
           $(ROOT)/src/string/memcmp.cpp $(ROOT)/src/string/memchr.cpp \
           $(ROOT)/src/string/memset.cpp
NDIST := -fno-tree-loop-distribute-patterns

# コンパイル失敗試験の全ファイル(TST §5.4: 1ファイルずつ対照/失敗/診断を検査)
CF_ALL := $(sort $(wildcard $(TST)/compilefail/cf_*.cpp))

# oneshot の各ケース(TC-ST-17、TC-HD-06/07。TST §5.6.4: 1ケース1実行ファイル)
OS_CASES := st17_strlen_null st17_memcpy_overlap st17_memcpy_ok \
            hd07_reenable hd07_ndebug_off hd07_ilibase_independent

.PHONY: help all check env-info check-toolchain headers constexpr compilefail \
        host sanitize runtime formatio oneshot artifact sizes coverage clean

help:
	@echo "test/Makefile のターゲット:"
	@echo "  check (既定)  段階0〜7のゲート付き一括実行"
	@echo "  headers       ヘッダ・ビルド試験(段階2): 単独コンパイル、順序非依存、"
	@echo "                NDEBUG再インクルード、ILP32検証、ASCII(H7)、マクロ漏れ(H5)"
	@echo "  constexpr     TC-CT-01〜06 / TC-ST-01〜03・05 / TC-SL-01〜04・08〜10・18〜20"
	@echo "                / TC-HD-06(c)(d) (C++20/C++23、cassert は NDEBUG あり・なし)"
	@echo "  compilefail   TC-CT-10 / TC-ST-04・16(a) / TC-SL-02・05・16・21 / TC-HD-06(d)"
	@echo "                / TC-IO-03・07・12・15・16・18 (対照成功 / 失敗+診断正規表現)"
	@echo "  host          TC-CT-07 / TC-ST-06〜09 / TC-SL-12・23・24 ホストlibc差分 -O0〜-O3"
	@echo "  sanitize      TC-ST-10 / TC-SL-13 ASan/UBSan 付ホスト差分(-O1)"
	@echo "  runtime       TC-CT-08/09 / TC-ST-11 / TC-SL-14・15 E1実行 -O0〜-O3"
	@echo "  formatio      TC-IO-01・02・04〜13・15・17 <ilibc/format.hpp>・<cstdio>"
	@echo "                単独コンパイル・リンク・E1実行・nm依存検査(C++20/23、-O0〜-O3)"
	@echo "  oneshot       TC-ST-17 / TC-HD-06(a)(b)・TC-HD-07 assert・ILIBC_ASSERT"
	@echo "                (1ケース1実行、終了42)"
	@echo "  artifact      TC-CT-11 / TC-ST-12・13 / TC-SL-17 / TC-HD-16・17"
	@echo "                成果物・リンク検査(ilibc.a・ilibc_hooks.a)"
	@echo "  sizes         TC-ST-14 基準値の記録(サイズ・スタック。情報)"
	@echo "  coverage      gcov計測(任意。check には含めない)"
	@echo "  clean         $(BUILD) の削除"
	@echo "  env-info      ツールチェーンの版情報"

all: check

# TST §11.3: ログの先頭にコンパイラ等の版を出力する
env-info:
	@echo "=== ビルド環境 (TST §11.3) ==="
	@echo "CXX        : $$($(CXX) --version | sed -n 1p)"
	@echo "CC         : $$($(CC) --version | sed -n 1p)"
	@echo "CXX dumpver: $$($(CXX) -dumpversion)  (sysroot パスに使用)"
	@echo "SYSROOT    : $(SYSROOT)"
	@echo "arch       : $$(uname -m) (E1 は -m32 で ILP32 にビルド)"
	@echo "gcov       : $$(gcov --version 2>/dev/null | sed -n 1p || echo '(なし)')"

# 段階0相当: sysroot と -m32 ツールチェーンの存在確認(TST §4、§3.2)
check-toolchain:
	@mkdir -p $(BUILD)
	@if [ ! -d "$(SYSROOT)/usr/include/x86_64-linux-gnu" ] || [ ! -d "$(SYSROOT)/usr/lib/gcc/x86_64-linux-gnu/$(CXXMAJOR)/32" ]; then
	  echo "ERROR: 32ビット sysroot が見つからない: $(SYSROOT)"
	  echo "  作成手順(root 不要、Debian/Ubuntu):"
	  echo "    mkdir -p $(SYSROOT) && cd $(SYSROOT)"
	  echo "    apt-get download lib32stdc++-14-dev libc6-dev-i386 lib32gcc-14-dev"
	  echo '    for d in *.deb; do dpkg-deb -x "$$d" "$(SYSROOT)"; done'
	  echo "  または SYSROOT=<path> で指定する"
	  exit 1
	fi
	printf '#include <climits>\nint main(){return 0;}\n' > $(BUILD)/toolchain_probe.cpp
	$(CXX) -m32 $(ISYS) -fsyntax-only $(BUILD)/toolchain_probe.cpp \
	  || { echo "ERROR: -m32 のC++コンパイルに失敗(TST §3.2 E1)"; exit 1; }
	printf 'int main(void){return 0;}\n' > $(BUILD)/toolchain_probe.c
	$(CC) -m32 -static $(LD32) $(BUILD)/toolchain_probe.c -o $(BUILD)/toolchain_probe \
	  || { echo "ERROR: -m32 静的リンクに失敗(LD32=$(LD32))"; exit 1; }
	echo "toolchain OK (E1 -m32)"

# 段階2: ヘッダ・ビルド試験(TST §4 段階2、H2/H3/H7/H8、TST D-T1 の ILP32 検証)
headers: check-toolchain
	@echo "=== 段階2: ヘッダ・ビルド試験 ==="
	pass=0
	fail=0
	ok() { echo "  PASS: $$1"; pass=$$((pass+1)); }
	ng() { echo "  FAIL: $$1"; fail=$$((fail+1)); }
	# <cctype> 単独コンパイル(-I で自ヘッダの警告も検査)。C++20/23 両方(TST §3.3)
	printf '#include <cctype>\nint main(){return ILIBC_NS::isalpha(65)?0:1;}\n' > $(BUILD)/h_cctype.cpp
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_cctype.cpp 2> $(BUILD)/h_cctype.err; then
	    ok "<cctype> 単独コンパイル, $$std"
	  else
	    ng "<cctype> 単独コンパイル, $$std"
	    sed -n 1,20p $(BUILD)/h_cctype.err
	  fi
	done
	# <ctype.h> で ::isalpha と std::isalpha の両方(TC-CT-10(b) のビルド部分)
	printf '#include <ctype.h>\nint main(){return (::isalpha(65)&&std::isalpha(65))?0:1;}\n' > $(BUILD)/h_ctype.cpp
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_ctype.cpp 2> $(BUILD)/h_ctype.err; then
	    ok "<ctype.h> + 両スペル, $$std"
	  else
	    ng "<ctype.h> + 両スペル, $$std"
	    sed -n 1,20p $(BUILD)/h_ctype.err
	  fi
	done
	# <cstring> 単独コンパイル(TC-ST-16 のビルド部分、-I で自ヘッダの警告も検査)
	printf '#include <cstring>\nint main(){return ILIBC_NS::strlen("a")==1?0:1;}\n' > $(BUILD)/h_cstring.cpp
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_cstring.cpp 2> $(BUILD)/h_cstring.err; then
	    ok "<cstring> 単独コンパイル, $$std"
	  else
	    ng "<cstring> 単独コンパイル, $$std"
	    sed -n 1,20p $(BUILD)/h_cstring.err
	  fi
	done
	# <string.h> で ::strlen と std::strlen の両方(TC-ST-16(b))
	printf '#include <string.h>\nint main(){return (::strlen("a")==1&&std::strlen("a")==1)?0:1;}\n' > $(BUILD)/h_string.cpp
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_string.cpp 2> $(BUILD)/h_string.err; then
	    ok "<string.h> + 両スペル, $$std"
	  else
	    ng "<string.h> + 両スペル, $$std"
	    sed -n 1,20p $(BUILD)/h_string.err
	  fi
	done
	# <ilibc/format.hpp> 単独コンパイル(TC-IO-01: I/O非依存、-I で自ヘッダの
	# 警告も検査)。-H の内訳で io_backend/unistd を取り込まないことも併せて検査する
	printf '#include <ilibc/format.hpp>\nchar b[16];\nint main(){return ilibc::snprint_f<"a%%d">(b,16,1)==2?0:1;}\n' > $(BUILD)/h_format.cpp
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -H -fsyntax-only $(BUILD)/h_format.cpp 2> $(BUILD)/h_format.err; then
	    if grep -qE 'io_backend|unistd\.h' $(BUILD)/h_format.err; then
	      ng "<ilibc/format.hpp> が I/O ヘッダを取り込む(TC-IO-01), $$std"
	      grep -E 'io_backend|unistd\.h' $(BUILD)/h_format.err | sed -n 1,5p
	    else
	      ok "<ilibc/format.hpp> 単独コンパイル + I/O非依存(H2・TC-IO-01), $$std"
	    fi
	  else
	    ng "<ilibc/format.hpp> 単独コンパイル, $$std (TC-IO-01)"
	    sed -n 1,20p $(BUILD)/h_format.err
	  fi
	done
	# <cstdio> と内部ヘッダ ilibc/io_backend.hpp・ilibc/io_sink.hpp の単独
	# コンパイル(ヘッダ仕様 §2.3、ILIBC-IO-001 §5.2/§10、-I で自ヘッダの警告も
	# 検査)。バックエンド未選択ではエラー+ilibc: メッセージが必須、POSIX 指定
	# では成功が必須。内部ヘッダは POSIX 指定で単独かつ自己完結(H2)であること
	printf '#include <ilibc/io_backend.hpp>\nint main(){return 0;}\n' > $(BUILD)/h_iobackend.cpp
	printf '#include <ilibc/io_sink.hpp>\nint main(){return 0;}\n' > $(BUILD)/h_iosink.cpp
	printf '#include <cstdio>\nint main(){return 0;}\n' > $(BUILD)/h_cstdio_none.cpp
	printf '#define ILIBC_IO_BACKEND_POSIX\n#include <cstdio>\nint main(){return ilibc::putstr("")?1:0;}\n' > $(BUILD)/h_cstdio_posix.cpp
	for std in c++20 c++23; do
	  # 未選択: エラー+ilibc: メッセージ(条件付き公開、TC-HD-01 相当)
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_cstdio_none.cpp 2> $(BUILD)/h_cstdio_none.err; then
	    ng "バックエンド未選択の <cstdio> がエラーにならない(TC-IO-15), $$std"
	  elif ! grep -q 'ilibc: <cstdio> is not provided' $(BUILD)/h_cstdio_none.err; then
	    ng "バックエンド未選択の <cstdio> のメッセージが仕様と異なる(TC-IO-15), $$std"
	    sed -n 1,10p $(BUILD)/h_cstdio_none.err
	  else
	    ok "バックエンド未選択の <cstdio> はエラー+ilibc: メッセージ(条件付き公開), $$std"
	  fi
	  # POSIX 指定: 成功
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_cstdio_posix.cpp 2> $(BUILD)/h_cstdio_posix.err; then
	    ok "POSIX 指定で <cstdio> 単独コンパイル(条件付き公開), $$std"
	  else
	    ng "POSIX 指定で <cstdio> 単独コンパイルが失敗(条件付き公開), $$std"
	    sed -n 1,20p $(BUILD)/h_cstdio_posix.err
	  fi
	  # 内部ヘッダ2個: POSIX 指定で単独コンパイルが通ること(自己完結 H2)
	  for hb in h_iobackend h_iosink; do
	    if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) -DILIBC_IO_BACKEND_POSIX $(INC) -fsyntax-only $(BUILD)/$$hb.cpp 2> $(BUILD)/$$hb.err; then
	      ok "内部ヘッダ $$(grep -o 'ilibc/[a-z_]*\.hpp' $(BUILD)/$$hb.cpp) 単独コンパイル(H2), $$std"
	    else
	      ng "内部ヘッダ単独コンパイル失敗(H2), $$hb, $$std"
	      sed -n 1,20p $(BUILD)/$$hb.err
	    fi
	  done
	done
	# <cerrno> 単独コンパイル(ヘッダ仕様 §5、-I で自ヘッダの警告も検査)。
	# 公開マクロ EDOM/ERANGE/EINVAL と ILIBC_ERRNO が使えること、ILP32 検証。
	# #if での値・正・相異の判定と、errno の左辺値操作(代入・++・&取得)も併せて
	# 検査する(ヘッダ仕様 §10 TC-HD-09(a)(b)、cerrno仕様 §9.1 の最小例)。
	printf '#include <cerrno>\n#if EDOM != 33 || ERANGE != 34 || EINVAL != 22\n#error cerrno values mismatch in #if\n#endif\n#if !(EDOM > 0 && ERANGE > 0 && EINVAL > 0)\n#error cerrno codes must be positive\n#endif\n#if EDOM == ERANGE || EDOM == EINVAL || ERANGE == EINVAL\n#error cerrno codes must be distinct\n#endif\nstatic_assert(EDOM==33 && ERANGE==34 && EINVAL==22);\nint main(){errno=0;++errno;int* p=&errno;return (ILIBC_ERRNO==1 && *p==1)?0:1;}\n' > $(BUILD)/h_cerrno.cpp
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_cerrno.cpp 2> $(BUILD)/h_cerrno.err; then
	    ok "<cerrno> 単独コンパイル + マクロ + #if・左辺値(TC-HD-09(a)(b)), $$std"
	  else
	    ng "<cerrno> 単独コンパイル + マクロ + #if・左辺値(TC-HD-09(a)(b)), $$std"
	    sed -n 1,20p $(BUILD)/h_cerrno.err
	  fi
	done
	# <errno.h> で EDOM/ERANGE/EINVAL/errno が使えること(ラッパ、ヘッダ仕様 §5)。
	# ラッパ経由でも #if 判定と左辺値操作が成立すること(TC-HD-05、TC-HD-09(a)(b))
	printf '#include <errno.h>\n#if EDOM != 33 || ERANGE != 34 || EINVAL != 22\n#error errno.h values mismatch in #if\n#endif\nint main(){errno=0;++errno;int* p=&errno;return (*p==1 && errno==1)?0:1;}\n' > $(BUILD)/h_errno.cpp
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_errno.cpp 2> $(BUILD)/h_errno.err; then
	    ok "<errno.h> 単独コンパイル + マクロ + #if・左辺値, $$std"
	  else
	    ng "<errno.h> 単独コンパイル + マクロ + #if・左辺値, $$std"
	    sed -n 1,20p $(BUILD)/h_errno.err
	  fi
	done
	# <errno.h>/<cerrno> の順序非依存・重複(H3、TC-HD-02)。両順序+重複の
	# コンパイルと、-dM -E のマクロ集合が単独 <cerrno> と一致することを検査する。
	printf '#include <errno.h>\n#include <cerrno>\n#include <errno.h>\nint main(){errno=0;return EDOM;}\n' > $(BUILD)/h_order_errno1.cpp
	printf '#include <cerrno>\n#include <errno.h>\n#include <cerrno>\nint main(){errno=0;return EDOM;}\n' > $(BUILD)/h_order_errno2.cpp
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_order_errno1.cpp 2> $(BUILD)/h_order_errno1.err \
	     && $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_order_errno2.cpp 2> $(BUILD)/h_order_errno2.err; then
	    ok "<errno.h>/<cerrno> 重複・順序非依存, $$std"
	  else
	    ng "<errno.h>/<cerrno> 重複・順序非依存, $$std"
	    sed -n 1,20p $(BUILD)/h_order_errno1.err $(BUILD)/h_order_errno2.err
	  fi
	done
	printf '#include <cerrno>\n' | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - | sort > $(BUILD)/mac_errno_base.txt
	printf '#include <errno.h>\n#include <cerrno>\n#include <errno.h>\n' | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - | sort > $(BUILD)/mac_errno_order1.txt
	printf '#include <cerrno>\n#include <errno.h>\n#include <cerrno>\n' | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - | sort > $(BUILD)/mac_errno_order2.txt
	if diff -q $(BUILD)/mac_errno_base.txt $(BUILD)/mac_errno_order1.txt > /dev/null \
	   && diff -q $(BUILD)/mac_errno_base.txt $(BUILD)/mac_errno_order2.txt > /dev/null; then
	  ok "<errno.h>/<cerrno> マクロ集合が単独 <cerrno> と一致(TC-HD-02)"
	else
	  ng "<errno.h>/<cerrno> マクロ集合が単独 <cerrno> と一致しない(TC-HD-02)"
	  diff $(BUILD)/mac_errno_base.txt $(BUILD)/mac_errno_order1.txt | sed -n 1,20p
	  diff $(BUILD)/mac_errno_base.txt $(BUILD)/mac_errno_order2.txt | sed -n 1,20p
	fi
	# <cstdlib> 単独コンパイル(TC-SL-16 のビルド部分)。定数マクロの値検証も含む(TC-SL-16(c))
	printf '#include <cstdlib>\nstatic_assert(EXIT_SUCCESS==0 && EXIT_FAILURE==1 && MB_CUR_MAX==1);\nint main(){return (ILIBC_NS::abs(-1)==1 && std::atoi("42")==42 && std::strtol("10",nullptr,10)==10L)?0:1;}\n' > $(BUILD)/h_cstdlib.cpp
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_cstdlib.cpp 2> $(BUILD)/h_cstdlib.err; then
	    ok "<cstdlib> 単独コンパイル + 定数マクロ, $$std"
	  else
	    ng "<cstdlib> 単独コンパイル + 定数マクロ, $$std"
	    sed -n 1,20p $(BUILD)/h_cstdlib.err
	  fi
	done
	# <stdlib.h> で ::atoi と std::atoi の両方(TC-SL-16(b) のビルド部分)
	printf '#include <stdlib.h>\nint main(){return (::atoi("1")==1 && std::atoi("1")==1)?0:1;}\n' > $(BUILD)/h_stdlib.cpp
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_stdlib.cpp 2> $(BUILD)/h_stdlib.err; then
	    ok "<stdlib.h> + 両スペル, $$std"
	  else
	    ng "<stdlib.h> + 両スペル, $$std"
	    sed -n 1,20p $(BUILD)/h_stdlib.err
	  fi
	done
	# <cassert> 単独コンパイル(TC-HD-01、-I で自ヘッダの警告も検査)。
	# 真の assert が定数式で使えること(TC-HD-06(d)の対照側)も含む
	printf '#include <cassert>\nconstexpr int f(int x){assert(x>=0);return x;}\nstatic_assert(f(1)==1);\nint main(){assert(1==1);return 0;}\n' > $(BUILD)/h_cassert.cpp
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_cassert.cpp 2> $(BUILD)/h_cassert.err; then
	    ok "<cassert> 単独コンパイル + 定数式の真, $$std"
	  else
	    ng "<cassert> 単独コンパイル, $$std"
	    sed -n 1,20p $(BUILD)/h_cassert.err
	  fi
	done
	# <assert.h> 単独コンパイル(互換ラッパ、TC-HD-01)
	printf '#include <assert.h>\nint main(){assert(1==1);return 0;}\n' > $(BUILD)/h_asserth.cpp
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_asserth.cpp 2> $(BUILD)/h_asserth.err; then
	    ok "<assert.h> 単独コンパイル, $$std"
	  else
	    ng "<assert.h> 単独コンパイル, $$std"
	    sed -n 1,20p $(BUILD)/h_asserth.err
	  fi
	done
	# NDEBUG の再インクルード(AST §3.3、TC-HD-06(a))。-dM -E で、同一翻訳単位に
	# 未定義→定義→未定義の順に取り込み、各時点の assert の定義を検査する
	printf '#include <cassert>\n' | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - > $(BUILD)/mac_ast_on.txt
	if grep -qE '^#define assert\(\.\.\..*__ilibc_assert_fail' $(BUILD)/mac_ast_on.txt; then
	  ok "<cassert> NDEBUG 未定義: assert がフックを呼ぶ定義(H5/TC-HD-06(a))"
	else
	  ng "<cassert> NDEBUG 未定義: assert の定義が期待と異なる"
	  grep 'assert' $(BUILD)/mac_ast_on.txt
	fi
	printf '#include <cassert>\n#define NDEBUG\n#include <cassert>\n' \
	  | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - > $(BUILD)/mac_ast_off.txt
	if grep -qF '#define assert(...) (static_cast<void>(0))' $(BUILD)/mac_ast_off.txt; then
	  ok "<cassert> NDEBUG 定義後の再インクルードで無効化(TC-HD-06(a))"
	else
	  ng "<cassert> NDEBUG 定義後の再インクルードが無効化されていない"
	  grep 'assert' $(BUILD)/mac_ast_off.txt
	fi
	printf '#include <cassert>\n#define NDEBUG\n#include <cassert>\n#undef NDEBUG\n#include <cassert>\n' \
	  | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - > $(BUILD)/mac_ast_reon.txt
	if grep -qE '^#define assert\(\.\.\..*__ilibc_assert_fail' $(BUILD)/mac_ast_reon.txt; then
	  ok "<cassert> #undef NDEBUG 後の再インクルードで再有効化(TC-HD-06(a))"
	else
	  ng "<cassert> #undef NDEBUG 後の再インクルードが再有効化されていない"
	  grep 'assert' $(BUILD)/mac_ast_reon.txt
	fi
	# インクルード順序非依存・重複(H3、TC-HD-02)。<assert.h> と <cassert> を
	# 重複・入れ違いで取り込んでも整合すること
	printf '#include <assert.h>\n#include <cassert>\n#include <assert.h>\nint main(){assert(1==1);return 0;}\n' > $(BUILD)/h_order_assert.cpp
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_order_assert.cpp 2> $(BUILD)/h_order_assert.err; then
	    ok "<assert.h>/<cassert> 重複・順序非依存, $$std"
	  else
	    ng "<assert.h>/<cassert> 重複・順序非依存, $$std"
	    sed -n 1,20p $(BUILD)/h_order_assert.err
	  fi
	done
	# インクルード順序非依存・重複インクルード(H3)
	printf '#include <ctype.h>\n#include <cctype>\n#include <ctype.h>\nint main(){return (std::isalpha(65)&&::isalpha(65))?0:1;}\n' > $(BUILD)/h_order.cpp
	if $(CXX) -m32 -std=c++23 $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_order.cpp 2> $(BUILD)/h_order.err; then
	  ok "インクルード順序非依存(H3)"
	else
	  ng "インクルード順序非依存(H3)"
	  sed -n 1,20p $(BUILD)/h_order.err
	fi
	# 単独コンパイル(H2)
	printf '#include <ilibc/config.hpp>\nint main(){return ILIBC_VERSION==100?0:1;}\n' > $(BUILD)/h_config.cpp
	if $(CXX) -m32 -std=c++23 $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_config.cpp 2> $(BUILD)/h_config.err; then
	  ok "ilibc/config.hpp 単独コンパイル"
	else
	  ng "ilibc/config.hpp 単独コンパイル"
	  sed -n 1,20p $(BUILD)/h_config.err
	fi
	printf '#include <ilibc/assert_fail.hpp>\nint main(){return 0;}\n' > $(BUILD)/h_af.cpp
	if $(CXX) -m32 -std=c++23 $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_af.cpp 2> $(BUILD)/h_af.err; then
	  ok "ilibc/assert_fail.hpp 単独コンパイル"
	else
	  ng "ilibc/assert_fail.hpp 単独コンパイル"
	  sed -n 1,20p $(BUILD)/h_af.err
	fi
	# ILIBC_DEBUG で ILIBC_ASSERT が式に展開されること(ヘッダ仕様 §4.4)
	printf '#include <cctype>\nint main(){ILIBC_ASSERT(ILIBC_NS::isalpha(65));return 0;}\n' > $(BUILD)/h_debug.cpp
	if $(CXX) -m32 -std=c++23 $(WARN) $(FREE) -DILIBC_DEBUG $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_debug.cpp 2> $(BUILD)/h_debug.err; then
	  ok "-DILIBC_DEBUG で ILIBC_ASSERT が展開(§4.4)"
	else
	  ng "-DILIBC_DEBUG で ILIBC_ASSERT が展開しない"
	  sed -n 1,20p $(BUILD)/h_debug.err
	fi
	# <cstring> の ILIBC_ASSERT(cstring仕様 §4.7: 文字列系のヘッダ内アサーション)
	printf '#include <cstring>\nint main(){ILIBC_ASSERT(ILIBC_NS::strlen("a")==1);return 0;}\n' > $(BUILD)/h_debug_str.cpp
	if $(CXX) -m32 -std=c++23 $(WARN) $(FREE) -DILIBC_DEBUG $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_debug_str.cpp 2> $(BUILD)/h_debug_str.err; then
	  ok "-DILIBC_DEBUG で <cstring> の ILIBC_ASSERT が展開(§4.7)"
	else
	  ng "-DILIBC_DEBUG で <cstring> の ILIBC_ASSERT が展開しない"
	  sed -n 1,20p $(BUILD)/h_debug_str.err
	fi
	# 64ビットでは ILP32 の static_assert で失敗しなければならない(TST D-T1、TC-GN-10 相当)
	if $(CXX) -std=c++23 $(INC) -fsyntax-only $(BUILD)/h_cctype.cpp > $(BUILD)/h64.out 2>&1; then
	  ng "64ビットビルドが成功してしまい不行(ILP32 必須)"
	else
	  if grep -q "ilibc: ILP32 is required" $(BUILD)/h64.out; then
	    ok "64ビットビルドは ILP32 エラーで失敗(D-T1)"
	  else
	    ng "64ビットビルドが ILP32 メッセージなしで失敗"
	    sed -n 1,20p $(BUILD)/h64.out
	  fi
	fi
	# ------------------------- スタブ(ヘッダ仕様 §7: TC-HD-01/12/13、TC-GN-03)
	# (TC-HD-01) スタブ11個は素通りなしで単独コンパイルがエラーになり、
	# メッセージに ilibc: が含まれる(ヘッダ仕様 §7.3 S1、C++20/23 両方)。
	# 条件付き公開の <cstdio> はバックエンド選択の要否で結果が分かれるため、
	# 次のブロックで別に検査する(ILIBC-IO-001 §5)
	for h in cmath ctime csignal clocale csetjmp \
	         stdio.h math.h time.h signal.h locale.h setjmp.h; do
	  printf '#include <%s>\nint main(){return 0;}\n' "$$h" > $(BUILD)/h_stub.cpp
	  stub_ok=1
	  for std in c++20 c++23; do
	    if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/h_stub.cpp 2> $(BUILD)/h_stub.err; then
	      ng "スタブ <$$h> がエラーにならない(TC-HD-01), $$std"
	      stub_ok=0
	    elif ! grep -q 'ilibc:' $(BUILD)/h_stub.err; then
	      ng "スタブ <$$h> のメッセージに ilibc: がない(TC-HD-01), $$std"
	      sed -n 1,10p $(BUILD)/h_stub.err
	      stub_ok=0
	    fi
	  done
	  if [ $$stub_ok -eq 1 ]; then
	    ok "スタブ <$$h> 単独でエラー+ilibc: メッセージ(TC-HD-01)"
	  fi
	done
	# (TC-HD-12) ILIBC_STUB_PASSTHROUGH でスタブ11個+条件付き <cstdio> の計12個が
	# 一斉に素通りし、コンパイラ付属の同名ヘッダの宣言が使えること(§7.3 S3)。
	# include_next は -isystem のヘッダでは警告が抑止される(§7.3 S5)ため素通り
	# 検査は -isystem で行う。ホスト付属ヘッダが freestanding 非対応(<cmath> 等)
	# なため、この検査だけは通常モード(非 freestanding)でコンパイルする
	printf '#define ILIBC_STUB_PASSTHROUGH\n#include <cstdio>\n#include <cmath>\n#include <ctime>\n#include <csignal>\n#include <clocale>\n#include <csetjmp>\n#include <stdio.h>\n#include <math.h>\n#include <time.h>\n#include <signal.h>\n#include <locale.h>\n#include <setjmp.h>\nint main() {\n    std::printf("stub passthrough\\n");\n    double d = std::sqrt(4.0);\n    time_t t = std::time(nullptr);\n    void (*prev)(int) = std::signal(SIGINT, SIG_DFL);\n    char *lp = std::setlocale(LC_ALL, "");\n    jmp_buf jb;\n    (void)d; (void)t; (void)prev; (void)lp; (void)jb;\n    return 0;\n}\n' > $(BUILD)/h_passthru.cpp
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(ISYS) -isystem $(ROOT)/include -c $(BUILD)/h_passthru.cpp -o $(BUILD)/h_passthru.o 2> $(BUILD)/h_passthru.err; then
	    ok "スタブ11個+条件付きcstdio の素通り+ホスト宣言が使える(TC-HD-12), $$std"
	  else
	    ng "スタブ11個+条件付きcstdio の素通り+ホスト宣言(TC-HD-12), $$std"
	    sed -n 1,30p $(BUILD)/h_passthru.err
	  fi
	done
	# (TC-HD-13) -isystem の順序で提供ヘッダがコンパイラ付属より先に選ばれること。
	# 路径は -H の出力で確認し、機能は設定ヘッダ(ILIBC_NS)が入ることで確認する
	printf '#include <cstring>\n#include <cstdlib>\n#ifdef ILIBC_NS\nint main(){return 0;}\n#else\n#error ilibc headers were not selected\n#endif\n' > $(BUILD)/h_shadow.cpp
	if $(CXX) -m32 -std=c++23 $(WARN) $(FREE) $(ISYS) -isystem $(ROOT)/include -H -c $(BUILD)/h_shadow.cpp -o $(BUILD)/h_shadow.o 2> $(BUILD)/h_shadow.err; then
	  if grep -qF "$(ROOT)/include/cstring" $(BUILD)/h_shadow.err \
	     && grep -qF "$(ROOT)/include/cstdlib" $(BUILD)/h_shadow.err \
	     && ! grep -qE "/usr/include/c\+\+/[0-9.]+/cstring" $(BUILD)/h_shadow.err \
	     && ! grep -qE "/usr/include/c\+\+/[0-9.]+/cstdlib" $(BUILD)/h_shadow.err; then
	    ok "-isystem で提供ヘッダがコンパイラ付属より先に選ばれる(TC-HD-13)"
	  else
	    ng "-H の遮蔽確認が期待と異なる(TC-HD-13)"
	    sed -n 1,40p $(BUILD)/h_shadow.err
	  fi
	else
	  ng "-isystem の遮蔽確認: コンパイル失敗(TC-HD-13)"
	  sed -n 1,30p $(BUILD)/h_shadow.err
	fi
	# 本ライブラリが遮蔽しないヘッダ(§2.3)がコンパイラ付属のまま選ばれること
	printf '#include <cstddef>\n#include <climits>\n#include <cstdarg>\n#include <cfloat>\nint main(){return 0;}\n' > $(BUILD)/h_noshadow.cpp
	if $(CXX) -m32 -std=c++23 $(WARN) $(FREE) $(ISYS) -isystem $(ROOT)/include -H -c $(BUILD)/h_noshadow.cpp -o $(BUILD)/h_noshadow.o 2> $(BUILD)/h_noshadow.err; then
	  ns_ok=1
	  for h in cstddef climits cstdarg cfloat; do
	    if ! grep -qE "/usr/include/c\+\+/[0-9.]+/$$h" $(BUILD)/h_noshadow.err; then
	      ng "<$$h> がコンパイラ付属から来ていない(TC-HD-13)"
	      ns_ok=0
	    fi
	    if [ -e $(ROOT)/include/$$h ]; then
	      ng "<$$h> が本ライブラリ側に存在し遮蔽される(TC-HD-13)"
	      ns_ok=0
	    fi
	  done
	  if [ $$ns_ok -eq 1 ]; then
	    ok "cstddef/climits/cstdarg/cfloat は遮蔽されない(TC-HD-13)"
	  fi
	else
	  ng "遮蔽されないヘッダのコンパイル失敗(TC-HD-13)"
	  sed -n 1,30p $(BUILD)/h_noshadow.err
	fi
	# (TC-GN-03) 統合: コンパイラ付属ヘッダ(概要仕様書 §2.2)と提供ヘッダ5・
	# 互換ラッパ5を、2種の順序で -ffreestanding に含けても、スタブの #error が
	# 発動しないこと(素通り定義なし)。スタブ対象のヘッダは誰も取り込まない
	printf '#include <cstddef>\n#include <cstdint>\n#include <climits>\n#include <limits>\n#include <type_traits>\n#include <concepts>\n#include <bit>\n#include <utility>\n#include <version>\n#include <cctype>\n#include <cstring>\n#include <cstdlib>\n#include <cassert>\n#include <cerrno>\n#include <ctype.h>\n#include <string.h>\n#include <stdlib.h>\n#include <assert.h>\n#include <errno.h>\nint main(){return 0;}\n' > $(BUILD)/h_gn03_a.cpp
	tac $(BUILD)/h_gn03_a.cpp > $(BUILD)/h_gn03_b.cpp
	for v in a b; do
	  if $(CXX) -m32 -std=c++23 $(WARN) $(FREE) $(ISYS) -isystem $(ROOT)/include -c $(BUILD)/h_gn03_$$v.cpp -o $(BUILD)/h_gn03_$$v.o 2> $(BUILD)/h_gn03_$$v.err; then
	    ok "統合: 付属+提供ヘッダの順序$$v でスタブ #error なし(TC-GN-03)"
	  else
	    ng "統合: 順序$$v でスタブ #error が発動、またはコンパイル失敗(TC-GN-03)"
	    sed -n 1,30p $(BUILD)/h_gn03_$$v.err
	  fi
	done
	# ヘッダは ASCII のみ(ヘッダ仕様 H7)
	if LC_ALL=C grep -nP '[^\x00-\x7F]' \
	    $(ROOT)/include/cctype $(ROOT)/include/ctype.h \
	    $(ROOT)/include/cstring $(ROOT)/include/string.h \
	    $(ROOT)/include/cerrno $(ROOT)/include/errno.h \
	    $(ROOT)/include/cstdlib $(ROOT)/include/stdlib.h \
	    $(ROOT)/include/cassert $(ROOT)/include/assert.h \
	    $(ROOT)/include/cstdio $(ROOT)/include/stdio.h \
	    $(ROOT)/include/cmath $(ROOT)/include/math.h \
	    $(ROOT)/include/ctime $(ROOT)/include/time.h \
	    $(ROOT)/include/csignal $(ROOT)/include/signal.h \
	    $(ROOT)/include/clocale $(ROOT)/include/locale.h \
	    $(ROOT)/include/csetjmp $(ROOT)/include/setjmp.h \
	    $(ROOT)/include/ilibc/config.hpp $(ROOT)/include/ilibc/assert_fail.hpp \
	    $(ROOT)/include/ilibc/string_mem.hpp $(ROOT)/include/ilibc/stdlib.hpp \
	    $(ROOT)/include/ilibc/format.hpp \
	    $(ROOT)/include/ilibc/io_backend.hpp $(ROOT)/include/ilibc/io_sink.hpp \
	    > $(BUILD)/ascii.txt; then
	  ng "ヘッダに非 ASCII 文字がある(H7)"
	  sed -n 1,10p $(BUILD)/ascii.txt
	else
	  ok "ヘッダは ASCII のみ(H7、29ファイル)"
	fi
	# マクロ漏れ(H5): <climits> 単独との -dM -E 差分が ILIBC_* のみであること
	printf '#include <climits>\n' | $(CXX) -m32 -std=c++23 $(ISYS) -dM -E -x c++ - | sort > $(BUILD)/mac_before.txt
	printf '#include <climits>\n#include <ctype.h>\n' | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - | sort > $(BUILD)/mac_after.txt
	bad=$$(diff $(BUILD)/mac_before.txt $(BUILD)/mac_after.txt | grep '^>' | grep -v '^> #define ILIBC_' || true)
	added=$$(diff $(BUILD)/mac_before.txt $(BUILD)/mac_after.txt | grep -c '^>' || true)
	if [ -n "$$bad" ]; then
	  ng "仕様外のマクロが追加されている(H5)"
	  echo "$$bad"
	else
	  ok "追加マクロは ILIBC_* のみ($$added個、H5)"
	fi
	# <cstring>/<string.h> のマクロ漏れ(H5)。<cstring> は仕様上 <cstddef>/<cstdint> を
	# 取り込む(ヘッダ仕様 §2.2)ため、標準ヘッダ由来のマクロは基準に含めて差分を取る
	printf '#include <climits>\n#include <cstddef>\n#include <cstdint>\n' | $(CXX) -m32 -std=c++23 $(ISYS) -dM -E -x c++ - | sort > $(BUILD)/mac_std.txt
	printf '#include <climits>\n#include <cstring>\n' | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - | sort > $(BUILD)/mac_cstring.txt
	bad=$$(diff $(BUILD)/mac_std.txt $(BUILD)/mac_cstring.txt | grep '^>' | grep -v '^> #define ILIBC_' || true)
	if [ -n "$$bad" ]; then
	  ng "<cstring> が仕様外のマクロを追加している(H5)"
	  echo "$$bad"
	else
	  ok "<cstring> の追加マクロは ILIBC_* のみ(H5)"
	fi
	printf '#include <climits>\n#include <string.h>\n' | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - | sort > $(BUILD)/mac_string.txt
	bad=$$(diff $(BUILD)/mac_std.txt $(BUILD)/mac_string.txt | grep '^>' | grep -v '^> #define ILIBC_' || true)
	if [ -n "$$bad" ]; then
	  ng "<string.h> が仕様外のマクロを追加している(H5)"
	  echo "$$bad"
	else
	  ok "<string.h> の追加マクロは ILIBC_* のみ(H5)"
	fi
	# <cerrno> のマクロ漏れ(H5)。基準は <cerrno> が取り込む config.hpp 経由の
	# <climits>。仕様の公開マクロ EDOM/ERANGE/EINVAL/errno は許可(ヘッダ仕様 §5)
	printf '#include <climits>\n' | $(CXX) -m32 -std=c++23 $(ISYS) -dM -E -x c++ - | sort > $(BUILD)/mac_clim.txt
	printf '#include <cerrno>\n' | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - | sort > $(BUILD)/mac_cerrno.txt
	bad=$$(diff $(BUILD)/mac_clim.txt $(BUILD)/mac_cerrno.txt | grep '^>' \
	  | grep -vE '^> #define (ILIBC_|EDOM |ERANGE |EINVAL |errno )' || true)
	if [ -n "$$bad" ]; then
	  ng "<cerrno> が仕様外のマクロを追加している(H5)"
	  echo "$$bad"
	else
	  ok "<cerrno> の追加マクロは ILIBC_* + 仕様公開マクロのみ(H5)"
	fi
	# <errno.h> のマクロ漏れ(H5、TC-HD-05)。追加マクロは <cerrno> の再帰分のみで、
	# 仕様の公開マクロ EDOM/ERANGE/EINVAL/errno を超えてはならない
	printf '#include <errno.h>\n' | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - | sort > $(BUILD)/mac_errh.txt
	bad=$$(diff $(BUILD)/mac_clim.txt $(BUILD)/mac_errh.txt | grep '^>' \
	  | grep -vE '^> #define (ILIBC_|EDOM |ERANGE |EINVAL |errno )' || true)
	if [ -n "$$bad" ]; then
	  ng "<errno.h> が仕様外のマクロを追加している(H5)"
	  echo "$$bad"
	else
	  ok "<errno.h> の追加マクロは ILIBC_* + 仕様公開マクロのみ(H5)"
	fi
	# <cstdlib>/<stdlib.h> のマクロ漏れ(H5)。基準は config.hpp(climits)、
	# ilibc/stdlib.hpp(cctype/cerrno)、cstddef の取り込み分。仕様の公開マクロ
	# EXIT_SUCCESS/EXIT_FAILURE/MB_CUR_MAX は許可(cstdlib仕様 §2.4)
	printf '#include <climits>\n#include <cctype>\n#include <cerrno>\n#include <cstddef>\n' \
	  | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - | sort > $(BUILD)/mac_sl_std.txt
	printf '#include <cstdlib>\n' | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - | sort > $(BUILD)/mac_cstdlib.txt
	bad=$$(diff $(BUILD)/mac_sl_std.txt $(BUILD)/mac_cstdlib.txt | grep '^>' \
	  | grep -vE '^> #define (ILIBC_|EXIT_SUCCESS|EXIT_FAILURE|MB_CUR_MAX) ' || true)
	if [ -n "$$bad" ]; then
	  ng "<cstdlib> が仕様外のマクロを追加している(H5)"
	  echo "$$bad"
	else
	  ok "<cstdlib> の追加マクロは ILIBC_* + 仕様公開マクロのみ(H5)"
	fi
	printf '#include <stdlib.h>\n' | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - | sort > $(BUILD)/mac_stdlib.txt
	bad=$$(diff $(BUILD)/mac_sl_std.txt $(BUILD)/mac_stdlib.txt | grep '^>' \
	  | grep -vE '^> #define (ILIBC_|EXIT_SUCCESS|EXIT_FAILURE|MB_CUR_MAX) ' || true)
	if [ -n "$$bad" ]; then
	  ng "<stdlib.h> が仕様外のマクロを追加している(H5)"
	  echo "$$bad"
	else
	  ok "<stdlib.h> の追加マクロは ILIBC_* + 仕様公開マクロのみ(H5)"
	fi
	# <cassert>/<assert.h> のマクロ漏れ(H5)。基準は <climits>(cassert は
	# assert_fail.hpp のみを取り込み、他のマクロを定義しない)。公開マクロ
	# assert は仕様の許可対象(ヘッダ仕様 §3.3)。assert_fail.hpp はマクロを
	# 定義しないため、再インクルードされても集合に影響しない
	printf '#include <cassert>\n' | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - | sort > $(BUILD)/mac_cassert.txt
	bad=$$(diff $(BUILD)/mac_clim.txt $(BUILD)/mac_cassert.txt | grep '^>' \
	  | grep -vE '^> #define (ILIBC_|assert)' || true)
	if [ -n "$$bad" ]; then
	  ng "<cassert> が仕様外のマクロを追加している(H5)"
	  echo "$$bad"
	else
	  ok "<cassert> の追加マクロは ILIBC_* + assert のみ(H5)"
	fi
	printf '#include <assert.h>\n' | $(CXX) -m32 -std=c++23 $(ISYS) $(INC) -dM -E -x c++ - | sort > $(BUILD)/mac_asserth.txt
	bad=$$(diff $(BUILD)/mac_clim.txt $(BUILD)/mac_asserth.txt | grep '^>' \
	  | grep -vE '^> #define (ILIBC_|assert)' || true)
	if [ -n "$$bad" ]; then
	  ng "<assert.h> が仕様外のマクロを追加している(H5)"
	  echo "$$bad"
	else
	  ok "<assert.h> の追加マクロは ILIBC_* + assert のみ(H5)"
	fi
	echo "headers summary: pass=$$pass fail=$$fail"
	[ $$fail -eq 0 ]

# 段階3: コンパイル時試験 TC-CT-01〜06 / TC-ST-01〜03・05 / TC-SL-01〜04・08〜10・18〜20
# (TST §5.3、CTP §8、STR §9、SL §8)
constexpr: check-toolchain
	@echo "=== 段階3: コンパイル時試験 TC-CT-01〜06 / TC-ST-01〜03・05 / TC-SL-01〜04・08〜10・18〜20 ==="
	for std in c++20 c++23; do
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/constexpr/cctype.cpp -o $(BUILD)/cctype_ct_$$std.o 2> $(BUILD)/cctype_ct_$$std.err; then
	    echo "  PASS: constexpr TC-CT-01〜06 ($$std)"
	  else
	    echo "  FAIL: constexpr TC-CT-01〜06 ($$std)"
	    sed -n 1,30p $(BUILD)/cctype_ct_$$std.err
	    exit 1
	  fi
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/constexpr/cstring.cpp -o $(BUILD)/cstring_ct_$$std.o 2> $(BUILD)/cstring_ct_$$std.err; then
	    echo "  PASS: constexpr TC-ST-01〜03・05 ($$std)"
	  else
	    echo "  FAIL: constexpr TC-ST-01〜03・05 ($$std)"
	    sed -n 1,30p $(BUILD)/cstring_ct_$$std.err
	    exit 1
	  fi
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/constexpr/cstdlib.cpp -o $(BUILD)/cstdlib_ct_$$std.o 2> $(BUILD)/cstdlib_ct_$$std.err; then
	    echo "  PASS: TC-SL-01〜04・08〜10・18〜20 ($$std)"
	  else
	    echo "  FAIL: TC-SL-01〜04・08〜10・18〜20 ($$std)"
	    sed -n 1,30p $(BUILD)/cstdlib_ct_$$std.err
	    exit 1
	  fi
	  # <cassert>: 真の assert が定数式で使えること、コンマを含む式(TC-HD-06(c)(d))。
	  # NDEBUG なしでは真のみ可(偽は cf_cassert_constexpr_false.cpp)。NDEBUG ありでは
	  # 式を評価しないため、どの定数式も成立する(AST §3.1)
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/constexpr/cassert.cpp -o $(BUILD)/cassert_ct_$$std.o 2> $(BUILD)/cassert_ct_$$std.err; then
	    echo "  PASS: TC-HD-06(c)(d) 定数式の真・コンマ式 ($$std)"
	  else
	    echo "  FAIL: TC-HD-06(c)(d) 定数式の真・コンマ式 ($$std)"
	    sed -n 1,30p $(BUILD)/cassert_ct_$$std.err
	    exit 1
	  fi
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) -DNDEBUG $(ISYS) $(IINC) -c $(TST)/constexpr/cassert.cpp -o $(BUILD)/cassert_ct_ndebug_$$std.o 2> $(BUILD)/cassert_ct_ndebug_$$std.err; then
	    echo "  PASS: TC-HD-06(c)(d) NDEBUG 定数式 ($$std)"
	  else
	    echo "  FAIL: TC-HD-06(c)(d) NDEBUG 定数式 ($$std)"
	    sed -n 1,30p $(BUILD)/cassert_ct_ndebug_$$std.err
	    exit 1
	  fi
	done

# 段階3: コンパイル失敗試験(TST §5.4 の手順1〜3)。cf_*.cpp を1ファイルずつ検査。
# TC-CT-10(cctype)、TC-ST-04・16(a)(cstring)、TC-SL-02・05・16・21(cstdlib)、
# TC-IO-03・07(ILIBC-IO-001 §3・§7: format の書式検査)、
# TC-IO-12・15・16・18(ILIBC-IO-001 §5・§9: printf マクロ OFF、バックエンド
# 選択、stdio.h 禁止)。
compilefail: check-toolchain
	@echo "=== 段階3: コンパイル失敗試験 (files: $(notdir $(CF_ALL))) ==="
	[ -n "$(CF_ALL)" ] || { echo "FAIL: cf_*.cpp が1つもない"; exit 1; }
	for cf in $(CF_ALL); do
	  b=$$(basename $$cf .cpp)
	  # 手順1: 対照 -DCF_CONTROL で成功すること(試験コード自体の正しさ)
	  if $(CXX) -m32 -std=c++23 $(WARN) $(FREE) $(ISYS) $(INC) -DCF_CONTROL -c $$cf -o $(BUILD)/$$b-ctrl.o 2> $(BUILD)/$$b-ctrl.err; then
	    echo "  PASS: $$b 手順1 対照 -DCF_CONTROL が成功"
	  else
	    echo "  FAIL: $$b 手順1 対照が失敗"
	    sed -n 1,20p $(BUILD)/$$b-ctrl.err
	    exit 1
	  fi
	  # 手順2: -DCF_FAIL で失敗すること
	  if $(CXX) -m32 -std=c++23 $(WARN) $(FREE) $(ISYS) $(INC) -DCF_FAIL -c $$cf -o $(BUILD)/$$b-fail.o 2> $(BUILD)/$$b-fail.err; then
	    echo "  FAIL: $$b 手順2 -DCF_FAIL が成功した(失敗が必須)"
	    exit 1
	  fi
	  # 手順2続: 診断が EXPECT-GCC 正規表現にマッチすること。正規表現は試験コード内の
	  # コメントに持ち、版の変化に合わせてそちらを更新する(TST D-T6)
	  re=$$(sed -n 's|^[[:space:]]*//[[:space:]]*EXPECT-GCC:[[:space:]]*||p' $$cf | sed -e 's/[[:space:]]*$$//')
	  if [ -z "$$re" ]; then
	    echo "  FAIL: $$cf に EXPECT-GCC 行がない"
	    exit 1
	  fi
	  if grep -Eq "$$re" $(BUILD)/$$b-fail.err; then
	    echo "  PASS: $$b 手順2 失敗 + 診断正規表現マッチ"
	  else
	    echo "  FAIL: $$b 診断が正規表現にマッチしない: $$re"
	    sed -n 1,10p $(BUILD)/$$b-fail.err
	    exit 1
	  fi
	done

# 段階4: ホスト差分 TC-CT-07 / TC-ST-06〜09(TST §5.7.1。-m32・ILIBC_HOST_TEST・-U_FORTIFY_SOURCE)。
# cstring は OPTS 全レベルで実行する(TC-ST-15)。NDIST は mem* 実装の自己再帰防止(cstring仕様 §3)。
host: check-toolchain
	@echo "=== 段階4: ホスト差分 TC-CT-07 ==="
	$(CXX) -m32 -static -std=c++23 -DILIBC_HOST_TEST -U_FORTIFY_SOURCE $(WARN) \
	  -fno-builtin -fno-exceptions -fno-rtti \
	  $(IINC) $(ISYS) $(LD32) $(TST)/host/cctype_host.cpp -o $(BUILD)/cctype_host \
	  || { echo "  FAIL: TC-CT-07 ビルド"; exit 1; }
	rc=0
	$(BUILD)/cctype_host > $(BUILD)/host.out 2>&1 || rc=$$?
	cat $(BUILD)/host.out
	if [ $$rc -ne 0 ]; then
	  echo "  FAIL: TC-CT-07 実行 (exit $$rc)"
	  exit 1
	fi
	grep -q 'TC-CT-07 PASS' $(BUILD)/host.out || { echo "  FAIL: TC-CT-07 出力に PASS 行がない"; exit 1; }
	echo "  PASS: TC-CT-07"
	@echo "=== 段階4: ホスト差分 TC-ST-06〜09 (opt: $(OPTS)) ==="
	for opt in $(OPTS); do
	  tag=$${opt#-}
	  mkdir -p $(BUILD)/host_str_$$tag
	  for f in $(SRC_STR); do
	    b=$$(basename $$f .cpp)
	    $(CXX) -m32 -std=c++23 $$opt -DILIBC_HOST_TEST -U_FORTIFY_SOURCE $(WARN) \
	      -fno-builtin -fno-exceptions -fno-rtti $(NDIST) $(IINC) $(ISYS) \
	      -c $$f -o $(BUILD)/host_str_$$tag/$$b.o \
	      || { echo "  FAIL: TC-ST-06 mem* コンパイル $$opt $$b"; exit 1; }
	  done
	  $(CXX) -m32 -static -std=c++23 $$opt -DILIBC_HOST_TEST -U_FORTIFY_SOURCE $(WARN) \
	    -fno-builtin -fno-exceptions -fno-rtti $(NDIST) $(IINC) $(ISYS) $(LD32) \
	    $(TST)/host/cstring_host.cpp $(BUILD)/host_str_$$tag/*.o -o $(BUILD)/cstring_host-$$tag \
	    || { echo "  FAIL: TC-ST-06 ビルド $$opt"; exit 1; }
	  rc=0
	  $(BUILD)/cstring_host-$$tag > $(BUILD)/host_str-$$tag.out 2>&1 || rc=$$?
	  cat $(BUILD)/host_str-$$tag.out
	  if [ $$rc -eq 0 ] && grep -q 'TC-ST-06 PASS' $(BUILD)/host_str-$$tag.out \
	      && grep -q 'SUMMARY pass=4 fail=0' $(BUILD)/host_str-$$tag.out; then
	    echo "  PASS: TC-ST-06〜09 $$opt"
	  else
	    echo "  FAIL: TC-ST-06〜09 $$opt (exit $$rc)"
	    exit 1
	  fi
	done
	@echo "=== 段階4: ホスト差分 TC-SL-12・23・24 (opt: $(OPTS)) ==="
	# cstdlib はヘッダのみ(リンク符号なし)。mem* の NDIST・SRC_STR は不要。
	for opt in $(OPTS); do
	  tag=$${opt#-}
	  $(CXX) -m32 -static -std=c++23 $$opt -DILIBC_HOST_TEST -U_FORTIFY_SOURCE $(WARN) \
	    -fno-builtin -fno-exceptions -fno-rtti $(IINC) $(ISYS) $(LD32) \
	    $(TST)/host/cstdlib_host.cpp -o $(BUILD)/cstdlib_host-$$tag \
	    || { echo "  FAIL: TC-SL-12 ビルド $$opt"; exit 1; }
	  rc=0
	  # argv[1] はシード上書き(§5.7.1)。最適化タグは渡さず既定シードで実行する。
	  $(BUILD)/cstdlib_host-$$tag > $(BUILD)/host_sl-$$tag.out 2>&1 || rc=$$?
	  cat $(BUILD)/host_sl-$$tag.out
	  if [ $$rc -eq 0 ] && grep -q 'TC-SL-12 PASS' $(BUILD)/host_sl-$$tag.out \
	      && grep -q 'SUMMARY pass=3 fail=0' $(BUILD)/host_sl-$$tag.out; then
	    echo "  PASS: TC-SL-12・23・24 $$opt"
	  else
	    echo "  FAIL: TC-SL-12・23・24 $$opt (exit $$rc)"
	    exit 1
	  fi
	done

# 段階4: サニタイザ TC-ST-10 / TC-SL-13(TST §5.7.1: -fsanitize=address,undefined
# -O1 -g、ビルドは E1=-m32)。ASan は -static と併用不可(GCC)ため動的リンクする。
# ホストに32ビット実行環境がないため、sysroot の libc6-i386 部分
# (lib32/ld-linux.so.2、lib32/libc.so.6)をインタプリタと rpath に指定し、
# libstdc++ は -static-libstdc++ で静的に入れる(共有版 lib32stdc++6 は不要)。
sanitize: check-toolchain
	@echo "=== 段階4: サニタイザ TC-ST-10 (ASan/UBSan, -O1, E1 -m32) ==="
	[ -e $(SYSROOT)/usr/lib32/ld-linux.so.2 ] || { \
	  echo "  FAIL: sysroot に32ビット ld-linux.so.2 がない(libc6-i386 を展開すること)"; exit 1; }
	mkdir -p $(BUILD)/host_str_san
	for f in $(SRC_STR); do
	  b=$$(basename $$f .cpp)
	  $(CXX) -m32 -std=c++23 -O1 -g -DILIBC_HOST_TEST -U_FORTIFY_SOURCE $(WARN) \
	    -fno-builtin -fno-exceptions -fno-rtti $(NDIST) $(IINC) $(ISYS) \
	    -fsanitize=address,undefined \
	    -c $$f -o $(BUILD)/host_str_san/$$b.o \
	    || { echo "  FAIL: TC-ST-10 mem* コンパイル $$b"; exit 1; }
	done
	$(CXX) -m32 -std=c++23 -O1 -g -DILIBC_HOST_TEST -U_FORTIFY_SOURCE $(WARN) \
	  -fno-builtin -fno-exceptions -fno-rtti $(NDIST) $(IINC) $(ISYS) $(LD32) \
	  -static-libstdc++ \
	  -Wl,--dynamic-linker=$(SYSROOT)/usr/lib32/ld-linux.so.2 \
	  -Wl,-rpath,$(SYSROOT)/usr/lib32 \
	  -fsanitize=address,undefined \
	  $(TST)/host/cstring_host.cpp $(BUILD)/host_str_san/*.o -o $(BUILD)/cstring_host-san \
	  || { echo "  FAIL: TC-ST-10 ビルド"; exit 1; }
	rc=0
	$(BUILD)/cstring_host-san > $(BUILD)/host_str-san.out 2>&1 || rc=$$?
	cat $(BUILD)/host_str-san.out
	if [ $$rc -eq 0 ] && grep -q 'TC-ST-06 PASS' $(BUILD)/host_str-san.out \
	    && grep -q 'SUMMARY pass=4 fail=0' $(BUILD)/host_str-san.out; then
	  echo "  PASS: TC-ST-10"
	else
	  echo "  FAIL: TC-ST-10 (exit $$rc)"
	  exit 1
	fi
	# TC-SL-13: cstdlib ホスト差分をサニタイザ付きで実行(ヘッダのみ、リンク符号なし)
	@echo "=== 段階4: サニタイザ TC-SL-13 (ASan/UBSan, -O1, E1 -m32) ==="
	$(CXX) -m32 -std=c++23 -O1 -g -DILIBC_HOST_TEST -U_FORTIFY_SOURCE $(WARN) \
	  -fno-builtin -fno-exceptions -fno-rtti $(IINC) $(ISYS) $(LD32) \
	  -static-libstdc++ \
	  -Wl,--dynamic-linker=$(SYSROOT)/usr/lib32/ld-linux.so.2 \
	  -Wl,-rpath,$(SYSROOT)/usr/lib32 \
	  -fsanitize=address,undefined \
	  $(TST)/host/cstdlib_host.cpp -o $(BUILD)/cstdlib_host-san \
	  || { echo "  FAIL: TC-SL-13 ビルド"; exit 1; }
	rc=0
	$(BUILD)/cstdlib_host-san > $(BUILD)/host_sl-san.out 2>&1 || rc=$$?
	cat $(BUILD)/host_sl-san.out
	if [ $$rc -eq 0 ] && grep -q 'TC-SL-12 PASS' $(BUILD)/host_sl-san.out \
	    && grep -q 'SUMMARY pass=3 fail=0' $(BUILD)/host_sl-san.out; then
	  echo "  PASS: TC-SL-13"
	else
	  echo "  FAIL: TC-SL-13 (exit $$rc)"
	  exit 1
	fi

# 段階5: E1 実行 TC-CT-08/09(TST §5.6。-O0/-Os/-O2、volatile 経由の全数検査)
runtime: check-toolchain
	@echo "=== 段階5: E1 実行 TC-CT-08/09 (opt: $(OPTS)) ==="
	for opt in $(OPTS); do
	  tag=$${opt#-}
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/cases/cctype_runtime.cpp -o $(BUILD)/rt_$$tag.o \
	    || { echo "  FAIL: TC-CT-08 コンパイル $$opt"; exit 1; }
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/platform/e1_stdio.cpp -o $(BUILD)/e1_$$tag.o \
	    || { echo "  FAIL: プラットフォーム層(e1_stdio) コンパイル $$opt"; exit 1; }
	  # libstdc++ を付けない gcc ドライバでリンクする(C++ランタイム非依存の実証を兼ねる)
	  $(CC) -m32 -static $(LD32) $(BUILD)/rt_$$tag.o $(BUILD)/e1_$$tag.o -o $(BUILD)/ct_rt-$$tag \
	    || { echo "  FAIL: TC-CT-08 リンク $$opt"; exit 1; }
	  rc=0
	  $(BUILD)/ct_rt-$$tag > $(BUILD)/rt-$$tag.out 2>&1 || rc=$$?
	  cat $(BUILD)/rt-$$tag.out
	  if [ $$rc -eq 0 ] && grep -q 'SUMMARY pass=1 fail=0' $(BUILD)/rt-$$tag.out; then
	    echo "  PASS: TC-CT-08/09 $$opt"
	  else
	    echo "  FAIL: TC-CT-08/09 $$opt (exit $$rc)"
	    exit 1
	  fi
	done
	@echo "=== 段階5: E1 実行 TC-ST-11(ブートストラップ+TC-ST-07〜09) (opt: $(OPTS)) ==="
	for opt in $(OPTS); do
	  tag=$${opt#-}
	  mkdir -p $(BUILD)/rt_str_$$tag
	  for f in $(SRC_STR); do
	    b=$$(basename $$f .cpp)
	    $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(NDIST) $(ISYS) $(IINC) \
	      -c $$f -o $(BUILD)/rt_str_$$tag/$$b.o \
	      || { echo "  FAIL: TC-ST-11 mem* コンパイル $$opt $$b"; exit 1; }
	  done
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(NDIST) $(ISYS) $(IINC) \
	    -c $(TST)/target/cases/cstring_runtime.cpp -o $(BUILD)/rt_str_$$tag/cstring_runtime.o \
	    || { echo "  FAIL: TC-ST-11 コンパイル $$opt"; exit 1; }
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) \
	    -c $(TST)/target/platform/e1_stdio.cpp -o $(BUILD)/rt_str_$$tag/e1.o \
	    || { echo "  FAIL: プラットフォーム層(e1_stdio) コンパイル $$opt"; exit 1; }
	  # libstdc++ を付けない gcc ドライバでリンクする(C++ランタイム非依存の実証を兼ねる)
	  $(CC) -m32 -static $(LD32) $(BUILD)/rt_str_$$tag/*.o -o $(BUILD)/st_rt-$$tag \
	    || { echo "  FAIL: TC-ST-11 リンク $$opt"; exit 1; }
	  rc=0
	  $(BUILD)/st_rt-$$tag > $(BUILD)/st_rt-$$tag.out 2>&1 || rc=$$?
	  cat $(BUILD)/st_rt-$$tag.out
	  if [ $$rc -eq 0 ] && grep -q 'SUMMARY pass=3 fail=0' $(BUILD)/st_rt-$$tag.out; then
	    echo "  PASS: TC-ST-11 $$opt"
	  else
	    echo "  FAIL: TC-ST-11 $$opt (exit $$rc)"
	    exit 1
	  fi
	done
	@echo "=== 段階5: E1 実行 TC-SL-14・15(ブートストラップ+TC-SL-06〜11・22〜24) (opt: $(OPTS)) ==="
	for opt in $(OPTS); do
	  tag=$${opt#-}
	  mkdir -p $(BUILD)/rt_sl_$$tag
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) \
	    -c $(TST)/target/cases/cstdlib_runtime.cpp -o $(BUILD)/rt_sl_$$tag/cstdlib_runtime.o \
	    || { echo "  FAIL: TC-SL-14 コンパイル $$opt"; exit 1; }
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) \
	    -c $(TST)/target/platform/e1_stdio.cpp -o $(BUILD)/rt_sl_$$tag/e1.o \
	    || { echo "  FAIL: プラットフォーム層(e1_stdio) コンパイル $$opt"; exit 1; }
	  # libstdc++ を付けない gcc ドライバでリンクする(C++ランタイム非依存の実証を兼ねる)
	  $(CC) -m32 -static $(LD32) $(BUILD)/rt_sl_$$tag/*.o -o $(BUILD)/sl_rt-$$tag \
	    || { echo "  FAIL: TC-SL-14 リンク $$opt"; exit 1; }
	  rc=0
	  $(BUILD)/sl_rt-$$tag > $(BUILD)/sl_rt-$$tag.out 2>&1 || rc=$$?
	  cat $(BUILD)/sl_rt-$$tag.out
	  if [ $$rc -eq 0 ] && grep -q 'SUMMARY pass=9 fail=0' $(BUILD)/sl_rt-$$tag.out; then
	    echo "  PASS: TC-SL-14・15 $$opt"
	  else
	    echo "  FAIL: TC-SL-14・15 $$opt (exit $$rc)"
	    exit 1
	  fi
	done

# 段階5相当: <ilibc/format.hpp>・<cstdio> の TC-IO-01・02・04〜13・15・17
# (ILIBC-IO-001 §11。TC-IO-03・07・16・18 は compilefail、TC-IO-14 は check)。
# E1実行は §5.6.2 のプラットフォーム層(e1_stdio)と同じリンク方式(libstdc++ なし)。
formatio: check-toolchain
	@echo "=== 段階5: E1 実行 TC-IO-02/04〜07 (opt: $(OPTS)) ==="
	for opt in $(OPTS); do
	  tag=$${opt#-}
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/cases/format_runtime.cpp -o $(BUILD)/io_$$tag.o \
	    || { echo "  FAIL: TC-IO コンパイル $$opt"; exit 1; }
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/platform/e1_stdio.cpp -o $(BUILD)/e1_io_$$tag.o \
	    || { echo "  FAIL: プラットフォーム層(e1_stdio) コンパイル $$opt"; exit 1; }
	  # libstdc++ を付けない gcc ドライバでリンクする(C++ランタイム非依存の実証を兼ねる)
	  $(CC) -m32 -static $(LD32) $(BUILD)/io_$$tag.o $(BUILD)/e1_io_$$tag.o -o $(BUILD)/io_rt-$$tag \
	    || { echo "  FAIL: TC-IO リンク $$opt"; exit 1; }
	  rc=0
	  $(BUILD)/io_rt-$$tag > $(BUILD)/io_rt-$$tag.out 2>&1 || rc=$$?
	  cat $(BUILD)/io_rt-$$tag.out
	  if [ $$rc -eq 0 ] && grep -q 'SUMMARY pass=5 fail=0' $(BUILD)/io_rt-$$tag.out; then
	    echo "  PASS: TC-IO-02/04〜07 $$opt"
	  else
	    echo "  FAIL: TC-IO-02/04〜07 $$opt (exit $$rc)"
	    exit 1
	  fi
	done
	@echo "=== 段階5: E1 実行 TC-IO-08/10/11/12 <cstdio> FD出力 (opt: $(OPTS)) ==="
	for opt in $(OPTS); do
	  tag=$${opt#-}
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/cases/fdio_runtime.cpp -o $(BUILD)/io_fd_$$tag.o \
	    || { echo "  FAIL: TC-IO-08/10/11/12 コンパイル $$opt"; exit 1; }
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/platform/e1_stdio.cpp -o $(BUILD)/e1_fd_$$tag.o \
	    || { echo "  FAIL: プラットフォーム層(e1_stdio) コンパイル $$opt"; exit 1; }
	  # libstdc++ を付けない gcc ドライバでリンクする(C++ランタイム非依存の実証を兼ねる)
	  $(CC) -m32 -static $(LD32) $(BUILD)/io_fd_$$tag.o $(BUILD)/e1_fd_$$tag.o -o $(BUILD)/io_fd_rt-$$tag \
	    || { echo "  FAIL: TC-IO-08/10/11/12 リンク $$opt"; exit 1; }
	  rc=0
	  $(BUILD)/io_fd_rt-$$tag > $(BUILD)/io_fd_rt-$$tag.out 2>&1 || rc=$$?
	  cat $(BUILD)/io_fd_rt-$$tag.out
	  if [ $$rc -eq 0 ] && grep -q 'SUMMARY pass=4 fail=0' $(BUILD)/io_fd_rt-$$tag.out; then
	    echo "  PASS: TC-IO-08/10/11/12 $$opt"
	  else
	    echo "  FAIL: TC-IO-08/10/11/12 $$opt (exit $$rc)"
	    exit 1
	  fi
	done
	@echo "=== 段階5: E1 実行 TC-IO-09 故障注入 (opt: $(OPTS)) ==="
	for opt in $(OPTS); do
	  tag=$${opt#-}
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/cases/fdio_inject.cpp -o $(BUILD)/io_inj_$$tag.o \
	    || { echo "  FAIL: TC-IO-09 コンパイル $$opt"; exit 1; }
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/platform/e1_stdio.cpp -o $(BUILD)/e1_inj_$$tag.o \
	    || { echo "  FAIL: プラットフォーム層(e1_stdio) コンパイル $$opt"; exit 1; }
	  # libstdc++ を付けない gcc ドライバでリンクする(C++ランタイム非依存の実証を兼ねる)
	  $(CC) -m32 -static $(LD32) $(BUILD)/io_inj_$$tag.o $(BUILD)/e1_inj_$$tag.o -o $(BUILD)/io_inj_rt-$$tag \
	    || { echo "  FAIL: TC-IO-09 リンク $$opt"; exit 1; }
	  rc=0
	  $(BUILD)/io_inj_rt-$$tag > $(BUILD)/io_inj_rt-$$tag.out 2>&1 || rc=$$?
	  cat $(BUILD)/io_inj_rt-$$tag.out
	  if [ $$rc -eq 0 ] && grep -q 'SUMMARY pass=1 fail=0' $(BUILD)/io_inj_rt-$$tag.out; then
	    echo "  PASS: TC-IO-09 $$opt"
	  else
	    echo "  FAIL: TC-IO-09 $$opt (exit $$rc)"
	    exit 1
	  fi
	done
	@echo "=== 段階5: E1 実行 TC-IO-19〜23/25 <cstdio> FD入力 (opt: $(OPTS)) ==="
	for opt in $(OPTS); do
	  tag=$${opt#-}
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/cases/fdio_input.cpp -o $(BUILD)/io_in_$$tag.o \
	    || { echo "  FAIL: TC-IO-19〜23/25 コンパイル $$opt"; exit 1; }
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/platform/e1_stdio.cpp -o $(BUILD)/e1_in_$$tag.o \
	    || { echo "  FAIL: プラットフォーム層(e1_stdio) コンパイル $$opt"; exit 1; }
	  # libstdc++ を付けない gcc ドライバでリンクする(C++ランタイム非依存の実証を兼ねる)
	  $(CC) -m32 -static $(LD32) $(BUILD)/io_in_$$tag.o $(BUILD)/e1_in_$$tag.o -o $(BUILD)/io_in_rt-$$tag \
	    || { echo "  FAIL: TC-IO-19〜23/25 リンク $$opt"; exit 1; }
	  rc=0
	  $(BUILD)/io_in_rt-$$tag > $(BUILD)/io_in_rt-$$tag.out 2>&1 || rc=$$?
	  cat $(BUILD)/io_in_rt-$$tag.out
	  if [ $$rc -eq 0 ] && grep -q 'SUMMARY pass=6 fail=0' $(BUILD)/io_in_rt-$$tag.out; then
	    echo "  PASS: TC-IO-19〜23/25 $$opt"
	  else
	    echo "  FAIL: TC-IO-19〜23/25 $$opt (exit $$rc)"
	    exit 1
	  fi
	done
	@echo "=== 段階5: E1 実行 TC-IO-24 入力故障注入 (opt: $(OPTS)) ==="
	for opt in $(OPTS); do
	  tag=$${opt#-}
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/cases/fdio_read_inject.cpp -o $(BUILD)/io_rinj_$$tag.o \
	    || { echo "  FAIL: TC-IO-24 コンパイル $$opt"; exit 1; }
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/platform/e1_stdio.cpp -o $(BUILD)/e1_rinj_$$tag.o \
	    || { echo "  FAIL: プラットフォーム層(e1_stdio) コンパイル $$opt"; exit 1; }
	  # libstdc++ を付けない gcc ドライバでリンクする(C++ランタイム非依存の実証を兼ねる)
	  $(CC) -m32 -static $(LD32) $(BUILD)/io_rinj_$$tag.o $(BUILD)/e1_rinj_$$tag.o -o $(BUILD)/io_rinj_rt-$$tag \
	    || { echo "  FAIL: TC-IO-24 リンク $$opt"; exit 1; }
	  rc=0
	  $(BUILD)/io_rinj_rt-$$tag > $(BUILD)/io_rinj_rt-$$tag.out 2>&1 || rc=$$?
	  cat $(BUILD)/io_rinj_rt-$$tag.out
	  if [ $$rc -eq 0 ] && grep -q 'SUMMARY pass=1 fail=0' $(BUILD)/io_rinj_rt-$$tag.out; then
	    echo "  PASS: TC-IO-24 $$opt"
	  else
	    echo "  FAIL: TC-IO-24 $$opt (exit $$rc)"
	    exit 1
	  fi
	done
	@echo "=== 段階5: E1 実行 CE-01〜03 <cerrno> clear_errno (opt: $(OPTS)) ==="
	for opt in $(OPTS); do
	  tag=$${opt#-}
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/cases/cerrno_clear.cpp -o $(BUILD)/ce_cl_$$tag.o \
	    || { echo "  FAIL: CE-01〜03 コンパイル $$opt"; exit 1; }
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/platform/e1_stdio.cpp -o $(BUILD)/e1_ce_$$tag.o \
	    || { echo "  FAIL: プラットフォーム層(e1_stdio) コンパイル $$opt"; exit 1; }
	  # libstdc++ を付けない gcc ドライバでリンクする(C++ランタイム非依存の実証を兼ねる)
	  $(CC) -m32 -static $(LD32) $(BUILD)/ce_cl_$$tag.o $(BUILD)/e1_ce_$$tag.o -o $(BUILD)/ce_cl_rt-$$tag \
	    || { echo "  FAIL: CE-01〜03 リンク $$opt"; exit 1; }
	  rc=0
	  $(BUILD)/ce_cl_rt-$$tag > $(BUILD)/ce_cl_rt-$$tag.out 2>&1 || rc=$$?
	  cat $(BUILD)/ce_cl_rt-$$tag.out
	  if [ $$rc -eq 0 ] && grep -q 'SUMMARY pass=3 fail=0' $(BUILD)/ce_cl_rt-$$tag.out; then
	    echo "  PASS: CE-01〜03 $$opt"
	  else
	    echo "  FAIL: CE-01〜03 $$opt (exit $$rc)"
	    exit 1
	  fi
	done
	@echo "=== 段階5: 依存検査 TC-IO-13 入力 nm (<cstdio> 入力利用コードの未解決参照) ==="
	$(CXX) -m32 -std=c++23 -O0 $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/cases/fdio_deps_input.cpp -o $(BUILD)/io_deps_in.o \
	  || { echo "  FAIL: TC-IO-13 入力 コンパイル"; exit 1; }
	# 許可集合は read・__errno_location と GOT のみ。ヒープ・va_arg・printf 等の
	# 混入を検査し、§14.5 のホストerrno取得参照と read 参照が無ければ検査が
	# 空振りしているためそれも失敗にする
	undef=$$(nm -u $(BUILD)/io_deps_in.o | sed -n 's/^[[:space:]]*U[[:space:]]*//p' | sort -u)
	extra=$$(printf '%s\n' "$$undef" | grep -vE '^(read|__errno_location|_GLOBAL_OFFSET_TABLE_)$$' || true)
	if [ -n "$$extra" ]; then
	  echo "  FAIL: TC-IO-13 入力 不要な実行時依存: $$extra"
	  exit 1
	fi
	if ! printf '%s\n' "$$undef" | grep -qx 'read'; then
	  echo "  FAIL: TC-IO-13 入力 read 参照がない(検査が空振り)"
	  exit 1
	fi
	if ! printf '%s\n' "$$undef" | grep -qx '__errno_location'; then
	  echo "  FAIL: TC-IO-13 入力 __errno_location 参照がない(§14.5 の検査が空振り)"
	  exit 1
	fi
	echo "  PASS: TC-IO-13 入力 未解決参照は read と __errno_location と GOT のみ"
	@echo "=== 段階5: 依存検査 TC-IO-13 nm (<cstdio> 利用コードの未解決参照) ==="
	$(CXX) -m32 -std=c++23 -O0 $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/cases/fdio_deps.cpp -o $(BUILD)/io_deps.o \
	  || { echo "  FAIL: TC-IO-13 コンパイル"; exit 1; }
	# 許可集合は write と GOT のみ。ヒープ・va_arg・printf 等の混入を検査し、
	# write 参照が無ければ検査が空振りしているためそれも失敗にする
	undef=$$(nm -u $(BUILD)/io_deps.o | sed -n 's/^[[:space:]]*U[[:space:]]*//p' | sort -u)
	extra=$$(printf '%s\n' "$$undef" | grep -vE '^(write|_GLOBAL_OFFSET_TABLE_)$$' || true)
	if [ -n "$$extra" ]; then
	  echo "  FAIL: TC-IO-13 不要な実行時依存: $$extra"
	  exit 1
	fi
	if ! printf '%s\n' "$$undef" | grep -qx 'write'; then
	  echo "  FAIL: TC-IO-13 write 参照がない(検査が空振り)"
	  exit 1
	fi
	echo "  PASS: TC-IO-13 未解決参照は write と GOT のみ"
	@echo "=== 段階5: format 単独コンパイル・リンク・依存検査 TC-IO-01/15/17 ==="
	for std in c++20 c++23; do
	  # TC-IO-01/15(書式側): I/Oバックエンドなしで自己完結し、リンク・実行もできること
	  printf '#include <ilibc/format.hpp>\nstatic int eq(const char *a, const char *b) {\n    while (*a != 0 && *a == *b) { ++a; ++b; }\n    return *a == 0 && *b == 0;\n}\nint main() {\n    char buf[16];\n    int n = ilibc::sprintf<"x=%%d">(buf, 42);\n    int m = ilibc::snprint_f<"abc">(nullptr, 0);\n    return (n == 4 && eq(buf, "x=42") && m == 3) ? 0 : 1;\n}\n' > $(BUILD)/io_alone_$$std.cpp
	  $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(IINC) -c $(BUILD)/io_alone_$$std.cpp -o $(BUILD)/io_alone_$$std.o \
	    || { echo "  FAIL: TC-IO-01 単独コンパイル $$std"; exit 1; }
	  # TC-IO-17: write/ヒープ等への未解決参照がないこと(nm 検査、ILIBC-IO-001 §11)
	  if nm -u $(BUILD)/io_alone_$$std.o | grep -E 'write|malloc|free|printf|_Znw|_Zna'; then
	    echo "  FAIL: TC-IO-17 format 単体に不要な実行時依存がある ($$std)"
	    exit 1
	  fi
	  # libstdc++ を付けない gcc ドライバでリンクする(C++ランタイム非依存の実証を兼ねる)
	  $(CC) -m32 -static $(LD32) $(BUILD)/io_alone_$$std.o -o $(BUILD)/io_alone_$$std \
	    || { echo "  FAIL: TC-IO-01 リンク $$std"; exit 1; }
	  rc=0
	  $(BUILD)/io_alone_$$std || rc=$$?
	  if [ $$rc -eq 0 ]; then
	    echo "  PASS: TC-IO-01/15/17 単独コンパイル・リンク・実行 $$std"
	  else
	    echo "  FAIL: TC-IO-01 単独実行 $$std (exit $$rc)"
	    exit 1
	  fi
	  # TC-IO-15 の另一半: バックエンド未指定の <cstdio> は失敗する
	  # (format.hpp は上のとおり成功する)
	  printf '#include <cstdio>\nint main(){return 0;}\n' > $(BUILD)/io_cfail_$$std.cpp
	  if $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(IINC) -fsyntax-only $(BUILD)/io_cfail_$$std.cpp 2> $(BUILD)/io_cfail_$$std.err; then
	    echo "  FAIL: TC-IO-15 未指定バックエンドの <cstdio> が成功した ($$std)"
	    exit 1
	  fi
	  if grep -q 'ilibc: <cstdio> is not provided' $(BUILD)/io_cfail_$$std.err; then
	    echo "  PASS: TC-IO-15 <cstdio> 未指定で失敗(format.hpp は成功) $$std"
	  else
	    echo "  FAIL: TC-IO-15 <cstdio> の診断が仕様と異なる ($$std)"
	    sed -n 1,10p $(BUILD)/io_cfail_$$std.err
	    exit 1
	  fi
	done
	@echo "=== 段階5: <cstdio> 単独コンパイル・リンク・実行 TC-IO-12 (printf マクロON) ==="
	for std in c++20 c++23; do
	  printf '#define ILIBC_ENABLE_PRINTF_MACRO\n#define ILIBC_IO_BACKEND_POSIX\n#include <cstdio>\nint main() {\n    int a = printf("ok\\n");\n    int b = printf("v=%%d", 7);\n    int c = ilibc::putstr("");\n    return (a == 3 && b == 3 && c == 0) ? 0 : 1;\n}\n' > $(BUILD)/io_cstd_$$std.cpp
	  $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(IINC) -c $(BUILD)/io_cstd_$$std.cpp -o $(BUILD)/io_cstd_$$std.o \
	    || { echo "  FAIL: <cstdio> 単独コンパイル $$std"; exit 1; }
	  # libstdc++ を付けない gcc ドライバでリンクする(C++ランタイム非依存の実証を兼ねる)
	  $(CC) -m32 -static $(LD32) $(BUILD)/io_cstd_$$std.o -o $(BUILD)/io_cstd_$$std \
	    || { echo "  FAIL: <cstdio> 単独リンク $$std"; exit 1; }
	  rc=0
	  $(BUILD)/io_cstd_$$std > $(BUILD)/io_cstd_$$std.out 2>&1 || rc=$$?
	  if [ $$rc -eq 0 ] && grep -q '^ok$$' $(BUILD)/io_cstd_$$std.out && grep -q 'v=7' $(BUILD)/io_cstd_$$std.out; then
	    echo "  PASS: TC-IO-12 <cstdio> 単独コンパイル・リンク・実行(printf マクロON) $$std"
	  else
	    echo "  FAIL: TC-IO-12 <cstdio> 単独実行 $$std (exit $$rc)"
	    cat $(BUILD)/io_cstd_$$std.out
	    exit 1
	  fi
	done
	@echo "=== 段階5: 対照成功 TC-IO-12/16 バックエンド指定の併存 ==="
	# (1) POSIX と ILIBC_HOST_IO_TEST の併存(独立設定、TC-IO-16)
	# (2) printf マクロ ON で <cstdlib> と併存(他ヘッダへの影響なし、TC-IO-12)
	# (3) ILIBC_HOST_IO_TEST 単独でもバックエンドとして機能すること(TC-IO-16)
	c_ok=1
	for std in c++20 c++23; do
	  printf '#define ILIBC_IO_BACKEND_POSIX\n#define ILIBC_HOST_IO_TEST\n#include <cstdio>\nint main(){return ilibc::putstr("")?1:0;}\n' > $(BUILD)/io_ctl1_$$std.cpp
	  printf '#define ILIBC_ENABLE_PRINTF_MACRO\n#define ILIBC_IO_BACKEND_POSIX\n#include <cstdlib>\n#include <cstdio>\nint main(){return printf("")?1:0;}\n' > $(BUILD)/io_ctl2_$$std.cpp
	  printf '#define ILIBC_HOST_IO_TEST\n#include <cstdio>\nint main(){return ilibc::putstr("")?1:0;}\n' > $(BUILD)/io_ctl3_$$std.cpp
	  for c in io_ctl1 io_ctl2 io_ctl3; do
	    if ! $(CXX) -m32 -std=$$std $(WARN) $(FREE) $(ISYS) $(INC) -fsyntax-only $(BUILD)/$${c}_$$std.cpp 2> $(BUILD)/$${c}_$$std.err; then
	      echo "  FAIL: 対照成功 $$c $$std"
	      sed -n 1,15p $(BUILD)/$${c}_$$std.err
	      c_ok=0
	    fi
	  done
	done
	[ $$c_ok -eq 1 ] || exit 1
	echo "  PASS: 対照成功(POSIX+HOST_IO_TEST・マクロ+cstdlib・HOST_IO_TEST単独) TC-IO-12/16"

# 段階5: oneshot TC-ST-17 / TC-HD-06(a)(b)・TC-HD-07(TST §5.6.4: 1ケース1実行
# ファイル、終了コード42)。
# -O0 -DILIBC_DEBUG でビルドし、フック(th_hook.cpp)が出力して42で終わることを検査する。
# 注意: e1_stdio の printf 経由で静的 glibc が入り、起動時にも mem* が呼ばれる。
# ILIBC_DEBUG のアサーションはその呼び出しも検査する(通常時は非重なりなので通る)。
oneshot: check-toolchain
	@echo "=== 段階5: oneshot TC-ST-17 / TC-HD-06(a)(b)・TC-HD-07 (ILIBC_DEBUG, -O0) ==="
	mkdir -p $(BUILD)/oneshot
	for f in $(SRC_STR); do
	  b=$$(basename $$f .cpp)
	  $(CXX) -m32 -std=c++23 -O0 $(WARN) $(FREE) -DILIBC_DEBUG $(NDIST) $(ISYS) $(IINC) \
	    -c $$f -o $(BUILD)/oneshot/$$b.o \
	    || { echo "  FAIL: TC-ST-17 mem* コンパイル ($$b)"; exit 1; }
	done
	$(CXX) -m32 -std=c++23 -O0 $(WARN) $(FREE) -DILIBC_DEBUG $(ISYS) $(IINC) \
	  -c $(TST)/common/harness/th_hook.cpp -o $(BUILD)/oneshot/th_hook.o \
	  || { echo "  FAIL: TC-ST-17 th_hook コンパイル"; exit 1; }
	$(CXX) -m32 -std=c++23 -O0 $(WARN) $(FREE) $(ISYS) $(IINC) \
	  -c $(TST)/target/platform/e1_stdio.cpp -o $(BUILD)/oneshot/e1.o \
	  || { echo "  FAIL: TC-ST-17 e1_stdio コンパイル"; exit 1; }
	for c in $(OS_CASES); do
	  $(CXX) -m32 -std=c++23 -O0 $(WARN) $(FREE) -DILIBC_DEBUG $(ISYS) $(IINC) \
	    -c $(TST)/target/oneshot/$$c.cpp -o $(BUILD)/oneshot/$$c.o \
	    || { echo "  FAIL: TC-ST-17 $$c コンパイル"; exit 1; }
	  $(CC) -m32 -static $(LD32) $(BUILD)/oneshot/$$c.o $(BUILD)/oneshot/th_hook.o \
	    $(BUILD)/oneshot/e1.o $(BUILD)/oneshot/memcpy.o $(BUILD)/oneshot/memmove.o \
	    $(BUILD)/oneshot/memcmp.o $(BUILD)/oneshot/memchr.o $(BUILD)/oneshot/memset.o \
	    -o $(BUILD)/oneshot/$$c \
	    || { echo "  FAIL: TC-ST-17 $$c リンク"; exit 1; }
	done
	# (1) ヌルポインタ: フックが呼ばれ、42で終わる
	rc=0
	$(BUILD)/oneshot/st17_strlen_null > $(BUILD)/oneshot/st17_strlen_null.out 2>&1 || rc=$$?
	cat $(BUILD)/oneshot/st17_strlen_null.out
	if [ $$rc -eq 42 ] && grep -Eq '^ASSERT expr=.* file=.* line=[0-9]+$$' $(BUILD)/oneshot/st17_strlen_null.out; then
	  echo "  PASS: TC-ST-17 (1/3) strlen(nullptr) でフック+42"
	else
	  echo "  FAIL: TC-ST-17 (1/3) (exit $$rc)"
	  exit 1
	fi
	# (2) memcpy の重なり: フックが呼ばれ、42で終わる
	rc=0
	$(BUILD)/oneshot/st17_memcpy_overlap > $(BUILD)/oneshot/st17_memcpy_overlap.out 2>&1 || rc=$$?
	cat $(BUILD)/oneshot/st17_memcpy_overlap.out
	if [ $$rc -eq 42 ] && grep -Eq '^ASSERT expr=.* file=.* line=[0-9]+$$' $(BUILD)/oneshot/st17_memcpy_overlap.out; then
	  echo "  PASS: TC-ST-17 (2/3) 重なり memcpy でフック+42"
	else
	  echo "  FAIL: TC-ST-17 (2/3) (exit $$rc)"
	  exit 1
	fi
	# (3) 正常経路: フックが呼ばれない(出力に ASSERT なし)、0で終わる
	rc=0
	$(BUILD)/oneshot/st17_memcpy_ok > $(BUILD)/oneshot/st17_memcpy_ok.out 2>&1 || rc=$$?
	cat $(BUILD)/oneshot/st17_memcpy_ok.out
	if [ $$rc -eq 0 ] && ! grep -q 'ASSERT' $(BUILD)/oneshot/st17_memcpy_ok.out; then
	  echo "  PASS: TC-ST-17 (3/3) 正常経路でフックなし+0"
	else
	  echo "  FAIL: TC-ST-17 (3/3) (exit $$rc)"
	  exit 1
	fi
	# ------------------------------------------------- TC-HD-07(a)(b): フックの引数
	# NDEBUG の解除と再インクルードで再び有効になり、偽の assert がフックを呼ぶ。
	# 式のテキスト・ファイル名・行番号が引数として渡ることを検査する(TC-HD-07(a)(b))。
	# 行番号はソースから導出し、ファイル改変に追従させる。
	rc=0
	$(BUILD)/oneshot/hd07_reenable > $(BUILD)/oneshot/hd07_reenable.out 2>&1 || rc=$$?
	cat $(BUILD)/oneshot/hd07_reenable.out
	exp_line=$$(grep -n 'assert(1 == 2)' $(TST)/target/oneshot/hd07_reenable.cpp | cut -d: -f1)
	if [ $$rc -eq 42 ] \
	    && grep -Eq "^ASSERT expr=1 == 2 file=.*/hd07_reenable\.cpp line=$${exp_line}$$" \
	       $(BUILD)/oneshot/hd07_reenable.out; then
	  echo "  PASS: TC-HD-06(a)/07 (4/6) 再有効化後の偽 assert でフック+42(式・行番号)"
	else
	  echo "  FAIL: TC-HD-06(a)/07 (4/6) (exit $$rc, 期待 line=$$exp_line)"
	  exit 1
	fi
	# ------------------------------------------------- TC-HD-06(b)/07(c): 非評価
	# NDEBUG 定義時の assert は式を評価せず、フックも呼ばず、0で終わる。
	# 副作用(++n)と、無効なら通るはずの偽の assert の2経路を検査する。
	rc=0
	$(BUILD)/oneshot/hd07_ndebug_off > $(BUILD)/oneshot/hd07_ndebug_off.out 2>&1 || rc=$$?
	cat $(BUILD)/oneshot/hd07_ndebug_off.out
	if [ $$rc -eq 0 ] && ! grep -q 'ASSERT' $(BUILD)/oneshot/hd07_ndebug_off.out; then
	  echo "  PASS: TC-HD-06(b)/07(c) (5/6) NDEBUG で非評価・フックなし+0"
	else
	  echo "  FAIL: TC-HD-06(b)/07(c) (5/6) (exit $$rc)"
	  exit 1
	fi
	# ------------------------------------- TC-HD-07(d): ILIBC_ASSERT の独立性
	# NDEBUG 定義時の assert は無効(expr=2 + 2 == 5 で呼ばれない)、ILIBC_ASSERT は
	# ILIBC_DEBUG のみで有効(expr=3 * 3 == 8 で呼ばれる)ことを、出力の式で切り分ける。
	rc=0
	$(BUILD)/oneshot/hd07_ilibase_independent > $(BUILD)/oneshot/hd07_ilibase.out 2>&1 || rc=$$?
	cat $(BUILD)/oneshot/hd07_ilibase.out
	exp_line=$$(grep -n 'ILIBC_ASSERT(3 \* 3 == 8)' $(TST)/target/oneshot/hd07_ilibase_independent.cpp | cut -d: -f1)
	if [ $$rc -eq 42 ] \
	    && grep -Eq "^ASSERT expr=3 \* 3 == 8 file=.*/hd07_ilibase_independent\.cpp line=$${exp_line}$$" \
	       $(BUILD)/oneshot/hd07_ilibase.out; then
	  echo "  PASS: TC-HD-07(d) (6/6) NDEBUG と独立に ILIBC_ASSERT が発火+42"
	else
	  echo "  FAIL: TC-HD-07(d) (6/6) (exit $$rc, 期待 line=$$exp_line)"
	  exit 1
	fi

# 段階6: 成果物・リンク検査 TC-CT-11 / TC-ST-12・13 / TC-SL-17 / TC-HD-16・17
# (TST §5.5、CTP §8、AST §8)。
# cctype 不使用の対照プログラムと比較して、(a)シンボル (b).rodata (c)C++ランタイム・
# 静的初期化の「cctype 由来分がゼロ」を帰属切り分けで判定する。
artifact: check-toolchain
	@echo "=== 段階6: 成果物・リンク検査 TC-CT-11 ==="
	printf '#include <climits>\nint main(int argc, char **) {\n    const int c = (argc > 1) ? 66 : 65;\n    return (c >= 65 && c <= 90 && c != 32) ? 0 : 1;\n}\n' > $(BUILD)/control.cpp
	for opt in -O2 -O0; do
	  tag=$${opt#-}
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(IINC) $(ISYS) -c $(TST)/link/cctype_sample.cpp -o $(BUILD)/sample-$$tag.o \
	    || { echo "  FAIL: サンプル コンパイル $$opt"; exit 1; }
	  # libstdc++ を付けない gcc ドライバでリンクする(cctype が C++ランタイムに
	  # 依存しないこと、ヘッダのみで完結することの実証)
	  $(CC) -m32 -static $(LD32) $(BUILD)/sample-$$tag.o -o $(BUILD)/sample-$$tag \
	    || { echo "  FAIL: サンプル リンク $$opt"; exit 1; }
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(IINC) $(ISYS) -c $(BUILD)/control.cpp -o $(BUILD)/control-$$tag.o
	  $(CC) -m32 -static $(LD32) $(BUILD)/control-$$tag.o -o $(BUILD)/control-$$tag
	  # 実行: 引数なし・ありの両分岐が終了コード0
	  $(BUILD)/sample-$$tag && $(BUILD)/sample-$$tag arg \
	    || { echo "  FAIL: TC-CT-11 サンプル実行 $$opt"; exit 1; }
	  # (c) 未解決シンボル。C++ランタイム記号と分類関数への外部参照は常に禁止
	  undef=$$(nm -u $(BUILD)/sample-$$tag.o | awk '{print $$2}' | sort -u)
	  badu=$$(printf '%s\n' "$$undef" | grep -E '^(_Unwind_|__cxa_|__gxx_personality|_Zn|_ZTV|_ZTI|_ZTS|is)' || true)
	  if [ -n "$$badu" ]; then
	    echo "  FAIL: TC-CT-11(c) C++ランタイム/分類関数への未解決依存: $$badu"
	    exit 1
	  fi
	  if [ "$$tag" = "O2" ]; then
	    if [ -n "$$undef" ]; then
	      echo "  FAIL: TC-CT-11(c) -O2 で未解決シンボルが残る: $$undef"
	      exit 1
	    fi
	    echo "  PASS: TC-CT-11(c) -O2 未解決シンボルなし"
	  else
	    # -O0 では GOT のみ許容(PIE 既定の再配置。C++ランタイム依存ではない)
	    allowed=$$(printf '%s\n' "$$undef" | grep -v '^$$' | grep -v '^_GLOBAL_OFFSET_TABLE_$$' || true)
	    if [ -n "$$allowed" ]; then
	      echo "  FAIL: TC-CT-11(c) -O0 で想定外の未解決シンボル: $$allowed"
	      exit 1
	    fi
	    echo "  PASS: TC-CT-11(c) -O0 未解決は GOT のみ"
	  fi
	  # (a) 対照とのシンボル集合比較。-O2 は完全一致が必須(cctype 由来ゼロ)
	  if diff <(nm $(BUILD)/sample-$$tag | awk '{print $$2, $$3}' | sort) \
	           <(nm $(BUILD)/control-$$tag | awk '{print $$2, $$3}' | sort) \
	           > $(BUILD)/nmdiff-$$tag.txt; then
	    echo "  PASS: TC-CT-11(a) $$tag 対照とシンボル集合が同一"
	  else
	    # -O0 のみ: インライン関数の弱 COMDAT 実体は自己完結(依存ではない)。
	    # 対照外の追加がすべて弱(W)の cctype/ilibc 実体であることだけを確認する。
	    extra=$$(grep '^<' $(BUILD)/nmdiff-$$tag.txt | grep -vE '^< W (_ZSt7is|_ZN5ilibc6detail)' || true)
	    if [ -n "$$extra" ]; then
	      echo "  FAIL: TC-CT-11(a) $$tag 対照外のシンボル:"
	      echo "$$extra"
	      exit 1
	    fi
	    echo "  INFO: TC-CT-11(a) $$tag 弱COMDAT実体のみ(仕様外の追加なし):"
	    sed -n 1,20p $(BUILD)/nmdiff-$$tag.txt
	  fi
	  # (b) .rodata サイズが対照と同一 = cctype 由来の定数データゼロ(実装は表を持たない)
	  r_s=$$(readelf -SW $(BUILD)/sample-$$tag | awk '/\.rodata /{for(i=1;i<=NF;i++) if($$i=="PROGBITS"){print $$(i+3);exit}}')
	  r_c=$$(readelf -SW $(BUILD)/control-$$tag | awk '/\.rodata /{for(i=1;i<=NF;i++) if($$i=="PROGBITS"){print $$(i+3);exit}}')
	  if [ -n "$$r_s" ] && [ "$$r_s" = "$$r_c" ]; then
	    echo "  PASS: TC-CT-11(b) $$tag .rodata サイズ同一 (0x$$r_s バイト)"
	  else
	    echo "  FAIL: TC-CT-11(b) $$tag .rodata サイズ不一致 sample=0x$$r_s control=0x$$r_c"
	    exit 1
	  fi
	  # (c) 静的初期化: .init_array/.fini_array が対照( crt の分)と同一 = 追加ゼロ
	  i_s=$$(readelf -SW $(BUILD)/sample-$$tag | awk '/\.init_array /{for(i=1;i<=NF;i++) if($$i=="INIT_ARRAY"){print $$(i+3);exit}}')
	  i_c=$$(readelf -SW $(BUILD)/control-$$tag | awk '/\.init_array /{for(i=1;i<=NF;i++) if($$i=="INIT_ARRAY"){print $$(i+3);exit}}')
	  if [ -n "$$i_s" ] && [ "$$i_s" = "$$i_c" ]; then
	    echo "  PASS: TC-CT-11(c) $$tag .init_array 対照同一 (0x$$i_s バイト、静的初期化追加なし)"
	  else
	    echo "  FAIL: TC-CT-11(c) $$tag .init_array 不一致 sample=0x$$i_s control=0x$$i_c"
	    exit 1
	  fi
	done
	# ------------------------------------------------- <cstring> (TC-ST-12・13)
	@echo "=== 段階6: 成果物・リンク検査 TC-ST-12・13 (ilibc.a) ==="
	# ilibc.a を -O2 + NDIST で作る(A1/A2 のグローバルシンボル検査は最適化版が対象。
	# -O0 では detail 核が弱 COMDAT になるため -O2 で作る(cstring仕様 D-15))
	rm -f $(BUILD)/ilibc.a
	mkdir -p $(BUILD)/lib_O2
	for f in $(SRC_STR); do
	  b=$$(basename $$f .cpp)
	  $(CXX) -m32 -std=c++23 -O2 $(WARN) $(FREE) $(NDIST) -ffunction-sections \
	    $(ISYS) -I$(ROOT)/include -c $$f -o $(BUILD)/lib_O2/$$b.o \
	    || { echo "  FAIL: TC-ST-12 mem* コンパイル -O2 ($$b)"; exit 1; }
	done
	ar rcs $(BUILD)/ilibc.a $(BUILD)/lib_O2/*.o
	# A1: 定義するグローバルシンボルが mem* の5個だけ(弱シンボルも不可)。
	# 並びはバイト順( memcmp の index4 は 'm'、memchr は 'h' )
	diff <(nm --defined-only -g $(BUILD)/ilibc.a | awk '$$2 ~ /^[A-Z]$$/ {print $$2, $$3}' | sort -u) \
	     <(printf 'T memchr\nT memcmp\nT memcpy\nT memmove\nT memset\n') \
	  && echo "  PASS: TC-ST-12(a) グローバル定義は mem* 5個のみ(弱なし)" \
	  || { echo "  FAIL: TC-ST-12(a) グローバル定義シンボルが期待と異なる"; exit 1; }
	# A2: 各オブジェクトに未解決シンボルがない(mem*.o は自己完結、cstring仕様 §9)
	for f in $(BUILD)/lib_O2/*.o; do
	  if [ -n "$$(nm -u $$f)" ]; then
	    echo "  FAIL: TC-ST-12(b) 未解決シンボルがある: $$f"
	    nm -u $$f
	    exit 1
	  fi
	done
	echo "  PASS: TC-ST-12(b) 全オブジェクトで nm -u が空"
	# A3: 関数本体に mem* への再帰・外部呼び出しがない。再帰や外部参照はリロケーション
	# エントリで現れる(.text 内の局所ジャンプは `<名+オフセット>` 表記なので誤検出しない)
	for f in $(BUILD)/lib_O2/*.o; do
	  if objdump -r $$f | grep -E '(^|[[:space:]])(memcpy|memmove|memcmp|memchr|memset)(-|[[:space:]]|$$)' > $(BUILD)/a3_bad.txt; then
	    echo "  FAIL: TC-ST-12(a) mem* へのリロケーション参照がある: $$f"
	    cat $(BUILD)/a3_bad.txt
	    exit 1
	  fi
	done
	echo "  PASS: TC-ST-12(a) mem* への再帰・外部参照なし"
	# A4: C++ランタイム依存がない
	bad=$$(nm $(BUILD)/ilibc.a | grep -E '__cxa_|_Unwind_|__gxx_personality|_Znw|_Zna|_ZdlPv|_ZTV|_ZTI|_ZTS' || true)
	if [ -n "$$bad" ]; then
	  echo "  FAIL: TC-ST-12(c) C++ランタイム記号がある:"
	  echo "$$bad"
	  exit 1
	fi
	echo "  PASS: TC-ST-12(c) C++ランタイム記号なし"
	# A5: 静的初期化がない(.init_array/.fini_array/.ctors/.dtors、__cxa_guard)
	if readelf -SW $(BUILD)/ilibc.a | grep -E '\.(init_array|fini_array|ctors|dtors)[[:space:]]' > $(BUILD)/a5_bad.txt; then
	  echo "  FAIL: TC-ST-12(c) 静的初期化セクションがある:"
	  cat $(BUILD)/a5_bad.txt
	  exit 1
	fi
	bad=$$(nm $(BUILD)/ilibc.a | grep '__cxa_guard' || true)
	if [ -n "$$bad" ]; then
	  echo "  FAIL: TC-ST-12(c) __cxa_guard がある:"
	  echo "$$bad"
	  exit 1
	fi
	echo "  PASS: TC-ST-12(c) 静的初期化なし"
	# TC-ST-13: 利用者側リンク検査(L1: ilibc.a なしで未解決が mem* の部分集合、
	# L2: あれば0)。仕様のフラグ -ffreestanding -fno-builtin -nostdlib + OPTS 全レベル。
	for opt in $(OPTS); do
	  tag=$${opt#-}
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) -ffreestanding -fno-builtin -nostdlib \
	    $(ISYS) -I$(ROOT)/include -c $(TST)/link/cstring_sample.cpp -o $(BUILD)/str_sample-$$tag.o \
	    || { echo "  FAIL: TC-ST-13 サンプル コンパイル $$opt"; exit 1; }
	  $(CC) -m32 -r $(BUILD)/str_sample-$$tag.o -o $(BUILD)/str_l1-$$tag.o \
	    || { echo "  FAIL: TC-ST-13 L1 リンク $$opt"; exit 1; }
	  undef=$$(nm -u $(BUILD)/str_l1-$$tag.o | awk '{print $$2}' | sort -u)
	  badu=$$(printf '%s\n' "$$undef" | grep -v '^$$' | grep -vE '^(memcpy|memmove|memcmp|memchr|memset|_GLOBAL_OFFSET_TABLE_)$$' || true)
	  if [ -n "$$badu" ]; then
	    echo "  FAIL: TC-ST-13 L1 $$opt: mem* 以外の未解決がある:"
	    echo "$$badu"
	    exit 1
	  fi
	  $(CC) -m32 -r $(BUILD)/str_sample-$$tag.o $(BUILD)/ilibc.a -o $(BUILD)/str_l2-$$tag.o \
	    || { echo "  FAIL: TC-ST-13 L2 リンク $$opt"; exit 1; }
	  undef=$$(nm -u $(BUILD)/str_l2-$$tag.o | awk '{print $$2}' | sort -u | grep -v '^$$' | grep -v '^_GLOBAL_OFFSET_TABLE_$$' || true)
	  if [ -n "$$undef" ]; then
	    echo "  FAIL: TC-ST-13 L2 $$opt: ilibc.a ありで未解決が残る:"
	    echo "$$undef"
	    exit 1
	  fi
	  echo "  PASS: TC-ST-13 $$opt (L1 部分集合 / L2 ゼロ)"
	done
	# ------------------------------------------------- <cstdlib> (TC-SL-17)
	@echo "=== 段階6: 成果物・リンク検査 TC-SL-17 (ヘッダのみ、リンク符号なし) ==="
	# cstdlib はヘッダのみ(cstdlib仕様 §2.1: 定数式関数、リンク符号なし)。
	# 仕様のフラグ -ffreestanding -fno-builtin でコンパイルし、(a)未解決が空
	# (GOT のみ許容)、(b)C++ランタイム記号なし、(c).init_array 追加なしを検査する。
	for opt in -O2 -O0; do
	  tag=$${opt#-}
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) -ffreestanding -fno-builtin -nostdlib \
	    $(ISYS) -I$(ROOT)/include -c $(TST)/link/cstdlib_sample.cpp -o $(BUILD)/sl_sample-$$tag.o \
	    || { echo "  FAIL: TC-SL-17 サンプル コンパイル $$opt"; exit 1; }
	  $(CC) -m32 -r $(BUILD)/sl_sample-$$tag.o -o $(BUILD)/sl_l-$$tag.o \
	    || { echo "  FAIL: TC-SL-17 リンク $$opt"; exit 1; }
	  # (a) 未解決シンボル: GOT のみ許容(PIE 既定の再配置。ヘッダのみなので空が理想)
	  undef=$$(nm -u $(BUILD)/sl_sample-$$tag.o | awk '{print $$2}' | sort -u)
	  badu=$$(printf '%s\n' "$$undef" | grep -v '^$$' | grep -v '^_GLOBAL_OFFSET_TABLE_$$' || true)
	  if [ -n "$$badu" ]; then
	    echo "  FAIL: TC-SL-17(a) $$opt: 想定外の未解決シンボル:"
	    echo "$$badu"
	    exit 1
	  fi
	  # (b) C++ランタイム記号がない
	  bad=$$(nm $(BUILD)/sl_sample-$$tag.o | grep -E '__cxa_|_Unwind_|__gxx_personality|_Znw|_Zna|_ZdlPv' || true)
	  if [ -n "$$bad" ]; then
	    echo "  FAIL: TC-SL-17(b) $$opt: C++ランタイム記号がある:"
	    echo "$$bad"
	    exit 1
	  fi
	  # (c) 静的初期化がない(.init_array 追加なし)
	  if readelf -SW $(BUILD)/sl_sample-$$tag.o | grep -E '\.init_array[[:space:]]' > $(BUILD)/sl_ic_bad.txt; then
	    echo "  FAIL: TC-SL-17(c) $$opt: .init_array セクションがある:"
	    cat $(BUILD)/sl_ic_bad.txt
	    exit 1
	  fi
	  echo "  PASS: TC-SL-17 $$opt (未解決は GOT のみ / C++RT なし / .init_array なし)"
	done
	# ------------------------------------------------- <cassert> (TC-HD-16・17)
	@echo "=== 段階6: 成果物・リンク検査 TC-HD-16・17 (assert 失敗フック) ==="
	# ilibc_hooks.a: 既定の弱フック(src/hooks/assert_fail.cpp)を格納する。
	# ilibc.a には入れない(HDR §9.1、AST §4.2)。リンクコマンドの末尾に置く。
	rm -f $(BUILD)/ilibc_hooks.a
	$(CXX) -m32 -std=c++23 -O2 $(WARN) $(FREE) $(ISYS) -I$(ROOT)/include \
	  -c $(ROOT)/src/hooks/assert_fail.cpp -o $(BUILD)/assert_fail.o \
	  || { echo "  FAIL: ilibc_hooks.a コンパイル"; exit 1; }
	ar rcs $(BUILD)/ilibc_hooks.a $(BUILD)/assert_fail.o
	# A7(HDR §9.1/AST §4.2): 定義するシンボルが弱 __ilibc_assert_fail のみ
	diff <(nm --defined-only -g $(BUILD)/ilibc_hooks.a \
	         | awk '$$2 ~ /^[A-Z]$$/ {print $$2, $$3}' | sort -u) \
	     <(printf 'W __ilibc_assert_fail\n') \
	  && echo "  PASS: ilibc_hooks.a は弱 __ilibc_assert_fail のみを定義" \
	  || { echo "  FAIL: ilibc_hooks.a の定義シンボルが期待と異なる"; \
	       nm $(BUILD)/ilibc_hooks.a; exit 1; }
	# サンプル(有効な assert で __ilibc_assert_fail への参照を持つ)をコンパイル
	$(CXX) -m32 -std=c++23 -O0 $(WARN) $(FREE) $(ISYS) -I$(ROOT)/include \
	  -c $(TST)/link/assert_sample.cpp -o $(BUILD)/assert_sample.o \
	  || { echo "  FAIL: TC-HD-16 assert_sample コンパイル"; exit 1; }
	$(CXX) -m32 -std=c++23 -O0 $(WARN) $(FREE) $(ISYS) -I$(ROOT)/include \
	  -c $(TST)/link/assert_user_hook.cpp -o $(BUILD)/assert_user_hook.o \
	  || { echo "  FAIL: TC-HD-16 assert_user_hook コンパイル"; exit 1; }
	# (c) フックなし: 未解決シンボルでリンクエラーになる(実行はしない)
	if $(CC) -m32 -static $(LD32) $(BUILD)/assert_sample.o -o $(BUILD)/assert_nohook \
	     > $(BUILD)/assert_nohook.err 2>&1; then
	  echo "  FAIL: TC-HD-16(c) フックなしでリンクが成功してしまい不行"
	  exit 1
	fi
	if grep -q '__ilibc_assert_fail' $(BUILD)/assert_nohook.err; then
	  echo "  PASS: TC-HD-16(c) フックなしで __ilibc_assert_fail の未解決エラー"
	else
	  echo "  FAIL: TC-HD-16(c) エラーに __ilibc_assert_fail が現れない"
	  sed -n 1,10p $(BUILD)/assert_nohook.err
	  exit 1
	fi
	# (b) ilibc_hooks.a の既定弱シンボルで解決する(リンクのみ。復帰しないため実行しない)
	$(CC) -m32 -static $(LD32) $(BUILD)/assert_sample.o $(BUILD)/ilibc_hooks.a \
	  -o $(BUILD)/assert_weak \
	  || { echo "  FAIL: TC-HD-16(b) 既定弱フックでのリンク"; exit 1; }
	if [ "$$(nm $(BUILD)/assert_weak | awk '$$3=="__ilibc_assert_fail"{print $$2}')" = "W" ]; then
	  echo "  PASS: TC-HD-16(b) 既定の弱シンボルで解決"
	else
	  echo "  FAIL: TC-HD-16(b) __ilibc_assert_fail のシンボル種別が W ではない"
	  nm $(BUILD)/assert_weak | grep assert_fail
	  exit 1
	fi
	# (a) 利用者の強フック: アーカイブを併記しても衝突せず、強い定義が使われる
	$(CC) -m32 -static $(LD32) $(BUILD)/assert_user_hook.o $(BUILD)/ilibc_hooks.a \
	  -o $(BUILD)/assert_strong \
	  || { echo "  FAIL: TC-HD-16(a) 強フック+アーカイブのリンク"; exit 1; }
	if [ "$$(nm $(BUILD)/assert_strong | awk '$$3=="__ilibc_assert_fail"{print $$2}')" = "T" ]; then
	  echo "  PASS: TC-HD-16(a) 利用者の強シンボルが使われる"
	else
	  echo "  FAIL: TC-HD-16(a) __ilibc_assert_fail のシンボル種別が T ではない"
	  nm $(BUILD)/assert_strong | grep assert_fail
	  exit 1
	fi
	# TC-HD-17: ilibc.a が __ilibc_assert_fail を定義しない(HDR §9.1、AST §4.2。
	# A1 の「グローバル定義は mem* 5個のみ」に加えて明示的に検査する)
	if nm --defined-only $(BUILD)/ilibc.a | grep -q '__ilibc_assert_fail'; then
	  echo "  FAIL: TC-HD-17 ilibc.a が __ilibc_assert_fail を定義している"
	  nm --defined-only $(BUILD)/ilibc.a | grep '__ilibc_assert_fail'
	  exit 1
	fi
	echo "  PASS: TC-HD-17 ilibc.a は __ilibc_assert_fail を定義しない"
	# ------------------------------------------------ <cerrno> (TC-HD-09・10)
	@echo "=== 段階6: 成果物・リンク検査 TC-HD-09・10 (errno の実体と初期化) ==="
	# (c)(d) 2翻訳単位のリンクと実行: 初期値0、g_errno の実体共有(R環境)。
	# ヘッダのみを使うため C++RT・mem* は不要で、静的リンクだけで完結する
	$(CXX) -m32 -std=c++23 -O2 $(WARN) $(FREE) $(ISYS) $(IINC) \
	  -c $(TST)/link/errno_tu_a.cpp -o $(BUILD)/errno_tu_a.o \
	  || { echo "  FAIL: TC-HD-09 errno_tu_a コンパイル"; exit 1; }
	$(CXX) -m32 -std=c++23 -O2 $(WARN) $(FREE) $(ISYS) $(IINC) \
	  -c $(TST)/link/errno_tu_b.cpp -o $(BUILD)/errno_tu_b.o \
	  || { echo "  FAIL: TC-HD-09 errno_tu_b コンパイル"; exit 1; }
	$(CC) -m32 -static $(LD32) $(BUILD)/errno_tu_a.o $(BUILD)/errno_tu_b.o \
	  -o $(BUILD)/errno_tu \
	  || { echo "  FAIL: TC-HD-09 2翻訳単位のリンク"; exit 1; }
	rc=0
	$(BUILD)/errno_tu || rc=$$?
	if [ $$rc -eq 0 ]; then
	  echo "  PASS: TC-HD-09(c)(d) 初期値0 と実体共有を実行で確認"
	else
	  echo "  FAIL: TC-HD-09(c)(d) 実行 (exit $$rc)"
	  exit 1
	fi
	# nm: g_errno の定義が1個だけであること(A環境、TC-HD-09(d))。inline 変数は
	# GNU unique(u、弱に由来する)または W で現れるため両方を許容する。サイズは4。
	g_lines=$$(nm --defined-only -S $(BUILD)/errno_tu \
	  | awk '$$4 ~ /g_errno/ {print $$2, $$3, $$4}')
	g_cnt=$$(printf '%s\n' "$$g_lines" | grep -c . || true)
	if [ "$$g_cnt" -eq 1 ] && printf '%s\n' "$$g_lines" | grep -qE '^0*4 [Wu] _'; then
	  echo "  PASS: TC-HD-09(d) g_errno 実体1個(サイズ4、弱系シンボル、.bss)"
	else
	  echo "  FAIL: TC-HD-09(d) g_errno の定義が期待と異なる:"
	  echo "$$g_lines"
	  exit 1
	fi
	# .bss 配置と __cxa_guard なし(TC-TL-04 と同証跡。TC-TL 本体は引き続き範囲外)
	if objdump -t $(BUILD)/errno_tu | grep 'g_errno' | grep -q '\.bss'; then
	  echo "  PASS: TC-HD-09(d) g_errno は .bss に配置"
	else
	  echo "  FAIL: TC-HD-09(d) g_errno が .bss にない"
	  objdump -t $(BUILD)/errno_tu | grep 'g_errno'
	  exit 1
	fi
	if nm $(BUILD)/errno_tu | grep -q '__cxa_guard'; then
	  echo "  FAIL: TC-HD-09(d) __cxa_guard_* がある"
	  nm $(BUILD)/errno_tu | grep '__cxa_guard'
	  exit 1
	fi
	echo "  PASS: TC-HD-09(d) __cxa_guard_* なし"
	# TC-HD-10: <cerrno> を使うオブジェクトに静的初期化がない(概要仕様書 §12.2)。
	# .init_array はリンク後でも crt の分しか出ないため、オブジェクトで検査する
	# (TC-SL-17(c) と同様の立場)
	for o in $(BUILD)/errno_tu_a.o $(BUILD)/errno_tu_b.o; do
	  if readelf -SW $$o | grep -E '\.init_array[[:space:]]' > $(BUILD)/hd10_bad.txt; then
	    echo "  FAIL: TC-HD-10 $$o に .init_array がある"
	    cat $(BUILD)/hd10_bad.txt
	    exit 1
	  fi
	  if nm $$o | grep -E '__cxa_guard|_GLOBAL__sub_I' > $(BUILD)/hd10_guard.txt; then
	    echo "  FAIL: TC-HD-10 $$o に静的初期化記号がある"
	    cat $(BUILD)/hd10_guard.txt
	    exit 1
	  fi
	done
	echo "  PASS: TC-HD-10 静的コンストラクタ・__cxa_guard・.init_array なし"

# 段階7: 基準値の記録 TC-ST-14(M: 記録のみ。-Os と -O2 のコードサイズ・スタック使用量)
sizes: check-toolchain
	@echo "=== 段階7: 基準値の記録 TC-ST-14 (-Os / -O2、情報) ==="
	for opt in -Os -O2; do
	  tag=$${opt#-}
	  rm -rf $(BUILD)/sz_$$tag
	  mkdir -p $(BUILD)/sz_$$tag
	  for f in $(SRC_STR); do
	    b=$$(basename $$f .cpp)
	    $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(NDIST) $(ISYS) -I$(ROOT)/include \
	      -fstack-usage -c $$f -o $(BUILD)/sz_$$tag/$$b.o \
	      || { echo "  FAIL: TC-ST-14 mem* コンパイル $$opt"; exit 1; }
	  done
	  echo "  $$opt mem* コードサイズ (nm --print-size):"
	  nm --print-size $(BUILD)/sz_$$tag/*.o | awk '$$3=="T"{printf "    %s size=0x%s\n", $$4, $$2}' | sort
	  echo "  $$opt mem* スタック使用量 (-fstack-usage):"
	  cat $(BUILD)/sz_$$tag/*.su | sed 's/^/    /'
	  # 文字列系は、非インライン化したときのサイズを記録する(TC-ST-14)。
	  # アドレス取得で out-of-line コピーを強制する(プローブは -c のみ、ilibc.a には入れない)
	  $(CXX) -m32 -std=c++23 $$opt $(WARN) $(FREE) $(NDIST) $(ISYS) -I$(ROOT)/include \
	    -c $(TST)/link/cstring_sizes_probe.cpp -o $(BUILD)/sz_$$tag/probe.o \
	    || { echo "  FAIL: TC-ST-14 文字列系プローブ コンパイル $$opt"; exit 1; }
	  echo "  $$opt 文字列系(非インライン化、demangle 済):"
	  nm --print-size $(BUILD)/sz_$$tag/probe.o \
	    | awk '$$3=="T"||$$3=="W"{printf "    %s %s size=0x%s\n", $$4, $$3, $$2}' \
	    | c++filt | grep -v 'get_pc_thunk' | sort || true
	done
	echo "  記録: $(BUILD)/sz_* (TC-ST-14)"

# 任意: カバレッジ計測(TST §7.2。E1 -O0 で gcov。情報として記録し check には含めない)
coverage: check-toolchain
	@echo "=== カバレッジ計測 (TST §7.2, E1 -O0, 任意ターゲット) ==="
	command -v gcov > /dev/null || { echo "ERROR: gcov がない"; exit 1; }
	rm -f $(BUILD)/*.gcda $(BUILD)/*.gcno $(BUILD)/cov_*.gcda $(BUILD)/cov_*.gcno
	$(CXX) -m32 -std=c++23 -O0 --coverage $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/cases/cctype_runtime.cpp -o $(BUILD)/cov_rt.o
	$(CXX) -m32 -std=c++23 -O0 --coverage $(WARN) $(FREE) $(ISYS) $(IINC) -c $(TST)/target/platform/e1_stdio.cpp -o $(BUILD)/cov_e1.o
	$(CC) -m32 -static --coverage $(LD32) $(BUILD)/cov_rt.o $(BUILD)/cov_e1.o -o $(BUILD)/cov_rt \
	  || { echo "  FAIL: カバレッジ版ビルド(cctype)"; exit 1; }
	# <cstring> 版も計測する(NDIST は mem* のビルドフラグ、cstring仕様 §3)
	mkdir -p $(BUILD)/cov_str
	for f in $(SRC_STR); do
	  b=$$(basename $$f .cpp)
	  $(CXX) -m32 -std=c++23 -O0 --coverage $(WARN) $(FREE) $(NDIST) $(ISYS) -I$(ROOT)/include \
	    -c $$f -o $(BUILD)/cov_str/$$b.o \
	    || { echo "  FAIL: カバレッジ版 コンパイル $$b"; exit 1; }
	done
	$(CXX) -m32 -std=c++23 -O0 --coverage $(WARN) $(FREE) $(NDIST) $(ISYS) $(IINC) \
	  -c $(TST)/target/cases/cstring_runtime.cpp -o $(BUILD)/cov_str/cstring_runtime.o \
	  || { echo "  FAIL: カバレッジ版 コンパイル cstring_runtime"; exit 1; }
	$(CXX) -m32 -std=c++23 -O0 --coverage $(WARN) $(FREE) $(ISYS) $(IINC) \
	  -c $(TST)/target/platform/e1_stdio.cpp -o $(BUILD)/cov_str/e1.o \
	  || { echo "  FAIL: カバレッジ版 コンパイル e1_stdio(cstring)"; exit 1; }
	$(CC) -m32 -static --coverage $(LD32) $(BUILD)/cov_str/*.o -o $(BUILD)/cov_str_rt \
	  || { echo "  FAIL: カバレッジ版ビルド(cstring)"; exit 1; }
	# <cstdlib> 版も計測する(ヘッダのみ、リンク符号なし)
	mkdir -p $(BUILD)/cov_sl
	$(CXX) -m32 -std=c++23 -O0 --coverage $(WARN) $(FREE) $(ISYS) $(IINC) \
	  -c $(TST)/target/cases/cstdlib_runtime.cpp -o $(BUILD)/cov_sl/cstdlib_runtime.o \
	  || { echo "  FAIL: カバレッジ版 コンパイル cstdlib_runtime"; exit 1; }
	$(CXX) -m32 -std=c++23 -O0 --coverage $(WARN) $(FREE) $(ISYS) $(IINC) \
	  -c $(TST)/target/platform/e1_stdio.cpp -o $(BUILD)/cov_sl/e1.o \
	  || { echo "  FAIL: カバレッジ版 コンパイル e1_stdio(cstdlib)"; exit 1; }
	$(CC) -m32 -static --coverage $(LD32) $(BUILD)/cov_sl/*.o -o $(BUILD)/cov_sl_rt \
	  || { echo "  FAIL: カバレッジ版ビルド(cstdlib)"; exit 1; }
	rc=0
	$(BUILD)/cov_rt > $(BUILD)/cov.out 2>&1 || rc=$$?
	cat $(BUILD)/cov.out
	[ $$rc -eq 0 ] || { echo "  FAIL: カバレッジ版の実行(cctype) (exit $$rc)"; exit 1; }
	rc=0
	$(BUILD)/cov_str_rt > $(BUILD)/cov_str.out 2>&1 || rc=$$?
	cat $(BUILD)/cov_str.out
	[ $$rc -eq 0 ] || { echo "  FAIL: カバレッジ版の実行(cstring) (exit $$rc)"; exit 1; }
	rc=0
	$(BUILD)/cov_sl_rt > $(BUILD)/cov_sl.out 2>&1 || rc=$$?
	cat $(BUILD)/cov_sl.out
	[ $$rc -eq 0 ] || { echo "  FAIL: カバレッジ版の実行(cstdlib) (exit $$rc)"; exit 1; }
	# gcov はソースパスをリポジトリルート基準の相対で持つため、ルートで実行して
	# 生成した .gcov を build/ に移動する(リポジトリ直下を汚さない)
	cd $(ROOT) && gcov $(BUILD)/cov_rt.gcda $(BUILD)/cov_e1.gcda \
	  $(BUILD)/cov_str/*.gcda $(BUILD)/cov_sl/*.gcda \
	  | tee $(BUILD)/gcov_summary.txt
	find $(ROOT) -maxdepth 1 -name '*.gcov' -exec mv -t $(BUILD)/ {} +
	echo "  gcov 出力: $(BUILD)/*.gcov / 要約: $(BUILD)/gcov_summary.txt"

# 既定ターゲット: TST §4 のゲート順に実行する。
# 段階2(headers)の失敗はハードゲート(以降を実行しない)。
# 段階3〜7の失敗は記録して続行し、最後に非ゼロで終了する。
check: env-info check-toolchain
	@st=0
	for t in headers constexpr compilefail host sanitize runtime formatio oneshot artifact sizes; do
	  echo ""
	  echo "---------- 段階: $$t ----------"
	  if $(MAKE) --no-print-directory $$t; then
	    :
	  else
	    if [ "$$t" = "headers" ]; then
	      echo ""
	      echo "GATE: 段階2(ヘッダ・ビルド試験)失敗のため、以降を実行しない (TST §4)"
	      exit 1
	    fi
	    st=1
	  fi
	done
	echo ""
	if [ $$st -eq 0 ]; then
	  echo "CHECK PASS: 段階2〜7すべて成功"
	else
	  echo "CHECK FAIL: 上記 FAIL を参照 (TST §4: 記録して続行)"
	fi
	exit $$st

clean:
	rm -rf $(BUILD)
