# Chapter 3. 프로세스 관리

## 프로세스 생성의 목적

- 목적 1 : 같은 프로그램의 처리를 여러 프로세스가 분산해서 처리 (웹 서버의 리퀘스트 처리 등)

- 목적 2 : 전혀 다른 프로그램 생성 (bash에서 여러 프로그램 실행 등)

- 위의 작업을 할 때 **fork()** 함수와 **execve()** 함수가 호출되며, 내부적으로는 **clone()** 과 **execve()** 시스템 콜이 호출된다.

---

## fork() 함수

- 실행 중인 프로세스에서 자식 프로세스를 생성한다.

### 프로세스 생성 순서

1. 자식 프로세스용 메모리 영역을 작성하고 부모 프로세스의 메모리를 복사한다.

2. **fork()** 함수의 리턴 값이 다른 것을 이용하여 코드를 분기한다.

### fork.c 프로그램 예제

1. 프로세스를 새로 만든다.
2. 부모 프로세스는 자신의 PID<sub>Process ID</sub>와 자식 프로세스의 PID를 출력한다.

```sh
$ gcc -o output/fork src/fork.c
$ output/fork
부모 프로세스의 PID : 13202, 자식 프로세스의 PID : 13203
자식 프로세스의 PID : 13203
```

- 부모 프로세스(13202)가 분기되어 자식 프로세스(13203)이 생성된 것을 볼 수 있고, 각각의 출력을 통해 두 프로세스 처리가 분기되어 실행되는 것을 알 수 있다.

---

## execve() 함수

- 전혀 다른 프로세스를 생성하는 함수이다.

### 함수의 흐름

1. 실행 파일을 읽어서 메모리 맵에 정보를 로딩한다.
2. 현재 프로세스의 메모리 영역을 새 프로세스의 데이터로 덮어쓴다.
3. 새로운 프로세스의 첫 번째 명령부터 실행한다.

- 프로세스의 수가 증가하는 것이 아니고, 현재 프로세스를 다른 프로세스로 변경하는 것이다.

### 실행 파일의 구성 요소

- 실행 파일은 코드와 데이터 외에도 다음과 같은 정보들이 필요하다.

  - 코드 영역의 파일상 오프셋, 사이즈, 메모리 맵 시작 주소
  
  - 데이터 영역의 파일상 오프셋, 사이즈, 메모리 맵 시작 주소

  - 엔트리 포인트 (최초로 실행할 명령의 주소)

- 프로그램을 실행하면 위의 정보들을 바탕으로 메모리에 매핑하고, 엔트리 포인트부터 명령을 실행한다.

- 리눅스의 실행 파일은 ELF<sub>Executable and Linkable Format</sub> 형식을 사용하는데, 이는 **readelf** 명령어로 볼 수 있다.

- -h 옵션 : ELF 파일 헤더 출력. 엔트리 포인트를 볼 수 있다.

  ```elf
  $ readelf -h /bin/sleep
  ELF Header:
    Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
    Class:                             ELF64
    Data:                              2's complement, little endian
    Version:                           1 (current)
    OS/ABI:                            UNIX - System V
    ABI Version:                       0
    Type:                              DYN (Shared object file)
    Machine:                           Advanced Micro Devices X86-64
    Version:                           0x1
    Entry point address:               0x2850
    Start of program headers:          64 (bytes into file)
    Start of section headers:          37336 (bytes into file)
    Flags:                             0x0
    Size of this header:               64 (bytes)
    Size of program headers:           56 (bytes)
    Number of program headers:         13
    Size of section headers:           64 (bytes)
    Number of section headers:         30
    Section header string table index: 29
  ```

    - Entry point address 항목이 엔트리 포인트의 주소이다.

