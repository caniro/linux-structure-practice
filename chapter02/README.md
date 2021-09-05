# 2장 실습

## 시스템 콜 호출 동작 순서

### strace - hello.c

- *hello.c* 컴파일

  ```sh
  $ gcc -o output/hello src/hello.c
  $ output/hello
  Hello world
  ```

- **strace** 로 시스템 콜 확인

  ```sh
  $ strace -o log/hello.log output/hello
  Hello world
  ```

- *hello.log* 파일의 34번째 줄에서 write 시스템 콜을 통해 화면에 출력하는 것을 알 수 있다.

### strace - hello.py

- strace로 *hello.py* 실행

  ```sh
  $ strace -o log/hello.py.log python src/hello.py
  Hello world
  ```

- *hello.py.log* 파일의 516번째 줄에서 똑같이 write 시스템 콜을 호출하는 것을 볼 수 있다.

  - C언어보다 파이썬이 상대적으로 느리다는 것은 알고 있었는데, 생각보다 파이썬의 처리량이 훨씬 많다.

### sar - loop.c

- 시스템 콜을 호출하지 않고 무한루프를 도는 프로그램이다.

  ```sh
  $ gcc -o output/loop src/loop.c
  $ output/loop &                 # 백그라운드로 실행
  [1] 30132

  $ sar -P ALL 1 1
  Linux 5.11.0-27-generic (ubun2-GL63-8RC) 	2021년 09월 06일 	_x86_64_	(8 CPU)

  01시 31분 19초     CPU     %user     %nice   %system   %iowait    %steal     %idle
  01시 31분 20초     all      0.13     12.59      0.00      0.00      0.00     87.28
  01시 31분 20초       0      0.00      0.00      0.00      0.00      0.00    100.00
  01시 31분 20초       1      0.00    100.00      0.00      0.00      0.00      0.00 # 여기
  01시 31분 20초       2      0.00      0.00      0.00      0.00      0.00    100.00
  01시 31분 20초       3      0.00      0.00      0.00      0.00      0.00    100.00
  01시 31분 20초       4      0.00      0.00      0.00      0.00      0.00    100.00
  01시 31분 20초       5      0.00      0.00      0.00      0.00      0.00    100.00
  01시 31분 20초       6      0.00      0.00      0.00      0.00      0.00    100.00
  01시 31분 20초       7      0.99      0.00      0.00      0.00      0.00     99.01
  ...
  $ kill 30132                    # 실행한 프로세스 종료
  [1]  + 30132 terminated  output/loop
  ```

- 1번 프로세서에서 nice가 100%이고 system은 0%임을 볼 수 있다. 즉 유저 모드에서만 돌아가는 프로그램이라는 뜻이다.

  - user와 nice는 모두 유저 모드에서 작동한 시간의 비율을 나타내는데, nice는 프로세스의 우선순위와 관련있다.

### sar - ppidloop.c

- **getppid()** : 부모 프로세스의 PID(Process ID)를 얻는 시스템 콜

  ```sh
  $ gcc -o output/ppidloop src/ppidloop.c
  $ output/ppidloop &
  [1] 31624

  $ sar -P ALL 1 1
  Linux 5.11.0-27-generic (ubun2-GL63-8RC) 	2021년 09월 06일 	_x86_64_	(8 CPU)

  01시 44분 03초     CPU     %user     %nice   %system   %iowait    %steal     %idle
  01시 44분 04초     all      0.00      8.53      4.14      0.00      0.00     87.33
  01시 44분 04초       0      0.00      0.00      1.01      0.00      0.00     98.99
  01시 44분 04초       1      0.00      0.00      0.00      0.00      0.00    100.00
  01시 44분 04초       2      0.00     68.00     32.00      0.00      0.00      0.00 # 여기
  01시 44분 04초       3      0.00      0.00      0.00      0.00      0.00    100.00
  01시 44분 04초       4      0.00      0.00      0.00      0.00      0.00    100.00
  01시 44분 04초       5      0.00      0.00      0.00      0.00      0.00    100.00
  01시 44분 04초       6      0.00      0.00      0.00      0.00      0.00    100.00
  01시 44분 04초       7      0.00      0.00      0.00      0.00      0.00    100.00
  ...
  $ kill 31624
  [1]  + 31624 terminated  output/ppidloop
  ```

- 아까의 결과는 nice가 100%였던 것에 비해, 이번에는 system이 약 32%에 도달한 것을 볼 수 있다. (값은 실행 환경에 따라 다르다.)

### 시스템 콜의 소요 시간

- strace 명령어에 **-T** 옵션을 주면 시스템 콜 처리에 걸린 시간을 마이크로초까지 정밀하게 볼 수 있다. (단위는 초)

  ```sh
  $ strace -T -o log/hello_T.log output/hello
  Hello world
  ```

    - *hello_T.log* 파일을 보면 34번째 줄의 write 시스템 콜에서 0.000018초(18us)인 것을 볼 수 있다.

      - 출력 함수가 되게 느린 줄 알았는데 생각보다 빠르다.

- strace 명령어에 **-tt** 옵션을 주면 시스템 콜이 호출된 시간을 볼 수 있다.

  ```sh
  $ strace -tt -o log/hello_tt.log output/hello
  Hello world
  ```

  ```sh
  $ strace -T -tt -o log/hello_T_tt.log output/hello
  Hello world
  ```

---

## 시스템 콜의 wrapper 함수

### 어셈블리 코드

- 시스템 콜은 일반 함수 호출과는 다르게, 아키텍처에 의존하는 어셈블리 코드를 통해 호출해야 한다.

  - x86_64 아키텍처에서 **getppid()** 시스템 콜은 아래처럼 호출된다.

    ```asm
    mov $0x6e, $eax
    syscall
    ```
    
      - eax 레지스터에 getppid의 시스템 콜 번호 0x6e를 넣고 시스템 콜을 호출하면서 커널 모드로 전환하는 것이다.

- 어셈블리 코드는 아키텍처에 의존하기 때문에 각 아키텍처마다 시스템 콜을 호출하는 방식이 다르고, 이를 통일하기 위해 OS에서 wrapper 함수를 사용한다.

  - 아키텍처 별로 어셈블리 코드를 만드는 것보다, OS에 있는 wrapper 함수를 사용하는게 효율적이다.

### 시스템 콜 wrapper

- OS에서 내부적으로 시스템 콜만 호출하는 함수를 의미한다.

- 각 아키텍처마다 존재하며, 고급 언어로 쓰여진 프로그램에서 해당 wrapper 함수를 호출하기만 하면 되므로 이식성이 높다.

### 표준 C 라이브러리

- C언어에는 ISO에 의해 정해진 표준 라이브러리가 존재하며, 대부분의 C 프로그램은 GNU 프로젝트가 제공하는 glibc를 링크한다.

- glibc는 시스템 콜의 wrapper 함수를 포함하고, POSIX 규격에 정의된 함수도 포함한다.

- **ldd** 명령어

  - 프로그램이 어떤 라이브러리를 링크하는지 확인할 수 있는 명령어이다.

    ```sh
    $ ldd /bin/echo
    linux-vdso.so.1 (0x00007ffd50bcd000)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f15598f6000)
    /lib64/ld-linux-x86-64.so.2 (0x00007f1559b06000)
    
    $ ldd output/ppidloop 
    linux-vdso.so.1 (0x00007ffeceb86000)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f376d2e5000)
    /lib64/ld-linux-x86-64.so.2 (0x00007f376d4ef000)

    $ ldd /usr/bin/python
    linux-vdso.so.1 (0x00007fff397f1000)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f1cebea0000) # 파이썬도 내부적으로 libc를 사용한다.
    libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f1cebe7d000)
    libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f1cebe77000)
    libutil.so.1 => /lib/x86_64-linux-gnu/libutil.so.1 (0x00007f1cebe72000)
    libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f1cebd23000)
    libexpat.so.1 => /lib/x86_64-linux-gnu/libexpat.so.1 (0x00007f1cebcf5000)
    libz.so.1 => /lib/x86_64-linux-gnu/libz.so.1 (0x00007f1cebcd7000)
    /lib64/ld-linux-x86-64.so.2 (0x00007f1cec0a5000)

    $ ldd /usr/bin/ls
    linux-vdso.so.1 (0x00007ffc42e54000)
    libselinux.so.1 => /lib/x86_64-linux-gnu/libselinux.so.1 (0x00007f6806574000)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f6806382000)
    libpcre2-8.so.0 => /lib/x86_64-linux-gnu/libpcre2-8.so.0 (0x00007f68062f2000)
    libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f68062ec000)
    /lib64/ld-linux-x86-64.so.2 (0x00007f68065d7000)
    libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f68062c9000)

    $ ldd /usr/bin/cat
    linux-vdso.so.1 (0x00007ffec5f46000)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fa363535000)
    /lib64/ld-linux-x86-64.so.2 (0x00007fa363746000)

    $ ldd /usr/bin/pwd
    linux-vdso.so.1 (0x00007fff9ab54000)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f242d8e0000)
    /lib64/ld-linux-x86-64.so.2 (0x00007f242daf1000)
    ```

      - 자주 사용하는 명령어들이 대부분 libc를 링크하는 것을 볼 수 있다.

---

### 참고

- [실습과 그림으로 이해하는 리눅스 구조](https://www.aladin.co.kr/shop/wproduct.aspx?ItemId=181554153)