- -S 옵션 : 섹션 헤더 출력. 코드와 데이터 영역의 파일상 오프셋, 사이즈, 메모리 맵 시작 주소를 볼 수 있다.

  ```elf
  $ readelf -S /bin/sleep
  There are 30 section headers, starting at offset 0x91d8:

  Section Headers:
    [Nr] Name              Type             Address           Offset
        Size              EntSize          Flags  Link  Info  Align
    [ 0]                   NULL             0000000000000000  00000000
        0000000000000000  0000000000000000           0     0     0
    [ 1] .interp           PROGBITS         0000000000000318  00000318
        000000000000001c  0000000000000000   A       0     0     1
    [ 2] .note.gnu.propert NOTE             0000000000000338  00000338
        0000000000000020  0000000000000000   A       0     0     8
    [ 3] .note.gnu.build-i NOTE             0000000000000358  00000358
        0000000000000024  0000000000000000   A       0     0     4
    [ 4] .note.ABI-tag     NOTE             000000000000037c  0000037c
        0000000000000020  0000000000000000   A       0     0     4
    [ 5] .gnu.hash         GNU_HASH         00000000000003a0  000003a0
        00000000000000a8  0000000000000000   A       6     0     8
    [ 6] .dynsym           DYNSYM           0000000000000448  00000448
        0000000000000600  0000000000000018   A       7     1     8
    [ 7] .dynstr           STRTAB           0000000000000a48  00000a48
        000000000000031f  0000000000000000   A       0     0     1
    [ 8] .gnu.version      VERSYM           0000000000000d68  00000d68
        0000000000000080  0000000000000002   A       6     0     2
    [ 9] .gnu.version_r    VERNEED          0000000000000de8  00000de8
        0000000000000060  0000000000000000   A       7     1     8
    [10] .rela.dyn         RELA             0000000000000e48  00000e48
        00000000000002b8  0000000000000018   A       6     0     8
    [11] .rela.plt         RELA             0000000000001100  00001100
        00000000000003f0  0000000000000018  AI       6    25     8
    [12] .init             PROGBITS         0000000000002000  00002000
        000000000000001b  0000000000000000  AX       0     0     4
    [13] .plt              PROGBITS         0000000000002020  00002020
        00000000000002b0  0000000000000010  AX       0     0     16
    [14] .plt.got          PROGBITS         00000000000022d0  000022d0
        0000000000000010  0000000000000010  AX       0     0     16
    [15] .plt.sec          PROGBITS         00000000000022e0  000022e0
        00000000000002a0  0000000000000010  AX       0     0     16
    [16] .text             PROGBITS         0000000000002580  00002580
        0000000000003692  0000000000000000  AX       0     0     16
    [17] .fini             PROGBITS         0000000000005c14  00005c14
        000000000000000d  0000000000000000  AX       0     0     4
    [18] .rodata           PROGBITS         0000000000006000  00006000
        0000000000000f6c  0000000000000000   A       0     0     32
    [19] .eh_frame_hdr     PROGBITS         0000000000006f6c  00006f6c
        00000000000002b4  0000000000000000   A       0     0     4
    [20] .eh_frame         PROGBITS         0000000000007220  00007220
        0000000000000d18  0000000000000000   A       0     0     8
    [21] .init_array       INIT_ARRAY       0000000000009bb0  00008bb0
        0000000000000008  0000000000000008  WA       0     0     8
    [22] .fini_array       FINI_ARRAY       0000000000009bb8  00008bb8
        0000000000000008  0000000000000008  WA       0     0     8
    [23] .data.rel.ro      PROGBITS         0000000000009bc0  00008bc0
        00000000000000b8  0000000000000000  WA       0     0     32
    [24] .dynamic          DYNAMIC          0000000000009c78  00008c78
        00000000000001f0  0000000000000010  WA       7     0     8
    [25] .got              PROGBITS         0000000000009e68  00008e68
        0000000000000190  0000000000000008  WA       0     0     8
    [26] .data             PROGBITS         000000000000a000  00009000
        0000000000000080  0000000000000000  WA       0     0     32
    [27] .bss              NOBITS           000000000000a080  00009080
        00000000000001b8  0000000000000000  WA       0     0     32
    [28] .gnu_debuglink    PROGBITS         0000000000000000  00009080
        0000000000000034  0000000000000000           0     0     4
    [29] .shstrtab         STRTAB           0000000000000000  000090b4
        000000000000011d  0000000000000000           0     0     1
  Key to Flags:
    W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
    L (link order), O (extra OS processing required), G (group), T (TLS),
    C (compressed), x (unknown), o (OS specific), E (exclude),
    l (large), p (processor specific)
  ```

    - [16] .text 영역과 [26] .data 영역이 각각 코드와 데이터 영역의 정보이다.

    - Address : 메모리 맵 시작 주소

    - Offset : 파일상 오프셋

    - Size : 사이즈

- 즉 */bin/sleep* 파일의 정보는 위의 내용에 의하면 다음과 같다.

  |구성|값 (16진수)|
  |:-:|:-:|
  |엔트리 포인트|0x2850|
  |코드 영역의 메모리 맵 시작 주소|0000000000002580|
  |코드 영역의 파일상 오프셋|00002580|
  |코드 영역의 사이즈|0000000000003692|
  |데이터 영역의 메모리 맵 시작 주소|000000000000a000|
  |데이터 영역의 파일상 오프셋|00009000|
  |데이터 영역의 사이즈|0000000000000080|

- */proc/PID번호/maps* 파일을 통해 프로세스의 메모리 맵을 볼 수 있다.

  ```sh
  $ /bin/sleep 10000 &
  [1] 16292
  $ cat /proc/16292/maps
  55addbc8d000-55addbc8f000 r--p 00000000 103:05 2098267                   /usr/bin/sleep
  55addbc8f000-55addbc93000 r-xp 00002000 103:05 2098267                   /usr/bin/sleep  # 코드 영역
  55addbc93000-55addbc95000 r--p 00006000 103:05 2098267                   /usr/bin/sleep
  55addbc96000-55addbc97000 r--p 00008000 103:05 2098267                   /usr/bin/sleep
  55addbc97000-55addbc98000 rw-p 00009000 103:05 2098267                   /usr/bin/sleep  # 데이터 영역
  55addcff5000-55addd016000 rw-p 00000000 00:00 0                          [heap]
  7f27b5a7a000-7f27b612f000 r--p 00000000 103:05 2097617                   /usr/lib/locale/locale-archive
  7f27b612f000-7f27b6154000 r--p 00000000 103:05 2099337                   /usr/lib/x86_64-linux-gnu/libc-2.31.so
  7f27b6154000-7f27b62cc000 r-xp 00025000 103:05 2099337                   /usr/lib/x86_64-linux-gnu/libc-2.31.so
  7f27b62cc000-7f27b6316000 r--p 0019d000 103:05 2099337                   /usr/lib/x86_64-linux-gnu/libc-2.31.so
  7f27b6316000-7f27b6317000 ---p 001e7000 103:05 2099337                   /usr/lib/x86_64-linux-gnu/libc-2.31.so
  7f27b6317000-7f27b631a000 r--p 001e7000 103:05 2099337                   /usr/lib/x86_64-linux-gnu/libc-2.31.so
  7f27b631a000-7f27b631d000 rw-p 001ea000 103:05 2099337                   /usr/lib/x86_64-linux-gnu/libc-2.31.so
  7f27b631d000-7f27b6323000 rw-p 00000000 00:00 0 
  7f27b6334000-7f27b6335000 r--p 00000000 103:05 2099333                   /usr/lib/x86_64-linux-gnu/ld-2.31.so
  7f27b6335000-7f27b6358000 r-xp 00001000 103:05 2099333                   /usr/lib/x86_64-linux-gnu/ld-2.31.so
  7f27b6358000-7f27b6360000 r--p 00024000 103:05 2099333                   /usr/lib/x86_64-linux-gnu/ld-2.31.so
  7f27b6361000-7f27b6362000 r--p 0002c000 103:05 2099333                   /usr/lib/x86_64-linux-gnu/ld-2.31.so
  7f27b6362000-7f27b6363000 rw-p 0002d000 103:05 2099333                   /usr/lib/x86_64-linux-gnu/ld-2.31.so
  7f27b6363000-7f27b6364000 rw-p 00000000 00:00 0 
  7ffd9fb3e000-7ffd9fb5f000 rw-p 00000000 00:00 0                          [stack]
  7ffd9fb67000-7ffd9fb6b000 r--p 00000000 00:00 0                          [vvar]
  7ffd9fb6b000-7ffd9fb6d000 r-xp 00000000 00:00 0                          [vdso]
  ffffffffff600000-ffffffffff601000 --xp 00000000 00:00 0                  [vsyscall]

  $ kill 16292
  [1]  + 16292 terminated  /bin/sleep 10000 
  ```

    - 각 항목은 다음과 같은 순서이다.

      ```sh
      address   perms   offset   dev   inode   pathname
      ```

    - 코드 영역 : r-xp로 읽기, 실행 권한이 주어진다.

    - 데이터 영역 : rw-p로 읽기, 쓰기 권한이 주어진다.

    - 책이나 인터넷에서는 address 부분이 가상 메모리로 깔끔하게 00400000 처럼 주어지는데, 나는 왜 저렇게 나오는지 모르겠다.

    - offset을 참고해서 보면, 코드 영역과 데이터 영역의 메모리 맵 시작 주소가 해당 범위에 들어가 있음을 볼 수 있다.

    - 참고 : <https://stackoverflow.com/questions/1401359/understanding-linux-proc-pid-maps-or-proc-self-maps>

### fork and exec

- 전혀 다른 프로세스를 생성할 때는, 부모가 될 프로세스로부터 fork() 함수를 호출한 다음 자식 프로세스가 exec() 함수를 호출하는 방식을 주로 사용한다.

  ```sh
  $ gcc -o output/fork-and-exec src/fork-and-exec.c
  $ output/fork-and-exec
  부모 프로세스의 PID : 18241, 자식 프로세스의 PID : 18242
  자식 프로세스의 PID : 18242
  hello
  ```

- 파이썬에서는 **OS.exec()** 함수를 통해 execve() 함수를 호출할 수 있다.

---

## 종료 처리

- 프로그램 종료는 **_exit()** 함수를 통해 이루어진다.

  - 내부적으로 **exit_group()** 시스템 콜을 호출한다.

  - 이를 통해 프로세스에 할당된 메모리를 전부 회수한다.

- 보통은 libc의 **exit()** 함수를 호출하여 종료하며, 이 함수는 자신의 종료 처리를 전부 수행한 후 _exit() 함수를 호출한다.

  - **main()** 함수로부터 리턴된 경우도 같은 동작을 수행한다.
